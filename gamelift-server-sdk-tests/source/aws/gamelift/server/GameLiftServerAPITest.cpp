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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <aws/gamelift/internal/GameLiftServerState.h>
#include <iostream>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Test {

static const std::string sdkVersion = "5.2.0";

TEST(GameLiftServerAPITest, GIVEN_SdkVersion_WHEN_GetSdkVersion_THEN_success) {
    // GIVEN
    // WHEN
    AwsStringOutcome outcome = Server::GetSdkVersion();
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
#ifdef GAMELIFT_USE_STD
    ASSERT_EQ(outcome.GetResult(), sdkVersion);
#else
    ASSERT_STREQ(outcome.GetResult(), sdkVersion.c_str());
#endif
}

} // namespace Test
} // namespace Server
} // namespace GameLift
} // namespace Aws