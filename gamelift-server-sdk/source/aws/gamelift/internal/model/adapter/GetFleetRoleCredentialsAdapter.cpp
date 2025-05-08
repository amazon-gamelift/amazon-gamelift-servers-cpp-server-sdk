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

#include <aws/gamelift/internal/model/adapter/GetFleetRoleCredentialsAdapter.h>

namespace Aws {
namespace GameLift {
namespace Internal {
Server::Model::GetFleetRoleCredentialsResult GetFleetRoleCredentialsAdapter::convert(const WebSocketGetFleetRoleCredentialsResponse *webSocketResponse) {
    Server::Model::GetFleetRoleCredentialsResult result;

    result.SetAssumedUserRoleArn(webSocketResponse->GetAssumedRoleUserArn().c_str());
    result.SetAssumedRoleId(webSocketResponse->GetAssumedRoleId().c_str());
    result.SetAccessKeyId(webSocketResponse->GetAccessKeyId().c_str());
    result.SetSecretAccessKey(webSocketResponse->GetSecretAccessKey().c_str());
    result.SetSessionToken(webSocketResponse->GetSessionToken().c_str());
    // time_t is Unix epoch in seconds, and webSocketResponse->GetExpiration() returns time in milliseconds
    // This is why we are dividing webSocketResponse->GetExpiration() by 1000
    int64_t expirationTimeInMilliSeconds = webSocketResponse->GetExpiration();
    int64_t expirationTimeInSeconds = expirationTimeInMilliSeconds / 1000;
    result.SetExpiration(static_cast<time_t>(expirationTimeInSeconds));

    return result;
}

WebSocketGetFleetRoleCredentialsRequest GetFleetRoleCredentialsAdapter::convert(const Server::Model::GetFleetRoleCredentialsRequest &request) {
    return WebSocketGetFleetRoleCredentialsRequest().WithRoleArn(request.GetRoleArn()).WithRoleSessionName(request.GetRoleSessionName());
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws