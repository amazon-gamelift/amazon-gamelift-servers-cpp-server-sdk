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
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

bool CreateGameSessionMessage::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ID, m_gameSessionId);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_NAME, m_gameSessionName);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_DATA, m_gameSessionData);
    JsonHelper::WriteNonEmptyString(writer, MATCHMAKER_DATA, m_matchmakerData);
    JsonHelper::WriteNonEmptyString(writer, DNS_NAME, m_dnsName);
    JsonHelper::WriteNonEmptyString(writer, IP_ADDRESS, m_ipAddress);
    JsonHelper::WritePositiveInt(writer, MAXIMUM_PLAYER_SESSION_COUNT, m_maximumPlayerSessionCount);
    JsonHelper::WritePositiveInt(writer, PORT, m_port);

    writer->String(GAME_PROPERTIES);
    writer->StartObject();
    for (auto const &property : m_gameProperties) {
        writer->String(property.first.c_str());
        writer->String(property.second.c_str());
    }
    writer->EndObject();
    return true;
}

bool CreateGameSessionMessage::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_gameSessionId = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ID);
    m_gameSessionName = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_NAME);
    m_gameSessionData = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_DATA);
    m_matchmakerData = JsonHelper::SafelyDeserializeString(value, MATCHMAKER_DATA);
    m_dnsName = JsonHelper::SafelyDeserializeString(value, DNS_NAME);
    m_ipAddress = JsonHelper::SafelyDeserializeString(value, IP_ADDRESS);
    m_maximumPlayerSessionCount = JsonHelper::SafelyDeserializeInt(value, MAXIMUM_PLAYER_SESSION_COUNT);
    m_port = JsonHelper::SafelyDeserializeInt(value, PORT);

    if (value.HasMember(GAME_PROPERTIES) && !value[GAME_PROPERTIES].IsNull()) {
        for (auto itr = value[GAME_PROPERTIES].MemberBegin(); itr != value[GAME_PROPERTIES].MemberEnd(); ++itr) {
            if (itr->name.IsString() && itr->value.IsString()) {
                m_gameProperties[itr->name.GetString()] = itr->value.GetString();
            }
        }
    }

    return true;
}

std::ostream &operator<<(std::ostream &os, const CreateGameSessionMessage &createGameSessionMessage) {
    const Message *message = &createGameSessionMessage;
    os << message->Serialize();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws