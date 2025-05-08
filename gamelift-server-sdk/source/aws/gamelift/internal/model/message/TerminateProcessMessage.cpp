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

#include <aws/gamelift/internal/model/message/TerminateProcessMessage.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

bool TerminateProcessMessage::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    JsonHelper::WritePositiveLong(writer, TERMINATION_TIME, m_terminationTime);
    return true;
}

bool TerminateProcessMessage::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_terminationTime = JsonHelper::SafelyDeserializeLong(value, TERMINATION_TIME);
    return true;
}

std::ostream &operator<<(std::ostream &os, const TerminateProcessMessage &terminateProcessMessage) {
    const Message *message = &terminateProcessMessage;
    os << message->Serialize();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws