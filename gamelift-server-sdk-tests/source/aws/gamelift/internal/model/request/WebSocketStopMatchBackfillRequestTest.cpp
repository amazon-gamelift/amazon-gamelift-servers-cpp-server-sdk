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
#include <aws/gamelift/internal/model/request/WebSocketStopMatchBackfillRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketStopMatchBackfillRequestTest : public ::testing::Test {
protected:
    const std::string testRequestId = "testRequestId";
    const std::string testTicketId = "testTicketId";
    const std::string testMatchmakingConfigArn = "testConfigArn";
    const std::string testGameSessionArn = "testGameSessionArn";

    WebSocketStopMatchBackfillRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetRequestId(testRequestId);
        testRequest.SetGameSessionArn(testGameSessionArn);
        testRequest.SetMatchmakingConfigurationArn(testMatchmakingConfigArn);
        testRequest.SetTicketId(testTicketId);

        std::stringstream ss;
        ss << "{\"Action\":\""
           << "StopMatchBackfill"
           << "\",\"RequestId\":\"" << testRequestId << "\",\"GameSessionArn\":\"" << testGameSessionArn << "\",\"MatchmakingConfigurationArn\":\""
           << testMatchmakingConfigArn << "\",\"TicketId\":\"" << testTicketId << "\"}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const WebSocketStopMatchBackfillRequest &request) {
        ASSERT_EQ(request.GetAction(), "StopMatchBackfill");
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetGameSessionArn(), testGameSessionArn);
        ASSERT_EQ(request.GetMatchmakingConfigurationArn(), testMatchmakingConfigArn);
        ASSERT_EQ(request.GetTicketId(), testTicketId);
    }
};

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStopMatchBackfillRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "StopMatchBackfill");
    ASSERT_NE(request.GetRequestId(), "");
    ASSERT_EQ(request.GetGameSessionArn(), "");
    ASSERT_EQ(request.GetMatchmakingConfigurationArn(), "");
    ASSERT_EQ(request.GetTicketId(), "");
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStopMatchBackfillRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStopMatchBackfillRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest request;
    // WHEN
    Message &requestRef = request.WithGameSessionArn(testGameSessionArn)
                              .WithMatchmakingConfigurationArn(testMatchmakingConfigArn)
                              .WithTicketId(testTicketId)
                              .WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &requestRef);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *request = &testRequest;
    // WHEN
    std::string serializedRequest = request->Serialize();
    // THEN
    ASSERT_EQ(serializedRequest, serializedTestRequest);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest stopMatchBackfillRequest;
    Message &request = stopMatchBackfillRequest;
    // WHEN
    request.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(stopMatchBackfillRequest);
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_emptyRequest_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest stopMatchBackfillRequest;
    Message &request = stopMatchBackfillRequest;
    // WHEN
    std::string serializedRequest = request.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"StopMatchBackfill\",\"RequestId\":\"" << request.GetRequestId() << "\"}";
    ASSERT_EQ(serializedRequest, ss.str());
}

TEST_F(WebSocketStopMatchBackfillRequestTest, GIVEN_requestWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStopMatchBackfillRequest stopMatchBackfillRequest;
    Message &request = stopMatchBackfillRequest;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"GameSessionArn\": null"
       << ",\"MatchmakingConfigurationArn\": null"
       << ",\"TicketId\": null }";
    std::string serializedTestRequest = ss.str();

    // WHEN
    request.Deserialize(serializedTestRequest);
    // THEN
    ASSERT_EQ(stopMatchBackfillRequest.GetAction(), "");
    ASSERT_EQ(stopMatchBackfillRequest.GetRequestId(), "");
    ASSERT_EQ(stopMatchBackfillRequest.GetGameSessionArn(), "");
    ASSERT_EQ(stopMatchBackfillRequest.GetMatchmakingConfigurationArn(), "");
    ASSERT_EQ(stopMatchBackfillRequest.GetTicketId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws