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
#include <aws/gamelift/server/LogParameters.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Test {

#ifdef GAMELIFT_USE_STD

TEST(LogParametersTest, GIVEN_validInput_WHEN_createLogParameters_std_THEN_success) {
    // GIVEN
    std::string logPath1 = "log/path1";
    std::string logPath2 = "log/path2";
    std::vector<std::string> logPaths = {logPath1, logPath2};
    // WHEN
    Server::LogParameters logParams = Server::LogParameters(logPaths);
    // THEN
    std::vector<std::string> storedLogPaths = logParams.getLogPaths();
    ASSERT_EQ(storedLogPaths[0], logPath1);
    ASSERT_EQ(storedLogPaths[1], logPath2);
}

#else

TEST(LogParametersTest, GIVEN_validInput_WHEN_createLogParameters_no_std_THEN_success) {
    // GIVEN
    const char *logPath1 = "log/path1";
    const char *logPath2 = "log/path2";
    const char *logPaths[2];
    logPaths[0] = logPath1;
    logPaths[1] = logPath2;
    // WHEN
    Server::LogParameters logParams = Server::LogParameters(logPaths, 2);
    // THEN
    ASSERT_EQ(logParams.getLogPathCount(), 2);
    ASSERT_STREQ(logParams.getLogPath(0), logPath1);
    ASSERT_STREQ(logParams.getLogPath(1), logPath2);
}

#endif

} // namespace Test
} // namespace Server
} // namespace GameLift
} // namespace Aws