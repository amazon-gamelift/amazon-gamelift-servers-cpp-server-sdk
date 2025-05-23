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
#include <aws/gamelift/internal/model/WebSocketGameSession.h>
#include <aws/gamelift/internal/model/message/UpdateGameSessionMessage.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class UpdateGameSessionMessageTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";

    const std::string testUpdateReason = "testUpdateReason";
    const std::string testBackfillId = "testBackfillId";

    const std::string testGameSessionId = "testGameSessionId";
    const std::string testGameSessionName = "testGameSessionName";
    const std::string testGameSessionData = "testGameSessionData";
    const std::string testMatchmakerData = "testMatchmakerData";
    const std::string testIpAddress = "testIpAddress";
    const std::string testDnsName = "testDnsName";
    const std::string testGamePropertyKey = "testGamePropertyKey";
    const std::string testGamePropertyValue = "testGamePropertyValue";
    const std::map<std::string, std::string> testGameProperties = {{testGamePropertyKey, testGamePropertyValue}};
    const int testMaximumPlayerSessionCount = 10;
    const int testPort = 8080;

    UpdateGameSessionMessage testMessage;
    WebSocketGameSession gameSession;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetAction(testAction);
        testMessage.SetRequestId(testRequestId);

        testMessage.SetUpdateReason(testUpdateReason);
        testMessage.SetBackfillTicketId(testBackfillId);

        testMessage.SetGameSession(gameSession);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId

           << "\",\"GameSession\":{"
           << "\"GameProperties\":{}}"

           << ",\"UpdateReason\":\"" << testUpdateReason << "\",\"BackfillTicketId\":\"" << testBackfillId << "\"}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const UpdateGameSessionMessage &message) {
        ASSERT_EQ(message.GetAction(), testAction);
        ASSERT_EQ(message.GetRequestId(), testRequestId);
        ASSERT_EQ(message.GetUpdateReason(), testUpdateReason);
        ASSERT_EQ(message.GetBackfillTicketId(), testBackfillId);
        // ASSERT_EQ(message.GetGameSession(), gameSession);
    }
};

TEST_F(UpdateGameSessionMessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSessionMessage message;
    // THEN
    ASSERT_EQ(message.GetAction(), "UpdateGameSession");
    ASSERT_NE(message.GetRequestId(), "");
    ASSERT_EQ(message.GetUpdateReason(), "");
    ASSERT_EQ(message.GetBackfillTicketId(), "");
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSessionMessage message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSessionMessage message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    UpdateGameSessionMessage message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    UpdateGameSessionMessage message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    UpdateGameSessionMessage updateGameSessionMessage;
    Message &message = updateGameSessionMessage;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(updateGameSessionMessage);
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    UpdateGameSessionMessage updateGameSessionMessage;
    Message &message = updateGameSessionMessage;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"UpdateGameSession\",\"RequestId\":\"" << message.GetRequestId() << "\",\"GameSession\":{\"GameProperties\":{}}}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(UpdateGameSessionMessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    UpdateGameSessionMessage updateGameSessionMessage;
    Message &message = updateGameSessionMessage;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"GameSession\": null"
       << ",\"UpdateReason\": null"
       << ",\"BackfillTicketId\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(updateGameSessionMessage.GetAction(), "");
    ASSERT_EQ(updateGameSessionMessage.GetRequestId(), "");
    ASSERT_EQ(updateGameSessionMessage.GetUpdateReason(), "");
    ASSERT_EQ(updateGameSessionMessage.GetBackfillTicketId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws