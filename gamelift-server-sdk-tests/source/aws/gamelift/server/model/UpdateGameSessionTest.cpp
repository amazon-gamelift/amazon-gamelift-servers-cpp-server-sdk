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
#include <aws/gamelift/server/model/UpdateGameSession.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class UpdateGameSessionTest : public ::testing::Test {
protected:
    const std::string testBackfillTicketId = "testBackfillTicketId";
    const GameSession testGameSession = GameSession().WithGameSessionId("gameSession-123");
    const UpdateReason testUpdateReason = UpdateReason::MATCHMAKING_DATA_UPDATED;
    UpdateGameSession *testUpdateGameSession;

    void SetUp() override {
#ifdef GAMELIFT_USE_STD
        testUpdateGameSession = new UpdateGameSession(testGameSession, testUpdateReason, testBackfillTicketId);
#else
        testUpdateGameSession = new UpdateGameSession(testGameSession, testUpdateReason, testBackfillTicketId.c_str());
#endif
    }

    void TearDown() override { delete testUpdateGameSession; }

    // Helper Methods
    void AssertSessionEqualsTestSession(const UpdateGameSession &session) {
        Utility::TestHelper::AssertStringsEqual(session.GetBackfillTicketId(), testBackfillTicketId);
        Utility::TestHelper::AssertStringsEqual(session.GetGameSession().GetGameSessionId(), testGameSession.GetGameSessionId());
        ASSERT_EQ(session.GetUpdateReason(), testUpdateReason);
    }
};

TEST_F(UpdateGameSessionTest, GIVEN_args_WHEN_allArgConstructor_THEN_success) {
    // WHEN
#ifdef GAMELIFT_USE_STD
    UpdateGameSession updateGameSession(testGameSession, testUpdateReason, testBackfillTicketId);
#else
    UpdateGameSession updateGameSession(testGameSession, testUpdateReason, testBackfillTicketId.c_str());
#endif
    // THEN
    AssertSessionEqualsTestSession(updateGameSession);
}

TEST_F(UpdateGameSessionTest, GIVEN_updateGameSession_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSession updateGameSession(*testUpdateGameSession);
    // THEN
    AssertSessionEqualsTestSession(updateGameSession);
    ASSERT_NE(&updateGameSession, testUpdateGameSession);
}

TEST_F(UpdateGameSessionTest, GIVEN_updateGameSession_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSession updateGameSession(std::move(*testUpdateGameSession));
    // THEN
    AssertSessionEqualsTestSession(updateGameSession);
    ASSERT_NE(&updateGameSession, testUpdateGameSession);
}

TEST_F(UpdateGameSessionTest, GIVEN_updateGameSession_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSession updateGameSession(GameSession(), UpdateReason::UNKNOWN, "");
    updateGameSession = *testUpdateGameSession;
    // THEN
    AssertSessionEqualsTestSession(updateGameSession);
    ASSERT_NE(&updateGameSession, testUpdateGameSession);
}

TEST_F(UpdateGameSessionTest, GIVEN_updateGameSession_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN / WHEN
    UpdateGameSession updateGameSession(GameSession(), UpdateReason::UNKNOWN, "");
    updateGameSession = std::move(*testUpdateGameSession);
    // THEN
    AssertSessionEqualsTestSession(updateGameSession);
    ASSERT_NE(&updateGameSession, testUpdateGameSession);
}

#ifndef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(UpdateGameSessionTest, GIVEN_tooLongString_WHEN_constructUpdateGameSession_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongString = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_BACKFILL_TICKET_ID_LENGTH);
    ASSERT_EQ(tooLongString.size(), MAX_BACKFILL_TICKET_ID_LENGTH);
    // WHEN
    UpdateGameSession updateGameSession(testGameSession, testUpdateReason, tooLongString.c_str());
    // THEN
    ASSERT_STRNE(updateGameSession.GetBackfillTicketId(), tooLongString.c_str());
    ASSERT_EQ(strlen(updateGameSession.GetBackfillTicketId()), MAX_BACKFILL_TICKET_ID_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws