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
#include <aws/gamelift/internal/model/WebSocketPlayerSession.h>
#include <aws/gamelift/internal/model/response/WebSocketDescribePlayerSessionsResponse.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketDescribePlayerSessionsResponseTest : public ::testing::Test {
protected:
    const std::string testAction = "DescribePlayerSessions";
    const std::string testRequestId = "testRequestId";

    const std::string testNextToken = "testNextToken";
    const std::vector<WebSocketPlayerSession> testPlayerSessions;

    WebSocketDescribePlayerSessionsResponse testResponse;
    std::string serializedTestResponse;

    void SetUp() override {
        testResponse.SetAction(testAction);
        testResponse.SetRequestId(testRequestId);
        testResponse.SetPlayerSessions(testPlayerSessions);
        testResponse.SetNextToken(testNextToken);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"NextToken\":\"" << testNextToken << "\",\"PlayerSessions\":[]"
           << "}";
        serializedTestResponse = ss.str();
    }

    void AssertResponseEqualsTestResponse(const WebSocketDescribePlayerSessionsResponse &response) {
        ASSERT_EQ(response.GetAction(), testAction);
        ASSERT_EQ(response.GetRequestId(), testRequestId);
        ASSERT_EQ(response.GetPlayerSessions().size(), testPlayerSessions.size());
        ASSERT_EQ(response.GetNextToken(), testNextToken);
    }
};

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsResponse response;
    // THEN
    ASSERT_EQ(response.GetAction(), "DescribePlayerSessions");
    ASSERT_FALSE(response.GetRequestId().empty());
    ASSERT_TRUE(response.GetPlayerSessions().empty());
    ASSERT_EQ(response.GetNextToken(), "");
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_response_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsResponse response(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_response_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketDescribePlayerSessionsResponse response(std::move(testResponse));
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_response_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    // WHEN
    response = testResponse;
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_response_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    // WHEN
    response = std::move(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    // WHEN
    Message &messageRef = response.WithPlayerSessions(std::vector<WebSocketPlayerSession>()).WithNextToken(testNextToken).WithRequestId(testRequestId);
    // THEN
    AssertResponseEqualsTestResponse(response);
    ASSERT_EQ(&response, &messageRef);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testResponse;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestResponse);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    Message &message = response;
    // WHEN
    message.Deserialize(serializedTestResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    Message &message = response;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"DescribePlayerSessions\",\"RequestId\":\"" << response.GetRequestId() << "\",\"PlayerSessions\":[]}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(WebSocketDescribePlayerSessionsResponseTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketDescribePlayerSessionsResponse response;
    Message &message = response;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"NextToken\": null"
       << ",\"PlayerSessions\": null"
       << "}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(response.GetAction(), "");
    ASSERT_EQ(response.GetRequestId(), "");
    ASSERT_EQ(response.GetNextToken(), "");
    ASSERT_EQ(response.GetPlayerSessions().size(), 0);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws