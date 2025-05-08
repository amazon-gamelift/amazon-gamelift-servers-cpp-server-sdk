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

#include "aws/gamelift/utility/TestHelper.h"
#include "gtest/gtest.h"
#include <aws/gamelift/server/model/DescribePlayerSessionsResult.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class DescribePlayerSessionsResultTest : public ::testing::Test {
protected:
    const std::string testNextToken = "testNextToken";
    const PlayerSession testPlayerSession = PlayerSession().WithPlayerId("player1");
    const std::vector<PlayerSession> testPlayerSessions = {testPlayerSession};

    DescribePlayerSessionsResult testResult;

    void SetUp() override {
        testResult.SetNextToken(testNextToken.c_str());
#ifdef GAMELIFT_USE_STD
        testResult.AddPlayerSession(testPlayerSession);
#else
        testResult.AddPlayerSession(testPlayerSession);
#endif
    }

    // Helper Methods
#ifdef GAMELIFT_USE_STD
    void AssertResultEqualsTestResult(const DescribePlayerSessionsResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetNextToken(), testNextToken);
        ASSERT_EQ((result.GetPlayerSessions())[0].GetPlayerId(), testPlayerSession.GetPlayerId());
    }

    void AssertResultIsEmpty(const DescribePlayerSessionsResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetNextToken(), "");
        ASSERT_TRUE(result.GetPlayerSessions().empty());
    }
#else
    void AssertResultEqualsTestResult(const DescribePlayerSessionsResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetNextToken(), testNextToken);
        int count;
        ASSERT_STREQ((result.GetPlayerSessions(count))[0].GetPlayerId(), testPlayerSession.GetPlayerId());
    }

    void AssertResultIsEmpty(const DescribePlayerSessionsResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetNextToken(), "");
        int count;
        result.GetPlayerSessions(count);
        ASSERT_EQ(count, 0);
    }
#endif
};

TEST_F(DescribePlayerSessionsResultTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    DescribePlayerSessionsResult result;
    // THEN
    AssertResultIsEmpty(result);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_result_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    DescribePlayerSessionsResult result(testResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_result_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    DescribePlayerSessionsResult result(std::move(testResult));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_result_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    result = testResult;
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_result_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    result = std::move(testResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_validCStrings_WHEN_withResultValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    DescribePlayerSessionsResult &resultRef = result.WithNextToken(testNextToken.c_str());
    // THEN
    Utility::TestHelper::AssertStringsEqual(result.GetNextToken(), testNextToken);
    ASSERT_EQ(&resultRef, &result);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(DescribePlayerSessionsResultTest, GIVEN_validInput_WHEN_setResultValues_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    result.SetNextToken(testNextToken);
    result.SetPlayerSessions(testPlayerSessions);
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_rValueInputs_WHEN_setResultValues_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    result.SetNextToken(std::string(testNextToken));
    result.SetPlayerSessions(std::vector<PlayerSession>(testPlayerSessions));
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_validInput_WHEN_withResultValues_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    DescribePlayerSessionsResult &resultRef = result.WithNextToken(testNextToken).WithPlayerSessions(testPlayerSessions);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_rValueInputs_WHEN_WithResultValues_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    result.SetNextToken(std::string(testNextToken));
    result.SetPlayerSessions(std::vector<PlayerSession>(testPlayerSessions));
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_playerSessions_WHEN_playerSessionOperations_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    PlayerSession player1 = PlayerSession().WithPlayerId("player1");
    // WHEN
    result.AddPlayerSession(player1);
    result.AddPlayerSession(PlayerSession().WithPlayerId("player2"));
    // THEN
    const std::vector<PlayerSession> playerSessions = result.GetPlayerSessions();
    ASSERT_EQ(playerSessions.size(), 2);
    ASSERT_EQ(playerSessions[0].GetPlayerId(), "player1");
    ASSERT_EQ(playerSessions[1].GetPlayerId(), "player2");
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(DescribePlayerSessionsResultTest, GIVEN_tooLongStrings_WHEN_SetValues_no_std_THEN_cutOffStrings) {
    // GIVEN
    std::string tooLongNextToken = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_NEXT_TOKEN_LENGTH);
    ASSERT_EQ(tooLongNextToken.size(), MAX_NEXT_TOKEN_LENGTH);
    DescribePlayerSessionsResult result;
    // WHEN
    result.SetNextToken(tooLongNextToken.c_str());
    // THEN
    ASSERT_STRNE(result.GetNextToken(), tooLongNextToken.c_str());
    ASSERT_EQ(strlen(result.GetNextToken()), MAX_NEXT_TOKEN_LENGTH - 1);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_tooManyPlayerSessions_WHEN_AddPlayerSession_no_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    // WHEN
    for (int i = 0; i < MAX_PLAYER_SESSIONS + 1; i++) {
        result.AddPlayerSession(testPlayerSession);
    }
    // THEN
    int count;
    result.GetPlayerSessions(count);
    ASSERT_EQ(count, MAX_PLAYER_SESSIONS);
}

TEST_F(DescribePlayerSessionsResultTest, GIVEN_playerSessions_WHEN_playerSessionOperations_no_std_THEN_success) {
    // GIVEN
    DescribePlayerSessionsResult result;
    PlayerSession player1 = PlayerSession().WithPlayerId("player1");
    PlayerSession player2 = PlayerSession().WithPlayerId("player2");
    // WHEN
    result = result.WithPlayerSessions(player1);
    result.AddPlayerSession(player2);
    // THEN
    int count;
    const PlayerSession *playerSessions = result.GetPlayerSessions(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(playerSessions[0].GetPlayerId(), "player1");
    ASSERT_STREQ(playerSessions[1].GetPlayerId(), "player2");
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws