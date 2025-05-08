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
#include <aws/gamelift/server/model/GetComputeCertificateResult.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class GetComputeCertificateResultTest : public ::testing::Test {
protected:
    const std::string testCertificatePath = "/test/certificate/path";
    const std::string testComputeName = "testComputeName";
    GetComputeCertificateResult testGetComputeCertificateResult;

    void SetUp() override {
        testGetComputeCertificateResult.SetCertificatePath(testCertificatePath.c_str());
        testGetComputeCertificateResult.SetComputeName(testComputeName.c_str());
    }

    // Helper Methods
    void AssertResultEqualsTestResult(const GetComputeCertificateResult &result) {
        Utility::TestHelper::AssertStringsEqual(result.GetCertificatePath(), testCertificatePath);
        Utility::TestHelper::AssertStringsEqual(result.GetComputeName(), testComputeName);
    }
};

TEST_F(GetComputeCertificateResultTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    GetComputeCertificateResult result;
    // THEN
    Utility::TestHelper::AssertStringsEqual(result.GetCertificatePath(), "");
    Utility::TestHelper::AssertStringsEqual(result.GetComputeName(), "");
}

TEST_F(GetComputeCertificateResultTest, GIVEN_result_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    GetComputeCertificateResult result(testGetComputeCertificateResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetComputeCertificateResult);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_result_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    GetComputeCertificateResult result(std::move(testGetComputeCertificateResult));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetComputeCertificateResult);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_result_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    result = testGetComputeCertificateResult;
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetComputeCertificateResult);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_result_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    result = std::move(testGetComputeCertificateResult);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_NE(&result, &testGetComputeCertificateResult);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_validCStrings_WHEN_withCertificateValues_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    GetComputeCertificateResult &resultRef = result.WithCertificatePath(testCertificatePath.c_str()).WithComputeName(testComputeName.c_str());
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(GetComputeCertificateResultTest, GIVEN_validInput_WHEN_setCertificateValues_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    result.SetCertificatePath(testCertificatePath);
    result.SetComputeName(testComputeName);
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_rValueInput_WHEN_setCertificateValues_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    result.SetCertificatePath(std::string(testCertificatePath));
    result.SetComputeName(std::string(testComputeName));
    // THEN
    AssertResultEqualsTestResult(result);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_validInput_WHEN_withCertificateValues_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    GetComputeCertificateResult &resultRef = result.WithCertificatePath(testCertificatePath).WithComputeName(testComputeName);
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

TEST_F(GetComputeCertificateResultTest, GIVEN_rValueInputs_WHEN_withCertificateValues_THEN_success) {
    // GIVEN
    GetComputeCertificateResult result;
    // WHEN
    GetComputeCertificateResult &resultRef = result.WithCertificatePath(std::string(testCertificatePath)).WithComputeName(std::string(testComputeName));
    // THEN
    AssertResultEqualsTestResult(result);
    ASSERT_EQ(&resultRef, &result);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(GetComputeCertificateResultTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongCertPath = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_CERT_PATH_LENGTH);
    std::string tooLongComputeName = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_COMPUTE_NAME_LENGTH);
    ASSERT_EQ(tooLongCertPath.size(), MAX_CERT_PATH_LENGTH);
    ASSERT_EQ(tooLongComputeName.size(), MAX_COMPUTE_NAME_LENGTH);
    GetComputeCertificateResult result;
    // WHEN
    result.SetCertificatePath(tooLongCertPath.c_str());
    result.SetComputeName(tooLongComputeName.c_str());
    // THEN
    ASSERT_STRNE(result.GetCertificatePath(), tooLongCertPath.c_str());
    ASSERT_EQ(strlen(result.GetCertificatePath()), MAX_CERT_PATH_LENGTH - 1);
    ASSERT_STRNE(result.GetComputeName(), tooLongComputeName.c_str());
    ASSERT_EQ(strlen(result.GetComputeName()), MAX_COMPUTE_NAME_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws