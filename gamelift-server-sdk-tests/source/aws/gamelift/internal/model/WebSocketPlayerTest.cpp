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
#include <aws/gamelift/internal/model/WebSocketPlayer.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketPlayerTest : public ::testing::Test {
protected:
    static constexpr const char *PLAYER_ID = "PlayerId";
    static constexpr const char *PLAYER_ATTRIBUTES = "PlayerAttributes";
    static constexpr const char *LATENCY_IN_MS = "LatencyInMs";
    static constexpr const char *TEAM = "Team";

    const std::string testPlayerId = "testPlayerId";
    const std::string testPlayerAttributes = "testPlayerAttributes";
    const std::string testLatencyInMs = "testLatencyInMs";
    const std::string testTeam = "testTeam";
    WebSocketAttributeValue testAttributeValue;

    const std::map<std::string, int> testLatencyMap = {{testLatencyInMs, 0}};
    const std::map<std::string, WebSocketAttributeValue> testPlayerAttributeMap = {{testPlayerAttributes, testAttributeValue}};

    WebSocketPlayer testWebSocketPlayer;

    std::string serializedString;

    void SetUp() override {

        testWebSocketPlayer.SetPlayerId(testPlayerId);
        testWebSocketPlayer.SetTeam(testTeam);
        testWebSocketPlayer.SetLatencyInMs(testLatencyMap);
        testWebSocketPlayer.SetPlayerAttributes(testPlayerAttributeMap);

        std::stringstream stringStream;
        stringStream << "{\"PlayerId\":\"" << testPlayerId << "\",\"PlayerAttributes\":{\"" << testPlayerAttributes << "\":{\"AttrType\":\"NONE\"}"
                     << "},\"LatencyInMs\":{\"" << testLatencyInMs << "\":0"
                     << "},\"Team\":\"" << testTeam << "\"}";
        serializedString = stringStream.str();
    }

    void AssertExpectedWebSocketPlayer(const WebSocketPlayer &webSocketPlayer) {
        ASSERT_EQ(webSocketPlayer.GetPlayerId(), testPlayerId);
        ASSERT_EQ(webSocketPlayer.GetTeam(), testTeam);
        ASSERT_TRUE(webSocketPlayer.GetLatencyInMs().at(testLatencyInMs) == 0);
        ASSERT_EQ(webSocketPlayer.GetPlayerAttributes().at(testPlayerAttributes).GetAttributeTypeAsString(), "NONE");
    }
};

TEST_F(WebSocketPlayerTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayer WebSocketPlayer;
    // THEN
    ASSERT_EQ(WebSocketPlayer.GetPlayerId(), "");
    ASSERT_EQ(WebSocketPlayer.GetTeam(), "");
}

TEST_F(WebSocketPlayerTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayer webSocketPlayer(testWebSocketPlayer);

    // THEN
    AssertExpectedWebSocketPlayer(webSocketPlayer);
}

TEST_F(WebSocketPlayerTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketPlayer webSocketPlayer(std::move(testWebSocketPlayer));
    // THEN
    AssertExpectedWebSocketPlayer(webSocketPlayer);
}

TEST_F(WebSocketPlayerTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketPlayer webSocketPlayer;
    // WHEN
    webSocketPlayer = testWebSocketPlayer;
    // THEN
    AssertExpectedWebSocketPlayer(webSocketPlayer);
}

TEST_F(WebSocketPlayerTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketPlayer webSocketPlayer;
    // WHEN
    webSocketPlayer = std::move(testWebSocketPlayer);
    // THEN
    AssertExpectedWebSocketPlayer(webSocketPlayer);
}

TEST_F(WebSocketPlayerTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketPlayer *message = &testWebSocketPlayer;
    // WHEN
    std::string serializedMessage = message->Serialize();

    // THEN
    ASSERT_EQ(serializedMessage, serializedString);
}

TEST_F(WebSocketPlayerTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketPlayer WebSocketPlayer;
    // WHEN
    WebSocketPlayer.Deserialize(serializedString);
    // THEN
    AssertExpectedWebSocketPlayer(WebSocketPlayer);
}

TEST_F(WebSocketPlayerTest, GIVEN_emptyObject_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketPlayer player;
    // WHEN
    std::string serializedPlayer = player.Serialize();
    // THEN
    ASSERT_EQ(serializedPlayer, "{\"PlayerAttributes\":{},\"LatencyInMs\":{}}");
}

TEST_F(WebSocketPlayerTest, GIVEN_objectWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketPlayer player;
    std::stringstream ss;
    ss << "{\"PlayerId\": null"
       << "\",\"PlayerAttributes\": null"
       << ",\"LatencyInMs\": null"
       << ",\"Team\": null"
       << "}";
    std::string serializedTestPlayer = ss.str();

    // WHEN
    player.Deserialize(serializedTestPlayer);
    // THEN
    ASSERT_EQ(player.GetPlayerId(), "");
    ASSERT_EQ(player.GetTeam(), "");
    ASSERT_EQ(player.GetLatencyInMs().size(), 0);
    ASSERT_EQ(player.GetPlayerAttributes().size(), 0);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws