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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <aws/gamelift/internal/model/Message.h>
#include <chrono>
#include <future>
#include <regex>
#include <unordered_set>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class MessageTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";

    Message testMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetAction(testAction);
        testMessage.SetRequestId(testRequestId);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\"}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const Message &message) { ASSERT_EQ(message.GetAction(), testAction); }
};

MATCHER_P(MatchesRegex, pattern, "") {
    std::regex regex(pattern);
    return std::regex_match(arg, regex);
}

TEST_F(MessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    Message message;
    // THEN
    ASSERT_EQ(message.GetAction(), "");
}

TEST_F(MessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    Message message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(MessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    Message message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(MessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    Message message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(MessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    Message message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(MessageTest, GIVEN_validInputs_WHEN_withMessageValues_THEN_success) {
    // GIVEN
    Message message;
    // WHEN
    Message &messageRef = message.WithAction(testAction);
    // THEN
    AssertMessageEqualsTestMessage(message);
    ASSERT_EQ(&message, &messageRef);
}

TEST_F(MessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(MessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    Message message;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(MessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    Message message;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_THAT(serializedMessage, MatchesRegex("\\{\"RequestId\":\"[a-zA-Z0-9]{32}\"\\}"));
}

TEST_F(MessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    Message message;
    std::stringstream ss;
    ss << "{\"Action\": null}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(message.GetAction(), "");
}

TEST_F(MessageTest, GIVEN_multipleMessages_WHEN_constructor_THEN_haveDifferentRequestIds) {
    // GIVEN / WHEN
    Message message1;
    Message message2;
    Message message3;
    Message message4;
    Message message5;

    // THEN
    ASSERT_NE(message1.GetRequestId(), message2.GetRequestId());
    ASSERT_NE(message2.GetRequestId(), message3.GetRequestId());
    ASSERT_NE(message3.GetRequestId(), message4.GetRequestId());
    ASSERT_NE(message4.GetRequestId(), message5.GetRequestId());
}

TEST_F(MessageTest, GIVEN_multipleMessagesInThreads_WHEN_constructor_THEN_haveDifferentRequestIds) {
    std::vector<std::future<std::string>> futureRequestIds;

    // Create 100 messages in parallel, store their request IDs as futures
    for (int i = 0; i < 100; i++) {
        futureRequestIds.emplace_back(std::async(std::launch::async, []() {
            Message message;
            return message.GetRequestId();
        }));
    }

    // Wait for all futures to resolve
    for (auto &future : futureRequestIds) {
        if (future.wait_for(std::chrono::milliseconds(1000)) != std::future_status::ready) {
            FAIL() << "Could not resolve message request ID in 1 second";
        }
    }

    // Verify all future request IDs are different
    std::unordered_set<std::string> requestIds;
    for (auto &future : futureRequestIds) {
        std::string requestId = future.get();
        requestIds.insert(requestId);
    }
    // If this assertion fails, we somehow made two unit message objects with the same request ID,
    // which indicates a failure in our random number generator to be thread-safe and
    // well-distributed.
    ASSERT_EQ(futureRequestIds.size(), requestIds.size());
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws