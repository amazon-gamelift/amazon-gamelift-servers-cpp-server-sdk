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
#include <aws/gamelift/server/model/StartMatchBackfillRequest.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

/**
 * Note: The StartMatchBackfillRequests in this class are pointers to place them on the heap not the
 * stack. Each StartMatchBackfillRequest is 1,015,056 bytes, and each Player is 25,360 bytes. The
 * default windows stack size is 1mb, so creating one StartMatchBackfillRequest and two Player
 * instances in a test causes a stack overflow. On MacOS and Linux the stack size is up to 8mb so
 * the issue is seen less often.
 */
class StartMatchBackfillRequestTest : public ::testing::Test {
protected:
    const std::string testTicketId = "ticket-123";
    const std::string testMatchmakingConfigurationArn = "testConfigArn";
    const std::string testGameSessionArn = "testGameSessionArn";
    const Player testPlayer = Player().WithPlayerId("player1");
    const std::vector<Player> testPlayers = {testPlayer};

    StartMatchBackfillRequest testRequest;

    void SetUp() override {
        testRequest.SetTicketId(testTicketId.c_str());
        testRequest.SetMatchmakingConfigurationArn(testMatchmakingConfigurationArn.c_str());
        testRequest.SetGameSessionArn(testGameSessionArn.c_str());
        testRequest.AddPlayer(testPlayer);
    }

    // Helper Methods
    void AssertStringFieldsEqualTestFields(const StartMatchBackfillRequest *request) {
        Utility::TestHelper::AssertStringsEqual(request->GetTicketId(), testTicketId);
        Utility::TestHelper::AssertStringsEqual(request->GetMatchmakingConfigurationArn(), testMatchmakingConfigurationArn);
        Utility::TestHelper::AssertStringsEqual(request->GetGameSessionArn(), testGameSessionArn);
    }

    void AssertRequestEqualsTestRequest(const StartMatchBackfillRequest *request) {
        AssertStringFieldsEqualTestFields(request);

#ifdef GAMELIFT_USE_STD
        ASSERT_EQ((request->GetPlayers())[0].GetPlayerId(), testPlayer.GetPlayerId());
#else
        int count = 0;
        ASSERT_STREQ((request->GetPlayers(count))[0].GetPlayerId(), testPlayer.GetPlayerId());
#endif
    }
};

TEST_F(StartMatchBackfillRequestTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // THEN
    Utility::TestHelper::AssertStringsEqual(request->GetTicketId(), "");
    Utility::TestHelper::AssertStringsEqual(request->GetMatchmakingConfigurationArn(), "");
    Utility::TestHelper::AssertStringsEqual(request->GetGameSessionArn(), "");
#ifdef GAMELIFT_USE_STD
    ASSERT_TRUE(request->GetPlayers().empty());
#else
    int count;
    request->GetPlayers(count);
    ASSERT_EQ(count, 0);
#endif
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_NE(request.get(), &testRequest);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest(std::move(testRequest)));
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_NE(request.get(), &testRequest);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    *request.get() = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_NE(request.get(), &testRequest);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    *request.get() = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_NE(request.get(), &testRequest);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_validCStrings_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    StartMatchBackfillRequest &requestRef = request->WithTicketId(testTicketId.c_str())
                                                .WithMatchmakingConfigurationArn(testMatchmakingConfigurationArn.c_str())
                                                .WithGameSessionArn(testGameSessionArn.c_str());
    // THEN
    AssertStringFieldsEqualTestFields(request.get());
    ASSERT_EQ(&requestRef, request.get());
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(StartMatchBackfillRequestTest, GIVEN_validInput_WHEN_setRequestValues_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    request->SetTicketId(testTicketId);
    request->SetMatchmakingConfigurationArn(testMatchmakingConfigurationArn);
    request->SetGameSessionArn(testGameSessionArn);
    request->SetPlayers(testPlayers);
    // THEN
    AssertRequestEqualsTestRequest(request.get());
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_rValueInputs_WHEN_setRequestValues_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    request->SetTicketId(std::string(testTicketId));
    request->SetMatchmakingConfigurationArn(std::string(testMatchmakingConfigurationArn));
    request->SetGameSessionArn(std::string(testGameSessionArn));
    request->SetPlayers(std::vector<Player>(testPlayers));
    // THEN
    AssertRequestEqualsTestRequest(request.get());
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_validInput_WHEN_withGameSessionValues_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    StartMatchBackfillRequest &requestRef = request->WithTicketId(testTicketId)
                                                .WithMatchmakingConfigurationArn(testMatchmakingConfigurationArn)
                                                .WithGameSessionArn(testGameSessionArn)
                                                .WithPlayer(testPlayers);
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_EQ(&requestRef, request.get());
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_rValueInputs_WHEN_withGameSessionValues_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    StartMatchBackfillRequest &requestRef = request->WithTicketId(std::string(testTicketId))
                                                .WithMatchmakingConfigurationArn(std::string(testMatchmakingConfigurationArn))
                                                .WithGameSessionArn(std::string(testGameSessionArn))
                                                .WithPlayer(std::vector<Player>(testPlayers));
    // THEN
    AssertRequestEqualsTestRequest(request.get());
    ASSERT_EQ(&requestRef, request.get());
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_players_WHEN_playerOperations_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    Player player1 = Player().WithPlayerId("player1");

    // WHEN
    request->AddPlayer(player1);
    request->AddPlayer(Player().WithPlayerId("player2"));
    // THEN
    const std::vector<Player> players = request->GetPlayers();
    ASSERT_EQ(players.size(), 2);
    ASSERT_EQ(players[0].GetPlayerId(), "player1");
    ASSERT_EQ(players[1].GetPlayerId(), "player2");
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(StartMatchBackfillRequestTest, GIVEN_tooLongStrings_WHEN_SetValues_no_std_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongTicketId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_TICKET_LENGTH);
    std::string tooLongConfigArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ARN_LENGTH);
    std::string tooLongSessionArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ARN_LENGTH);
    ASSERT_EQ(tooLongTicketId.size(), MAX_TICKET_LENGTH);
    ASSERT_EQ(tooLongConfigArn.size(), MAX_ARN_LENGTH);
    ASSERT_EQ(tooLongSessionArn.size(), MAX_ARN_LENGTH);
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());

    // WHEN
    request->SetTicketId(tooLongTicketId.c_str());
    request->SetMatchmakingConfigurationArn(tooLongConfigArn.c_str());
    request->SetGameSessionArn(tooLongSessionArn.c_str());
    // THEN
    ASSERT_STRNE(request->GetTicketId(), tooLongTicketId.c_str());
    ASSERT_EQ(strlen(request->GetTicketId()), MAX_TICKET_LENGTH - 1);
    ASSERT_STRNE(request->GetMatchmakingConfigurationArn(), tooLongConfigArn.c_str());
    ASSERT_EQ(strlen(request->GetMatchmakingConfigurationArn()), MAX_ARN_LENGTH - 1);
    ASSERT_STRNE(request->GetGameSessionArn(), tooLongSessionArn.c_str());
    ASSERT_EQ(strlen(request->GetGameSessionArn()), MAX_ARN_LENGTH - 1);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_tooManyPlayers_WHEN_AddPlayer_no_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    // WHEN
    for (int i = 0; i < MAX_PLAYERS + 1; i++) {
        request->AddPlayer(testPlayer);
    }
    // THEN
    int count;
    request->GetPlayers(count);
    ASSERT_EQ(count, MAX_PLAYERS);
}

TEST_F(StartMatchBackfillRequestTest, GIVEN_players_WHEN_playerOperations_no_std_THEN_success) {
    // GIVEN
    std::unique_ptr<StartMatchBackfillRequest> request(new StartMatchBackfillRequest());
    Player player1 = Player().WithPlayerId("player1");
    Player player2 = Player().WithPlayerId("player2");

    // WHEN
    request->WithPlayer(player1).AddPlayer(player2);
    // THEN
    int count;
    const Player *players = request->GetPlayers(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(players[0].GetPlayerId(), "player1");
    ASSERT_STREQ(players[1].GetPlayerId(), "player2");
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws