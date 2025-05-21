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
#include <aws/gamelift/internal/model/ResponseMessage.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class ResponseMessageTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";
    const int testStatusCode = 200;
    const std::string testErrorMessage = "testErrorMessage";

    ResponseMessage testResponseMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testResponseMessage.SetAction(testAction);
        testResponseMessage.SetRequestId(testRequestId);
        testResponseMessage.SetStatusCode(testStatusCode);
        testResponseMessage.SetErrorMessage(testErrorMessage);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"StatusCode\":" << testStatusCode << ",\"ErrorMessage\":\""
           << testErrorMessage << "\"}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const ResponseMessage &responseMessage) {
        ASSERT_EQ(responseMessage.GetAction(), testAction);
        ASSERT_EQ(responseMessage.GetRequestId(), testRequestId);
        ASSERT_EQ(responseMessage.GetStatusCode(), testStatusCode);
        ASSERT_EQ(responseMessage.GetErrorMessage(), testErrorMessage);
    }
};

TEST_F(ResponseMessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    ResponseMessage responseMessage;

    // THEN
    ASSERT_EQ(responseMessage.GetAction(), "");
    ASSERT_EQ(responseMessage.GetRequestId(), "");
    ASSERT_EQ(responseMessage.GetStatusCode(), -1);
    ASSERT_EQ(responseMessage.GetErrorMessage(), "");
}

TEST_F(ResponseMessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    ResponseMessage message(testResponseMessage);

    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ResponseMessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    ResponseMessage message(std::move(testResponseMessage));

    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ResponseMessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    ResponseMessage message;

    // WHEN
    message = testResponseMessage;

    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ResponseMessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    ResponseMessage message;

    // WHEN
    message = std::move(testResponseMessage);

    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(ResponseMessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testResponseMessage;

    // WHEN
    std::string serializedMessage = message->Serialize();

    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(ResponseMessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    ResponseMessage responseMessage;
    Message &message = responseMessage;

    // WHEN
    message.Deserialize(serializedTestMessage);

    // THEN
    AssertMessageEqualsTestMessage(responseMessage);
}

TEST_F(ResponseMessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    ResponseMessage responseMessage;
    Message &message = responseMessage;

    // WHEN
    std::string serializedMessage = message.Serialize();

    // THEN
    ASSERT_EQ(serializedMessage, "{}");
}

TEST_F(ResponseMessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    ResponseMessage responseMessage;
    Message &message = responseMessage;
    std::stringstream ss;
    ss << "{\"Action\": null, \"RequestId\": null, \"StatusCode\": null, \"ErrorMessage\": null}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);

    // THEN
    ASSERT_EQ(responseMessage.GetAction(), "");
    ASSERT_EQ(responseMessage.GetRequestId(), "");
    ASSERT_EQ(responseMessage.GetStatusCode(), -1);
    ASSERT_EQ(responseMessage.GetErrorMessage(), "");
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
