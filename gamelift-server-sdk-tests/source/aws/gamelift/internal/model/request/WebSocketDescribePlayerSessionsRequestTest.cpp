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
#include <aws/gamelift/internal/model/request/WebSocketDescribePlayerSessionsRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketDescribePlayerSessionsRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "DescribePlayerSessions";
    const std::string testRequestId = "testRequestId";

    const std::string testGameSessionId = "testGameSessionId";
    const std::string testPlayerId = "testPlayerId";
    const std::string testPlayerSessionId = "testPlayerSessionId";
    const std::string testPlayerSessionStatusFilter = "testPlayerSessionStatusFilter";
    const std::string testNextToken = "testNextToken";
    const int testLimit = 10;

    WebSocketDescribePlayerSessionsRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);
        testRequest.SetGameSessionId(testGameSessionId);
        testRequest.SetPlayerId(testPlayerId);
        testRequest.SetPlayerSessionId(testPlayerSessionId);
        testRequest.SetPlayerSessionStatusFilter(testPlayerSessionStatusFilter);
        testRequest.SetNextToken(testNextToken);
        testRequest.SetLimit(testLimit);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"GameSessionId\":\"" << testGameSessionId << "\",\"PlayerId\":\""
           << testPlayerId << "\",\"PlayerSessionId\":\"" << testPlayerSessionId << "\",\"PlayerSessionStatusFilter\":\"" << testPlayerSessionStatusFilter
           << "\",\"NextToken\":\"" << testNextToken << "\",\"Limit\":" << testLimit << "}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const WebSocketDescribePlayerSessionsRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetGameSessionId(), testGameSessionId);
        ASSERT_EQ(request.GetPlayerId(), testPlayerId);
        ASSERT_EQ(request.GetPlayerSessionId(), testPlayerSessionId);
        ASSERT_EQ(request.GetPlayerSessionStatusFilter(), testPlayerSessionStatusFilter);
        ASSERT_EQ(request.GetNextToken(), testNextToken);
        ASSERT_EQ(request.GetLimit(), testLimit);
    }
};

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "DescribePlayerSessions");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetGameSessionId(), "");
    ASSERT_EQ(request.GetPlayerId(), "");
    ASSERT_EQ(request.GetPlayerSessionId(), "");
    ASSERT_EQ(request.GetPlayerSessionStatusFilter(), "");
    ASSERT_EQ(request.GetNextToken(), "");
    ASSERT_EQ(request.GetLimit(), -1);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    // WHEN
    Message &messageRef = request.WithGameSessionId(testGameSessionId)
                              .WithPlayerId(testPlayerId)
                              .WithPlayerSessionId(testPlayerSessionId)
                              .WithPlayerSessionStatusFilter(testPlayerSessionStatusFilter)
                              .WithNextToken(testNextToken)
                              .WithLimit(testLimit)
                              .WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"DescribePlayerSessions\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(WebSocketDescribePlayerSessionsRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"PlayerId\": null"
       << ",\"PlayerSessionId\": null"
       << ",\"PlayerSessionStatusFilter\": null"
       << ",\"NextToken\": null"
       << ",\"Limit\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
    ASSERT_EQ(request.GetGameSessionId(), "");
    ASSERT_EQ(request.GetPlayerId(), "");
    ASSERT_EQ(request.GetPlayerSessionId(), "");
    ASSERT_EQ(request.GetPlayerSessionStatusFilter(), "");
    ASSERT_EQ(request.GetNextToken(), "");
    ASSERT_EQ(request.GetLimit(), -1);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws