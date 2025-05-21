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
#include <aws/gamelift/internal/model/message/CreateGameSessionMessage.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class CreateGameSessionMessageTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";
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

    CreateGameSessionMessage testMessage;
    std::string serializedTestMessage;

    void SetUp() override {
        testMessage.SetAction(testAction);
        testMessage.SetRequestId(testRequestId);
        testMessage.SetGameSessionId(testGameSessionId);
        testMessage.SetGameSessionName(testGameSessionName);
        testMessage.SetGameSessionData(testGameSessionData);
        testMessage.SetMatchmakerData(testMatchmakerData);
        testMessage.SetIpAddress(testIpAddress);
        testMessage.SetDnsName(testDnsName);
        testMessage.SetMaximumPlayerSessionCount(testMaximumPlayerSessionCount);
        testMessage.SetPort(testPort);
        testMessage.SetGameProperties(testGameProperties);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"GameSessionId\":\"" << testGameSessionId
           << "\",\"GameSessionName\":\"" << testGameSessionName << "\",\"GameSessionData\":\"" << testGameSessionData << "\",\"MatchmakerData\":\""
           << testMatchmakerData << "\",\"DnsName\":\"" << testDnsName << "\",\"IpAddress\":\"" << testIpAddress
           << "\",\"MaximumPlayerSessionCount\":" << testMaximumPlayerSessionCount << ",\"Port\":" << testPort << ",\"GameProperties\":{\""
           << testGamePropertyKey << "\":\"" << testGamePropertyValue << "\"}}";
        serializedTestMessage = ss.str();
    }

    void AssertMessageEqualsTestMessage(const CreateGameSessionMessage &message) {
        ASSERT_EQ(message.GetAction(), testAction);
        ASSERT_EQ(message.GetRequestId(), testRequestId);
        ASSERT_EQ(message.GetGameSessionId(), testGameSessionId);
        ASSERT_EQ(message.GetGameSessionName(), testGameSessionName);
        ASSERT_EQ(message.GetGameSessionData(), testGameSessionData);
        ASSERT_EQ(message.GetMatchmakerData(), testMatchmakerData);
        ASSERT_EQ(message.GetIpAddress(), testIpAddress);
        ASSERT_EQ(message.GetDnsName(), testDnsName);
        ASSERT_EQ(message.GetMaximumPlayerSessionCount(), testMaximumPlayerSessionCount);
        ASSERT_EQ(message.GetPort(), testPort);
        ASSERT_EQ(message.GetGameProperties(), testGameProperties);
    }
};

TEST_F(CreateGameSessionMessageTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    CreateGameSessionMessage message;
    // THEN
    ASSERT_EQ(message.GetAction(), "CreateGameSession");
    ASSERT_NE(message.GetRequestId(), "");
    ASSERT_EQ(message.GetGameSessionId(), "");
    ASSERT_EQ(message.GetGameSessionName(), "");
    ASSERT_EQ(message.GetGameSessionData(), "");
    ASSERT_EQ(message.GetMatchmakerData(), "");
    ASSERT_EQ(message.GetIpAddress(), "");
    ASSERT_EQ(message.GetDnsName(), "");
    ASSERT_EQ(message.GetMaximumPlayerSessionCount(), -1);
    ASSERT_EQ(message.GetPort(), -1);
    ASSERT_EQ(message.GetGameProperties().size(), 0);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    CreateGameSessionMessage message(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    CreateGameSessionMessage message(std::move(testMessage));
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    CreateGameSessionMessage message;
    // WHEN
    message = testMessage;
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    CreateGameSessionMessage message;
    // WHEN
    message = std::move(testMessage);
    // THEN
    AssertMessageEqualsTestMessage(message);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_validInputs_WHEN_withMessageValues_THEN_success) {
    // GIVEN
    CreateGameSessionMessage message;
    // WHEN
    Message &messageRef = message.WithGameSessionId(testGameSessionId)
                              .WithGameSessionName(testGameSessionName)
                              .WithGameSessionData(testGameSessionData)
                              .WithMatchmakerData(testMatchmakerData)
                              .WithIpAddress(testIpAddress)
                              .WithDnsName(testDnsName)
                              .WithMaximumPlayerSessionCount(testMaximumPlayerSessionCount)
                              .WithPort(testPort)
                              .WithGameProperties(testGameProperties)
                              .WithRequestId(testRequestId)
                              .WithAction(testAction);
    // THEN
    AssertMessageEqualsTestMessage(message);
    ASSERT_EQ(&message, &messageRef);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testMessage;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestMessage);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    CreateGameSessionMessage createGameSessionMessage;
    Message &message = createGameSessionMessage;
    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    AssertMessageEqualsTestMessage(createGameSessionMessage);
}

TEST_F(CreateGameSessionMessageTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    CreateGameSessionMessage createGameSessionMessage;
    Message &message = createGameSessionMessage;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"CreateGameSession\",\"RequestId\":\"" << message.GetRequestId() << "\",\"GameProperties\":{}}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(CreateGameSessionMessageTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    CreateGameSessionMessage createGameSessionMessage;
    Message &message = createGameSessionMessage;

    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"GameSessionId\": null"
       << ",\"GameSessionName\": null"
       << ",\"GameSessionData\": null"
       << ",\"MatchmakerData\": null"
       << ",\"DnsName\": null"
       << ",\"IpAddress\": null"
       << ",\"MaximumPlayerSessionCount\": null"
       << ",\"Port\": null"
       << ",\"GameProperties\": null}";
    std::string nullStringMessage = ss.str();

    // WHEN
    message.Deserialize(nullStringMessage);
    // THEN
    ASSERT_EQ(createGameSessionMessage.GetAction(), "");
    ASSERT_EQ(createGameSessionMessage.GetRequestId(), "");
    ASSERT_EQ(createGameSessionMessage.GetGameSessionId(), "");
    ASSERT_EQ(createGameSessionMessage.GetGameSessionName(), "");
    ASSERT_EQ(createGameSessionMessage.GetGameSessionData(), "");
    ASSERT_EQ(createGameSessionMessage.GetMatchmakerData(), "");
    ASSERT_EQ(createGameSessionMessage.GetIpAddress(), "");
    ASSERT_EQ(createGameSessionMessage.GetDnsName(), "");
    ASSERT_EQ(createGameSessionMessage.GetMaximumPlayerSessionCount(), -1);
    ASSERT_EQ(createGameSessionMessage.GetPort(), -1);
    ASSERT_EQ(createGameSessionMessage.GetGameProperties().size(), 0);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws