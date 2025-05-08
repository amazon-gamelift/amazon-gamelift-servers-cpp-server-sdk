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

#include <aws/gamelift/internal/model/WebSocketPlayer.h>
#include <aws/gamelift/internal/util/JsonHelper.h>
#include <iostream>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

std::string WebSocketPlayer::Serialize() const {
    // Create the buffer & Writer for the object
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // Start the object and call Serialize to serialize.
    writer.StartObject();
    if (Serialize(&writer)) {
        writer.EndObject();
        return buffer.GetString();
    }
    spdlog::warn("Could not parse into WebSocketPlayer");
    return "";
}

bool WebSocketPlayer::Deserialize(const std::string &jsonString) {
    // Parse the json into a document
    rapidjson::Document doc;
    if (doc.Parse(jsonString.c_str()).HasParseError()) {
        spdlog::error("WebSocketPlayer: Parse error found for: {}", jsonString);
        return false;
    }

    // Call Deserialize to populate the object's member variables
    return Deserialize(doc);
}

bool WebSocketPlayer::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {

    JsonHelper::WriteNonEmptyString(writer, PLAYER_ID, m_playerId);

    writer->String(PLAYER_ATTRIBUTES);
    writer->StartObject();
    for (auto const &playerAttributesIter : m_playerAttributes) {
        writer->String(playerAttributesIter.first.c_str());
        writer->StartObject();
        playerAttributesIter.second.Serialize(writer);
        writer->EndObject();
    }
    writer->EndObject();

    writer->String(LATENCY_IN_MS);
    writer->StartObject();
    for (auto const &latencyMsIter : m_latencyInMs) {
        writer->String(latencyMsIter.first.c_str());
        writer->Int(latencyMsIter.second);
    }
    writer->EndObject();

    JsonHelper::WriteNonEmptyString(writer, TEAM, m_team);

    return true;
}

bool WebSocketPlayer::Deserialize(const rapidjson::Value &value) {

    m_playerId = JsonHelper::SafelyDeserializeString(value, PLAYER_ID);
    m_team = JsonHelper::SafelyDeserializeString(value, TEAM);

    if (value.HasMember(PLAYER_ATTRIBUTES) && !value[PLAYER_ATTRIBUTES].IsNull()) {
        for (auto itr = value[PLAYER_ATTRIBUTES].MemberBegin(); itr != value[PLAYER_ATTRIBUTES].MemberEnd(); ++itr) {
            if (itr->name.IsString() && !itr->value.IsNull()) {
                WebSocketAttributeValue value;
                value.Deserialize(itr->value);
                m_playerAttributes[itr->name.GetString()] = value;
            }
        }
    }

    if (value.HasMember(LATENCY_IN_MS) && !value[LATENCY_IN_MS].IsNull()) {
        for (auto itr = value[LATENCY_IN_MS].MemberBegin(); itr != value[LATENCY_IN_MS].MemberEnd(); ++itr) {
            if (itr->name.IsString() && itr->value.IsInt()) {
                m_latencyInMs[itr->name.GetString()] = itr->value.GetInt();
            }
        }
    }

    return true;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws