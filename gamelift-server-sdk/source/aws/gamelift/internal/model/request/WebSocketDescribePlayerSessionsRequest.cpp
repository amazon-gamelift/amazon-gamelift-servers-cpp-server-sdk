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

#include <aws/gamelift/internal/model/request/WebSocketDescribePlayerSessionsRequest.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool WebSocketDescribePlayerSessionsRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);

    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ID, m_gameSessionId);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_ID, m_playerId);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_SESSION_ID, m_playerSessionId);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_SESSION_STATUS_FILTER, m_playerSessionStatusFilter);
    JsonHelper::WriteNonEmptyString(writer, NEXT_TOKEN, m_nextToken);
    JsonHelper::WritePositiveInt(writer, LIMIT, m_limit);

    return true;
}

bool WebSocketDescribePlayerSessionsRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);

    m_gameSessionId = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ID);
    m_playerId = JsonHelper::SafelyDeserializeString(value, PLAYER_ID);
    m_playerSessionId = JsonHelper::SafelyDeserializeString(value, PLAYER_SESSION_ID);
    m_playerSessionStatusFilter = JsonHelper::SafelyDeserializeString(value, PLAYER_SESSION_STATUS_FILTER);
    m_nextToken = JsonHelper::SafelyDeserializeString(value, NEXT_TOKEN);
    m_limit = JsonHelper::SafelyDeserializeInt(value, LIMIT);

    return true;
}

std::ostream &operator<<(std::ostream &os, const WebSocketDescribePlayerSessionsRequest &describePlayerSessionsRequest) {
    const Message *message = &describePlayerSessionsRequest;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws