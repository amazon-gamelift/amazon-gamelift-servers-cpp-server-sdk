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

#include <aws/gamelift/common/GameLift_EXPORTS.h>
#include <aws/gamelift/internal/model/adapter/DescribePlayerSessionsAdapter.h>
#include <aws/gamelift/server/model/PlayerSession.h>
#include <aws/gamelift/server/model/PlayerSessionStatus.h>

namespace Aws {
namespace GameLift {
namespace Internal {
Server::Model::DescribePlayerSessionsResult DescribePlayerSessionsAdapter::convert(const WebSocketDescribePlayerSessionsResponse *webSocketResponse) {
    Server::Model::DescribePlayerSessionsResult result;
#ifdef GAMELIFT_USE_STD
    result.SetNextToken(webSocketResponse->GetNextToken());
    for (auto &webSocketPlayerSession : webSocketResponse->GetPlayerSessions()) {
        Server::Model::PlayerSession playerSession =
            Server::Model::PlayerSession()
                .WithPlayerSessionId(webSocketPlayerSession.GetPlayerSessionId())
                .WithPlayerId(webSocketPlayerSession.GetPlayerId())
                .WithGameSessionId(webSocketPlayerSession.GetGameSessionId())
                .WithFleetId(webSocketPlayerSession.GetFleetId())
                .WithCreationTime(webSocketPlayerSession.GetCreationTime())
                .WithTerminationTime(webSocketPlayerSession.GetTerminationTime())
                .WithStatus(static_cast<Server::Model::PlayerSessionStatus>(static_cast<int>(webSocketPlayerSession.GetStatus())))
                .WithIpAddress(webSocketPlayerSession.GetIpAddress())
                .WithPort(webSocketPlayerSession.GetPort())
                .WithPlayerData(webSocketPlayerSession.GetPlayerData())
                .WithDnsName(webSocketPlayerSession.GetDnsName());
        result.AddPlayerSession(playerSession);
    }
#else
    result.SetNextToken(webSocketResponse->GetNextToken().c_str());
    for (auto &webSocketPlayerSession : webSocketResponse->GetPlayerSessions()) {
        Server::Model::PlayerSession playerSession =
            Server::Model::PlayerSession()
                .WithPlayerSessionId(webSocketPlayerSession.GetPlayerSessionId().c_str())
                .WithPlayerId(webSocketPlayerSession.GetPlayerId().c_str())
                .WithGameSessionId(webSocketPlayerSession.GetGameSessionId().c_str())
                .WithFleetId(webSocketPlayerSession.GetFleetId().c_str())
                .WithCreationTime(webSocketPlayerSession.GetCreationTime())
                .WithTerminationTime(webSocketPlayerSession.GetTerminationTime())
                .WithStatus(static_cast<Server::Model::PlayerSessionStatus>(static_cast<int>(webSocketPlayerSession.GetStatus())))
                .WithIpAddress(webSocketPlayerSession.GetIpAddress().c_str())
                .WithPort(webSocketPlayerSession.GetPort())
                .WithPlayerData(webSocketPlayerSession.GetPlayerData().c_str())
                .WithDnsName(webSocketPlayerSession.GetDnsName().c_str());
        result.AddPlayerSession(playerSession);
    }
#endif

    return result;
}

WebSocketDescribePlayerSessionsRequest DescribePlayerSessionsAdapter::convert(const Server::Model::DescribePlayerSessionsRequest &request) {
    return WebSocketDescribePlayerSessionsRequest()
        .WithGameSessionId(request.GetGameSessionId())
        .WithPlayerId(request.GetPlayerId())
        .WithPlayerSessionId(request.GetPlayerSessionId())
        .WithPlayerSessionStatusFilter(request.GetPlayerSessionStatusFilter())
        .WithNextToken(request.GetNextToken())
        .WithLimit(request.GetLimit());
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws