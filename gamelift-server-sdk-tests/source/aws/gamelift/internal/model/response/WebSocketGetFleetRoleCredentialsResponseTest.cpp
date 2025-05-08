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

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class WebSocketGetFleetRoleCredentialsResponseTest : public ::testing::Test {
protected:
    const std::string testAction = "GetFleetRoleCredentials";
    const std::string testRequestId = "testRequestId";

    const std::string testAssumedRoleUserArn = "assumedRoleUserArn";
    const std::string testAssumedRoleId = "assumedRoleId";
    const std::string testAccessKeyId = "accessKeyId";
    const std::string testSecretAccessKey = "secretAccessKey";
    const std::string testSessionToken = "sessionToken";
    const int64_t testExpiration = 12345;

    WebSocketGetFleetRoleCredentialsResponse testResponse;
    std::string serializedTestResponse;

    void SetUp() override {
        testResponse.SetAction(testAction);
        testResponse.SetRequestId(testRequestId);
        testResponse.SetAssumedRoleUserArn(testAssumedRoleUserArn);
        testResponse.SetAssumedRoleId(testAssumedRoleId);
        testResponse.SetAccessKeyId(testAccessKeyId);
        testResponse.SetSecretAccessKey(testSecretAccessKey);
        testResponse.SetSessionToken(testSessionToken);
        testResponse.SetExpiration(testExpiration);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"AssumedRoleUserArn\":\"" << testAssumedRoleUserArn
           << "\",\"AssumedRoleId\":\"" << testAssumedRoleId << "\",\"AccessKeyId\":\"" << testAccessKeyId << "\",\"SecretAccessKey\":\"" << testSecretAccessKey
           << "\",\"SessionToken\":\"" << testSessionToken << "\",\"Expiration\":" << testExpiration << "}";
        serializedTestResponse = ss.str();
    }

    void AssertResponseEqualsTestResponse(const WebSocketGetFleetRoleCredentialsResponse &response) {
        ASSERT_EQ(response.GetAction(), testAction);
        ASSERT_EQ(response.GetRequestId(), testRequestId);
        ASSERT_EQ(response.GetAssumedRoleUserArn(), testAssumedRoleUserArn);
        ASSERT_EQ(response.GetAssumedRoleId(), testAssumedRoleId);
        ASSERT_EQ(response.GetAccessKeyId(), testAccessKeyId);
        ASSERT_EQ(response.GetSecretAccessKey(), testSecretAccessKey);
        ASSERT_EQ(response.GetSessionToken(), testSessionToken);
        ASSERT_EQ(response.GetExpiration(), testExpiration);
    }
};

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsResponse response;
    // THEN
    ASSERT_EQ(response.GetAction(), "GetFleetRoleCredentials");
    ASSERT_FALSE(response.GetRequestId().empty());
    ASSERT_EQ(response.GetAssumedRoleUserArn(), "");
    ASSERT_EQ(response.GetAssumedRoleId(), "");
    ASSERT_EQ(response.GetAccessKeyId(), "");
    ASSERT_EQ(response.GetSecretAccessKey(), "");
    ASSERT_EQ(response.GetSessionToken(), "");
    ASSERT_EQ(response.GetExpiration(), -1);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_response_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsResponse response(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_response_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsResponse response(std::move(testResponse));
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_response_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    // WHEN
    response = testResponse;
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_response_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    // WHEN
    response = std::move(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    // WHEN
    Message &messageRef = response.WithAssumedRoleUserArn(testAssumedRoleUserArn)
                              .WithAssumedRoleId(testAssumedRoleId)
                              .WithAccessKeyId(testAccessKeyId)
                              .WithSecretAccessKey(testSecretAccessKey)
                              .WithSessionToken(testSessionToken)
                              .WithExpiration(testExpiration)
                              .WithRequestId(testRequestId);
    // THEN
    AssertResponseEqualsTestResponse(response);
    ASSERT_EQ(&response, &messageRef);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testResponse;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestResponse);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    Message &message = response;
    // WHEN
    message.Deserialize(serializedTestResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    Message &message = response;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"GetFleetRoleCredentials\",\"RequestId\":\"" << message.GetRequestId() << "\"}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(WebSocketGetFleetRoleCredentialsResponseTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsResponse response;
    Message &message = response;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"AssumedRoleUserArn\": null"
       << ",\"AssumedRoleId\": null"
       << ",\"AccessKeyId\": null"
       << ",\"SecretAccessKey\": null"
       << ",\"SessionToken\": null"
       << ",\"Expiration\": null"
       << "}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(response.GetAction(), "");
    ASSERT_EQ(response.GetRequestId(), "");
    ASSERT_EQ(response.GetAssumedRoleUserArn(), "");
    ASSERT_EQ(response.GetAccessKeyId(), "");
    ASSERT_EQ(response.GetSecretAccessKey(), "");
    ASSERT_EQ(response.GetSessionToken(), "");
    ASSERT_EQ(response.GetExpiration(), -1);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws