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

#include <aws/gamelift/internal/model/WebSocketGameSession.h>
#include <aws/gamelift/internal/util/JsonHelper.h>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

std::string WebSocketGameSession::Serialize() const {
    // Create the buffer & Writer for the object
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // Start the object and call Serialize to serialize.
    writer.StartObject();
    if (Serialize(&writer)) {
        writer.EndObject();
        return buffer.GetString();
    }
    spdlog::warn("Could not parse into WebSocketGameSession");
    return "";
}

bool WebSocketGameSession::Deserialize(const std::string &jsonString) {
    // Parse the json into a document
    rapidjson::Document doc;
    if (doc.Parse(jsonString.c_str()).HasParseError()) {
        spdlog::error("WebSocketGameSession: Parse error found for: {}", jsonString);
        return false;
    }

    // Call Deserialize to populate the object's member variables
    return Deserialize(doc);
}

bool WebSocketGameSession::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ID, m_gameSessionId);
    JsonHelper::WriteNonEmptyString(writer, NAME, m_name);
    JsonHelper::WriteNonEmptyString(writer, FLEET_ID, m_fleetId);
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

bool WebSocketGameSession::Deserialize(const rapidjson::Value &value) {
    m_gameSessionId = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ID);
    m_name = JsonHelper::SafelyDeserializeString(value, NAME);
    m_fleetId = JsonHelper::SafelyDeserializeString(value, FLEET_ID);
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

std::ostream &operator<<(std::ostream &os, const WebSocketGameSession &gameSession) {
    const WebSocketGameSession *message = &gameSession;
    os << message->Serialize();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws