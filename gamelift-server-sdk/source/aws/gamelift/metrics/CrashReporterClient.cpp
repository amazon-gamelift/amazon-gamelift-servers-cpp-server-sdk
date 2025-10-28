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

#include <aws/gamelift/metrics/CrashReporterClient.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>
#include <aws/gamelift/internal/retry/JitteredGeometricBackoffRetryStrategy.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Aws {
namespace GameLift {
namespace Metrics {

CrashReporterClient::CrashReporterClient(const std::string& host, int port)
    : httpClient(std::make_shared<Aws::GameLift::Internal::HttpClient>()) {
    baseUrl = "http://" + host + ":" + std::to_string(port) + "/";
}

CrashReporterClient::CrashReporterClient(std::shared_ptr<Aws::GameLift::Internal::HttpClient> httpClient, const std::string &host, int port)
    : httpClient(std::move(httpClient)) {
    baseUrl = "http://" + host + ":" + std::to_string(port) + "/";
}

bool CrashReporterClient::isRetryableError(const std::string& errorMessage) const {
    return errorMessage.find("Connection refused") != std::string::npos || 
           errorMessage.find("Connection failed") != std::string::npos;
}

void CrashReporterClient::RegisterProcessWithRetries() {
#ifdef _WIN32
    int processPid = static_cast<int>(GetCurrentProcessId());
#else
    int processPid = static_cast<int>(getpid());
#endif
    std::string requestUri = baseUrl + RegisterProcessUrlPath + "?" +
                             ProcessPidParameterName + "=" + std::to_string(processPid);
    
    spdlog::info("Registering process with {} {} in OTEL Collector Crash Reporter", ProcessPidParameterName, processPid);

    // 5 retries, 1s base delay with jitter (default)
    // Total max wait time: ~1s + 2s + 4s + 8s + 16s = ~31s
    Aws::GameLift::Internal::JitteredGeometricBackoffRetryStrategy retryStrategy;

    auto callable = [this, &requestUri, processPid]() -> bool {
        try {
            auto response = httpClient->SendGetRequest(requestUri);
            if (response.IsSuccessfulStatusCode()) {
                spdlog::info("Successfully registered {} {} to OTEL Collector Crash Reporter", ProcessPidParameterName, processPid);
                return true;
            } else {
                spdlog::error("Failed to register {} {} to OTEL Collector Crash Reporter, Http response: {} - {}", 
                             ProcessPidParameterName, processPid, response.statusCode, response.body);
                return true; // Don't retry on HTTP errors (4xx, 5xx)
            }
        } catch (const std::exception& e) {
            std::string errorMsg = e.what();
            if (isRetryableError(errorMsg)) {
                spdlog::warn("Failed to register {} {} to OTEL Collector Crash Reporter due to connection error: {}", 
                             ProcessPidParameterName, processPid, e.what());
                return false; // Retry on connection errors
            } else {
                spdlog::error("Failed to register {} {} to OTEL Collector Crash Reporter due to error: {}", 
                             ProcessPidParameterName, processPid, e.what());
                return true; // Don't retry on other errors
            }
        }
    };
    
    Aws::GameLift::Internal::RetryingCallable::Builder()
        .WithRetryStrategy(&retryStrategy)
        .WithCallable(callable)
        .Build()
        .call();
}

void CrashReporterClient::RegisterProcess() {
    RegisterProcessWithRetries();
}

void CrashReporterClient::TagGameSession(const std::string& sessionId) {
#ifdef _WIN32
    int processPid = static_cast<int>(GetCurrentProcessId());
#else
    int processPid = static_cast<int>(getpid());
#endif
    std::string requestUri = baseUrl + UpdateProcessUrlPath + "?" +
                             ProcessPidParameterName + "=" + std::to_string(processPid) + "&" +
                             SessionIdParameterName + "=" + sessionId;

    try {
        spdlog::info("Adding {} tag {} to process with {} {} to the OTEL Collector Crash Reporter",
                     SessionIdParameterName, sessionId, ProcessPidParameterName, processPid);
        auto response = httpClient->SendGetRequest(requestUri);
        if (!response.IsSuccessfulStatusCode()) {
            spdlog::error("Failed to add {} tag {} to process with {} {} in the OTEL Collector Crash Reporter, Http response: {} - {}",
                          SessionIdParameterName, sessionId, ProcessPidParameterName, processPid,
                          response.statusCode, response.body);
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to add {} tag {} to process with {} {} in the OTEL Collector Crash Reporter due to error: {}",
                      SessionIdParameterName, sessionId, ProcessPidParameterName, processPid, e.what());
    }
}

void CrashReporterClient::DeregisterProcess() {
#ifdef _WIN32
    int processPid = static_cast<int>(GetCurrentProcessId());
#else
    int processPid = static_cast<int>(getpid());
#endif
    std::string requestUri = baseUrl + DeregisterProcessUrlPath + "?" +
                             ProcessPidParameterName + "=" + std::to_string(processPid);

    try {
        spdlog::info("Unregistering process with {} {} in OTEL Collector Crash Reporter",
                     ProcessPidParameterName, processPid);
        auto response = httpClient->SendGetRequest(requestUri);
        if (!response.IsSuccessfulStatusCode()) {
            spdlog::error("Failed to deregister {} {} in the OTEL Collector Crash Reporter, Http response: {} - {}",
                          ProcessPidParameterName, processPid, response.statusCode, response.body);
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to deregister {} {} in the OTEL Collector Crash Reporter due to error: {}",
                      ProcessPidParameterName, processPid, e.what());
    }
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
