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
#include <aws/gamelift/internal/model/request/RemovePlayerSessionRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class RemovePlayerSessionRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";

    const std::string testGameSessionId = "testGameSessionId";
    const std::string testPlayerSessionId = "testPlayerSessionId";

    RemovePlayerSessionRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);
        testRequest.SetGameSessionId(testGameSessionId);
        testRequest.SetPlayerSessionId(testPlayerSessionId);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"GameSessionId\":\"" << testGameSessionId
           << "\",\"PlayerSessionId\":\"" << testPlayerSessionId << "\"}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const RemovePlayerSessionRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetGameSessionId(), testGameSessionId);
        ASSERT_EQ(request.GetPlayerSessionId(), testPlayerSessionId);
    }
};

TEST_F(RemovePlayerSessionRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    RemovePlayerSessionRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "RemovePlayerSession");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetGameSessionId(), "");
    ASSERT_EQ(request.GetPlayerSessionId(), "");
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_args_WHEN_allArgConstructor_THEN_success) {
    // GIVEN / WHEN
    RemovePlayerSessionRequest request(testGameSessionId, testPlayerSessionId);
    // THEN
    ASSERT_EQ(request.GetAction(), "RemovePlayerSession");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetGameSessionId(), testGameSessionId);
    ASSERT_EQ(request.GetPlayerSessionId(), testPlayerSessionId);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    RemovePlayerSessionRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    RemovePlayerSessionRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    // WHEN
    Message &messageRef =
        request.WithGameSessionId(testGameSessionId).WithPlayerSessionId(testPlayerSessionId).WithRequestId(testRequestId).WithAction(testAction);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"RemovePlayerSession\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(RemovePlayerSessionRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    RemovePlayerSessionRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"GameSessionId\": null"
       << ",\"PlayerSessionId\": null}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
    ASSERT_EQ(request.GetGameSessionId(), "");
    ASSERT_EQ(request.GetPlayerSessionId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws