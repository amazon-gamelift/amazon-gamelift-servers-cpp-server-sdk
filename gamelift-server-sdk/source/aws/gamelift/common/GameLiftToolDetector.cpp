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

#include <aws/gamelift/common/GameLiftToolDetector.h>
#include <cstdlib>
#include <cstring>

namespace Aws {
namespace GameLift {
namespace Common {

void GameLiftToolDetector::SetGameLiftTool() {
    static constexpr const char* ENV_VAR_SDK_TOOL_NAME = "GAMELIFT_SDK_TOOL_NAME";
    static constexpr const char* ENV_VAR_SDK_TOOL_VERSION = "GAMELIFT_SDK_TOOL_VERSION";
    
    const char* existingToolName = std::getenv(ENV_VAR_SDK_TOOL_NAME);
    if ((existingToolName == nullptr || strlen(existingToolName) == 0) && IsToolRunning()) {
        std::string toolName = GetToolName();
        std::string toolVersion = GetToolVersion();
        
#ifdef _WIN32
        _putenv_s(ENV_VAR_SDK_TOOL_NAME, toolName.c_str());
        _putenv_s(ENV_VAR_SDK_TOOL_VERSION, toolVersion.c_str());
#else
        setenv(ENV_VAR_SDK_TOOL_NAME, toolName.c_str(), 1);
        setenv(ENV_VAR_SDK_TOOL_VERSION, toolVersion.c_str(), 1);
#endif
    }
}

} // namespace Common
} // namespace GameLift
} // namespace Aws
