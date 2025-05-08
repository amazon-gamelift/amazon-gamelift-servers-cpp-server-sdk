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
#include <aws/gamelift/server/model/GetFleetRoleCredentialsResult.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {
class GetFleetRoleCredentialsResultTest : public ::testing::Test {
protected:
    const std::string testAssumedUserRoleArn = "assumedUserRoleArn";
    const std::string testAssumedRoleId = "assumedRoleId";
    const std::string testAccessKeyId = "accessKeyId";
    const std::string testSecretAccessKey = "secretAccess";
    const std::string testSessionToken = "sessionToken";
    // Jan 1, 2022
    const time_t testNoStdExpiration = 1640995200;
    const std::tm testStdExpiration = {
        0,   // seconds after the minute - [0, 60] including leap second
        0,   // minutes after the hour - [0, 59]
        0,   // hours since midnight - [0, 23]
        1,   // day of the month - [1, 31]
        0,   // months since January - [0, 11]
        122, // years since 1900
        6,   // days since Sunday - [0, 6]
        0,   // days since January 1 - [0, 365]
        0    // daylight savings time flag
    };

    GetFleetRoleCredentialsResult testGetFleetRoleCredentialsResult;

    void SetUp() override {
        testGetFleetRoleCredentialsResult.SetAssumedUserRoleArn(testAssumedUserRoleArn.c_str());
        testGetFleetRoleCredentialsResult.SetAssumedRoleId(testAssumedRoleId.c_str());
        testGetFleetRoleCredentialsResult.SetAccessKeyId(testAccessKeyId.c_str());
        testGetFleetRoleCredentialsResult.SetSecretAccessKey(testSecretAccessKey.c_str());
        testGetFleetRoleCredentialsResult.SetSessionToken(testSessionToken.c_str());
        testGetFleetRoleCredentialsResult.SetExpiration(testNoStdExpiration);
    }

    void AssertStdExpirationsEqual(const std::tm &first, const std::tm &second) {
        EXPECT_EQ(first.tm_sec, second.tm_sec);
        EXPECT_EQ(first.tm_min, second.tm_min);
        EXPECT_EQ(first.tm_hour, second.tm_hour);
        EXPECT_EQ(first.tm_mday, second.tm_mday);
        EXPECT_EQ(first.tm_mon, second.tm_mon);
        EXPECT_EQ(first.tm_year, second.tm_year);
        EXPECT_EQ(first.tm_wday, second.tm_wday);
        EXPECT_EQ(first.tm_yday, second.tm_yday);
        EXPECT_EQ(first.tm_isdst, second.tm_isdst);
    }

    void AssertResultEqualsTestResult(const GetFleetRoleCredentialsResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetAssumedUserRoleArn(), testAssumedUserRoleArn);
        Utility::TestHelper::AssertStringsEqual(result.GetAssumedRoleId(), testAssumedRoleId);
        Utility::TestHelper::AssertStringsEqual(result.GetAccessKeyId(), testAccessKeyId);
        Utility::TestHelper::AssertStringsEqual(result.GetSecretAccessKey(), testSecretAccessKey);
        Utility::TestHelper::AssertStringsEqual(result.GetSessionToken(), testSessionToken);
#ifdef GAMELIFT_USE_STD
        std::tm expiration = result.GetExpiration();
        AssertStdExpirationsEqual(expiration, testStdExpiration);
#else
        EXPECT_EQ(result.GetExpiration(), testNoStdExpiration);
#endif
    }
};

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    GetFleetRoleCredentialsResult result;
    // THEN
    Utility::TestHelper::AssertStringsEqual(result.GetAssumedUserRoleArn(), "");
    Utility::TestHelper::AssertStringsEqual(result.GetAssumedRoleId(), "");
    Utility::TestHelper::AssertStringsEqual(result.GetAccessKeyId(), "");
    Utility::TestHelper::AssertStringsEqual(result.GetSecretAccessKey(), "");
    Utility::TestHelper::AssertStringsEqual(result.GetSessionToken(), "");
#ifdef GAMELIFT_USE_STD
    AssertStdExpirationsEqual(result.GetExpiration(), {0});
#else
    EXPECT_EQ(result.GetExpiration(), 0);
#endif
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_result_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    GetFleetRoleCredentialsResult result(testGetFleetRoleCredentialsResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetFleetRoleCredentialsResult);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_result_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    GetFleetRoleCredentialsResult result(std::move(testGetFleetRoleCredentialsResult));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetFleetRoleCredentialsResult);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_result_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    result = testGetFleetRoleCredentialsResult;
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetFleetRoleCredentialsResult);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_result_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    result = std::move(testGetFleetRoleCredentialsResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetFleetRoleCredentialsResult);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_validCStrings_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    GetFleetRoleCredentialsResult &resultRef = result.WithAssumedUserRoleArn(testAssumedUserRoleArn.c_str())
                                                   .WithAssumedRoleId(testAssumedRoleId.c_str())
                                                   .WithAccessKeyId(testAccessKeyId.c_str())
                                                   .WithSecretAccessKey(testSecretAccessKey.c_str())
                                                   .WithSessionToken(testSessionToken.c_str())
                                                   .WithExpiration(testNoStdExpiration);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}
#ifdef GAMELIFT_USE_STD
TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_validInput_WHEN_setValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    result.SetAssumedUserRoleArn(testAssumedUserRoleArn);
    result.SetAssumedRoleId(testAssumedRoleId);
    result.SetAccessKeyId(testAccessKeyId);
    result.SetSecretAccessKey(testSecretAccessKey);
    result.SetSessionToken(testSessionToken);
    result.SetExpiration(testStdExpiration);
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_rValueInput_WHEN_setValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    result.SetAssumedUserRoleArn(std::string(testAssumedUserRoleArn));
    result.SetAssumedRoleId(std::string(testAssumedRoleId));
    result.SetAccessKeyId(std::string(testAccessKeyId));
    result.SetSecretAccessKey(std::string(testSecretAccessKey));
    result.SetSessionToken(std::string(testSessionToken));
    result.SetExpiration({
        0,   // seconds after the minute - [0, 60] including leap second
        0,   // minutes after the hour - [0, 59]
        0,   // hours since midnight - [0, 23]
        1,   // day of the month - [1, 31]
        0,   // months since January - [0, 11]
        122, // years since 1900
        6,   // days since Sunday - [0, 6]
        0,   // days since January 1 - [0, 365]
        0    // daylight savings time flag
    });
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_validInput_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    GetFleetRoleCredentialsResult &resultRef = result.WithAssumedUserRoleArn(testAssumedUserRoleArn)
                                                   .WithAssumedRoleId(testAssumedRoleId)
                                                   .WithAccessKeyId(testAccessKeyId)
                                                   .WithSecretAccessKey(testSecretAccessKey)
                                                   .WithSessionToken(testSessionToken)
                                                   .WithExpiration(testStdExpiration);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_rValueInputs_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsResult result;
    // WHEN
    GetFleetRoleCredentialsResult &resultRef = result.WithAssumedUserRoleArn(std::string(testAssumedUserRoleArn))
                                                   .WithAssumedRoleId(std::string(testAssumedRoleId))
                                                   .WithAccessKeyId(std::string(testAccessKeyId))
                                                   .WithSecretAccessKey(std::string(testSecretAccessKey))
                                                   .WithSessionToken(std::string(testSessionToken))
                                                   .WithExpiration({
                                                       0,   // seconds after the minute - [0, 60] including leap second
                                                       0,   // minutes after the hour - [0, 59]
                                                       0,   // hours since midnight - [0, 23]
                                                       1,   // day of the month - [1, 31]
                                                       0,   // months since January - [0, 11]
                                                       122, // years since 1900
                                                       6,   // days since Sunday - [0, 6]
                                                       0,   // days since January 1 - [0, 365]
                                                       0    // daylight savings time flag
                                                   });
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}
#else
TEST_F(GetFleetRoleCredentialsResultTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongAssumedUserRoleArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ASSUMED_USER_ROLE_ARN_LENGTH + 1);
    std::string tooLongAssumedRoleId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ASSUMED_ROLE_ID_LENGTH + 1);
    std::string tooLongAccessKeyId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ACCESS_KEY_ID_LENGTH + 1);
    std::string tooLongSecretAccessKey = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_SECRET_ACCESS_KEY_LENGTH + 1);
    // Can't be too long, no size limit is defined. Use 128kb to test
    std::string sessionToken = Utility::TestHelper::GenerateRandomAlphaNumericString(128000);

    ASSERT_EQ(tooLongAssumedUserRoleArn.size(), MAX_ASSUMED_USER_ROLE_ARN_LENGTH + 1);
    ASSERT_EQ(tooLongAssumedRoleId.size(), MAX_ASSUMED_ROLE_ID_LENGTH + 1);
    ASSERT_EQ(tooLongAccessKeyId.size(), MAX_ACCESS_KEY_ID_LENGTH + 1);
    ASSERT_EQ(tooLongSecretAccessKey.size(), MAX_SECRET_ACCESS_KEY_LENGTH + 1);
    ASSERT_EQ(sessionToken.size(), 128000);
    GetFleetRoleCredentialsResult result;

    // WHEN
    result.SetAssumedUserRoleArn(tooLongAssumedUserRoleArn.c_str());
    result.SetAssumedRoleId(tooLongAssumedRoleId.c_str());
    result.SetAccessKeyId(tooLongAccessKeyId.c_str());
    result.SetSecretAccessKey(tooLongSecretAccessKey.c_str());
    result.SetSessionToken(sessionToken.c_str());

    // THEN
    ASSERT_STRNE(result.GetAssumedUserRoleArn(), tooLongAssumedUserRoleArn.c_str());
    ASSERT_EQ(strlen(result.GetAssumedUserRoleArn()), MAX_ASSUMED_USER_ROLE_ARN_LENGTH);
    ASSERT_STRNE(result.GetAssumedRoleId(), tooLongAssumedRoleId.c_str());
    ASSERT_EQ(strlen(result.GetAssumedRoleId()), MAX_ASSUMED_ROLE_ID_LENGTH);
    ASSERT_STRNE(result.GetAccessKeyId(), tooLongAccessKeyId.c_str());
    ASSERT_EQ(strlen(result.GetAccessKeyId()), MAX_ACCESS_KEY_ID_LENGTH);
    ASSERT_STRNE(result.GetSecretAccessKey(), tooLongSecretAccessKey.c_str());
    ASSERT_EQ(strlen(result.GetSecretAccessKey()), MAX_SECRET_ACCESS_KEY_LENGTH);
    ASSERT_STREQ(result.GetSessionToken(), sessionToken.c_str());
    ASSERT_EQ(strlen(result.GetSessionToken()), 128000);
}
#endif
} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws
