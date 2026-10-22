/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */
#include "gtest/gtest.h"
#include <aws/gamelift/internal/network/WebSocketppClientWrapper.h>
#include <aws/gamelift/internal/retry/RetryStrategy.h>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

static constexpr const char *AUTH_TOKEN = "test-auth-token";
static const std::chrono::milliseconds TLS_ATTEMPT_WAIT_TIME(10000);

// Connect makes a single attempt, so that a failed connection fails the test quickly
class SingleAttemptRetryStrategy : public RetryStrategy {
public:
    void apply(const std::function<bool(void)> &callable) override { callable(); }
};

struct EvpPkeyDeleter {
    void operator()(EVP_PKEY *key) const { EVP_PKEY_free(key); }
};
struct X509Deleter {
    void operator()(X509 *certificate) const { X509_free(certificate); }
};
typedef std::unique_ptr<EVP_PKEY, EvpPkeyDeleter> EvpPkeyPtr;
typedef std::unique_ptr<X509, X509Deleter> X509Ptr;

static EvpPkeyPtr GenerateKey() {
    EVP_PKEY *key = nullptr;
    EVP_PKEY_CTX *keyContext = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (keyContext != nullptr && EVP_PKEY_keygen_init(keyContext) == 1 &&
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(keyContext, NID_X9_62_prime256v1) == 1) {
        EVP_PKEY_keygen(keyContext, &key);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        // OpenSSL 1.0.x encodes the curve of a generated key as explicit parameters unless it is asked for a named curve. The
        // public key in the certificate then no longer matches the private key, SSL_CTX_use_PrivateKey fails, and the test
        // server has no certificate ("no shared cipher"). OpenSSL 1.1.0 and later use a named curve by default.
        if (key != nullptr) {
            EC_KEY *ecKey = EVP_PKEY_get1_EC_KEY(key);
            EC_KEY_set_asn1_flag(ecKey, OPENSSL_EC_NAMED_CURVE);
            EC_KEY_free(ecKey);
        }
#endif
    }
    EVP_PKEY_CTX_free(keyContext);
    return EvpPkeyPtr(key);
}

static void AddExtension(X509 *certificate, X509 *issuer, int nid, const char *value) {
    X509V3_CTX extensionContext;
    X509V3_set_ctx(&extensionContext, issuer, certificate, nullptr, nullptr, 0);
    X509_EXTENSION *extension = X509V3_EXT_conf_nid(nullptr, &extensionContext, nid, const_cast<char *>(value));
    ASSERT_NE(extension, nullptr);
    X509_add_ext(certificate, extension, -1);
    X509_EXTENSION_free(extension);
}

// Creates a certificate for hostName. It is self-signed when issuer is null; otherwise issuer signs it.
static X509Ptr CreateCertificate(const std::string &hostName, EVP_PKEY *key, bool isCa, X509 *issuer, EVP_PKEY *issuerKey) {
    X509Ptr certificate(X509_new());
    X509_set_version(certificate.get(), 2);
    static long serialNumber = 1;
    ASN1_INTEGER_set(X509_get_serialNumber(certificate.get()), serialNumber++);
    X509_gmtime_adj(X509_get_notBefore(certificate.get()), -24 * 60 * 60);
    X509_gmtime_adj(X509_get_notAfter(certificate.get()), 24 * 60 * 60);
    X509_set_pubkey(certificate.get(), key);

    auto *subjectName = X509_get_subject_name(certificate.get());
    X509_NAME_add_entry_by_txt(subjectName, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>(hostName.c_str()), -1, -1, 0);
    X509 *signer = issuer != nullptr ? issuer : certificate.get();
    X509_set_issuer_name(certificate.get(), X509_get_subject_name(signer));

    if (isCa) {
        AddExtension(certificate.get(), signer, NID_basic_constraints, "critical,CA:TRUE");
        AddExtension(certificate.get(), signer, NID_key_usage, "critical,keyCertSign,cRLSign");
    } else {
        AddExtension(certificate.get(), signer, NID_subject_alt_name, ("DNS:" + hostName).c_str());
        AddExtension(certificate.get(), signer, NID_ext_key_usage, "serverAuth");
    }
    X509_sign(certificate.get(), issuerKey != nullptr ? issuerKey : key, EVP_sha256());
    return certificate;
}

/**
 * TLS server that the websocket can connect to for unit testing certificate validation.
 * It accepts TLS connections with the given certificate and records the first HTTP request it receives
 * (the websocket upgrade request, which carries the auth token), then answers it with 403 Forbidden.
 * The server starts when it is constructed and stops when it is destroyed.
 */
class TlsRequestCaptureServer {
public:
    TlsRequestCaptureServer(X509 *certificate, EVP_PKEY *key)
        : m_tlsContext(asio::ssl::context::tlsv12_server),
          m_acceptor(m_ioContext, asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), 0)), m_tlsAttempted(false) {
        SSL_CTX_use_certificate(m_tlsContext.native_handle(), certificate);
        SSL_CTX_use_PrivateKey(m_tlsContext.native_handle(), key);
        Accept();
        m_thread = std::thread([this] { m_ioContext.run(); });
    }

    ~TlsRequestCaptureServer() {
        m_ioContext.stop();
        m_thread.join();
    }

    // Ends with "/" like the URLs that GameLiftWebSocketClientManager builds, so that websocketpp keeps the query string
    std::string GetWebSocketUrl() const { return "wss://localhost:" + std::to_string(m_acceptor.local_endpoint().port()) + "/"; }

    bool WaitForTlsAttempt() {
        std::unique_lock<std::mutex> lock(m_lock);
        return m_condition.wait_for(lock, TLS_ATTEMPT_WAIT_TIME, [this] { return m_tlsAttempted; });
    }

    // The HTTP request received after a successful TLS handshake, or empty if there was none
    std::string GetReceivedRequest() {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_receivedRequest;
    }

private:
    typedef asio::ssl::stream<asio::ip::tcp::socket> TlsStream;

    void Accept() {
        std::shared_ptr<TlsStream> stream = std::make_shared<TlsStream>(m_ioContext, m_tlsContext);
        m_acceptor.async_accept(stream->lowest_layer(), [this, stream](const asio::error_code &acceptError) {
            if (acceptError) {
                return;
            }
            stream->async_handshake(asio::ssl::stream_base::server, [this, stream](const asio::error_code &handshakeError) {
                if (handshakeError) {
                    // The client rejected the certificate and aborted the handshake
                    SignalTlsAttempted("");
                    return;
                }
                ReadRequest(stream);
            });
            Accept();
        });
    }

    void ReadRequest(const std::shared_ptr<TlsStream> &stream) {
        std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>(16 * 1024);
        asio::async_read_until(*stream, *buffer, "\r\n\r\n", [this, stream, buffer](const asio::error_code &readError, std::size_t) {
            std::string request(asio::buffers_begin(buffer->data()), asio::buffers_end(buffer->data()));
            SignalTlsAttempted(request);
            if (readError) {
                return;
            }
            // Fail the websocket upgrade quickly instead of letting the client wait for its handshake timeout
            std::shared_ptr<std::string> response =
                std::make_shared<std::string>("HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            asio::async_write(*stream, asio::buffer(*response), [stream, response](const asio::error_code &, std::size_t) {});
        });
    }

    void SignalTlsAttempted(const std::string &request) {
        {
            std::lock_guard<std::mutex> lock(m_lock);
            if (m_receivedRequest.empty()) {
                m_receivedRequest = request;
            }
            m_tlsAttempted = true;
        }
        m_condition.notify_all();
    }

    asio::io_context m_ioContext;
    asio::ssl::context m_tlsContext;
    asio::ip::tcp::acceptor m_acceptor;
    std::thread m_thread;
    std::mutex m_lock;
    std::condition_variable m_condition;
    bool m_tlsAttempted;
    std::string m_receivedRequest;
};

class WebSocketppClientWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_caKey = GenerateKey();
        m_serverKey = GenerateKey();
        ASSERT_NE(m_caKey, nullptr);
        ASSERT_NE(m_serverKey, nullptr);
        m_caCertificate = CreateCertificate("GameLift Server SDK Test CA", m_caKey.get(), true, nullptr, nullptr);
        m_trustedCaFile = testing::TempDir() + "gamelift-websocket-test-ca.pem";
    }

    void TearDown() override {
        SetEnvironmentVariable("SSL_CERT_FILE", nullptr);
        std::remove(m_trustedCaFile.c_str());
    }

    // Makes the SDK trust the test CA, in addition to the host's CAs
    void TrustTestCa() {
        // Write the PEM through a memory BIO: on Windows, passing a FILE* to the OpenSSL DLL aborts without applink.c
        BIO *pemBio = BIO_new(BIO_s_mem());
        ASSERT_NE(pemBio, nullptr);
        ASSERT_EQ(PEM_write_bio_X509(pemBio, m_caCertificate.get()), 1);
        char *pemData = nullptr;
        const long pemLength = BIO_get_mem_data(pemBio, &pemData);
        std::ofstream caFile(m_trustedCaFile, std::ios::binary);
        caFile.write(pemData, pemLength);
        caFile.close();
        BIO_free(pemBio);
        ASSERT_TRUE(caFile.good());
        SetEnvironmentVariable("SSL_CERT_FILE", m_trustedCaFile.c_str());
    }

    X509Ptr CreateSelfSignedServerCertificate(const std::string &hostName) {
        return CreateCertificate(hostName, m_serverKey.get(), false, nullptr, nullptr);
    }

    X509Ptr CreateCaSignedServerCertificate(const std::string &hostName) {
        return CreateCertificate(hostName, m_serverKey.get(), false, m_caCertificate.get(), m_caKey.get());
    }

    GenericOutcome ConnectTo(TlsRequestCaptureServer &tlsServer) {
        std::shared_ptr<WebSocketppClientType> webSocketClient = std::make_shared<WebSocketppClientType>();
        WebSocketppClientWrapper webSocketClientWrapper(webSocketClient, std::make_shared<SingleAttemptRetryStrategy>());
        Uri uri = Uri::UriBuilder().WithBaseUri(tlsServer.GetWebSocketUrl()).AddQueryParam("Authorization", AUTH_TOKEN).Build();
        GenericOutcome outcome = webSocketClientWrapper.Connect(uri);
        EXPECT_FALSE(webSocketClientWrapper.IsConnected());
        return outcome;
    }

    void AssertConnectRejectedForCertificate(X509 *serverCertificate) {
        // GIVEN
        TlsRequestCaptureServer tlsServer(serverCertificate, m_serverKey.get());

        // WHEN
        GenericOutcome outcome = ConnectTo(tlsServer);

        // THEN
        ASSERT_FALSE(outcome.IsSuccess()) << "Connect must fail when the server certificate is not valid";
        ASSERT_TRUE(tlsServer.WaitForTlsAttempt()) << "The client must start a TLS handshake with the server";
        ASSERT_EQ(tlsServer.GetReceivedRequest(), "") << "The client must reject the certificate before it sends the upgrade request, so the "
                                                         "auth token never reaches the server";
    }

    static void SetEnvironmentVariable(const char *key, const char *value) {
#ifdef WIN32
        _putenv_s(key, value == nullptr ? "" : value);
#else
        if (value == nullptr) {
            unsetenv(key);
        } else {
            setenv(key, value, 1);
        }
#endif
    }

    EvpPkeyPtr m_caKey;
    EvpPkeyPtr m_serverKey;
    X509Ptr m_caCertificate;
    std::string m_trustedCaFile;
};

TEST_F(WebSocketppClientWrapperTest, GIVEN_selfSignedServerCertificateForOtherHost_WHEN_connect_THEN_connectFailsAndAuthTokenNotSent) {
    X509Ptr serverCertificate = CreateSelfSignedServerCertificate("definitely-not-amazongamelift.invalid");
    AssertConnectRejectedForCertificate(serverCertificate.get());
}

TEST_F(WebSocketppClientWrapperTest, GIVEN_untrustedSelfSignedServerCertificateForHost_WHEN_connect_THEN_connectFailsAndAuthTokenNotSent) {
    X509Ptr serverCertificate = CreateSelfSignedServerCertificate("localhost");
    AssertConnectRejectedForCertificate(serverCertificate.get());
}

TEST_F(WebSocketppClientWrapperTest, GIVEN_trustedServerCertificateForOtherHost_WHEN_connect_THEN_connectFailsAndAuthTokenNotSent) {
    TrustTestCa();
    X509Ptr serverCertificate = CreateCaSignedServerCertificate("definitely-not-amazongamelift.invalid");
    AssertConnectRejectedForCertificate(serverCertificate.get());
}

TEST_F(WebSocketppClientWrapperTest, GIVEN_trustedServerCertificateForHost_WHEN_connect_THEN_upgradeRequestWithAuthTokenSent) {
    // GIVEN
    TrustTestCa();
    X509Ptr serverCertificate = CreateCaSignedServerCertificate("localhost");
    TlsRequestCaptureServer tlsServer(serverCertificate.get(), m_serverKey.get());

    // WHEN
    GenericOutcome outcome = ConnectTo(tlsServer);

    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_FORBIDDEN);
    ASSERT_TRUE(tlsServer.WaitForTlsAttempt());
    ASSERT_NE(tlsServer.GetReceivedRequest().find(std::string("Authorization=") + AUTH_TOKEN), std::string::npos)
        << "The client must send the upgrade request once the certificate is valid. Received request: " << tlsServer.GetReceivedRequest();
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
