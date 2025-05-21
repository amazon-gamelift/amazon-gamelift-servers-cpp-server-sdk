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

#include <aws/gamelift/internal/model/request/WebSocketStartMatchBackfillRequest.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

WebSocketStartMatchBackfillRequest::WebSocketStartMatchBackfillRequest() { SetAction(ACTION); };

bool WebSocketStartMatchBackfillRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);

    JsonHelper::WriteNonEmptyString(writer, TICKET_ID, m_ticketId);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ARN, m_gameSessionArn);
    JsonHelper::WriteNonEmptyString(writer, MATCHMAKING_CONFIGURATION_ARN, m_matchmakingConfigurationArn);

    writer->String(PLAYERS);
    writer->StartArray();
    for (const WebSocketPlayer &player : m_players) {
        writer->StartObject();
        player.Serialize(writer);
        writer->EndObject();
    }
    writer->EndArray();

    return true;
}

bool WebSocketStartMatchBackfillRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);

    m_ticketId = JsonHelper::SafelyDeserializeString(value, TICKET_ID);
    m_gameSessionArn = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ARN);
    m_matchmakingConfigurationArn = JsonHelper::SafelyDeserializeString(value, MATCHMAKING_CONFIGURATION_ARN);

    m_players.clear();
    if (value.HasMember(PLAYERS) && !value[PLAYERS].IsNull()) {
        auto playerList = value[PLAYERS].GetArray();
        for (rapidjson::SizeType i = 0; i < playerList.Size(); i++) {
            auto &player = playerList[i];
            if (!player.IsNull()) {
                WebSocketPlayer webSocketPlayer;
                webSocketPlayer.Deserialize(player);
                m_players.push_back(webSocketPlayer);
            }
        }
    }

    return true;
}

std::ostream &operator<<(std::ostream &os, const WebSocketStartMatchBackfillRequest &startMatchBackfillRequest) {
    const Message *message = &startMatchBackfillRequest;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws