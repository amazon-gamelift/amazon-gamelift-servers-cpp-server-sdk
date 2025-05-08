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
#include <aws/gamelift/server/ProcessParameters.h>
#include <iostream>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Test {

#ifdef GAMELIFT_USE_STD

TEST(ProcessParametersTest, GIVEN_validInput_WHEN_createProcessParameters_std_THEN_success) {
    // GIVEN
    std::function<void(Server::Model::GameSession)> onStartGameSession = [](Server::Model::GameSession) {};
    std::function<void(Server::Model::UpdateGameSession)> onUpdateGameSession = [](Server::Model::UpdateGameSession) {};
    std::function<void()> onProcessTerminate = []() {};
    std::function<bool()> onHealthCheck = []() { return true; };
    int port = 900;
    std::vector<std::string> logPaths = {std::string("/log/path")};
    Server::LogParameters logParams = Server::LogParameters(logPaths);

    // WHEN
    Server::ProcessParameters processParams =
        Server::ProcessParameters(onStartGameSession, onUpdateGameSession, onProcessTerminate, onHealthCheck, port, logParams);

    // THEN
    ASSERT_TRUE(processParams.getOnStartGameSession());
    ASSERT_TRUE(processParams.getOnProcessTerminate());
    ASSERT_TRUE(processParams.getOnHealthCheck());
    ASSERT_EQ(processParams.getPort(), port);
    ASSERT_EQ(processParams.getLogParameters().getLogPaths(), logPaths);
}

#else

TEST(ProcessParametersTest, GIVEN_validInput_WHEN_createProcessParameters_no_std_THEN_success) {
    // GIVEN
    Server::StartGameSessionFn onStartGameSession = [](Server::Model::GameSession, void *) {};
    Server::UpdateGameSessionFn onUpdateGameSession = [](Server::Model::UpdateGameSession, void *) {};
    Server::ProcessTerminateFn onProcessTerminate = [](void *) {};
    Server::HealthCheckFn onHealthCheck = [](void *) { return true; };
    int port = 900;
    const char *logPath = "log/path";
    const char *logPaths[1];
    logPaths[0] = logPath;
    Server::LogParameters logParams = Server::LogParameters(logPaths, 1);

    // WHEN
    Aws::GameLift::Server::ProcessParameters processParams = Aws::GameLift::Server::ProcessParameters(
        onStartGameSession, this, onUpdateGameSession, this, onProcessTerminate, this, onHealthCheck, this, port, logParams);

    // THEN
    ASSERT_EQ(processParams.getOnStartGameSession(), onStartGameSession);
    ASSERT_EQ(processParams.getStartGameSessionState(), this);
    ASSERT_EQ(processParams.getOnUpdateGameSession(), onUpdateGameSession);
    ASSERT_EQ(processParams.getUpdateGameSessionState(), this);
    ASSERT_EQ(processParams.getOnProcessTerminate(), onProcessTerminate);
    ASSERT_EQ(processParams.getProcessTerminateState(), this);
    ASSERT_EQ(processParams.getOnHealthCheck(), onHealthCheck);
    ASSERT_EQ(processParams.getHealthCheckState(), this);
    ASSERT_EQ(processParams.getPort(), port);
    ASSERT_EQ(processParams.getLogParameters().getLogPathCount(), 1);
    ASSERT_STREQ(processParams.getLogParameters().getLogPath(0), logPath);
}

#endif

} // namespace Test
} // namespace Server
} // namespace GameLift
} // namespace Aws