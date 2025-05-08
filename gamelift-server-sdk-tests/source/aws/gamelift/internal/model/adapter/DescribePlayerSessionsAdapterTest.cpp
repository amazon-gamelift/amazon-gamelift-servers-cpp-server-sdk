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
#include <aws/gamelift/internal/model/adapter/DescribePlayerSessionsAdapter.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class DescribePlayerSessionsAdapterTest : public ::testing::Test {};

TEST_F(DescribePlayerSessionsAdapterTest, GIVEN_WebSocketDescribePlayerSessionsResponse_WHEN_convert_THEN_success) {
    // GIVEN
    Server::Model::DescribePlayerSessionsRequest request = Server::Model::DescribePlayerSessionsRequest()
                                                               .WithGameSessionId("gameSessionId")
                                                               .WithPlayerId("playerId")
                                                               .WithPlayerSessionId("playerSessionId")
                                                               .WithPlayerSessionStatusFilter("playerSessionStatusFilter")
                                                               .WithNextToken("nextToken")
                                                               .WithLimit(100);
    // WHEN
    auto converted = Internal::DescribePlayerSessionsAdapter::convert(request);
    // THEN
    EXPECT_EQ(request.GetGameSessionId(), converted.GetGameSessionId());
    EXPECT_EQ(request.GetPlayerId(), converted.GetPlayerId());
    EXPECT_EQ(request.GetPlayerSessionId(), converted.GetPlayerSessionId());
    EXPECT_EQ(request.GetPlayerSessionStatusFilter(), converted.GetPlayerSessionStatusFilter());
    EXPECT_EQ(request.GetNextToken(), converted.GetNextToken());
    EXPECT_EQ(request.GetLimit(), converted.GetLimit());
}

TEST_F(DescribePlayerSessionsAdapterTest, GIVEN_DescribePlayerSessionsRequest_WHEN_convert_THEN_success) {
    // GIVEN
    std::unique_ptr<WebSocketDescribePlayerSessionsResponse> response(new WebSocketDescribePlayerSessionsResponse());
    WebSocketPlayerSession playerSession = WebSocketPlayerSession()
                                               .WithPlayerSessionId("playerSessionId")
                                               .WithPlayerId("playerId")
                                               .WithGameSessionId("gameSessionId")
                                               .WithFleetId("fleetId")
                                               .WithCreationTime(30)
                                               .WithTerminationTime(50)
                                               .WithStatus(WebSocketPlayerSessionStatus::COMPLETED)
                                               .WithIpAddress("ipAddress")
                                               .WithPort(25565)
                                               .WithPlayerData("playerData")
                                               .WithDnsName("dnsName");
    response->SetNextToken("abc");
    std::vector<WebSocketPlayerSession> webPlayerSessions;
    webPlayerSessions.push_back(playerSession);
    response->SetPlayerSessions(webPlayerSessions);
    // WHEN
    auto converted = Internal::DescribePlayerSessionsAdapter::convert(response.get());

    // THEN
    EXPECT_EQ(converted.GetNextToken(), response->GetNextToken());
#ifdef GAMELIFT_USE_STD
    const std::vector<Server::Model::PlayerSession> &playerSessions = converted.GetPlayerSessions();
    EXPECT_EQ(playerSessions.size(), 1);
#else
    int count;
    const Server::Model::PlayerSession *playerSessions = converted.GetPlayerSessions(count);
    EXPECT_EQ(count, 1);
#endif
    const Server::Model::PlayerSession &convertedPlayerSession = playerSessions[0];
    EXPECT_EQ(convertedPlayerSession.GetPlayerSessionId(), playerSession.GetPlayerSessionId());
    EXPECT_EQ(convertedPlayerSession.GetPlayerId(), playerSession.GetPlayerId());
    EXPECT_EQ(convertedPlayerSession.GetGameSessionId(), playerSession.GetGameSessionId());
    EXPECT_EQ(convertedPlayerSession.GetFleetId(), playerSession.GetFleetId());
    EXPECT_EQ(convertedPlayerSession.GetCreationTime(), playerSession.GetCreationTime());
    EXPECT_EQ(convertedPlayerSession.GetTerminationTime(), playerSession.GetTerminationTime());
    EXPECT_EQ(static_cast<int>(convertedPlayerSession.GetStatus()), static_cast<int>(playerSession.GetStatus()));
    EXPECT_EQ(convertedPlayerSession.GetIpAddress(), playerSession.GetIpAddress());
    EXPECT_EQ(convertedPlayerSession.GetPort(), playerSession.GetPort());
    EXPECT_EQ(convertedPlayerSession.GetPlayerData(), playerSession.GetPlayerData());
    EXPECT_EQ(convertedPlayerSession.GetDnsName(), playerSession.GetDnsName());
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws