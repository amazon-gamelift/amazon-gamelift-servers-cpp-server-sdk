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

#include <aws/gamelift/internal/model/Message.h>
#include <aws/gamelift/internal/util/JsonHelper.h>
#include <aws/gamelift/internal/util/RandomStringGenerator.h>

namespace Aws {
namespace GameLift {
namespace Internal {

std::string Message::Serialize() const {
    // Create the buffer & Writer for the object
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // Start the object and call Serialize to serialize.
    writer.StartObject();
    if (Serialize(&writer)) {
        writer.EndObject();
        return buffer.GetString();
    }
    return "";
}

bool Message::Deserialize(const std::string &jsonString) {
    // Parse the json into a document
    rapidjson::Document doc;
    if (doc.Parse(jsonString.c_str()).HasParseError()) {
        return false;
    }

    // Call Deserialize to populate the object's member variables
    return Deserialize(doc);
}

bool Message::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    JsonHelper::WriteNonEmptyString(writer, ACTION, m_action);
    JsonHelper::WriteNonEmptyString(writer, REQUEST_ID, m_requestId);
    return true;
}

bool Message::Deserialize(const rapidjson::Value &value) {
    m_action = JsonHelper::SafelyDeserializeString(value, ACTION);
    m_requestId = JsonHelper::SafelyDeserializeString(value, REQUEST_ID);
    return true;
}

std::ostream &operator<<(std::ostream &os, const Message &message) {
    os << message.Serialize();
    return os;
}

std::string Message::GenerateRandomRequestId() { return RandomStringGenerator::GenerateRandomAlphaNumericString(32); }

} // namespace Internal
} // namespace GameLift
} // namespace Aws