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
#include <aws/gamelift/server/model/Player.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class PlayerTest : public ::testing::Test {
protected:
    const std::string testPlayerId = "player1";
    const std::string testTeam = "team1";
    const std::string testPlayerAttributeKey = "skill";
    const AttributeValue testPlayerAttributeValue = AttributeValue(2.0);
    const std::string testLatencyInMsKey = "us-west-2";
    const int testLatencyInMsValue = 20;
    Player testPlayer;

    void SetUp() override {
        testPlayer.SetPlayerId(testPlayerId.c_str());
        testPlayer.SetTeam(testTeam.c_str());
#ifdef GAMELIFT_USE_STD
        testPlayer.AddPlayerAttribute(testPlayerAttributeKey, testPlayerAttributeValue);
        testPlayer.AddLatencyInMs(testLatencyInMsKey, testLatencyInMsValue);
#else
        testPlayer.AddPlayerAttribute(testPlayerAttributeKey.c_str(), testPlayerAttributeValue);
        testPlayer.AddLatencyMs(testLatencyInMsKey.c_str(), testLatencyInMsValue);
#endif
    }

    // Helper Methods
    void AssertPlayerEqualsTestPlayer(const Player &player) {
        Utility::TestHelper::AssertStringsEqual(player.GetPlayerId(), testPlayerId);
        Utility::TestHelper::AssertStringsEqual(player.GetTeam(), testTeam);
#ifdef GAMELIFT_USE_STD
        auto playerAttributes = player.GetPlayerAttributes();
        auto playerLatencyInMs = player.GetLatencyInMs();
        ASSERT_EQ(playerAttributes[testPlayerAttributeKey].GetN(), testPlayerAttributeValue.GetN());
        ASSERT_EQ(playerLatencyInMs[testLatencyInMsKey], testLatencyInMsValue);
#else
        int count;
        ASSERT_STREQ((player.GetPlayerAttributes(count))[0].GetName(), testPlayerAttributeKey.c_str());
        ASSERT_EQ((player.GetPlayerAttributes(count))[0].GetValue().GetN(), testPlayerAttributeValue.GetN());
        ASSERT_STREQ((player.GetLatencyMs(count))[0].GetRegion(), testLatencyInMsKey.c_str());
        ASSERT_EQ((player.GetLatencyMs(count))[0].GetLatencyMs(), testLatencyInMsValue);
#endif
    }
};

TEST_F(PlayerTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    Player player;
    // THEN
    Utility::TestHelper::AssertStringsEqual(player.GetPlayerId(), "");
    Utility::TestHelper::AssertStringsEqual(player.GetTeam(), "");
#ifdef GAMELIFT_USE_STD
    ASSERT_TRUE(player.GetPlayerAttributes().empty());
    ASSERT_TRUE(player.GetLatencyInMs().empty());
#else
    int count;
    player.GetPlayerAttributes(count);
    ASSERT_EQ(count, 0);
    player.GetLatencyMs(count);
    ASSERT_EQ(count, 0);
#endif
}

TEST_F(PlayerTest, GIVEN_player_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    Player player(testPlayer);
    // THEN
    AssertPlayerEqualsTestPlayer(player);
    ASSERT_NE(&player, &testPlayer);
}

TEST_F(PlayerTest, GIVEN_player_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    Player player(std::move(testPlayer));
    // THEN
    AssertPlayerEqualsTestPlayer(player);
    ASSERT_NE(&player, &testPlayer);
}

TEST_F(PlayerTest, GIVEN_player_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    Player player;
    // WHEN
    player = testPlayer;
    // THEN
    AssertPlayerEqualsTestPlayer(player);
    ASSERT_NE(&player, &testPlayer);
}

TEST_F(PlayerTest, GIVEN_player_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    Player player;
    // WHEN
    player = std::move(testPlayer);
    // THEN
    AssertPlayerEqualsTestPlayer(player);
    ASSERT_NE(&player, &testPlayer);
}

TEST_F(PlayerTest, GIVEN_validCStrings_WHEN_withPlayerValues_THEN_success) {
    // GIVEN
    Player player;
    // WHEN
    Player &playerRef = player.WithPlayerId(testPlayerId.c_str()).WithTeam(testTeam.c_str());
    // THEN
    Utility::TestHelper::AssertStringsEqual(player.GetPlayerId(), testPlayerId);
    Utility::TestHelper::AssertStringsEqual(player.GetTeam(), testTeam);
    ASSERT_EQ(&playerRef, &player);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(PlayerTest, GIVEN_validInput_WHEN_setPlayerValues_std_THEN_success) {
    // GIVEN
    Player player;
    // WHEN
    player.SetPlayerId(testPlayerId);
    player.SetTeam(testTeam);
    player.AddPlayerAttribute(testPlayerAttributeKey, testPlayerAttributeValue);
    player.AddLatencyInMs(testLatencyInMsKey, testLatencyInMsValue);
    // THEN
    AssertPlayerEqualsTestPlayer(player);
}

TEST_F(PlayerTest, GIVEN_validInput_WHEN_withPlayerValues_std_THEN_success) {
    // GIVEN
    Player player;
    std::map<std::string, AttributeValue> attributes{{testPlayerAttributeKey, testPlayerAttributeValue}};
    std::map<std::string, int> latency{{testLatencyInMsKey, testLatencyInMsValue}};
    // WHEN
    Player &playerRef = player.WithPlayerId(testPlayerId).WithTeam(testTeam).WithPlayerAttributes(attributes).WithLatencyInMs(latency);
    // THEN
    AssertPlayerEqualsTestPlayer(player);
    ASSERT_EQ(&playerRef, &player);
}

TEST_F(PlayerTest, GIVEN_validAttributes_WHEN_playerAttributeOperations_std_THEN_success) {
    // GIVEN
    Player player;
    std::string playerAttr1Name = "attrName1";
    AttributeValue playerAttr1Value = AttributeValue(1.0);
    // WHEN
    player.AddPlayerAttribute(playerAttr1Name, playerAttr1Value);
    player.AddPlayerAttribute("attrName2", AttributeValue(2.0));
    // THEN
    const std::map<std::string, AttributeValue> playerAttributes = player.GetPlayerAttributes();
    ASSERT_EQ(playerAttributes.size(), 2);
    ASSERT_EQ(playerAttributes.at(playerAttr1Name).GetN(), 1.0);
    ASSERT_EQ(playerAttributes.at("attrName2").GetN(), 2.0);
}

TEST_F(PlayerTest, GIVEN_validLatencies_WHEN_latencyOperations_std_THEN_success) {
    // GIVEN
    Player player;
    std::string region1 = "us-west-1";
    int latency1 = 10;
    // WHEN
    player.AddLatencyInMs(region1, latency1);
    player.AddLatencyInMs("us-west-2", 20);
    // THEN
    const std::map<std::string, int> latencies = player.GetLatencyInMs();
    ASSERT_EQ(latencies.size(), 2);
    ASSERT_EQ(latencies.at(region1), 10);
    ASSERT_EQ(latencies.at("us-west-2"), 20);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(PlayerTest, GIVEN_tooLongStrings_WHEN_SetValues_no_std_THEN_cutOffStrings) {
    // GIVEN
    std::string tooLongTeam = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_TEAM_STRING_LENGTH);
    std::string tooLongPlayerId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_ID_STRING_LENGTH);
    std::string tooLongRegion = Utility::TestHelper::GenerateRandomAlphaNumericString(REGION_LENGTH);
    std::string tooLongAttrString = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ATTR_STRING_LENGTH);
    ASSERT_EQ(tooLongTeam.size(), MAX_TEAM_STRING_LENGTH);
    ASSERT_EQ(tooLongPlayerId.size(), MAX_PLAYER_ID_STRING_LENGTH);
    ASSERT_EQ(tooLongRegion.size(), REGION_LENGTH);
    ASSERT_EQ(tooLongAttrString.size(), MAX_ATTR_STRING_LENGTH);
    int count;
    Player player;
    // WHEN
    player.SetTeam(tooLongTeam.c_str());
    player.SetPlayerId(tooLongPlayerId.c_str());
    player.AddLatencyMs(tooLongRegion.c_str(), 10);
    player.AddPlayerAttribute(tooLongAttrString.c_str(), AttributeValue(2.0));
    // THEN
    ASSERT_STRNE(player.GetTeam(), tooLongTeam.c_str());
    ASSERT_EQ(strlen(player.GetTeam()), MAX_TEAM_STRING_LENGTH - 1);
    ASSERT_STRNE(player.GetPlayerId(), tooLongPlayerId.c_str());
    ASSERT_EQ(strlen(player.GetPlayerId()), MAX_PLAYER_ID_STRING_LENGTH - 1);
    ASSERT_STRNE((player.GetLatencyMs(count))[0].GetRegion(), tooLongRegion.c_str());
    ASSERT_EQ(strlen((player.GetLatencyMs(count))[0].GetRegion()), REGION_LENGTH - 1);
    ASSERT_STRNE((player.GetPlayerAttributes(count))[0].GetName(), tooLongAttrString.c_str());
    ASSERT_EQ(strlen((player.GetPlayerAttributes(count))[0].GetName()), MAX_ATTR_STRING_LENGTH - 1);
}

TEST_F(PlayerTest, GIVEN_tooManyAttributes_WHEN_AddAttributes_no_std_THEN_success) {
    // GIVEN
    Player player;
    for (int i = 0; i < MAX_PLAYER_ATTRIBUTES_SIZE + 1; i++) {
        player.AddPlayerAttribute(testPlayerAttributeKey.c_str(), testPlayerAttributeValue);
    }
    // THEN
    int count;
    player.GetPlayerAttributes(count);
    ASSERT_EQ(count, MAX_PLAYER_ATTRIBUTES_SIZE);
}

TEST_F(PlayerTest, GIVEN_tooManyLatencies_WHEN_AddLatencies_no_std_THEN_success) {
    Player player;
    for (int i = 0; i < MAX_LATENCY_SIZE + 1; i++) {
        player.AddLatencyMs(testLatencyInMsKey.c_str(), testLatencyInMsValue);
    }
    // THEN
    int count;
    player.GetLatencyMs(count);
    ASSERT_EQ(count, MAX_LATENCY_SIZE);
}

TEST_F(PlayerTest, GIVEN_validAttributes_WHEN_playerAttributeOperations_no_std_THEN_success) {
    // GIVEN
    Player player;
    const char *playerAttr1Name = "attrName1";
    AttributeValue playerAttr1Value = AttributeValue(1.0);
    const char *playerAttr2Name = "attrName2";
    AttributeValue playerAttr2Value = AttributeValue(2.0);
    // WHEN
    player.WithPlayerAttribute(playerAttr1Name, playerAttr1Value).AddPlayerAttribute(playerAttr2Name, playerAttr2Value);
    // THEN
    int count;
    const Player::NamedAttribute *attributes = player.GetPlayerAttributes(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(attributes[0].GetName(), playerAttr1Name);
    ASSERT_EQ(attributes[0].GetValue().GetN(), playerAttr1Value.GetN());
    ASSERT_STREQ(attributes[1].GetName(), playerAttr2Name);
    ASSERT_EQ(attributes[1].GetValue().GetN(), playerAttr2Value.GetN());
}

TEST_F(PlayerTest, GIVEN_validLatencies_WHEN_latencyOperations_no_std_THEN_success) {
    // GIVEN
    Player player;
    const char *region1 = "us-west-1";
    int latency1 = 10;
    const char *region2 = "us-west-2";
    int latency2 = 20;
    // WHEN
    player.WithLatencyMs(region1, latency1).AddLatencyMs(region2, latency2);
    // THEN
    int count;
    const Player::RegionAndLatency *latencies = player.GetLatencyMs(count);
    ASSERT_EQ(count, 2);
    ASSERT_EQ(latencies[0].GetLatencyMs(), latency1);
    ASSERT_STREQ(latencies[0].GetRegion(), region1);
    ASSERT_EQ(latencies[1].GetLatencyMs(), latency2);
    ASSERT_STREQ(latencies[1].GetRegion(), region2);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws