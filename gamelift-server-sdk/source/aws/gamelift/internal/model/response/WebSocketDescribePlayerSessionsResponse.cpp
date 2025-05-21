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

#include <aws/gamelift/internal/model/response/WebSocketDescribePlayerSessionsResponse.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool WebSocketDescribePlayerSessionsResponse::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);

    JsonHelper::WriteNonEmptyString(writer, NEXT_TOKEN, m_nextToken);

    writer->String(PLAYER_SESSIONS);
    writer->StartArray();
    for (const WebSocketPlayerSession &playerSession : m_playerSessions) {
        writer->StartObject();
        playerSession.Serialize(writer);
        writer->EndObject();
    }
    writer->EndArray();

    return true;
}

bool WebSocketDescribePlayerSessionsResponse::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);

    m_nextToken = JsonHelper::SafelyDeserializeString(value, NEXT_TOKEN);
    m_playerSessions.clear();
    if (value.HasMember(PLAYER_SESSIONS) && !value[PLAYER_SESSIONS].IsNull()) {
        auto playerSessions = value[PLAYER_SESSIONS].GetArray();
        for (rapidjson::SizeType i = 0; i < playerSessions.Size(); i++) {
            auto &playerSession = playerSessions[i];
            if (!playerSession.IsNull()) {
                WebSocketPlayerSession webSocketPlayerSession;
                webSocketPlayerSession.Deserialize(playerSession);
                m_playerSessions.push_back(webSocketPlayerSession);
            }
        }
    }

    return true;
}

std::ostream &operator<<(std::ostream &os, const WebSocketDescribePlayerSessionsResponse &describePlayerSessionsResponse) {
    const Message *message = &describePlayerSessionsResponse;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws