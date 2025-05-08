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

#include <aws/gamelift/internal/model/request/ActivateServerProcessRequest.h>
#include <aws/gamelift/internal/util/JsonHelper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

ActivateServerProcessRequest::ActivateServerProcessRequest() : m_port(-1) { SetAction(ACTIVATE_SERVER_PROCESS); }

ActivateServerProcessRequest::ActivateServerProcessRequest(std::string sdkVersion, std::string sdkLanguage, int port,
                                                           const Aws::GameLift::Server::LogParameters &logParameters)
    : m_sdkVersion(sdkVersion), m_sdkLanguage(sdkLanguage), m_port(port), m_logParameters(logParameters) {
    SetAction(ACTIVATE_SERVER_PROCESS);
};

bool ActivateServerProcessRequest::Serialize(rapidjson::Writer<rapidjson::StringBuffer> *writer) const {
    Message::Serialize(writer);
    JsonHelper::WriteNonEmptyString(writer, SDK_VERSION, m_sdkVersion);
    JsonHelper::WriteNonEmptyString(writer, SDK_LANGUAGE, m_sdkLanguage);
    JsonHelper::WritePositiveInt(writer, PORT, m_port);
    JsonHelper::WriteLogParameters(writer, LOG_PATHS, m_logParameters);
    return true;
}

bool ActivateServerProcessRequest::Deserialize(const rapidjson::Value &value) {
    Message::Deserialize(value);
    m_sdkVersion = JsonHelper::SafelyDeserializeString(value, SDK_VERSION);
    m_sdkLanguage = JsonHelper::SafelyDeserializeString(value, SDK_LANGUAGE);
    m_port = JsonHelper::SafelyDeserializeInt(value, PORT);
    m_logParameters = JsonHelper::SafelyDeserializeLogParameters(value, LOG_PATHS);
    return true;
}

std::ostream &operator<<(std::ostream &os, const ActivateServerProcessRequest &activateServerProcessRequest) {
    const Message *message = &activateServerProcessRequest;
    os << message->Serialize();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws