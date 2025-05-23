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

#include <aws/gamelift/internal/model/request/UpdatePlayerSessionCreationPolicyRequest.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

bool UpdatePlayerSessionCreationPolicyRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    JsonHelper::WriteNonEmptyString(writer, GAME_SESSION_ID, m_gameSessionId);
    JsonHelper::WriteNonEmptyString(writer, PLAYER_SESSION_POLICY, GetPlayerSessionCreationPolicyAsString());
    return true;
}

bool UpdatePlayerSessionCreationPolicyRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_gameSessionId = JsonHelper::SafelyDeserializeString(value, GAME_SESSION_ID);
    std::string policyAsString = JsonHelper::SafelyDeserializeString(value, PLAYER_SESSION_POLICY);
    SetPlayerSessionCreationPolicy(policyAsString);
    return true;
}

std::ostream &operator<<(std::ostream &os, const UpdatePlayerSessionCreationPolicyRequest &updatePlayerSessionCreationPolicyRequest) {
    const Message *message = &updatePlayerSessionCreationPolicyRequest;
    os << message->Serialize();
    return os;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws