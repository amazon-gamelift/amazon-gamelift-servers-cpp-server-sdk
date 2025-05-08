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

#include <aws/gamelift/internal/model/request/WebSocketGetFleetRoleCredentialsRequest.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool WebSocketGetFleetRoleCredentialsRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);

    JsonHelper::WriteNonEmptyString(writer, ROLE_ARN, m_roleArn);
    JsonHelper::WriteNonEmptyString(writer, ROLE_SESSION_NAME, m_roleSessionName);

    return true;
}

bool WebSocketGetFleetRoleCredentialsRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);

    m_roleArn = JsonHelper::SafelyDeserializeString(value, ROLE_ARN);
    m_roleSessionName = JsonHelper::SafelyDeserializeString(value, ROLE_SESSION_NAME);

    return true;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws