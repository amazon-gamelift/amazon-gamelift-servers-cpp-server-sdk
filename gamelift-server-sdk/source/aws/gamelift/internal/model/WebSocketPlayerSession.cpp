/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include <aws/gamelift/internal/model/WebSocketPlayerSession.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool WebSocketPlayerSession::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    JsonHelper::WriteNonEmptyString(writer, PLAYER_SESSION_ID, m_playerSessionId);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_ID, m_playerId);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ID, m_gameSessionId);
    JsonHelper::WriteNonEmptyString(writer, FLEET_ID, m_fleetId);
    JsonHelper::WritePositiveLong(writer, CREATION_TIME, m_creationTime);
    JsonHelper::WritePositiveLong(writer, TERMINATION_TIME, m_terminationTime);
    JsonHelper::WriteNonEmptyString(writer, STATUS, WebSocketPlayerSessionStatusMapper::GetNameForStatus(m_status));
    JsonHelper::WriteNonEmptyString(writer, IP_ADDRESS, m_ipAddress);
    JsonHelper::WritePositiveInt(writer, PORT, m_port);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_DATA, m_playerData);
    JsonHelper::WriteNonEmptyString(writer, DNS_NAME, m_dnsName);
    return true;
}

bool WebSocketPlayerSession::Deserialize(const rapidjson::Value &value) {
    m_playerSessionId = JsonHelper::SafelyDeserializeString(value, PLAYER_SESSION_ID);
    m_playerId = JsonHelper::SafelyDeserializeString(value, PLAYER_ID);
    m_gameSessionId = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ID);
    m_fleetId = JsonHelper::SafelyDeserializeString(value, FLEET_ID);
    m_creationTime = JsonHelper::SafelyDeserializeLong(value, CREATION_TIME);
    m_terminationTime = JsonHelper::SafelyDeserializeLong(value, TERMINATION_TIME);
    m_status = WebSocketPlayerSessionStatusMapper::GetStatusForName(JsonHelper::SafelyDeserializeString(value, STATUS));
    m_ipAddress = JsonHelper::SafelyDeserializeString(value, IP_ADDRESS);
    m_port = JsonHelper::SafelyDeserializeInt(value, PORT);
    m_playerData = JsonHelper::SafelyDeserializeString(value, PLAYER_DATA);
    m_dnsName = JsonHelper::SafelyDeserializeString(value, DNS_NAME);
    return true;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws