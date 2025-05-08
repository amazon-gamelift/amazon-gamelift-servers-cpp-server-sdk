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

#include <aws/gamelift/internal/model/message/UpdateGameSessionMessage.h>
#include <aws/gamelift/internal/network/callback/UpdateGameSessionCallback.h>
#include <spdlog/spdlog.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {

GenericOutcome UpdateGameSessionCallback::OnUpdateGameSession(const std::string &data) {
    spdlog::info("OnUpdateGameSession Received with raw data: {}", data);
    UpdateGameSessionMessage updateGameSessionMessage;
    Message &message = updateGameSessionMessage;
    message.Deserialize(data);

    GameSession gameSession;
    WebSocketGameSession webSocketGameSession = updateGameSessionMessage.GetGameSession();
    gameSession.WithGameSessionId(webSocketGameSession.GetGameSessionId().c_str())
        .WithName(webSocketGameSession.GetName().c_str())
        .WithFleetId(webSocketGameSession.GetFleetId().c_str())
        .WithMaximumPlayerSessionCount(webSocketGameSession.GetMaximumPlayerSessionCount())
        .WithIpAddress(webSocketGameSession.GetIpAddress().c_str())
        .WithPort(webSocketGameSession.GetPort())
        .WithGameSessionData(webSocketGameSession.GetGameSessionData().c_str())
        .WithMatchmakerData(webSocketGameSession.GetMatchmakerData().c_str())
        .WithDnsName(webSocketGameSession.GetDnsName().c_str());

    std::map<std::string, std::string>::const_iterator mapIterator;
    for (mapIterator = webSocketGameSession.GetGameProperties().begin(); mapIterator != webSocketGameSession.GetGameProperties().end(); mapIterator++) {
        GameProperty gameProperty;
        gameProperty.SetKey(mapIterator->first.c_str());
        gameProperty.SetValue(mapIterator->second.c_str());
        gameSession.AddGameProperty(gameProperty);
    }

    UpdateGameSession updateGameSession(gameSession, UpdateReasonMapper::GetUpdateReasonForName(updateGameSessionMessage.GetUpdateReason().c_str()),
                                        updateGameSessionMessage.GetBackfillTicketId().c_str());

    m_gameLiftMessageHandler->OnUpdateGameSession(updateGameSession);

    return GenericOutcome(nullptr);
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws