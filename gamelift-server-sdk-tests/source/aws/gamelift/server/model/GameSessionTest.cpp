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
#include <aws/gamelift/server/model/GameSession.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class GameSessionTest : public ::testing::Test {
protected:
    const std::string testGameSessionId = "testGameSession";
    const std::string testName = "testName";
    const std::string testFleetId = "testFleetId";
    const std::string testIpAddress = "127.0.0.1";
    const std::string testGameSessionData = "Test Game Session Data";
    const std::string testMatchmakerData = "Test Matchmaker Data";
    const std::string testDnsName = "Test DNS Name";
    const int testMaxPlayerSessionCount = 2;
    const int testPort = 8443;
    const GameSessionStatus testStatus = GameSessionStatus::ACTIVATING;
    const GameProperty testGameProperty = GameProperty().WithKey("testKey").WithValue("testValue");
    const std::vector<GameProperty> testGameProperties = {testGameProperty};

    GameSession testGameSession;

    void SetUp() override {
        testGameSession.SetGameSessionId(testGameSessionId.c_str());
        testGameSession.SetName(testName.c_str());
        testGameSession.SetFleetId(testFleetId.c_str());
        testGameSession.SetIpAddress(testIpAddress.c_str());
        testGameSession.SetGameSessionData(testGameSessionData.c_str());
        testGameSession.SetMatchmakerData(testMatchmakerData.c_str());
        testGameSession.SetDnsName(testDnsName.c_str());
        testGameSession.SetMaximumPlayerSessionCount(testMaxPlayerSessionCount);
        testGameSession.SetPort(testPort);
        testGameSession.SetStatus(testStatus);
        testGameSession.AddGameProperty(testGameProperty);
    }

    void TearDown() override {}

    // Helper Methods
    void AssertStringFieldsEqualTestFields(const GameSession &session) {
        Utility::TestHelper::AssertStringsEqual(session.GetGameSessionId(), testGameSessionId);
        Utility::TestHelper::AssertStringsEqual(session.GetName(), testName);
        Utility::TestHelper::AssertStringsEqual(session.GetFleetId(), testFleetId);
        Utility::TestHelper::AssertStringsEqual(session.GetIpAddress(), testIpAddress);
        Utility::TestHelper::AssertStringsEqual(session.GetGameSessionData(), testGameSessionData);
        Utility::TestHelper::AssertStringsEqual(session.GetMatchmakerData(), testMatchmakerData);
        Utility::TestHelper::AssertStringsEqual(session.GetDnsName(), testDnsName);
    }

    void AssertSessionEqualsTestSession(const GameSession &session) {
        AssertStringFieldsEqualTestFields(session);
        ASSERT_EQ(session.GetMaximumPlayerSessionCount(), testMaxPlayerSessionCount);
        ASSERT_EQ(session.GetPort(), testPort);
        ASSERT_EQ(session.GetStatus(), testStatus);
#ifdef GAMELIFT_USE_STD
        ASSERT_EQ((session.GetGameProperties())[0].GetKey(), testGameProperty.GetKey());
        ASSERT_EQ((session.GetGameProperties())[0].GetValue(), testGameProperty.GetValue());
#else
        int count;
        ASSERT_STREQ((session.GetGameProperties(count))[0].GetKey(), testGameProperty.GetKey());
        ASSERT_STREQ((session.GetGameProperties(count))[0].GetValue(), testGameProperty.GetValue());
#endif
    }
};

TEST_F(GameSessionTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    GameSession session;
    // THEN
    Utility::TestHelper::AssertStringsEqual(session.GetGameSessionId(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetName(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetFleetId(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetIpAddress(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetGameSessionData(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetMatchmakerData(), "");
    Utility::TestHelper::AssertStringsEqual(session.GetDnsName(), "");
    ASSERT_EQ(session.GetMaximumPlayerSessionCount(), 0);
    ASSERT_EQ(session.GetPort(), 0);
    ASSERT_EQ(session.GetStatus(), GameSessionStatus::NOT_SET);
#ifdef GAMELIFT_USE_STD
    ASSERT_TRUE(session.GetGameProperties().empty());
#else
    int count;
    session.GetGameProperties(count);
    ASSERT_EQ(count, 0);
#endif
}

TEST_F(GameSessionTest, GIVEN_session_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    GameSession session(testGameSession);
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_NE(&session, &testGameSession);
}

TEST_F(GameSessionTest, GIVEN_session_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    GameSession session(std::move(testGameSession));
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_NE(&session, &testGameSession);
}

TEST_F(GameSessionTest, GIVEN_session_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    session = testGameSession;
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_NE(&session, &testGameSession);
}

TEST_F(GameSessionTest, GIVEN_session_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    session = std::move(testGameSession);
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_NE(&session, &testGameSession);
}

TEST_F(GameSessionTest, GIVEN_validCStrings_WHEN_withGameSessionValues_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    GameSession &sessionRef = session.WithGameSessionId(testGameSessionId.c_str())
                                  .WithName(testName.c_str())
                                  .WithFleetId(testFleetId.c_str())
                                  .WithIpAddress(testIpAddress.c_str())
                                  .WithGameSessionData(testGameSessionData.c_str())
                                  .WithMatchmakerData(testMatchmakerData.c_str())
                                  .WithDnsName(testDnsName.c_str());
    // THEN
    AssertStringFieldsEqualTestFields(session);
    ASSERT_EQ(&sessionRef, &session);
}

// STD specific Tests
#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(GameSessionTest, GIVEN_validInput_WHEN_setGameSessionValues_std_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    session.SetGameSessionId(testGameSessionId);
    session.SetName(testName);
    session.SetFleetId(testFleetId);
    session.SetIpAddress(testIpAddress);
    session.SetGameSessionData(testGameSessionData);
    session.SetMatchmakerData(testMatchmakerData);
    session.SetDnsName(testDnsName);
    session.SetMaximumPlayerSessionCount(testMaxPlayerSessionCount);
    session.SetPort(testPort);
    session.SetStatus(testStatus);
    session.SetGameProperties(testGameProperties);
    // THEN
    AssertSessionEqualsTestSession(session);
}

TEST_F(GameSessionTest, GIVEN_rValueInputs_WHEN_setGameSessionValues_std_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    session.SetGameSessionId(std::string(testGameSessionId));
    session.SetName(std::string(testName));
    session.SetFleetId(std::string(testFleetId));
    session.SetIpAddress(std::string(testIpAddress));
    session.SetGameSessionData(std::string(testGameSessionData));
    session.SetMatchmakerData(std::string(testMatchmakerData));
    session.SetDnsName(std::string(testDnsName));
    session.SetMaximumPlayerSessionCount(testMaxPlayerSessionCount);
    session.SetPort(testPort);
    session.SetStatus(testStatus);
    session.SetGameProperties(std::vector<GameProperty>(testGameProperties));
    // THEN
    AssertSessionEqualsTestSession(session);
}

TEST_F(GameSessionTest, GIVEN_validInput_WHEN_withGameSessionValues_std_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    GameSession &sessionRef = session.WithGameSessionId(testGameSessionId)
                                  .WithName(testName)
                                  .WithFleetId(testFleetId)
                                  .WithIpAddress(testIpAddress)
                                  .WithGameSessionData(testGameSessionData)
                                  .WithMatchmakerData(testMatchmakerData)
                                  .WithDnsName(testDnsName)
                                  .WithMaximumPlayerSessionCount(testMaxPlayerSessionCount)
                                  .WithPort(testPort)
                                  .WithStatus(testStatus)
                                  .WithGameProperties(testGameProperties);
    // THEN
    AssertSessionEqualsTestSession(session);
    ASSERT_EQ(&sessionRef, &session);
}

TEST_F(GameSessionTest, GIVEN_rValueInputs_WHEN_withGameSessionValues_std_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    GameSession &sessionRef = session.WithGameSessionId(std::string(testGameSessionId))
                                  .WithName(std::string(testName))
                                  .WithFleetId(std::string(testFleetId))
                                  .WithIpAddress(std::string(testIpAddress))
                                  .WithGameSessionData(std::string(testGameSessionData))
                                  .WithMatchmakerData(std::string(testMatchmakerData))
                                  .WithDnsName(std::string(testDnsName))
                                  .WithMaximumPlayerSessionCount(testMaxPlayerSessionCount)
                                  .WithPort(testPort)
                                  .WithStatus(testStatus)
                                  .WithGameProperties(std::vector<GameProperty>(testGameProperties));
    // THEN
    AssertSessionEqualsTestSession(session);
}

TEST_F(GameSessionTest, GIVEN_gameProperties_WHEN_gamePropertyOperations_std_THEN_success) {
    // GIVEN
    GameSession session;
    GameProperty gameProperty1 = GameProperty().WithKey("testKey1");
    // WHEN
    session.AddGameProperty(gameProperty1);
    session.AddGameProperty(GameProperty().WithKey("testKey2"));
    // THEN
    const std::vector<GameProperty> gameProperties = session.GetGameProperties();
    ASSERT_EQ(gameProperties.size(), 2);
    ASSERT_EQ(gameProperties[0].GetKey(), "testKey1");
    ASSERT_EQ(gameProperties[1].GetKey(), "testKey2");
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(GameSessionTest, GIVEN_tooLongStrings_WHEN_SetValues_no_std_THEN_cutOffStrings) {
    // GIVEN
    std::string tooLongSessionId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_SESSION_ID_LENGTH);
    std::string tooLongSessionName = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_SESSION_NAME_LENGTH);
    std::string tooLongFleetId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_FLEET_ID_LENGTH);
    std::string tooLongIpAddress = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_IP_LENGTH);
    std::string tooLongGameSessionData = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_GAME_SESSION_DATA_LENGTH);
    std::string tooLongMatchmakerData = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_MATCHMAKER_DATA_LENGTH);
    std::string tooLongDnsName = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_DNS_NAME_LENGTH);
    ASSERT_EQ(tooLongSessionId.size(), MAX_SESSION_ID_LENGTH);
    ASSERT_EQ(tooLongSessionName.size(), MAX_SESSION_NAME_LENGTH);
    ASSERT_EQ(tooLongFleetId.size(), MAX_FLEET_ID_LENGTH);
    ASSERT_EQ(tooLongGameSessionData.size(), MAX_GAME_SESSION_DATA_LENGTH);
    ASSERT_EQ(tooLongMatchmakerData.size(), MAX_MATCHMAKER_DATA_LENGTH);
    ASSERT_EQ(tooLongDnsName.size(), MAX_DNS_NAME_LENGTH);
    GameSession session;
    // WHEN
    session.SetGameSessionId(tooLongSessionId.c_str());
    session.SetName(tooLongSessionName.c_str());
    session.SetFleetId(tooLongFleetId.c_str());
    session.SetIpAddress(tooLongIpAddress.c_str());
    session.SetGameSessionData(tooLongGameSessionData.c_str());
    session.SetMatchmakerData(tooLongMatchmakerData.c_str());
    session.SetDnsName(tooLongDnsName.c_str());
    // THEN
    ASSERT_STRNE(session.GetGameSessionId(), tooLongSessionId.c_str());
    ASSERT_EQ(strlen(session.GetGameSessionId()), MAX_SESSION_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetName(), tooLongSessionName.c_str());
    ASSERT_EQ(strlen(session.GetName()), MAX_SESSION_NAME_LENGTH - 1);
    ASSERT_STRNE(session.GetFleetId(), tooLongFleetId.c_str());
    ASSERT_EQ(strlen(session.GetFleetId()), MAX_FLEET_ID_LENGTH - 1);
    ASSERT_STRNE(session.GetIpAddress(), tooLongIpAddress.c_str());
    ASSERT_EQ(strlen(session.GetIpAddress()), MAX_IP_LENGTH - 1);
    ASSERT_STRNE(session.GetGameSessionData(), tooLongGameSessionData.c_str());
    ASSERT_EQ(strlen(session.GetGameSessionData()), MAX_GAME_SESSION_DATA_LENGTH - 1);
    ASSERT_STRNE(session.GetMatchmakerData(), tooLongMatchmakerData.c_str());
    ASSERT_EQ(strlen(session.GetMatchmakerData()), MAX_MATCHMAKER_DATA_LENGTH - 1);
    ASSERT_STRNE(session.GetDnsName(), tooLongDnsName.c_str());
    ASSERT_EQ(strlen(session.GetDnsName()), MAX_DNS_NAME_LENGTH - 1);
}

TEST_F(GameSessionTest, GIVEN_tooManyGameProperties_WHEN_AddGameProperty_no_std_THEN_success) {
    // GIVEN
    GameSession session;
    // WHEN
    for (int i = 0; i < MAX_GAME_PROPERTIES + 1; i++) {
        session.AddGameProperty(testGameProperty);
    }
    // THEN
    int count;
    session.GetGameProperties(count);
    ASSERT_EQ(count, MAX_GAME_PROPERTIES);
}

TEST_F(GameSessionTest, GIVEN_gameProperties_WHEN_gamePropertyOperations_no_std_THEN_success) {
    // GIVEN
    GameSession session;
    GameProperty gameProperty1 = GameProperty().WithKey("testKey1");
    GameProperty gameProperty2 = GameProperty().WithKey("testKey2");
    // WHEN
    session.WithGameProperty(gameProperty1).AddGameProperty(gameProperty2);
    // THEN
    int count;
    const GameProperty *gameProperties = session.GetGameProperties(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(gameProperties[0].GetKey(), "testKey1");
    ASSERT_STREQ(gameProperties[1].GetKey(), "testKey2");
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws