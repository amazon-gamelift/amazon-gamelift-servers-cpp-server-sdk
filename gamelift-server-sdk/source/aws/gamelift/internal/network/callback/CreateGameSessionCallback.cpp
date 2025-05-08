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

#include <aws/gamelift/internal/model/message/CreateGameSessionMessage.h>
#include <aws/gamelift/internal/network/callback/CreateGameSessionCallback.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {

GenericOutcome CreateGameSessionCallback::OnStartGameSession(const std::string &data) {
    CreateGameSessionMessage createGameSessionMessage;
    Message &message = createGameSessionMessage;
    message.Deserialize(data);

    GameSession gameSession;
    gameSession.WithGameSessionId(createGameSessionMessage.GetGameSessionId().c_str())
        .WithName(createGameSessionMessage.GetGameSessionName().c_str())
        .WithMaximumPlayerSessionCount(createGameSessionMessage.GetMaximumPlayerSessionCount())
        .WithIpAddress(createGameSessionMessage.GetIpAddress().c_str())
        .WithPort(createGameSessionMessage.GetPort())
        .WithGameSessionData(createGameSessionMessage.GetGameSessionData().c_str())
        .WithMatchmakerData(createGameSessionMessage.GetMatchmakerData().c_str())
        .WithDnsName(createGameSessionMessage.GetDnsName().c_str());

    std::map<std::string, std::string>::const_iterator mapIterator;
    for (mapIterator = createGameSessionMessage.GetGameProperties().begin(); mapIterator != createGameSessionMessage.GetGameProperties().end(); mapIterator++) {
        GameProperty gameProperty;
        gameProperty.SetKey(mapIterator->first.c_str());
        gameProperty.SetValue(mapIterator->second.c_str());
        gameSession.AddGameProperty(gameProperty);
    }

    m_gameLiftMessageHandler->OnStartGameSession(gameSession);

    return GenericOutcome(nullptr);
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws