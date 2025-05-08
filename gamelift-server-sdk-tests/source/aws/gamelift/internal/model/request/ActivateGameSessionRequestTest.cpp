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
#include <aws/gamelift/internal/model/request/ActivateGameSessionRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class ActivateGameSessionRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";

    const std::string testRequestId = "testRequestId";

    const std::string testGameSessionId = "testGameSessionId";

    ActivateGameSessionRequest testMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetAction(testAction);
        testMessage.SetRequestId(testRequestId);
        testMessage.SetGameSessionId(testGameSessionId);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"GameSessionId\":\"" << testGameSessionId << "\"}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const ActivateGameSessionRequest &message) {
        ASSERT_EQ(message.GetAction(), testAction);
        ASSERT_EQ(message.GetGameSessionId(), testGameSessionId);
    }
};

TEST_F(ActivateGameSessionRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateGameSessionRequest message;
    // THEN
    ASSERT_EQ(message.GetAction(), "ActivateGameSession");
    ASSERT_EQ(message.GetGameSessionId(), "");
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateGameSessionRequest message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateGameSessionRequest message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_validInputs_WHEN_withMessageValues_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest message;
    // WHEN
    Message &messageRef = message.WithGameSessionId(testGameSessionId).WithAction(testAction);
    // THEN
    AssertMessageEqualsTestMessage(message);
    ASSERT_EQ(&message, &messageRef);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest activateGameSessionRequest;
    Message &message = activateGameSessionRequest;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(activateGameSessionRequest);
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest activateGameSessionRequest;
    activateGameSessionRequest.SetRequestId(testRequestId);
    Message &message = activateGameSessionRequest;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"ActivateGameSession\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(ActivateGameSessionRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    ActivateGameSessionRequest activateGameSessionRequest;
    Message &message = activateGameSessionRequest;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"GameSessionId\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(activateGameSessionRequest.GetAction(), "");
    ASSERT_EQ(activateGameSessionRequest.GetRequestId(), "");
    ASSERT_EQ(activateGameSessionRequest.GetGameSessionId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws