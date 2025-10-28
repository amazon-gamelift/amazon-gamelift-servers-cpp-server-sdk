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
#pragma once

#include <aws/gamelift/internal/util/HttpClient.h>
#include <memory>
#include <string>

namespace Aws {
namespace GameLift {
namespace Metrics {

class CrashReporterClient {
private:
    const std::string RegisterProcessUrlPath = "register";
    const std::string UpdateProcessUrlPath = "update";
    const std::string DeregisterProcessUrlPath = "deregister";
    const std::string ProcessPidParameterName = "process_pid";
    const std::string SessionIdParameterName = "session_id";

    std::shared_ptr<Aws::GameLift::Internal::HttpClient> httpClient;
    std::string baseUrl;

    bool isRetryableError(const std::string& errorMessage) const;

    void RegisterProcessWithRetries();

public:
    CrashReporterClient(const std::string &host, int port);
    CrashReporterClient(std::shared_ptr<Aws::GameLift::Internal::HttpClient> httpClient, const std::string &host, int port);

    void RegisterProcess();
    void TagGameSession(const std::string &sessionId);
    void DeregisterProcess();
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
