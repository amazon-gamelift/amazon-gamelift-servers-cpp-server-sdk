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
#include <aws/gamelift/server/model/DescribePlayerSessionsRequest.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class DescribePlayerSessionsRequestTest : public ::testing::Test {
protected:
    const std::string testGameSessionId = "testGameSessionId";
    const std::string testPlayerId = "testPlayerId";
    const std::string testPlayerSessionId = "testPlayerSessionId";
    const std::string testPlayerSessionStatusFilter = "testPlayerSessionStatusFilter";
    const std::string testNextToken = "testNextToken";
    const int testLimit = 10;

    DescribePlayerSessionsRequest testRequest;

    void SetUp() override {
        testRequest.SetGameSessionId(testGameSessionId.c_str());
        testRequest.SetPlayerId(testPlayerId.c_str());
        testRequest.SetPlayerSessionId(testPlayerSessionId.c_str());
        testRequest.SetPlayerSessionStatusFilter(testPlayerSessionStatusFilter.c_str());
        testRequest.SetNextToken(testNextToken.c_str());
        testRequest.SetLimit(testLimit);
    }

    // Helper Methods
    void AssertRequestEqualsTestRequest(const DescribePlayerSessionsRequest &request) {
        Utility::TestHelper::AssertStringsEqual(request.GetGameSessionId(), testGameSessionId);
        Utility::TestHelper::AssertStringsEqual(request.GetPlayerId(), testPlayerId);
        Utility::TestHelper::AssertStringsEqual(request.GetPlayerSessionId(), testPlayerSessionId);
        Utility::TestHelper::AssertStringsEqual(request.GetPlayerSessionStatusFilter(), testPlayerSessionStatusFilter);
        Utility::TestHelper::AssertStringsEqual(request.GetNextToken(), testNextToken);
        ASSERT_EQ(request.GetLimit(), testLimit);
    }
};

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    DescribePlayerSessionsRequest request;
    // THEN
    Utility::TestHelper::AssertStringsEqual(request.GetGameSessionId(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetPlayerId(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetPlayerSessionId(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetPlayerSessionStatusFilter(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetNextToken(), "");
    ASSERT_EQ(request.GetLimit(), 0);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    DescribePlayerSessionsRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    DescribePlayerSessionsRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_validCStrings_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    DescribePlayerSessionsRequest &requestRef = request.WithLimit(testLimit)
                                                    .WithGameSessionId(testGameSessionId.c_str())
                                                    .WithPlayerId(testPlayerId.c_str())
                                                    .WithPlayerSessionId(testPlayerSessionId.c_str())
                                                    .WithPlayerSessionStatusFilter(testPlayerSessionStatusFilter.c_str())
                                                    .WithNextToken(testNextToken.c_str());
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_validInput_WHEN_setRequestValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    request.SetLimit(testLimit);
    request.SetGameSessionId(testGameSessionId);
    request.SetPlayerId(testPlayerId);
    request.SetPlayerSessionId(testPlayerSessionId);
    request.SetPlayerSessionStatusFilter(testPlayerSessionStatusFilter);
    request.SetNextToken(testNextToken);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_rValueInputs_WHEN_setRequestValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    request.SetLimit(testLimit);
    request.SetGameSessionId(std::string(testGameSessionId));
    request.SetPlayerId(std::string(testPlayerId));
    request.SetPlayerSessionId(std::string(testPlayerSessionId));
    request.SetPlayerSessionStatusFilter(std::string(testPlayerSessionStatusFilter));
    request.SetNextToken(std::string(testNextToken));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_validInput_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    DescribePlayerSessionsRequest &requestRef = request.WithLimit(testLimit)
                                                    .WithGameSessionId(testGameSessionId)
                                                    .WithPlayerId(testPlayerId)
                                                    .WithPlayerSessionId(testPlayerSessionId)
                                                    .WithPlayerSessionStatusFilter(testPlayerSessionStatusFilter)
                                                    .WithNextToken(testNextToken);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_rValueInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    DescribePlayerSessionsRequest request;
    // WHEN
    DescribePlayerSessionsRequest &requestRef = request.WithLimit(testLimit)
                                                    .WithGameSessionId(std::string(testGameSessionId))
                                                    .WithPlayerId(std::string(testPlayerId))
                                                    .WithPlayerSessionId(std::string(testPlayerSessionId))
                                                    .WithPlayerSessionStatusFilter(std::string(testPlayerSessionStatusFilter))
                                                    .WithNextToken(std::string(testNextToken));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(DescribePlayerSessionsRequestTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongGameSessionId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_GAME_SESSION_ID_LENGTH);
    std::string tooLongPlayerId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_ID_LENGTH);
    std::string tooLongPlayerSessionId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_SESSION_ID_LENGTH);
    std::string tooLongPlayerSessionStatusFilter = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PLAYER_SESSION_STATUS_FILTER_LENGTH);
    std::string tooLongNextToken = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_NEXT_TOKEN_LENGTH);
    ASSERT_EQ(tooLongGameSessionId.size(), MAX_GAME_SESSION_ID_LENGTH);
    ASSERT_EQ(tooLongPlayerId.size(), MAX_PLAYER_ID_LENGTH);
    ASSERT_EQ(tooLongPlayerSessionId.size(), MAX_PLAYER_SESSION_ID_LENGTH);
    ASSERT_EQ(tooLongPlayerSessionStatusFilter.size(), MAX_PLAYER_SESSION_STATUS_FILTER_LENGTH);
    ASSERT_EQ(tooLongNextToken.size(), MAX_NEXT_TOKEN_LENGTH);
    DescribePlayerSessionsRequest request;
    // WHEN
    request.SetGameSessionId(tooLongGameSessionId.c_str());
    request.SetPlayerId(tooLongPlayerId.c_str());
    request.SetPlayerSessionId(tooLongPlayerSessionId.c_str());
    request.SetPlayerSessionStatusFilter(tooLongPlayerSessionStatusFilter.c_str());
    request.SetNextToken(tooLongNextToken.c_str());
    // THEN
    ASSERT_STRNE(request.GetGameSessionId(), tooLongGameSessionId.c_str());
    ASSERT_EQ(strlen(request.GetGameSessionId()), MAX_GAME_SESSION_ID_LENGTH - 1);
    ASSERT_STRNE(request.GetPlayerId(), tooLongPlayerId.c_str());
    ASSERT_EQ(strlen(request.GetPlayerId()), MAX_PLAYER_ID_LENGTH - 1);
    ASSERT_STRNE(request.GetPlayerSessionId(), tooLongPlayerSessionId.c_str());
    ASSERT_EQ(strlen(request.GetPlayerSessionId()), MAX_PLAYER_SESSION_ID_LENGTH - 1);
    ASSERT_STRNE(request.GetPlayerSessionStatusFilter(), tooLongPlayerSessionStatusFilter.c_str());
    ASSERT_EQ(strlen(request.GetPlayerSessionStatusFilter()), MAX_PLAYER_SESSION_STATUS_FILTER_LENGTH - 1);
    ASSERT_STRNE(request.GetNextToken(), tooLongNextToken.c_str());
    ASSERT_EQ(strlen(request.GetNextToken()), MAX_NEXT_TOKEN_LENGTH - 1);
}

#endif
} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws