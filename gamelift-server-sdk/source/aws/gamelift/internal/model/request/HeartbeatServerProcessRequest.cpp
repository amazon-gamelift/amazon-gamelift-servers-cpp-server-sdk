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

#include <aws/gamelift/internal/model/request/HeartbeatServerProcessRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {

bool HeartbeatServerProcessRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    writer->String(HEALTH_STATUS);
    writer->Bool(m_healthy);
    return true;
}

bool HeartbeatServerProcessRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_healthy = value.HasMember(HEALTH_STATUS) && value[HEALTH_STATUS].IsBool() ? value[HEALTH_STATUS].GetBool() : false;
    return true;
}

std::ostream &operator<<(std::ostream &os, const HeartbeatServerProcessRequest &heartbeatServerProcessRequest) {
    const Message *message = &heartbeatServerProcessRequest;
    os << message->Serialize();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws