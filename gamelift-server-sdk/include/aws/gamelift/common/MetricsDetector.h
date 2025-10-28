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

#include <aws/gamelift/common/GameLiftToolDetector.h>
#include <functional>

namespace Aws {
namespace GameLift {
namespace Common {

class MetricsDetector : public GameLiftToolDetector {
public:
    bool IsToolRunning() override;
    std::string GetToolName() override;
    std::string GetToolVersion() override;

private:
    static constexpr const char* TOOL_NAME = "Metrics";
    static constexpr const char* TOOL_VERSION = "1.0.0";
    static constexpr const char* WINDOWS_SERVICE_COMMAND = "sc";
    static constexpr const char* WINDOWS_SERVICE_NAME = "GLOTelCollector";
    static constexpr const char* WINDOWS_SERVICE_ARGS = "query ";
    static constexpr const char* WINDOWS_RUNNING_STATUS = "RUNNING";
    static constexpr const char* LINUX_SERVICE_COMMAND = "systemctl";
    static constexpr const char* LINUX_SERVICE_NAME = "gl-otel-collector.service";
    static constexpr const char* LINUX_SERVICE_ARGS = "is-active ";
    static constexpr const char* LINUX_ACTIVE_STATUS = "active";

    bool CheckService(const std::string& command, const std::string& arguments, std::function<bool(const std::string&)> outputValidator);
};

} // namespace Common
} // namespace GameLift
} // namespace Aws
