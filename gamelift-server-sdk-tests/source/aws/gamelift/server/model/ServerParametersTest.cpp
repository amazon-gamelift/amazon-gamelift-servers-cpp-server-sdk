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
#include <aws/gamelift/server/model/ServerParameters.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class ServerParametersTest : public ::testing::Test {
protected:
    const std::string testWebSocketUrl = "testWebSocketUrl";
    const std::string testAuthToken = "testAuthToken";
    const std::string testProcessId = "testProcessId";
    const std::string testHostId = "testHostId";
    const std::string testFleetId = "testFleetId";
    ServerParameters testServerParameters;

    void SetUp() override {
        testServerParameters.SetWebSocketUrl(testWebSocketUrl.c_str());
        testServerParameters.SetAuthToken(testAuthToken.c_str());
        testServerParameters.SetProcessId(testProcessId.c_str());
        testServerParameters.SetHostId(testHostId.c_str());
        testServerParameters.SetFleetId(testFleetId.c_str());
    }

    // Helper Methods
    void AssertEqualsTestParameters(const ServerParameters &parameters) {
        Utility::TestHelper::AssertStringsEqual(parameters.GetWebSocketUrl(), testWebSocketUrl);
        Utility::TestHelper::AssertStringsEqual(parameters.GetAuthToken(), testAuthToken);
        Utility::TestHelper::AssertStringsEqual(parameters.GetProcessId(), testProcessId);
        Utility::TestHelper::AssertStringsEqual(parameters.GetHostId(), testHostId);
        Utility::TestHelper::AssertStringsEqual(parameters.GetFleetId(), testFleetId);
    }
};

TEST_F(ServerParametersTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    ServerParameters parameters;
    // THEN
    Utility::TestHelper::AssertStringsEqual(parameters.GetWebSocketUrl(), "");
    Utility::TestHelper::AssertStringsEqual(parameters.GetAuthToken(), "");
    Utility::TestHelper::AssertStringsEqual(parameters.GetProcessId(), "");
    Utility::TestHelper::AssertStringsEqual(parameters.GetHostId(), "");
    Utility::TestHelper::AssertStringsEqual(parameters.GetFleetId(), "");
}

TEST_F(ServerParametersTest, GIVEN_allcStringArgs_WHEN_constructor_THEN_success) {
    // WHEN
    ServerParameters parameters(testWebSocketUrl.c_str(), testAuthToken.c_str(), testFleetId.c_str(), testHostId.c_str(), testProcessId.c_str());
    // THEN
    AssertEqualsTestParameters(parameters);
}

TEST_F(ServerParametersTest, GIVEN_parameters_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    ServerParameters parameters(testServerParameters);
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_NE(&parameters, &testServerParameters);
}

TEST_F(ServerParametersTest, GIVEN_parameters_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    ServerParameters parameters(std::move(testServerParameters));
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_NE(&parameters, &testServerParameters);
}

TEST_F(ServerParametersTest, GIVEN_parameters_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    ServerParameters parameters;
    // WHEN
    parameters = testServerParameters;
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_NE(&parameters, &testServerParameters);
}

TEST_F(ServerParametersTest, GIVEN_result_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    ServerParameters parameters;
    // WHEN
    parameters = std::move(testServerParameters);
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_NE(&parameters, &testServerParameters);
}

TEST_F(ServerParametersTest, GIVEN_validCStrings_WHEN_withServerParameterValues_THEN_success) {
    // GIVEN
    ServerParameters parameters;
    // WHEN
    ServerParameters &parametersRef = parameters.WithWebSocketUrl(testWebSocketUrl.c_str())
                                          .WithAuthToken(testAuthToken.c_str())
                                          .WithFleetId(testFleetId.c_str())
                                          .WithHostId(testHostId.c_str())
                                          .WithProcessId(testProcessId.c_str());
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_EQ(&parameters, &parametersRef);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(ServerParametersTest, GIVEN_allArgs_WHEN_constructor_THEN_success) {
    // WHEN
    ServerParameters parameters(testWebSocketUrl, testAuthToken, testFleetId, testHostId, testProcessId);
    // THEN
    AssertEqualsTestParameters(parameters);
}

TEST_F(ServerParametersTest, GIVEN_validInput_WHEN_setParameterValues_THEN_success) {
    // GIVEN
    ServerParameters parameters;
    // WHEN
    parameters.SetWebSocketUrl(testWebSocketUrl);
    parameters.SetAuthToken(testAuthToken);
    parameters.SetFleetId(testFleetId);
    parameters.SetHostId(testHostId);
    parameters.SetProcessId(testProcessId);
    // THEN
    AssertEqualsTestParameters(parameters);
}

TEST_F(ServerParametersTest, GIVEN_validInput_WHEN_withParameterValues_THEN_success) {
    // GIVEN
    ServerParameters parameters;
    // WHEN
    ServerParameters &parametersRef =
        parameters.WithWebSocketUrl(testWebSocketUrl).WithAuthToken(testAuthToken).WithFleetId(testFleetId).WithHostId(testHostId).WithProcessId(testProcessId);
    // THEN
    AssertEqualsTestParameters(parameters);
    ASSERT_EQ(&parametersRef, &parameters);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(ServerParametersTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongWebSocketUrl = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_WEBSOCKET_URL_LENGTH);
    std::string tooLongAuthToken = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_AUTH_TOKEN_LENGTH);
    std::string tooLongFleetId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_FLEET_ID_LENGTH);
    std::string tooLongHostId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_HOST_ID_LENGTH);
    std::string tooLongProcessId = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_PROCESS_ID_LENGTH);
    ASSERT_EQ(tooLongWebSocketUrl.size(), MAX_WEBSOCKET_URL_LENGTH);
    ASSERT_EQ(tooLongAuthToken.size(), MAX_AUTH_TOKEN_LENGTH);
    ASSERT_EQ(tooLongFleetId.size(), MAX_FLEET_ID_LENGTH);
    ASSERT_EQ(tooLongHostId.size(), MAX_HOST_ID_LENGTH);
    ASSERT_EQ(tooLongProcessId.size(), MAX_PROCESS_ID_LENGTH);
    ServerParameters parameters;
    // WHEN
    parameters.SetWebSocketUrl(tooLongWebSocketUrl.c_str());
    parameters.SetAuthToken(tooLongAuthToken.c_str());
    parameters.SetFleetId(tooLongFleetId.c_str());
    parameters.SetHostId(tooLongHostId.c_str());
    parameters.SetProcessId(tooLongProcessId.c_str());
    // THEN
    ASSERT_STRNE(parameters.GetWebSocketUrl(), tooLongWebSocketUrl.c_str());
    ASSERT_EQ(strlen(parameters.GetWebSocketUrl()), MAX_WEBSOCKET_URL_LENGTH - 1);
    ASSERT_STRNE(parameters.GetAuthToken(), tooLongAuthToken.c_str());
    ASSERT_EQ(strlen(parameters.GetAuthToken()), MAX_AUTH_TOKEN_LENGTH - 1);
    ASSERT_STRNE(parameters.GetFleetId(), tooLongFleetId.c_str());
    ASSERT_EQ(strlen(parameters.GetFleetId()), MAX_FLEET_ID_LENGTH - 1);
    ASSERT_STRNE(parameters.GetHostId(), tooLongHostId.c_str());
    ASSERT_EQ(strlen(parameters.GetHostId()), MAX_HOST_ID_LENGTH - 1);
    ASSERT_STRNE(parameters.GetProcessId(), tooLongProcessId.c_str());
    ASSERT_EQ(strlen(parameters.GetProcessId()), MAX_PROCESS_ID_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws