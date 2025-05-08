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
#include <aws/gamelift/internal/model/message/RefreshConnectionMessage.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class RefreshConnectionMessageTest : public ::testing::Test {
protected:
    const std::string testRequestId = "testRequestId";
    const std::string testRefreshConnectionEndpoint = "testRefreshConnectionEndpoint";
    const std::string testAuthToken = "testAuthToken";

    RefreshConnectionMessage testMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetRequestId(testRequestId);
        testMessage.SetRefreshConnectionEndpoint(testRefreshConnectionEndpoint);
        testMessage.SetAuthToken(testAuthToken);

        std::stringstream ss;
        ss << "{\"Action\":\""
           << "RefreshConnection"
           << "\",\"RequestId\":\"" << testRequestId << "\",\"RefreshConnectionEndpoint\":\"" << testRefreshConnectionEndpoint << "\",\"AuthToken\":\""
           << testAuthToken << "\"}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const RefreshConnectionMessage &message) {
        ASSERT_EQ(message.GetAction(), "RefreshConnection");
        ASSERT_EQ(message.GetRequestId(), testRequestId);
        ASSERT_EQ(message.GetRefreshConnectionEndpoint(), testRefreshConnectionEndpoint);
        ASSERT_EQ(message.GetAuthToken(), testAuthToken);
    }
};

TEST_F(RefreshConnectionMessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    RefreshConnectionMessage message;
    // THEN
    ASSERT_EQ(message.GetAction(), "RefreshConnection");
    ASSERT_NE(message.GetRequestId(), "");
    ASSERT_EQ(message.GetRefreshConnectionEndpoint(), "");
    ASSERT_EQ(message.GetAuthToken(), "");
}

TEST_F(RefreshConnectionMessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    RefreshConnectionMessage message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    RefreshConnectionMessage message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    RefreshConnectionMessage message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    RefreshConnectionMessage message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_validInputs_WHEN_withMessageValues_THEN_success) {
    // GIVEN
    RefreshConnectionMessage message;
    // WHEN
    Message &messageRef = message.WithRefreshConnectionEndpoint(testRefreshConnectionEndpoint).WithAuthToken(testAuthToken).WithRequestId(testRequestId);
    // THEN
    AssertMessageEqualsTestMessage(message);
    ASSERT_EQ(&message, &messageRef);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    RefreshConnectionMessage refreshConnectionMessage;
    Message &message = refreshConnectionMessage;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(refreshConnectionMessage);
}

TEST_F(RefreshConnectionMessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    RefreshConnectionMessage refreshConnectionMessage;
    Message &message = refreshConnectionMessage;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"RefreshConnection\",\"RequestId\":\"" << message.GetRequestId() << "\"}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(RefreshConnectionMessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    RefreshConnectionMessage refreshConnectionMessage;
    Message &message = refreshConnectionMessage;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"RefreshConnectionEndpoint\": null"
       << ",\"AuthToken\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(refreshConnectionMessage.GetAction(), "");
    ASSERT_EQ(refreshConnectionMessage.GetRequestId(), "");
    ASSERT_EQ(refreshConnectionMessage.GetRefreshConnectionEndpoint(), "");
    ASSERT_EQ(refreshConnectionMessage.GetAuthToken(), "");
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws