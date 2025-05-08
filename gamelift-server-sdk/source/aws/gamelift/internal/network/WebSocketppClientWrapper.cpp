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
#include <aws/gamelift/internal/model/Message.h>
#include <aws/gamelift/internal/model/ResponseMessage.h>
#include <aws/gamelift/internal/network/WebSocketppClientWrapper.h>
#include <aws/gamelift/internal/retry/GeometricBackoffRetryStrategy.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>
#include <memory>
#include <websocketpp/error.hpp>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

WebSocketppClientWrapper::WebSocketppClientWrapper(std::shared_ptr<WebSocketppClientType> webSocketClient)
    : m_webSocketClient(webSocketClient), m_connectionStateChanged(false) {
    // configure logging. comment these out to get websocket logs on stdout for debugging
    m_webSocketClient->clear_access_channels(websocketpp::log::alevel::all);
    m_webSocketClient->clear_error_channels(websocketpp::log::elevel::all);

    // initialize ASIO
    m_webSocketClient->init_asio();

    // start in perpetual mode (do not exit processing loop when there are no connections)
    m_webSocketClient->start_perpetual();

    // Kick off two threads that will own up to two parallel connections. These threads will live
    // until the SDK is shutdown and alternate handling connections. If a connection fails to open,
    // the thread will not hang. Instead, the thread will simply wait for another connection to
    // appear in queue.
    //
    // Flow of logic in the two threads:
    // --- SDK initialized ---
    // socket_thread_1: waiting for connection...
    // socket_thread_2: waiting for connection...
    // --- Initial connection happens ---
    // socket_thread_1: handling 1st connection
    // socket_thread_2: waiting for connection...
    // --- Connection refresh begins ---
    // socket_thread_1: finish handling 1st connection messages
    // socket_thread_2: handling 2nd connection
    // --- Connection 1 closes ---
    // socket_thread_1: waiting for connection...
    // socket_thread_2: handling 2nd connection
    // --- SDK shut down, and WebSocket client "->stop_perpetual()" is invoked ---
    // socket_thread_1: No longer waits for a connection, thread ends
    // socket_thread_2: Finishes handling 2nd connection, then thread ends
    m_socket_thread_1 = std::unique_ptr<std::thread>(new std::thread([this] { m_webSocketClient->run(); }));
    m_socket_thread_2 = std::unique_ptr<std::thread>(new std::thread([this] { m_webSocketClient->run(); }));

    // Set callbacks
    using std::placeholders::_1;
    using std::placeholders::_2;

    // Set timeout waiting for GameLift websocket server to respond on initial connection.
    // See: https://github.com/zaphoyd/websocketpp/blob/master/websocketpp/connection.hpp#L501
    m_webSocketClient->set_open_handshake_timeout(WEBSOCKET_OPEN_HANDSHAKE_TIMEOUT_MILLIS);

    m_webSocketClient->set_tls_init_handler(std::bind(&WebSocketppClientWrapper::OnTlsInit, this, _1));
    m_webSocketClient->set_open_handler(std::bind(&WebSocketppClientWrapper::OnConnected, this, _1));
    m_webSocketClient->set_message_handler(std::bind(&WebSocketppClientWrapper::OnMessage, this, _1, _2));
    m_webSocketClient->set_fail_handler(std::bind(&WebSocketppClientWrapper::OnError, this, _1));
    m_webSocketClient->set_close_handler(std::bind(&WebSocketppClientWrapper::OnClose, this, _1));
    m_webSocketClient->set_interrupt_handler(std::bind(&WebSocketppClientWrapper::OnInterrupt, this, _1));
}

WebSocketppClientWrapper::~WebSocketppClientWrapper() {
    // stop perpetual mode, allowing the websocketClient to destroy itself
    if (m_webSocketClient) {
        m_webSocketClient->stop_perpetual();
    }

    spdlog::info("Destroying WebsocketPPClientWraper");
    // close connections and join the thread
    if (m_connection && m_connection->get_state() == websocketpp::session::state::open) {
        Disconnect();
    }
    if (m_socket_thread_1 && m_socket_thread_1->joinable()) {
        m_socket_thread_1->join();
    }
    if (m_socket_thread_2 && m_socket_thread_2->joinable()) {
        m_socket_thread_2->join();
    }
}

GenericOutcome WebSocketppClientWrapper::Connect(const Uri &uri) {
    spdlog::info("Opening Connection");
    // Perform connection with retries.
    // This attempts to start up a new websocket connection / thread
    m_uri = uri;
    websocketpp::lib::error_code errorCode;
    GeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder()
                                    .WithRetryStrategy(&retryStrategy)
                                    .WithCallable([this, &uri, &errorCode] {
                                        spdlog::info("Attempting to perform connection");
                                        WebSocketppClientType::connection_ptr newConnection = PerformConnect(uri, errorCode);
                                        if (newConnection && newConnection->get_state() == websocketpp::session::state::open) {
                                            spdlog::info("Connection established, transitioning traffic");
                                            // "Flip" traffic from our old websocket to our new websocket. Close the old one
                                            // if necessary
                                            WebSocketppClientType::connection_ptr oldConnection = m_connection;
                                            m_connection = newConnection;
                                            if (oldConnection && oldConnection->get_state() == websocketpp::session::state::open) {
                                                spdlog::info("Closing previous connection");
                                                websocketpp::lib::error_code closeErrorCode;
                                                m_webSocketClient->close(oldConnection->get_handle(), websocketpp::close::status::going_away,
                                                                         "Websocket client reconnecting", closeErrorCode);
                                                if (errorCode.value()) {
                                                    spdlog::warn("Failed to close old websocket after a connection refresh, ignoring.");
                                                }
                                            }
                                            return true;
                                        } else {
                                            spdlog::warn("Connection to GameLift websocket server failed. Retrying connection if possible.");
                                            return false;
                                        }
                                    })
                                    .Build();
    callable.call();

    if (IsConnected()) {
        spdlog::info("Connected to endpoint");
        return GenericOutcome(nullptr);
    } else {
        spdlog::error("Connection to GameLift websocket server failed. See error message in InitSDK() outcome for details.");
        m_connection = nullptr;
        switch (errorCode.value()) {
        case websocketpp::error::server_only:
            switch (m_fail_response_code) {
            case websocketpp::http::status_code::value::forbidden:
                return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_FORBIDDEN);
            default:
                return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE, errorCode.category().message(errorCode.value()).c_str(),
                                                    errorCode.message().c_str()));
            }
        case 11001: // Host not found
        case websocketpp::error::invalid_uri:
            return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_INVALID_URL);
        // case websocketpp::error::timeout:
        case 0: // No Response after multiple retries, i.e. timeout
        case websocketpp::error::open_handshake_timeout:
        case websocketpp::error::close_handshake_timeout:
            return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_TIMEOUT);
        case websocketpp::error::endpoint_not_secure:
        case websocketpp::error::no_outgoing_buffers:
        case websocketpp::error::no_incoming_buffers:
        case websocketpp::error::invalid_state:
        case websocketpp::error::bad_close_code:
        case websocketpp::error::reserved_close_code:
        case websocketpp::error::invalid_close_code:
        case websocketpp::error::invalid_utf8:
        case websocketpp::error::invalid_subprotocol:
        case websocketpp::error::bad_connection:
        case websocketpp::error::con_creation_failed:
        case websocketpp::error::unrequested_subprotocol:
        case websocketpp::error::client_only:
        case websocketpp::error::http_connection_ended:
        case websocketpp::error::invalid_port:
        case websocketpp::error::async_accept_not_listening:
        case websocketpp::error::upgrade_required:
        case websocketpp::error::invalid_version:
        case websocketpp::error::unsupported_version:
        case websocketpp::error::http_parse_error:
        case websocketpp::error::extension_neg_failed:
        default:
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE, errorCode.category().message(errorCode.value()).c_str(),
                                                errorCode.message().c_str()));
        }
    }
}

WebSocketppClientType::connection_ptr WebSocketppClientWrapper::PerformConnect(const Uri &uri, websocketpp::lib::error_code &errorCode) {
    spdlog::info("Performing connection");
    errorCode.clear();
    // Create connection request
    WebSocketppClientType::connection_ptr newConnection = m_webSocketClient->get_connection(uri.GetUriString(), errorCode);
    if (errorCode.value()) {
        spdlog::error("Failed to GetConnection. ERROR: {}", errorCode.message());
        return newConnection;
    } else {
        spdlog::info("Connection request created successfully. Waiting for connection to establish...");
    }

    // Queue a new connection request (the socket thread will act on it and attempt to connect)
    try {
        m_webSocketClient->connect(newConnection);
    }
    catch (const std::exception& e) {
        spdlog::error("Exception while trying to connect with the webSocketClient: {}", e.what());
    }
    spdlog::info("Connection request queued.");
    // Wait for connection to succeed or fail (this makes connection synchronous)
    {
        std::unique_lock<std::mutex> lk(m_lock);
        m_cond.wait(lk, [this] { return m_connectionStateChanged; });
        spdlog::info("Connection state changed: {}", m_fail_error_code.message());
        errorCode = m_fail_error_code;
        // Reset
        m_connectionStateChanged = false;
        m_fail_error_code.clear();
    }

    if (errorCode.value()) {
        spdlog::error("Connection failed with errorCode: {}", errorCode.message());
    }
    else {
        spdlog::info("Connection established successfully.");
    }

    return newConnection;
}

GenericOutcome WebSocketppClientWrapper::SendSocketMessage(const std::string &requestId, const std::string &message) {
    if (requestId.empty()) {
        spdlog::error("Request does not have request ID, cannot process");
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION));
    }

    auto waitForReconnectRetryCount = 0;
    while(!IsConnected()) {
        // m_connection will be null if reconnect failed after max reties
        if(m_connection == nullptr || ++waitForReconnectRetryCount >= WAIT_FOR_RECONNECT_MAX_RETRIES) {
            spdlog::warn("WebSocket is not connected... WebSocket failed to send message due to an error.");
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_SEND_MESSAGE_FAILURE));
        }
        spdlog::warn("WebSocket is not connected... isConnected: {}, remoteEndpoint: {}, host: {}, port: {}",
                IsConnected(), m_connection->get_remote_endpoint(), m_connection->get_host(), m_connection->get_port());
        std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR_RECONNECT_RETRY_DELAY_SECONDS));
    }

    std::future<GenericOutcome> responseFuture;
    // Lock whenever we make use of 'm_requestIdToPromise' to avoid concurrent writes/reads
    {
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        // This indicates we've already sent this message, and it's still in flight
        if (m_requestIdToPromise.count(requestId) > 0) {
            spdlog::error("Request {} already exists", requestId);
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
        }

        std::promise<GenericOutcome> responsePromise;
        responseFuture = responsePromise.get_future();
        m_requestIdToPromise[requestId] = std::move(responsePromise);
    }

    GenericOutcome immediateResponse = SendSocketMessageAsync(message);

    if (!immediateResponse.IsSuccess()) {
        spdlog::error("Send Socket Message immediate response failed with error {}: {}",
                      immediateResponse.GetError().GetErrorName(), immediateResponse.GetError().GetErrorMessage());
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        m_requestIdToPromise.erase(requestId);
        return immediateResponse;
    }

    std::future_status promiseStatus = responseFuture.wait_for(std::chrono::milliseconds(SERVICE_CALL_TIMEOUT_MILLIS));

    if (promiseStatus == std::future_status::timeout) {
        std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
        spdlog::error("Response not received within the time limit of {} ms for request {}", SERVICE_CALL_TIMEOUT_MILLIS, requestId);
        spdlog::warn("isConnected: {}, remoteEndpoint: {}, host: {}, port: {}", IsConnected(),
                     m_connection->get_remote_endpoint(), m_connection->get_host(), m_connection->get_port());
        m_requestIdToPromise.erase(requestId);
        // If a call times out, retry
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE));
    }

    return responseFuture.get();
}

GenericOutcome WebSocketppClientWrapper::SendSocketMessageAsync(const std::string &message) {
    spdlog::info("Sending Socket Message, isConnected:{}, endpoint: {}, host: {}, port: {}", IsConnected(),
                 m_connection->get_remote_endpoint(), m_connection->get_host(), m_connection->get_port());
    websocketpp::lib::error_code errorCode;
    m_webSocketClient->send(m_connection->get_handle(), message.c_str(), websocketpp::frame::opcode::text, errorCode);
    if (errorCode.value()) {
        spdlog::error("Error Sending Socket Message: {}", errorCode.value());
        switch (errorCode.value()) {
        case websocketpp::error::no_outgoing_buffers:
            // If buffers are full, send will fail. Retryable since buffers can free up as messages
            // send.
            return GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE);
        default:
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::WEBSOCKET_SEND_MESSAGE_FAILURE, errorCode.category().message(errorCode.value()).c_str(),
                                                errorCode.message().c_str()));
        }
    }
    return GenericOutcome(nullptr);
}

void WebSocketppClientWrapper::Disconnect() {
    spdlog::info("Disconnecting WebSocket");
    if (m_connection != nullptr) {
        websocketpp::lib::error_code ec;
        m_webSocketClient->close(m_connection->get_handle(), websocketpp::close::status::going_away, "Websocket client closing", ec);
        if (ec) {
	        spdlog::error("Error initiating close: {}",ec.message());
        }
        m_connection = nullptr;
    }
}

void WebSocketppClientWrapper::RegisterGameLiftCallback(const std::string &gameLiftEvent, const std::function<GenericOutcome(std::string)> &callback) {
    spdlog::info("Registering GameLift CallBack for: {}", gameLiftEvent);
    m_eventHandlers[gameLiftEvent] = callback;
}

bool WebSocketppClientWrapper::IsConnected() {
    // m_connection is nullptr if 'm_webSocketClient->get_connection()' fails
    return m_connection != nullptr && m_connection->get_state() == websocketpp::session::state::open;
}

void WebSocketppClientWrapper::OnConnected(websocketpp::connection_hdl connection) {
    spdlog::info("Connected to WebSocket");
    // aquire lock and set condition variables (let main thread know connection is successful)
    {
        std::lock_guard<std::mutex> lk(m_lock);
        // set the state change variables and notify the thread that is connecting
        m_connectionStateChanged = true;
    }
    m_cond.notify_one();
}

void WebSocketppClientWrapper::OnError(websocketpp::connection_hdl connection) {
    auto con = m_webSocketClient->get_con_from_hdl(connection);
    spdlog::error("Error Connecting to WebSocket");

    // aquire lock and set condition variables (let main thread know an error has occurred)
    {
        std::lock_guard<std::mutex> lk(m_lock);
        // set the state change variables and notify the thread that is connecting
        m_connectionStateChanged = true;
        m_fail_error_code = con->get_ec();
        m_fail_response_code = con->get_response_code();
    }
    m_cond.notify_one();
}

void WebSocketppClientWrapper::OnMessage(websocketpp::connection_hdl connection, websocketpp::config::asio_client::message_type::ptr msg) {
    std::string message = msg->get_payload();
    spdlog::info("Received message from websocket endpoint: {}, host: {}, port: {}",
                 m_connection->get_remote_endpoint(), m_connection->get_host(), m_connection->get_port());

    ResponseMessage responseMessage;
    Message &gameLiftMessage = responseMessage;
    if (!gameLiftMessage.Deserialize(message)) {
        spdlog::error("Error Deserializing Message");
        return;
    }

    const std::string &action = responseMessage.GetAction();
    spdlog::info("Deserialized Message has Action: {}", action);
    const std::string &requestId = responseMessage.GetRequestId();
    const int statusCode = responseMessage.GetStatusCode();
    const std::string &errorMessage = responseMessage.GetErrorMessage();

    // Default to a success response with no result pointer
    GenericOutcome response(nullptr);
    // Check if the response was an error. If so, update the response based on status code.
    // RequestId will be empty when we get a message not associated with a request, in which case we
    // don't expect a 200 status code either.
    if (statusCode != OK_STATUS_CODE && !requestId.empty()) {
        response = GenericOutcome(GameLiftError(statusCode, message.c_str()));
    } else {
        // If we got a success response, and we have a special event handler for this action, invoke
        // it to get the real parsed result
        if (m_eventHandlers.count(action)) {
            spdlog::info("Executing GameLift Event Handler for {}", action);
            response = m_eventHandlers[action](message);
        }
    }

    // Lock whenever we make use of 'm_requestIdToPromise' to avoid concurrent writes/reads
    std::lock_guard<std::mutex> lock(m_requestToPromiseLock);
    if (m_requestIdToPromise.count(requestId) > 0) {
        m_requestIdToPromise[requestId].set_value(response);
        m_requestIdToPromise.erase(requestId);
    }
}

websocketpp::lib::shared_ptr<asio::ssl::context> WebSocketppClientWrapper::OnTlsInit(websocketpp::connection_hdl hdl) {
    websocketpp::lib::shared_ptr<asio::ssl::context> contextPtr(new asio::ssl::context(asio::ssl::context::tlsv12));
    return contextPtr;
}

void WebSocketppClientWrapper::OnClose(websocketpp::connection_hdl connection) {
    auto connectionPointer = m_webSocketClient->get_con_from_hdl(connection);
    auto localCloseCode = connectionPointer->get_local_close_code();
    auto remoteCloseCode = connectionPointer->get_remote_close_code();
    bool isNormalClosure = localCloseCode == websocketpp::close::status::normal
                           || localCloseCode == websocketpp::close::status::going_away
                           || remoteCloseCode == websocketpp::close::status::normal
                           || remoteCloseCode == websocketpp::close::status::going_away;
    spdlog::info("Connection to GameLift websocket server lost, Local Close Code = {}, Remote Close Code = {}.",
           websocketpp::close::status::get_string(localCloseCode).c_str(),
           websocketpp::close::status::get_string(remoteCloseCode).c_str());
    if(isNormalClosure) {
        spdlog::info("Normal Connection Closure, skipping reconnect.");
        return;
    } else {
        spdlog::info("Abnormal Connection Closure, reconnecting.");
        WebSocketppClientWrapper::Connect(m_uri);
    }
}

void WebSocketppClientWrapper::OnInterrupt(websocketpp::connection_hdl connection) {
    auto connectionPointer = m_webSocketClient->get_con_from_hdl(connection);
    auto remoteEndpoint = connectionPointer->get_remote_endpoint();
    auto host = connectionPointer->get_host();
    auto port = connectionPointer->get_port();
    spdlog::warn("Interruption Happened");
    spdlog::info("In OnInterrupt(), isConnected:{}, endpoint: {}, host: {}, port: {}", IsConnected(), remoteEndpoint, host, port);
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws