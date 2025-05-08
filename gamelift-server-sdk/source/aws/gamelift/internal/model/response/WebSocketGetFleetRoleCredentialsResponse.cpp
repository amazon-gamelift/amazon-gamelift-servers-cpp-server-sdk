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

#include <aws/gamelift/internal/model/response/WebSocketGetFleetRoleCredentialsResponse.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
bool WebSocketGetFleetRoleCredentialsResponse::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);

    JsonHelper::WriteNonEmptyString(writer, ASSUMED_ROLE_USER_ARN, m_assumedRoleUserArn);
    JsonHelper::WriteNonEmptyString(writer, ASSUMED_ROLE_ID, m_assumedRoleId);
    JsonHelper::WriteNonEmptyString(writer, ACCESS_KEY_ID, m_accessKeyId);
    JsonHelper::WriteNonEmptyString(writer, SECRET_ACCESS_KEY, m_secretAccessKey);
    JsonHelper::WriteNonEmptyString(writer, SESSION_TOKEN, m_sessionToken);
    JsonHelper::WritePositiveLong(writer, EXPIRATION, m_expiration);

    return true;
}

bool WebSocketGetFleetRoleCredentialsResponse::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);

    m_assumedRoleUserArn = JsonHelper::SafelyDeserializeString(value, ASSUMED_ROLE_USER_ARN);
    m_assumedRoleId = JsonHelper::SafelyDeserializeString(value, ASSUMED_ROLE_ID);
    m_accessKeyId = JsonHelper::SafelyDeserializeString(value, ACCESS_KEY_ID);
    m_secretAccessKey = JsonHelper::SafelyDeserializeString(value, SECRET_ACCESS_KEY);
    m_sessionToken = JsonHelper::SafelyDeserializeString(value, SESSION_TOKEN);
    m_expiration = JsonHelper::SafelyDeserializeLong(value, EXPIRATION);

    return true;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws