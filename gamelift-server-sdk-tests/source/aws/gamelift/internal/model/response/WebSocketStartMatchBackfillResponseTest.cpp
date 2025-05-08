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
#include <aws/gamelift/internal/model/response/WebSocketStartMatchBackfillResponse.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketStartMatchBackfillResponseTest : public ::testing::Test {
protected:
    const std::string testAction = "StartMatchBackfill";
    const std::string testRequestId = "testRequestId";

    const std::string testTicketId = "testTicketId";

    WebSocketStartMatchBackfillResponse testResponse;
    std::string serializedTestResponse;

    void SetUp() override {
        testResponse.SetAction(testAction);
        testResponse.SetRequestId(testRequestId);
        testResponse.SetTicketId(testTicketId);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"TicketId\":\"" << testTicketId << "\"}";
        serializedTestResponse = ss.str();
    }

    void AssertResponseEqualsTestResponse(const WebSocketStartMatchBackfillResponse &response) {
        ASSERT_EQ(response.GetAction(), testAction);
        ASSERT_EQ(response.GetRequestId(), testRequestId);
        ASSERT_EQ(response.GetTicketId(), testTicketId);
    }
};

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillResponse response;
    // THEN
    ASSERT_EQ(response.GetAction(), "StartMatchBackfill");
    ASSERT_FALSE(response.GetRequestId().empty());
    ASSERT_EQ(response.GetTicketId(), "");
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_response_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillResponse response(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_response_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketStartMatchBackfillResponse response(std::move(testResponse));
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_response_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    // WHEN
    response = testResponse;
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_response_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    // WHEN
    response = std::move(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    // WHEN
    Message &messageRef = response.WithTicketId(testTicketId).WithRequestId(testRequestId);
    // THEN
    AssertResponseEqualsTestResponse(response);
    ASSERT_EQ(&response, &messageRef);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testResponse;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestResponse);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    Message &message = response;
    // WHEN
    message.Deserialize(serializedTestResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    Message &message = response;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"StartMatchBackfill\",\"RequestId\":\"" << message.GetRequestId() << "\"}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(WebSocketStartMatchBackfillResponseTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketStartMatchBackfillResponse response;
    Message &message = response;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"TicketId\": null"
       << "}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(response.GetAction(), "");
    ASSERT_EQ(response.GetRequestId(), "");
    ASSERT_EQ(response.GetTicketId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws