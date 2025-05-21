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
#include <aws/gamelift/server/model/PlayerSession.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class PlayerSessionTest : public ::testing::Test {
protected:
    const std::string testPlayerSessionId = "psess-123";
    const std::string testPlayerId = "player1";
    const std::string testGameSessionId = "test-game-session";
    const std::string testFleetId = "fleet-123";
    const std::string testPlayerData = "test-player-data";
    const std::string testDnsName = "testDnsName";
    const std::string testIpAddress = "127.0.0.1";
    const long testCreationTime = 1000;
    const long testTerminationTime = 1001;
    const int testPort = 8000;
    const PlayerSessionStatus testStatus = PlayerSessionStatus::ACTIVE;

    PlayerSession testPlayerSession;

    void SetUp() override {
        testPlayerSession.SetPlayerSessionId(testPlayerSessionId.c_str());
        testPlayerSession.SetPlayerId(testPlayerId.c_str());
        testPlayerSession.SetGameSessionId(testGameSessionId.c_str());
        testPlayerSession.SetFleetId(testFleetId.c_str());
        testPlayerSession.SetPlayerData(testPlayerData.c_str());
        testPlayerSession.SetDnsName(testDnsName.c_str());
        testPlayerSession.SetIpAddress(testIpAddress.c_str());
        testPlayerSession.SetCreationTime(testCreationTime);
        testPlayerSession.SetTerminationTime(testTerminationTime);
        testPlayerSession.SetPort(testPort);
        testPlayerSession.SetStatus(testStatus);
    }

    // Helper Methods
    void AssertSessionEqualsTestSession(const PlayerSession &playerSession) {
        Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerSessionId(), testPlayerSessionId);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerId(), testPlayerId);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetGameSessionId(), testGameSessionId);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetFleetId(), testFleetId);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerData(), testPlayerData);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetDnsName(), testDnsName);
        Utility::TestHelper::AssertStringsEqual(playerSession.GetIpAddress(), testIpAddress);
        ASSERT_EQ(playerSession.GetCreationTime(), testCreationTime);
        ASSERT_EQ(playerSession.GetTerminationTime(), testTerminationTime);
        ASSERT_EQ(playerSession.GetPort(), testPort);
        ASSERT_EQ(playerSession.GetStatus(), testStatus);
    }
};

TEST_F(PlayerSessionTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    PlayerSession playerSession;
    // THEN
    Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerSessionId(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerId(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetGameSessionId(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetFleetId(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetPlayerData(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetDnsName(), "");
    Utility::TestHelper::AssertStringsEqual(playerSession.GetIpAddress(), "");
    ASSERT_EQ(playerSession.GetStatus(), PlayerSessionStatus::NOT_SET);
    ASSERT_EQ(playerSession.GetCreationTime(), 0);
    ASSERT_EQ(playerSession.GetTerminationTime(), 0);
    ASSERT_EQ(playerSession.GetPort(), 0);
}

TEST_F(PlayerSessionTest, GIVEN_playerSession_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    PlayerSession playerSession(testPlayerSession);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_NE(&playerSession, &testPlayerSession);
}

TEST_F(PlayerSessionTest, GIVEN_playerSession_WHEN1_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    PlayerSession playerSession(std::move(testPlayerSession));
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_NE(&playerSession, &testPlayerSession);
}

TEST_F(PlayerSessionTest, GIVEN_playerSession_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    playerSession = testPlayerSession;
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_NE(&playerSession, &testPlayerSession);
}

TEST_F(PlayerSessionTest, GIVEN_playerSession_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    playerSession = std::move(testPlayerSession);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_NE(&playerSession, &testPlayerSession);
}

TEST_F(PlayerSessionTest, GIVEN_validCStrings_WHEN_withSessionValues_THEN_success) {
    // GIVEN
    PlayerSession session;
    // WHEN
    PlayerSession &sessionRef = session.WithPlayerSessionId(testPlayerSessionId.c_str())
                                    .WithPlayerId(testPlayerId.c_str())
                                    .WithGameSessionId(testGameSessionId.c_str())
                                    .WithFleetId(testFleetId.c_str())
                                    .WithPlayerData(testPlayerData.c_str())
                                    .WithDnsName(testDnsName.c_str())
                                    .WithIpAddress(testIpAddress.c_str())
                                    .WithCreationTime(testCreationTime)
                                    .WithTerminationTime(testTerminationTime)
                                    .WithPort(testPort)
                                    .WithStatus(testStatus);
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_EQ(&sessionRef, &session);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(PlayerSessionTest, GIVEN_validInput_WHEN_setPlayerSessionValues_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    playerSession.SetPlayerSessionId(testPlayerSessionId);
    playerSession.SetPlayerId(testPlayerId);
    playerSession.SetGameSessionId(testGameSessionId);
    playerSession.SetFleetId(testFleetId);
    playerSession.SetPlayerData(testPlayerData);
    playerSession.SetDnsName(testDnsName);
    playerSession.SetIpAddress(testIpAddress);
    playerSession.SetCreationTime(testCreationTime);
    playerSession.SetTerminationTime(testTerminationTime);
    playerSession.SetPort(testPort);
    playerSession.SetStatus(testStatus);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
}

TEST_F(PlayerSessionTest, GIVEN_rValueInputs_WHEN_setPlayerSessionValues_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    playerSession.SetPlayerSessionId(std::string(testPlayerSessionId));
    playerSession.SetPlayerId(std::string(testPlayerId));
    playerSession.SetGameSessionId(std::string(testGameSessionId));
    playerSession.SetFleetId(std::string(testFleetId));
    playerSession.SetPlayerData(std::string(testPlayerData));
    playerSession.SetDnsName(std::string(testDnsName));
    playerSession.SetIpAddress(std::string(testIpAddress));
    playerSession.SetCreationTime(testCreationTime);
    playerSession.SetTerminationTime(testTerminationTime);
    playerSession.SetPort(testPort);
    playerSession.SetStatus(testStatus);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
}

TEST_F(PlayerSessionTest, GIVEN_validInput_WHEN_withPlayerSessionValues_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    PlayerSession &sessionRef = playerSession.WithPlayerSessionId(testPlayerSessionId)
                                    .WithPlayerId(testPlayerId)
                                    .WithGameSessionId(testGameSessionId)
                                    .WithFleetId(testFleetId)
                                    .WithPlayerData(testPlayerData)
                                    .WithDnsName(testDnsName)
                                    .WithIpAddress(testIpAddress)
                                    .WithCreationTime(testCreationTime)
                                    .WithTerminationTime(testTerminationTime)
                                    .WithPort(testPort)
                                    .WithStatus(testStatus);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_EQ(&sessionRef, &playerSession);
}

TEST_F(PlayerSessionTest, GIVEN_rValueInputs_WHEN_withPlayerSessionValues_THEN_success) {
    // GIVEN
    PlayerSession playerSession;
    // WHEN
    PlayerSession &sessionRef = playerSession.WithPlayerSessionId(std::string(testPlayerSessionId))
                                    .WithPlayerId(std::string(testPlayerId))
                                    .WithGameSessionId(std::string(testGameSessionId))
                                    .WithFleetId(std::string(testFleetId))
                                    .WithPlayerData(std::string(testPlayerData))
                                    .WithDnsName(std::string(testDnsName))
                                    .WithIpAddress(std::string(testIpAddress))
                                    .WithCreationTime(testCreationTime)
                                    .WithTerminationTime(testTerminationTime)
                                    .WithPort(testPort)
                                    .WithStatus(testStatus);
    // THEN
    AssertSessionEqualsTestSession(playerSession);
    ASSERT_EQ(&sessionRef, &playerSession);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(PlayerSessionTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongSessionId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_SESSION_ID_LENGTH);
    std::string tooLongPlayerId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_ID_LENGTH);
    std::string tooLongGameSessionId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_GAME_SESSION_ID_LENGTH);
    std::string tooLongFleetId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_FLEET_ID_LENGTH);
    std::string tooLongPlayerData = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_DATA_LENGTH);
    std::string tooLongDnsName = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_DNS_NAME_LENGTH);
    std::string tooLongIpAddress = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_IP_ADDRESS_LENGTH);
    ASSERT_EQ(tooLongSessionId.size(), MAX_PLAYER_SESSION_ID_LENGTH);
    ASSERT_EQ(tooLongPlayerId.size(), MAX_PLAYER_ID_LENGTH);
    ASSERT_EQ(tooLongGameSessionId.size(), MAX_GAME_SESSION_ID_LENGTH);
    ASSERT_EQ(tooLongFleetId.size(), MAX_FLEET_ID_LENGTH);
    ASSERT_EQ(tooLongPlayerData.size(), MAX_PLAYER_DATA_LENGTH);
    ASSERT_EQ(tooLongDnsName.size(), MAX_DNS_NAME_LENGTH);
    ASSERT_EQ(tooLongIpAddress.size(), MAX_IP_ADDRESS_LENGTH);
    PlayerSession session;
    // WHEN
    session.SetPlayerSessionId(tooLongSessionId.c_str());
    session.SetPlayerId(tooLongPlayerId.c_str());
    session.SetGameSessionId(tooLongGameSessionId.c_str());
    session.SetFleetId(tooLongFleetId.c_str());
    session.SetPlayerData(tooLongPlayerData.c_str());
    session.SetIpAddress(tooLongIpAddress.c_str());
    session.SetDnsName(tooLongDnsName.c_str());
    // THEN
    ASSERT_STRNE(session.GetPlayerSessionId(), tooLongSessionId.c_str());
    ASSERT_EQ(strlen(session.GetPlayerSessionId()), MAX_PLAYER_SESSION_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetPlayerId(), tooLongPlayerId.c_str());
    ASSERT_EQ(strlen(session.GetPlayerId()), MAX_PLAYER_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetGameSessionId(), tooLongGameSessionId.c_str());
    ASSERT_EQ(strlen(session.GetGameSessionId()), MAX_GAME_SESSION_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetFleetId(), tooLongFleetId.c_str());
    ASSERT_EQ(strlen(session.GetFleetId()), MAX_FLEET_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetPlayerData(), tooLongPlayerData.c_str());
    ASSERT_EQ(strlen(session.GetPlayerData()), MAX_PLAYER_DATA_LENGTH - 1);
    ASSERT_STRNE(session.GetDnsName(), tooLongDnsName.c_str());
    ASSERT_EQ(strlen(session.GetDnsName()), MAX_DNS_NAME_LENGTH - 1);
    ASSERT_STRNE(session.GetIpAddress(), tooLongIpAddress.c_str());
    ASSERT_EQ(strlen(session.GetIpAddress()), MAX_IP_ADDRESS_LENGTH - 1);
}
#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws