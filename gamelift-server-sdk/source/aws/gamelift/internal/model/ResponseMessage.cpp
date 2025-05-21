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

#include <aws/gamelift/internal/model/ResponseMessage.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool ResponseMessage::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    JsonHelper::WritePositiveInt(writer, STATUS_CODE, m_statusCode);
    JsonHelper::WriteNonEmptyString(writer, ERROR_MESSAGE, m_errorMessage);
    return true;
}

bool ResponseMessage::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_statusCode = JsonHelper::SafelyDeserializeInt(value, STATUS_CODE);
    m_errorMessage = JsonHelper::SafelyDeserializeString(value, ERROR_MESSAGE);
    return true;
}

std::ostream &operator<<(std::ostream &os, const ResponseMessage &responseMessage) {
    const Message *message = &responseMessage;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws