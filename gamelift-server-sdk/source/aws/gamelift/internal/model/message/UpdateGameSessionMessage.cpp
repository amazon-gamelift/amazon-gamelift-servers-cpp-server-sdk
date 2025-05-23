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
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

bool UpdateGameSessionMessage::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    // Serialize game session object
    writer->String(GAME_SESSION);
    writer->StartObject();
    m_gameSession.Serialize(writer);
    writer->EndObject();
    // Serialize update game session fields
    JsonHelper::WriteNonEmptyString(writer, UPDATE_REASON, m_updateReason);
    JsonHelper::WriteNonEmptyString(writer, BACKFILL_TICKET_ID, m_backfillTicketId);

    return true;
}

bool UpdateGameSessionMessage::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    // Deserialize GameSession
    WebSocketGameSession gameSession;
    if (value.HasMember(GAME_SESSION) && !value[GAME_SESSION].IsNull()) {
        gameSession.Deserialize(value[GAME_SESSION]);
    }
    m_gameSession = gameSession;
    // Deserialize rest of fields
    m_updateReason = JsonHelper::SafelyDeserializeString(value, UPDATE_REASON);
    m_backfillTicketId = JsonHelper::SafelyDeserializeString(value, BACKFILL_TICKET_ID);

    return true;
}

std::ostream &operator<<(std::ostream &os, const UpdateGameSessionMessage &updateGameSessionMessage) {
    const Message *message = &updateGameSessionMessage;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws