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

#include <aws/gamelift/common/MetricsDetector.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace Aws {
namespace GameLift {
namespace Common {

bool MetricsDetector::IsToolRunning() {
    try {
#ifdef _WIN32
        std::string windowsServiceArgs = std::string(WINDOWS_SERVICE_ARGS) + WINDOWS_SERVICE_NAME;
        return CheckService(WINDOWS_SERVICE_COMMAND, windowsServiceArgs, [](const std::string& output) {
            return output.find(WINDOWS_RUNNING_STATUS) != std::string::npos;
        });
#else
        std::string linuxServiceArgs = std::string(LINUX_SERVICE_ARGS) + LINUX_SERVICE_NAME;
        return CheckService(LINUX_SERVICE_COMMAND, linuxServiceArgs, [](const std::string& output) {
            std::string trimmedOutput = output;
            // Trim whitespace
            trimmedOutput.erase(trimmedOutput.find_last_not_of(" \n\r\t") + 1);
            trimmedOutput.erase(0, trimmedOutput.find_first_not_of(" \n\r\t"));
            return trimmedOutput == LINUX_ACTIVE_STATUS;
        });
#endif
    } catch (...) {
        return false;
    }
}

bool MetricsDetector::CheckService(const std::string& command, const std::string& arguments, std::function<bool(const std::string&)> outputValidator) {
    std::string fullCommand = command + " " + arguments;
    
#ifdef _WIN32
    FILE* pipe = _popen(fullCommand.c_str(), "r");
#else
    FILE* pipe = popen(fullCommand.c_str(), "r");
#endif
    
    if (!pipe) {
        return false;
    }

    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.append(buffer);
    }

#ifdef _WIN32
    int exitCode = _pclose(pipe);
#else
    int exitCode = pclose(pipe);
#endif

    return exitCode == 0 && outputValidator(result);
}

std::string MetricsDetector::GetToolName() {
    return TOOL_NAME;
}

std::string MetricsDetector::GetToolVersion() {
    return TOOL_VERSION;
}

} // namespace Common
} // namespace GameLift
} // namespace Aws
