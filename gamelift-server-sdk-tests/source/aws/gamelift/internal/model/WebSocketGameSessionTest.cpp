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
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketGameSessionTest : public ::testing::Test {
protected:
    const std::string testGameSessionId = "testGameSessionId";
    const std::string testName = "testName";
    const std::string testFleetId = "testFleetId";
    const std::string testGameSessionData = "testGameSessionData";
    const std::string testMatchmakerData = "testMatchmakerData";
    const std::string testIpAddress = "testIpAddress";
    const std::string testDnsName = "testDnsName";
    const std::string testGamePropertyKey = "testGamePropertyKey";
    const std::string testGamePropertyValue = "testGamePropertyValue";
    const std::map<std::string, std::string> testGameProperties = {{testGamePropertyKey, testGamePropertyValue}};
    const int testMaximumPlayerSessionCount = 10;
    const int testPort = 8080;

    WebSocketGameSession testGameSession;
    std::string serializedTestSession;

    void SetUp() override {
        testGameSession.SetGameSessionId(testGameSessionId);
        testGameSession.SetName(testName);
        testGameSession.SetFleetId(testFleetId);
        testGameSession.SetGameSessionData(testGameSessionData);
        testGameSession.SetMatchmakerData(testMatchmakerData);
        testGameSession.SetIpAddress(testIpAddress);
        testGameSession.SetDnsName(testDnsName);
        testGameSession.SetMaximumPlayerSessionCount(testMaximumPlayerSessionCount);
        testGameSession.SetPort(testPort);
        testGameSession.SetGameProperties(testGameProperties);

        std::stringstream ss;
        ss << "{\"GameSessionId\":\"" << testGameSessionId << "\",\"Name\":\"" << testName << "\",\"FleetId\":\"" << testFleetId << "\",\"GameSessionData\":\""
           << testGameSessionData << "\",\"MatchmakerData\":\"" << testMatchmakerData << "\",\"DnsName\":\"" << testDnsName << "\",\"IpAddress\":\""
           << testIpAddress << "\",\"MaximumPlayerSessionCount\":" << testMaximumPlayerSessionCount << ",\"Port\":" << testPort << ",\"GameProperties\":{\""
           << testGamePropertyKey << "\":\"" << testGamePropertyValue << "\"}}";
        serializedTestSession = ss.str();
    }

    void AssertMessageEqualsTestMessage(const WebSocketGameSession &gameSession) {
        ASSERT_EQ(gameSession.GetFleetId(), testFleetId);
        ASSERT_EQ(gameSession.GetGameSessionId(), testGameSessionId);
        ASSERT_EQ(gameSession.GetName(), testName);
        ASSERT_EQ(gameSession.GetGameSessionData(), testGameSessionData);
        ASSERT_EQ(gameSession.GetMatchmakerData(), testMatchmakerData);
        ASSERT_EQ(gameSession.GetIpAddress(), testIpAddress);
        ASSERT_EQ(gameSession.GetDnsName(), testDnsName);
        ASSERT_EQ(gameSession.GetMaximumPlayerSessionCount(), testMaximumPlayerSessionCount);
        ASSERT_EQ(gameSession.GetPort(), testPort);
        ASSERT_EQ(gameSession.GetGameProperties(), testGameProperties);
    }
};

TEST_F(WebSocketGameSessionTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGameSession gameSession;
    // THEN
    ASSERT_EQ(gameSession.GetFleetId(), "");
    ASSERT_EQ(gameSession.GetGameSessionId(), "");
    ASSERT_EQ(gameSession.GetName(), "");
    ASSERT_EQ(gameSession.GetGameSessionData(), "");
    ASSERT_EQ(gameSession.GetMatchmakerData(), "");
    ASSERT_EQ(gameSession.GetIpAddress(), "");
    ASSERT_EQ(gameSession.GetDnsName(), "");
    ASSERT_EQ(gameSession.GetMaximumPlayerSessionCount(), -1);
    ASSERT_EQ(gameSession.GetPort(), -1);
    ASSERT_EQ(gameSession.GetGameProperties().size(), 0);
}

TEST_F(WebSocketGameSessionTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGameSession session(testGameSession);
    // THEN
    AssertMessageEqualsTestMessage(session);
}

TEST_F(WebSocketGameSessionTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGameSession session(std::move(testGameSession));
    // THEN
    AssertMessageEqualsTestMessage(session);
}

TEST_F(WebSocketGameSessionTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGameSession session;
    // WHEN
    session = testGameSession;
    // THEN
    AssertMessageEqualsTestMessage(session);
}

TEST_F(WebSocketGameSessionTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGameSession session;
    // WHEN
    session = std::move(testGameSession);
    // THEN
    AssertMessageEqualsTestMessage(session);
}

TEST_F(WebSocketGameSessionTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketGameSession *message = &testGameSession;
    // WHEN
    std::string serializedSession = message->Serialize();
    // THEN
    ASSERT_EQ(serializedSession, serializedTestSession);
}

TEST_F(WebSocketGameSessionTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGameSession gameSession;
    // WHEN
    gameSession.Deserialize(serializedTestSession);
    // THEN
    AssertMessageEqualsTestMessage(gameSession);
}

TEST_F(WebSocketGameSessionTest, GIVEN_emptyObject_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketGameSession gameSession;
    // WHEN
    std::string serializedSession = gameSession.Serialize();
    // THEN
    ASSERT_EQ(serializedSession, "{\"GameProperties\":{}}");
}

TEST_F(WebSocketGameSessionTest, GIVEN_objectWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGameSession gameSession;
    std::stringstream ss;
    ss << "{\"GameSessionId\": null"
       << "\",\"Name\": null"
       << "\",\"FleetId\": null"
       << "\",\"GameSessionData\": null"
       << "\",\"MatchmakerData\": null"
       << "\",\"DnsName\": null"
       << "\",\"IpAddress\": null"
       << "\",\"MaximumPlayerSessionCount\": null"
       << ",\"Port\": null"
       << ",\"GameProperties\": null";
    std::string serializedTestSession = ss.str();

    // WHEN
    gameSession.Deserialize(serializedTestSession);
    // THEN
    ASSERT_EQ(gameSession.GetFleetId(), "");
    ASSERT_EQ(gameSession.GetGameSessionId(), "");
    ASSERT_EQ(gameSession.GetName(), "");
    ASSERT_EQ(gameSession.GetGameSessionData(), "");
    ASSERT_EQ(gameSession.GetMatchmakerData(), "");
    ASSERT_EQ(gameSession.GetIpAddress(), "");
    ASSERT_EQ(gameSession.GetDnsName(), "");
    ASSERT_EQ(gameSession.GetMaximumPlayerSessionCount(), -1);
    ASSERT_EQ(gameSession.GetPort(), -1);
    ASSERT_EQ(gameSession.GetGameProperties().size(), 0);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws