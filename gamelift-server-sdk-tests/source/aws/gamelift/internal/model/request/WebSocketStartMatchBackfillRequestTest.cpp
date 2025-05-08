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
#include <aws/gamelift/internal/model/request/WebSocketStartMatchBackfillRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketStartMatchBackfillRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "StartMatchBackfill";
    const std::string testRequestId = "testRequestId";

    const std::string testTicketId = "testTicketId";
    const std::string testGameSessionArn = "testGameSessionArn";
    const std::string testMmConfigArn = "testMmConfigArn";
    const std::vector<WebSocketPlayer> testPlayers;

    WebSocketStartMatchBackfillRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);
        testRequest.SetTicketId(testTicketId);
        testRequest.SetGameSessionArn(testGameSessionArn);
        testRequest.SetMatchmakingConfigurationArn(testMmConfigArn);
        testRequest.SetPlayers(testPlayers);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"TicketId\":\"" << testTicketId << "\",\"GameSessionArn\":\""
           << testGameSessionArn << "\",\"MatchmakingConfigurationArn\":\"" << testMmConfigArn << "\",\"Players\":"
           << "[]"
           << "}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const WebSocketStartMatchBackfillRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetTicketId(), testTicketId);
        ASSERT_EQ(request.GetGameSessionArn(), testGameSessionArn);
        ASSERT_EQ(request.GetMatchmakingConfigurationArn(), testMmConfigArn);
        ASSERT_EQ(request.GetPlayers().size(), 0);
    }
};

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "StartMatchBackfill");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetTicketId(), "");
    ASSERT_EQ(request.GetGameSessionArn(), "");
    ASSERT_EQ(request.GetMatchmakingConfigurationArn(), "");
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    // WHEN
    Message &messageRef = request.WithTicketId(testTicketId)
                              .WithGameSessionArn(testGameSessionArn)
                              .WithMatchmakingConfigurationArn(testMmConfigArn)
                              .WithPlayers(testPlayers)
                              .WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"StartMatchBackfill\",\"RequestId\":\"" + testRequestId + "\",\"Players\":[]}");
}

TEST_F(WebSocketStartMatchBackfillRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"TicketId\": null"
       << ",\"GameSessionArn\": null"
       << ",\"MatchmakingConfigurationArn\": null"
       << ",\"Players\": null"
       << "}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
    ASSERT_EQ(request.GetTicketId(), "");
    ASSERT_EQ(request.GetGameSessionArn(), "");
    ASSERT_EQ(request.GetMatchmakingConfigurationArn(), "");
    ASSERT_EQ(request.GetPlayers().size(), 0);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws