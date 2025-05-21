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
#include <aws/gamelift/internal/model/response/WebSocketDescribePlayerSessionsResponse.h>
#include <aws/gamelift/internal/network/MockGameLiftMessageHandler.h>
#include <aws/gamelift/internal/network/callback/DescribePlayerSessionsCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class DescribePlayerSessionsCallbackTest : public ::testing::Test {
protected:
    DescribePlayerSessionsCallback *describePlayerSessionCallback;

    void SetUp() override { describePlayerSessionCallback = new DescribePlayerSessionsCallback(); }

    void TearDown() override {
        if (describePlayerSessionCallback) {
            delete describePlayerSessionCallback;
            describePlayerSessionCallback = nullptr;
        }
    }
};

TEST_F(DescribePlayerSessionsCallbackTest, GIVEN_createEmptyMessage_WHEN_onDescribePlayerSessions_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    // WHEN
    GenericOutcome genericOutcome = describePlayerSessionCallback->OnDescribePlayerSessions(jsonMessage);
    WebSocketDescribePlayerSessionsResponse *response = static_cast<WebSocketDescribePlayerSessionsResponse *>(genericOutcome.GetResult());

    // THEN
    EXPECT_EQ(response->GetNextToken(), "");
    EXPECT_EQ(response->GetPlayerSessions().size(), 0);

    delete response;
}

TEST_F(DescribePlayerSessionsCallbackTest, GIVEN_createMessage_WHEN_onDescribePlayerSessions_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"NextToken\":\"nextToken\",\"PlayerSessions\":[{}]}";

    // WHEN
    GenericOutcome genericOutcome = describePlayerSessionCallback->OnDescribePlayerSessions(jsonMessage);
    WebSocketDescribePlayerSessionsResponse *response = static_cast<WebSocketDescribePlayerSessionsResponse *>(genericOutcome.GetResult());

    // THEN
    EXPECT_EQ(response->GetNextToken(), "nextToken");
    EXPECT_EQ(response->GetPlayerSessions().size(), 1);

    delete response;
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws