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
#include <aws/gamelift/server/model/StopMatchBackfillRequest.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class StopMatchBackfillRequestTest : public ::testing::Test {
protected:
    const std::string testTicketId = "ticket-123";
    const std::string testMatchmakingConfigurationArn = "testConfigArn";
    const std::string testGameSessionArn = "testGameSessionArn";

    StopMatchBackfillRequest testRequest;

    void SetUp() override {
        testRequest.SetTicketId(testTicketId.c_str());
        testRequest.SetMatchmakingConfigurationArn(testMatchmakingConfigurationArn.c_str());
        testRequest.SetGameSessionArn(testGameSessionArn.c_str());
    }

    // Helper Methods
    void AssertRequestEqualsTestRequest(const StopMatchBackfillRequest &request) {
        Utility::TestHelper::AssertStringsEqual(request.GetTicketId(), testTicketId);
        Utility::TestHelper::AssertStringsEqual(request.GetMatchmakingConfigurationArn(), testMatchmakingConfigurationArn);
        Utility::TestHelper::AssertStringsEqual(request.GetGameSessionArn(), testGameSessionArn);
    }
};

TEST_F(StopMatchBackfillRequestTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    StopMatchBackfillRequest request;
    // THEN
    Utility::TestHelper::AssertStringsEqual(request.GetTicketId(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetMatchmakingConfigurationArn(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetGameSessionArn(), "");
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    StopMatchBackfillRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    StopMatchBackfillRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testRequest);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_validCStrings_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    StopMatchBackfillRequest &requestRef = request.WithTicketId(testTicketId.c_str())
                                               .WithMatchmakingConfigurationArn(testMatchmakingConfigurationArn.c_str())
                                               .WithGameSessionArn(testGameSessionArn.c_str());
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(StopMatchBackfillRequestTest, GIVEN_validInput_WHEN_setRequestValues_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    request.SetTicketId(testTicketId);
    request.SetMatchmakingConfigurationArn(testMatchmakingConfigurationArn);
    request.SetGameSessionArn(testGameSessionArn);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_rValueStrings_WHEN_setRequestValues_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    request.SetTicketId(std::string(testTicketId));
    request.SetMatchmakingConfigurationArn(std::string(testMatchmakingConfigurationArn));
    request.SetGameSessionArn(std::string(testGameSessionArn));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_validInput_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    StopMatchBackfillRequest &requestRef =
        request.WithTicketId(testTicketId).WithMatchmakingConfigurationArn(testMatchmakingConfigurationArn).WithGameSessionArn(testGameSessionArn);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

TEST_F(StopMatchBackfillRequestTest, GIVEN_rValueStrings_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    StopMatchBackfillRequest request;
    // WHEN
    StopMatchBackfillRequest &requestRef = request.WithTicketId(std::string(testTicketId))
                                               .WithMatchmakingConfigurationArn(std::string(testMatchmakingConfigurationArn))
                                               .WithGameSessionArn(std::string(testGameSessionArn));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(StopMatchBackfillRequestTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongTicketId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_TICKET_LENGTH);
    std::string tooLongMatchmakingConfigArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ARN_LENGTH);
    std::string tooLongGameSessionArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ARN_LENGTH);
    ASSERT_EQ(tooLongTicketId.size(), MAX_TICKET_LENGTH);
    ASSERT_EQ(tooLongMatchmakingConfigArn.size(), MAX_ARN_LENGTH);
    ASSERT_EQ(tooLongGameSessionArn.size(), MAX_ARN_LENGTH);
    StopMatchBackfillRequest request;
    // WHEN
    request.SetTicketId(tooLongTicketId.c_str());
    request.SetMatchmakingConfigurationArn(tooLongMatchmakingConfigArn.c_str());
    request.SetGameSessionArn(tooLongGameSessionArn.c_str());
    // THEN
    ASSERT_STRNE(request.GetTicketId(), tooLongTicketId.c_str());
    ASSERT_EQ(strlen(request.GetTicketId()), MAX_TICKET_LENGTH - 1);
    ASSERT_STRNE(request.GetMatchmakingConfigurationArn(), tooLongMatchmakingConfigArn.c_str());
    ASSERT_EQ(strlen(request.GetMatchmakingConfigurationArn()), MAX_ARN_LENGTH - 1);
    ASSERT_STRNE(request.GetGameSessionArn(), tooLongGameSessionArn.c_str());
    ASSERT_EQ(strlen(request.GetGameSessionArn()), MAX_ARN_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws