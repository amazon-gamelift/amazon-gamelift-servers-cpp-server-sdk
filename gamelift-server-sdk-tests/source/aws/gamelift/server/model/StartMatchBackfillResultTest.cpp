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
#include <aws/gamelift/server/model/StartMatchBackfillResult.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class StartMatchBackfillResultTest : public ::testing::Test {
protected:
    const std::string testTicketId = "ticket-123";

    StartMatchBackfillResult testResult;

    void SetUp() override { testResult.SetTicketId(testTicketId.c_str()); }

    void TearDown() override {}

    // Helper Methods
    void AssertResultEqualsTestResult(const StartMatchBackfillResult &result) { Utility::TestHelper::AssertStringsEqual(result.GetTicketId(), testTicketId); }
};

TEST_F(StartMatchBackfillResultTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    StartMatchBackfillResult result;
    // THEN
    Utility::TestHelper::AssertStringsEqual(result.GetTicketId(), "");
}

TEST_F(StartMatchBackfillResultTest, GIVEN_result_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    StartMatchBackfillResult result(testResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_result_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    StartMatchBackfillResult result(std::move(testResult));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_result_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    result = testResult;
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_result_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    result = std::move(testResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testResult);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_validCStrings_WHEN_withResultValues_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    StartMatchBackfillResult &resultRef = result.WithTicketId(testTicketId.c_str());
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(StartMatchBackfillResultTest, GIVEN_validInput_WHEN_setResultValues_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    result.SetTicketId(testTicketId);
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_rValueStrings_WHEN_setResultValues_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    result.SetTicketId(std::string(testTicketId));
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_validInput_WHEN_withResultValues_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    StartMatchBackfillResult &resultRef = result.WithTicketId(testTicketId);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

TEST_F(StartMatchBackfillResultTest, GIVEN_rValueStrings_WHEN_withResultValues_THEN_success) {
    // GIVEN
    StartMatchBackfillResult result;
    // WHEN
    StartMatchBackfillResult &resultRef = result.WithTicketId(std::string(testTicketId));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(StartMatchBackfillResultTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongTicketId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_TICKET_ID_LENGTH);
    ASSERT_EQ(tooLongTicketId.size(), MAX_TICKET_ID_LENGTH);
    StartMatchBackfillResult result;
    // WHEN
    result.SetTicketId(tooLongTicketId.c_str());
    // THEN
    ASSERT_STRNE(result.GetTicketId(), tooLongTicketId.c_str());
    ASSERT_EQ(strlen(result.GetTicketId()), MAX_TICKET_ID_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws