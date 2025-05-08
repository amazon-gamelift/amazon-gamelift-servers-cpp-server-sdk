/*
 * All or terminationTimeions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
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
#include <aws/gamelift/internal/model/message/TerminateProcessMessage.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class TerminateProcessMessageTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";

    const long testTerminationTime = 10000;

    TerminateProcessMessage testMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetAction(testAction);
        testMessage.SetRequestId(testRequestId);
        testMessage.SetTerminationTime(testTerminationTime);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"TerminationTime\":" << testTerminationTime << "}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const TerminateProcessMessage &message) {
        ASSERT_EQ(message.GetAction(), testAction);
        ASSERT_EQ(message.GetTerminationTime(), testTerminationTime);
    }
};

TEST_F(TerminateProcessMessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    TerminateProcessMessage message;
    // THEN
    ASSERT_EQ(message.GetAction(), "TerminateProcess");
    ASSERT_NE(message.GetRequestId(), "");
    ASSERT_EQ(message.GetTerminationTime(), -1);
}

TEST_F(TerminateProcessMessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    TerminateProcessMessage message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(TerminateProcessMessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    TerminateProcessMessage message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(TerminateProcessMessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    TerminateProcessMessage message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(TerminateProcessMessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    TerminateProcessMessage message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(TerminateProcessMessageTest, GIVEN_validInputs_WHEN_withMessageValues_THEN_success) {
    // GIVEN
    TerminateProcessMessage message;
    // WHEN
    Message &messageRef = message.WithTerminationTime(testTerminationTime).WithAction(testAction).WithRequestId(testRequestId);
    // THEN
    AssertMessageEqualsTestMessage(message);
    ASSERT_EQ(&message, &messageRef);
}

TEST_F(TerminateProcessMessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(TerminateProcessMessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    TerminateProcessMessage terminateProcessMessage;
    Message &message = terminateProcessMessage;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(terminateProcessMessage);
}

TEST_F(TerminateProcessMessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    TerminateProcessMessage terminateProcessMessage;
    Message &message = terminateProcessMessage;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"TerminateProcess\",\"RequestId\":\"" << message.GetRequestId() << "\"}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(TerminateProcessMessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    TerminateProcessMessage terminateProcessMessage;
    Message &message = terminateProcessMessage;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"TerminationTime\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(terminateProcessMessage.GetAction(), "");
    ASSERT_EQ(terminateProcessMessage.GetRequestId(), "");
    ASSERT_EQ(terminateProcessMessage.GetTerminationTime(), -1);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws