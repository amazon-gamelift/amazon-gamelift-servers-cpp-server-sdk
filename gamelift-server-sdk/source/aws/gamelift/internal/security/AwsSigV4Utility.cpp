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

#include <aws/gamelift/internal/security/AwsSigV4Utility.h>
#include <aws/gamelift/internal/util/UriEncoder.h>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>

using namespace Aws::GameLift::Internal;

Aws::GameLift::Outcome<std::map<std::string, std::string>, std::string> AwsSigV4Utility::GenerateSigV4QueryParameters(const SigV4Parameters &parameters) {

    try {
        ValidateParameters(parameters);
    } catch (std::invalid_argument const &ex) {
        return std::string(ex.what());
    }

    char dateBuffer[16];
    char dateTimeBuffer[32];
    strftime(dateBuffer, sizeof(dateBuffer), DateFormat, &parameters.RequestTime);
    strftime(dateTimeBuffer, sizeof(dateTimeBuffer), DateTimeFormat, &parameters.RequestTime);

    const std::string formattedRequestDate(dateBuffer);
    const std::string formattedRequestDateTime(dateTimeBuffer);

    const std::string canonicalRequest = ToSortedEncodedQueryString(parameters.QueryParams);

    const std::string hashedCanonicalRequest = ComputeSha256Hash(canonicalRequest);

    const std::string scope = formattedRequestDate + "/" + parameters.AwsRegion + "/" + ServiceName + "/" + TerminationString;

    const std::string stringToSign = std::string(Algorithm) + "\n" + formattedRequestDateTime + "\n" + scope + "\n" + hashedCanonicalRequest;

    const std::string credential = parameters.Credentials.AccessKey + "/" + scope;

    const std::string signature = GenerateSignature(
            parameters.AwsRegion,
            parameters.Credentials.SecretKey,
            formattedRequestDate,
            ServiceName,
            stringToSign);

    return GenerateSigV4QueryParameters(
            credential,
            formattedRequestDateTime,
            parameters.Credentials.SessionToken,
            signature);
}

void AwsSigV4Utility::ValidateParameters(const SigV4Parameters &parameters) {

    if (parameters.AwsRegion.empty()) {
        throw std::invalid_argument("AwsRegion is required");
    }

    if (parameters.Credentials.AccessKey.empty()) {
        throw std::invalid_argument("AccessKey is required");
    }

    if (parameters.Credentials.SecretKey.empty()) {
        throw std::invalid_argument("SecretKey is required");
    }

    if (parameters.QueryParams.empty()) {
        throw std::invalid_argument("QueryParams is required");
    }

    if (parameters.RequestTime.tm_year == 0) {
        throw std::invalid_argument("RequestTime is required");
    }
}

std::string AwsSigV4Utility::GenerateSignature(
        const std::string &region,
        const std::string &secretKey,
        const std::string &formattedRequestDateTime,
        const std::string &serviceName,
        const std::string &stringToSign) {

    std::vector<uint8_t> encodedKeySecret = std::vector<uint8_t>(SignatureSecretKeyPrefix, SignatureSecretKeyPrefix +
                                                                                           strlen(SignatureSecretKeyPrefix));
    encodedKeySecret.insert(encodedKeySecret.end(), secretKey.begin(), secretKey.end());

    auto hashDate = ComputeHmacSha256(encodedKeySecret, formattedRequestDateTime);
    auto hashRegion = ComputeHmacSha256(hashDate, region);
    auto hashService = ComputeHmacSha256(hashRegion, serviceName);
    auto signingKey = ComputeHmacSha256(hashService, TerminationString);

    return ToHex(ComputeHmacSha256(signingKey, stringToSign));
}

std::map<std::string, std::string> AwsSigV4Utility::GenerateSigV4QueryParameters(
        const std::string &credential,
        const std::string &formattedRequestDateTime,
        const std::string &sessionToken,
        const std::string &signature) {

    std::map<std::string, std::string> sigV4QueryParameters;
    sigV4QueryParameters[AuthorizationKey] = AuthorizationValue;
    sigV4QueryParameters[AmzAlgorithmKey] = Algorithm;
    sigV4QueryParameters[AmzCredentialKey] = UriEncoder::UriEncode(credential);
    sigV4QueryParameters[AmzDateKey] = formattedRequestDateTime;
    sigV4QueryParameters[AmzSignatureKey] = UriEncoder::UriEncode(signature);

    if (!sessionToken.empty()) {
        sigV4QueryParameters[AmzSecurityTokenHeadersKey] = UriEncoder::UriEncode(sessionToken);
    }

    return sigV4QueryParameters;
}

std::string AwsSigV4Utility::ToSortedEncodedQueryString(const std::map<std::string, std::string> &queryParameters) {
    std::vector<std::pair<std::string, std::string> > sortedParams(queryParameters.begin(), queryParameters.end());
    std::sort(sortedParams.begin(), sortedParams.end());

    std::ostringstream stringBuffer;
    for (auto sortedParam = sortedParams.begin(); sortedParam != sortedParams.end(); ++sortedParam) {
        if (sortedParam != sortedParams.begin()) {
            stringBuffer << "&";
        }
        stringBuffer << UriEncoder::UriEncode(sortedParam->first) << "=" << UriEncoder::UriEncode(sortedParam->second);
    }

    return stringBuffer.str();
}

// Refer to documentation in AwsSigV4Utility.h
std::string AwsSigV4Utility::ComputeSha256Hash(const std::string &data) {
    // Because the following methods do not throw exceptions, they are not being surrounded by try-catch or use RAII for cleaning memory.
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);

    const std::vector<uint8_t> sha256Hash = std::vector<uint8_t>(hash, hash + SHA256_DIGEST_LENGTH);
    return ToHex(sha256Hash);
}

// Refer to documentation in AwsSigV4Utility.h
std::vector<uint8_t> AwsSigV4Utility::ComputeHmacSha256(const std::vector<uint8_t> &key, const std::string &data) {
    // Because the following methods do not throw exceptions, they are not being surrounded by try-catch or use RAII for cleaning memory.
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L // OpenSSL 1.1.0 or newer
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), nullptr);
    HMAC_Update(ctx, reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
    HMAC_Final(ctx, hash, &len);
    HMAC_CTX_free(ctx);
#else // Older versions of OpenSSL
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key.data(), key.size(), EVP_sha256(), nullptr);
    HMAC_Update(&ctx, reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
    HMAC_Final(&ctx, hash, &len);
    HMAC_CTX_cleanup(&ctx);
#endif
    return std::vector<uint8_t>(hash, hash + len);
}

std::string AwsSigV4Utility::ToHex(const std::vector<uint8_t> &hashBytes) {
    std::ostringstream stringBuffer;
    for (auto b: hashBytes) {
        stringBuffer << std::hex << std::setw(2) << std::setfill('0') << (int) b;
    }
    return stringBuffer.str();
}
