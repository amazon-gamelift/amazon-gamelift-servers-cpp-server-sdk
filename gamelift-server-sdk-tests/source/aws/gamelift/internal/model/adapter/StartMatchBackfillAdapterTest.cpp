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
#include <aws/gamelift/internal/model/adapter/StartMatchBackfillAdapter.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class StartMatchBackfillAdapterTest : public ::testing::Test {};

TEST_F(StartMatchBackfillAdapterTest, GIVEN_WebSocketStartMatchBackfillResponse_WHEN_convert_THEN_success) {
    // GIVEN
    std::unique_ptr<WebSocketStartMatchBackfillResponse> response(new WebSocketStartMatchBackfillResponse());
    response->SetTicketId("testTicketId");

    // WHEN
    auto converted = Aws::GameLift::Internal::StartMatchBackfillAdapter::convert(response.get());

    // THEN
    EXPECT_EQ(response->GetTicketId(), converted.GetTicketId());
}

TEST_F(StartMatchBackfillAdapterTest, GIVEN_StartMatchBackfillRequest_WHEN_convert_THEN_success) {
    // GIVEN
    Server::Model::Player playerOne;
    Server::Model::Player playerTwo;
    // Don't allocate StartMatchBackfillRequests on the stack, it will overflow on Windows
    std::unique_ptr<Server::Model::StartMatchBackfillRequest> request(new Server::Model::StartMatchBackfillRequest());
    request->WithTicketId("testTicketId")
        .WithMatchmakingConfigurationArn("testMmConfigArn")
        .WithGameSessionArn("testGameSessionArn")
        .AddPlayer(playerOne)
        .AddPlayer(playerTwo);

    // WHEN
    auto converted = Aws::GameLift::Internal::StartMatchBackfillAdapter::convert(*request);

    // THEN
    EXPECT_EQ(request->GetTicketId(), converted.GetTicketId());
    EXPECT_EQ(request->GetMatchmakingConfigurationArn(), converted.GetMatchmakingConfigurationArn());
    EXPECT_EQ(request->GetGameSessionArn(), converted.GetGameSessionArn());
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws