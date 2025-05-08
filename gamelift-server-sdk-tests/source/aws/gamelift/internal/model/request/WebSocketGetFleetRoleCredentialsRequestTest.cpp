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
#include <aws/gamelift/internal/model/request/WebSocketGetFleetRoleCredentialsRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class WebSocketGetFleetRoleCredentialsRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "GetFleetRoleCredentials";
    const std::string testRequestId = "testRequestId";

    const std::string testRoleArn = "roleArn";
    const std::string testRoleSessionName = "roleSessionName";

    WebSocketGetFleetRoleCredentialsRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);
        testRequest.SetRoleArn(testRoleArn);
        testRequest.SetRoleSessionName(testRoleSessionName);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"RoleArn\":\"" << testRoleArn << "\",\"RoleSessionName\":\""
           << testRoleSessionName << "\"}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const WebSocketGetFleetRoleCredentialsRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetRoleArn(), testRoleArn);
        ASSERT_EQ(request.GetRoleSessionName(), testRoleSessionName);
    }
};

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "GetFleetRoleCredentials");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetRoleArn(), "");
    ASSERT_EQ(request.GetRoleSessionName(), "");
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetFleetRoleCredentialsRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    // WHEN
    Message &messageRef = request.WithRoleArn(testRoleArn).WithRoleSessionName(testRoleSessionName).WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"GetFleetRoleCredentials\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(WebSocketGetFleetRoleCredentialsRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetFleetRoleCredentialsRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"RoleArn\": null"
       << ",\"RoleSessionName\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
    ASSERT_EQ(request.GetRoleArn(), "");
    ASSERT_EQ(request.GetRoleSessionName(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws