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
#include <aws/gamelift/server/model/GetFleetRoleCredentialsRequest.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {
class GetFleetRoleCredentialsRequestTest : public ::testing::Test {
protected:
    const std::string testRoleArn = "roleArn";
    const std::string testRoleSessionName = "roleSessionName";
    GetFleetRoleCredentialsRequest testGetFleetRoleCredentialsRequest;

    void SetUp() override {
        testGetFleetRoleCredentialsRequest.SetRoleArn(testRoleArn.c_str());
        testGetFleetRoleCredentialsRequest.SetRoleSessionName(testRoleSessionName.c_str());
    }

    void AssertRequestEqualsTestRequest(const GetFleetRoleCredentialsRequest &request) {
        Utility::TestHelper::AssertStringsEqual(request.GetRoleArn(), testRoleArn);
        Utility::TestHelper::AssertStringsEqual(request.GetRoleSessionName(), testRoleSessionName);
    }
};

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    GetFleetRoleCredentialsRequest request;
    // THEN
    Utility::TestHelper::AssertStringsEqual(request.GetRoleArn(), "");
    Utility::TestHelper::AssertStringsEqual(request.GetRoleSessionName(), "");
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    GetFleetRoleCredentialsRequest request(testGetFleetRoleCredentialsRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testGetFleetRoleCredentialsRequest);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    GetFleetRoleCredentialsRequest request(std::move(testGetFleetRoleCredentialsRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testGetFleetRoleCredentialsRequest);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    request = testGetFleetRoleCredentialsRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testGetFleetRoleCredentialsRequest);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    request = std::move(testGetFleetRoleCredentialsRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_NE(&request, &testGetFleetRoleCredentialsRequest);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_validCStrings_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    GetFleetRoleCredentialsRequest &requestRef = request.WithRoleArn(testRoleArn.c_str()).WithRoleSessionName(testRoleSessionName.c_str());
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}
#ifdef GAMELIFT_USE_STD
TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_validInput_WHEN_setValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    request.SetRoleArn(testRoleArn);
    request.SetRoleSessionName(testRoleSessionName);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_rValueInput_WHEN_setValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    request.SetRoleArn(std::string(testRoleArn));
    request.SetRoleSessionName(std::string(testRoleSessionName));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_validInput_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    GetFleetRoleCredentialsRequest &requestRef = request.WithRoleArn(testRoleArn).WithRoleSessionName(testRoleSessionName);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}

TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_rValueInputs_WHEN_withValues_THEN_success) {
    // GIVEN
    GetFleetRoleCredentialsRequest request;
    // WHEN
    GetFleetRoleCredentialsRequest &requestRef = request.WithRoleArn(std::string(testRoleArn)).WithRoleSessionName(std::string(testRoleSessionName));
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&requestRef, &request);
}
#else
TEST_F(GetFleetRoleCredentialsRequestTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongRoleArn = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ROLE_ARN_LENGTH + 1);
    std::string tooLongRoleSessionName = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_ROLE_SESSION_NAME_LENGTH + 1);
    ASSERT_EQ(tooLongRoleArn.size(), MAX_ROLE_ARN_LENGTH + 1);
    ASSERT_EQ(tooLongRoleSessionName.size(), MAX_ROLE_SESSION_NAME_LENGTH + 1);
    GetFleetRoleCredentialsRequest request;
    // WHEN
    request.SetRoleArn(tooLongRoleArn.c_str());
    request.SetRoleSessionName(tooLongRoleSessionName.c_str());
    // THEN
    ASSERT_STRNE(request.GetRoleArn(), tooLongRoleArn.c_str());
    ASSERT_EQ(strlen(request.GetRoleArn()), MAX_ROLE_ARN_LENGTH);
    ASSERT_STRNE(request.GetRoleSessionName(), tooLongRoleSessionName.c_str());
    ASSERT_EQ(strlen(request.GetRoleSessionName()), MAX_ROLE_SESSION_NAME_LENGTH);
}
#endif
} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws