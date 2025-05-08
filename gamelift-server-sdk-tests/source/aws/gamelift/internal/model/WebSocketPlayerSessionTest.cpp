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
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class WebSocketPlayerSessionTest : public ::testing::Test {
protected:
    const std::string testPlayerSessionId = "testPlayerSessionId";
    const std::string testPlayerId = "testPlayerId";
    const std::string testGameSessionId = "testGameSessionId";
    const std::string testFleetId = "testFleetId";
    const long testCreationTime = 100;
    const long testTerminationTime = 1000;
    const WebSocketPlayerSessionStatus testStatus = WebSocketPlayerSessionStatus::COMPLETED;
    const std::string testIpAddress = "testIpAddress";
    const int testPort = 25565;
    const std::string testPlayerData = "testPlayerData";
    const std::string testDnsName = "testDnsName";

    WebSocketPlayerSession testPlayerSession;
    std::string serializedTestPlayerSession;

    void SetUp() override {
        testPlayerSession.SetPlayerSessionId(testPlayerSessionId);
        testPlayerSession.SetPlayerId(testPlayerId);
        testPlayerSession.SetGameSessionId(testGameSessionId);
        testPlayerSession.SetFleetId(testFleetId);
        testPlayerSession.SetCreationTime(testCreationTime);
        testPlayerSession.SetTerminationTime(testTerminationTime);
        testPlayerSession.SetStatus(testStatus);
        testPlayerSession.SetIpAddress(testIpAddress);
        testPlayerSession.SetPort(testPort);
        testPlayerSession.SetPlayerData(testPlayerData);
        testPlayerSession.SetDnsName(testDnsName);

        std::stringstream ss;
        ss << "{\"PlayerSessionId\":\"" << testPlayerSessionId << "\",\"PlayerId\":\"" << testPlayerId << "\",\"GameSessionId\":\"" << testGameSessionId
           << "\",\"FleetId\":\"" << testFleetId << "\",\"CreationTime\":" << testCreationTime << ",\"TerminationTime\":" << testTerminationTime
           << ",\"Status\":"
           << "\"COMPLETED\""
           << ",\"IpAddress\":\"" << testIpAddress << "\",\"Port\":" << testPort << ",\"PlayerData\":\"" << testPlayerData << "\",\"DnsName\":\"" << testDnsName
           << "\"}";
        serializedTestPlayerSession = ss.str();
    }

    void AssertPlayerSessionEqualsTestPlayerSession(const WebSocketPlayerSession &playerSession) {
        ASSERT_EQ(playerSession.GetPlayerSessionId(), testPlayerSessionId);
        ASSERT_EQ(playerSession.GetPlayerId(), testPlayerId);
        ASSERT_EQ(playerSession.GetGameSessionId(), testGameSessionId);
        ASSERT_EQ(playerSession.GetFleetId(), testFleetId);
        ASSERT_EQ(playerSession.GetCreationTime(), testCreationTime);
        ASSERT_EQ(playerSession.GetTerminationTime(), testTerminationTime);
        ASSERT_EQ(playerSession.GetStatus(), testStatus);
        ASSERT_EQ(playerSession.GetIpAddress(), testIpAddress);
        ASSERT_EQ(playerSession.GetPort(), testPort);
        ASSERT_EQ(playerSession.GetPlayerData(), testPlayerData);
        ASSERT_EQ(playerSession.GetDnsName(), testDnsName);
    }
};

TEST_F(WebSocketPlayerSessionTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayerSession playerSession;
    // THEN
    ASSERT_EQ(playerSession.GetPlayerSessionId(), "");
    ASSERT_EQ(playerSession.GetPlayerId(), "");
    ASSERT_EQ(playerSession.GetGameSessionId(), "");
    ASSERT_EQ(playerSession.GetFleetId(), "");
    ASSERT_EQ(playerSession.GetCreationTime(), -1);
    ASSERT_EQ(playerSession.GetTerminationTime(), -1);
    ASSERT_EQ(playerSession.GetStatus(), WebSocketPlayerSessionStatus::NOT_SET);
    ASSERT_EQ(playerSession.GetIpAddress(), "");
    ASSERT_EQ(playerSession.GetPort(), -1);
    ASSERT_EQ(playerSession.GetPlayerData(), "");
    ASSERT_EQ(playerSession.GetDnsName(), "");
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_playerSession_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayerSession playerSession(testPlayerSession);
    // THEN
    AssertPlayerSessionEqualsTestPlayerSession(playerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_playerSession_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayerSession playerSession(std::move(testPlayerSession));
    // THEN
    AssertPlayerSessionEqualsTestPlayerSession(playerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_playerSession_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketPlayerSession playerSession;
    // WHEN
    playerSession = testPlayerSession;
    // THEN
    AssertPlayerSessionEqualsTestPlayerSession(playerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_playerSession_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketPlayerSession playerSession;
    // WHEN
    playerSession = std::move(testPlayerSession);
    // THEN
    AssertPlayerSessionEqualsTestPlayerSession(playerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    // WHEN
    testPlayerSession.Serialize(&writer);
    writer.EndObject();
    // THEN
    ASSERT_EQ(buffer.GetString(), serializedTestPlayerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketPlayerSession playerSession;
    rapidjson::Document doc;
    doc.Parse(serializedTestPlayerSession.c_str());
    // WHEN
    playerSession.Deserialize(doc);
    // THEN
    AssertPlayerSessionEqualsTestPlayerSession(playerSession);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_emptyObject_WHEN_serialize_THEN_success) {
    // GIVEN
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    WebSocketPlayerSession playerSession;
    // WHEN
    writer.StartObject();
    playerSession.Serialize(&writer);
    writer.EndObject();
    // THEN
    printf("Buffer String is: %s", buffer.GetString());
    std::string expectedString = "{\"Status\":\"NOT_SET\"}";
    ASSERT_EQ(buffer.GetString(), expectedString);
}

TEST_F(WebSocketPlayerSessionTest, GIVEN_objectWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketPlayerSession playerSession;
    std::stringstream ss;
    ss << "{\"PlayerSessionId\": null"
       << ",\"PlayerId\": null"
       << ",\"GameSessionId\": null"
       << ",\"FleetId\": null"
       << ",\"CreationTime\": null"
       << ",\"TerminationTime\": null"
       << ",\"Status\": null"
       << ",\"IpAddress\": null"
       << ",\"Port\": null"
       << ",\"PlayerData\": null"
       << ",\"DnsName\": null"
       << "}";
    std::string serializedTestSession = ss.str();
    rapidjson::Document doc;
    doc.Parse(serializedTestSession.c_str());

    // WHEN
    playerSession.Deserialize(doc);

    // THEN
    ASSERT_EQ(playerSession.GetPlayerSessionId(), "");
    ASSERT_EQ(playerSession.GetPlayerId(), "");
    ASSERT_EQ(playerSession.GetGameSessionId(), "");
    ASSERT_EQ(playerSession.GetFleetId(), "");
    ASSERT_EQ(playerSession.GetCreationTime(), -1);
    ASSERT_EQ(playerSession.GetTerminationTime(), -1);
    ASSERT_EQ(playerSession.GetStatus(), WebSocketPlayerSessionStatus::NOT_SET);
    ASSERT_EQ(playerSession.GetIpAddress(), "");
    ASSERT_EQ(playerSession.GetPort(), -1);
    ASSERT_EQ(playerSession.GetPlayerData(), "");
    ASSERT_EQ(playerSession.GetDnsName(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws