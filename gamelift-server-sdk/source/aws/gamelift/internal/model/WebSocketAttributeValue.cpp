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

#include <aws/gamelift/internal/model/WebSocketAttributeValue.h>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

std::string WebSocketAttributeValue::Serialize() const {
    // Create the buffer & Writer for the object
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // Start the object and call Serialize to serialize.
    writer.StartObject();
    if (Serialize(&writer)) {
        writer.EndObject();
        return buffer.GetString();
    }
    spdlog::warn("Could not parse into WebSocketAttributeValue");
    return "";
}

bool WebSocketAttributeValue::Deserialize(const std::string &jsonString) {
    // Parse the json into a document
    rapidjson::Document doc;
    if (doc.Parse(jsonString.c_str()).HasParseError()) {
        spdlog::error("WebSocketAttributeValue: Parse error found for: {}", jsonString);
        return false;
    }

    // Call Deserialize to populate the object's member variables
    return Deserialize(doc);
}

bool WebSocketAttributeValue::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {

    writer->String(ATTR_TYPE);
    writer->String(GetAttributeTypeAsString().c_str());

    switch (m_attrType) {
    case WebSocketAttrType::STRING:
        writer->String(S);
        writer->String(m_S.c_str());
        break;
    case WebSocketAttrType::DOUBLE:
        writer->String(N);
        writer->Double(m_N);
        break;
    case WebSocketAttrType::STRING_LIST:
        writer->String(SL);
        writer->StartArray();
        for (auto it = m_SL.begin(); it != m_SL.end(); ++it) {
            writer->String(it->c_str());
        }
        writer->EndArray();
        break;
    case WebSocketAttrType::STRING_DOUBLE_MAP:
        writer->String(SDM);
        writer->StartObject();
        for (auto const &property : m_SDM) {
            writer->String(property.first.c_str());
            writer->Double(property.second);
        }
        writer->EndObject();
        break;
    default:
        // Do Nothing
        break;
    }

    return true;
}

bool WebSocketAttributeValue::Deserialize(const rapidjson::Value &value) {
    std::string attrString = value.HasMember(ATTR_TYPE) ? value[ATTR_TYPE].GetString() : "";
    SetAttributeType(attrString);

    switch (m_attrType) {
    case WebSocketAttrType::STRING:
        m_S = value.HasMember(S) ? value[S].GetString() : "";
        break;
    case WebSocketAttrType::DOUBLE:
        m_N = value.HasMember(N) ? value[N].GetDouble() : 0;
        break;
    case WebSocketAttrType::STRING_LIST:
        if (value.HasMember(SL)) {
            const rapidjson::Value &slValue = value[SL];
            std::vector<std::string> sl;
            for (rapidjson::SizeType i = 0; i < slValue.GetArray().Size(); i++) {
                sl.push_back(slValue.GetArray()[i].GetString());
            }
            m_SL = sl;
        }
        break;
    case WebSocketAttrType::STRING_DOUBLE_MAP:
        if (value.HasMember(SDM)) {
            const rapidjson::Value &sdmValue = value[SDM];
            std::map<std::string, double> sdm;
            for (rapidjson::Value::ConstMemberIterator iter = sdmValue.MemberBegin(); iter != sdmValue.MemberEnd(); ++iter) {
                sdm[iter->name.GetString()] = iter->value.GetDouble();
            }
            m_SDM = sdm;
        }
        break;
    default:
        // Do Nothing
        break;
    }

    return true;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws