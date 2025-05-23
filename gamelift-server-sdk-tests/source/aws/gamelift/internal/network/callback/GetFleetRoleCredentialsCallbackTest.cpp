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
#include "gtest/gtest.h"
#include <aws/gamelift/internal/model/response/WebSocketGetFleetRoleCredentialsResponse.h>
#include <aws/gamelift/internal/network/callback/GetFleetRoleCredentialsCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class GetFleetRoleCredentialsCallbackTest : public ::testing::Test {
protected:
    GetFleetRoleCredentialsCallback getFleetRoleCredentialsCallback;
};

TEST_F(GetFleetRoleCredentialsCallbackTest, GIVEN_createEmptyMessage_WHEN_onGetFleetRoleCredentials_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    // WHEN
    GenericOutcome genericOutcome = getFleetRoleCredentialsCallback.OnGetFleetRoleCredentials(jsonMessage);
    std::unique_ptr<WebSocketGetFleetRoleCredentialsResponse> response(static_cast<WebSocketGetFleetRoleCredentialsResponse *>(genericOutcome.GetResult()));

    // THEN
    EXPECT_EQ(response->GetAssumedRoleUserArn(), "");
    EXPECT_EQ(response->GetAssumedRoleId(), "");
    EXPECT_EQ(response->GetAccessKeyId(), "");
    EXPECT_EQ(response->GetSecretAccessKey(), "");
    EXPECT_EQ(response->GetSessionToken(), "");
    EXPECT_EQ(response->GetExpiration(), -1);
}

TEST_F(GetFleetRoleCredentialsCallbackTest, GIVEN_createMessage_WHEN_onGetFleetRoleCredentials_THEN_success) {
    // GIVEN / EXPECT
    std::stringstream ss;
    ss << "{\"AssumedRoleUserArn\":\"AssumedRoleUserArn\","
       << "\"AssumedRoleId\":\"AssumedRoleId\","
       << "\"AccessKeyId\":\"AccessKeyId\","
       << "\"SecretAccessKey\":\"SecretAccessKey\","
       << "\"SessionToken\":\"SessionToken\","
       << "\"Expiration\":123}";
    std::string jsonMessage = ss.str();

    // WHEN
    GenericOutcome genericOutcome = getFleetRoleCredentialsCallback.OnGetFleetRoleCredentials(jsonMessage);
    std::unique_ptr<WebSocketGetFleetRoleCredentialsResponse> response(static_cast<WebSocketGetFleetRoleCredentialsResponse *>(genericOutcome.GetResult()));

    // THEN
    EXPECT_EQ(response->GetAssumedRoleUserArn(), "AssumedRoleUserArn");
    EXPECT_EQ(response->GetAssumedRoleId(), "AssumedRoleId");
    EXPECT_EQ(response->GetAccessKeyId(), "AccessKeyId");
    EXPECT_EQ(response->GetSecretAccessKey(), "SecretAccessKey");
    EXPECT_EQ(response->GetSessionToken(), "SessionToken");
    EXPECT_EQ(response->GetExpiration(), 123);
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws