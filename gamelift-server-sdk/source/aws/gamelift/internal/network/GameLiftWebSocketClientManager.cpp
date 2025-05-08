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

#include <aws/gamelift/internal/GameLiftServerState.h>
#include <aws/gamelift/internal/network/GameLiftWebSocketClientManager.h>

#include <aws/gamelift/internal/model/response/WebSocketDescribePlayerSessionsResponse.h>

#include <spdlog/spdlog.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {

GenericOutcome GameLiftWebSocketClientManager::Connect(std::string websocketUrl, const std::string &authToken, const std::string &processId,
                                                       const std::string &hostId, const std::string &fleetId) {
    spdlog::info("Connecting to GameLift WebSocket server. websocketUrl: {}, processId: {}, hostId: {}, fleetId: {}",
           websocketUrl.c_str(), processId.c_str(), hostId.c_str(), fleetId.c_str());

    // Due to the websocket library we're using, base URLs must end with a "/". Ensure that it is
    // present.
    if (!EndsWith(websocketUrl, REQUIRED_URL_ENDING)) {
        websocketUrl = websocketUrl + REQUIRED_URL_ENDING;
    }

    // Build the WebSocket URI
    std::string sdkVersion = Server::GetSdkVersion().GetResult();
    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(websocketUrl)
                  .AddQueryParam(PID_KEY, processId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, GameLiftServerState::LANGUAGE)
                  .AddQueryParam(AUTH_TOKEN_KEY, authToken)
                  .AddQueryParam(COMPUTE_ID_KEY, hostId)
                  .AddQueryParam(FLEET_ID_KEY, fleetId)
                  .Build();

    // delegate to the websocket client wrapper to connect
    return m_webSocketClientWrapper->Connect(uri);
}

GenericOutcome GameLiftWebSocketClientManager::SendSocketMessage(Message &message) {
    // Serialize the message
    std::string jsonMessage = message.Serialize();

    GenericOutcome outcome = m_webSocketClientWrapper->SendSocketMessage(message.GetRequestId(), jsonMessage);
    return outcome;
}

void GameLiftWebSocketClientManager::Disconnect() { m_webSocketClientWrapper->Disconnect(); }

bool GameLiftWebSocketClientManager::EndsWith(const std::string &actualString, const std::string &ending) {
    const int lengthDifference = (int)(actualString.length() - ending.length());
    if (lengthDifference >= 0) {
        return actualString.compare(lengthDifference, ending.length(), ending) == 0;
    } else {
        return false;
    }
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws
