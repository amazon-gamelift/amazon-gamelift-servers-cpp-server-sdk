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

#include <gtest/gtest.h>
#include <aws/gamelift/internal/security/AwsSigV4Utility.h>
#include <aws/gamelift/internal/security/AwsCredentials.h>
#include <map>
#include <string>
#include <ctime>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class AwsSigV4UtilityTest : public ::testing::Test {
public:

    void SetUp() override {
    }

    void TearDown() override {
    }
};

SigV4Parameters CreateSigV4Parameters() {
    std::tm timeInfo = {};
    timeInfo.tm_year = 2024 - 1900;
    timeInfo.tm_mon = 7;
    timeInfo.tm_mday = 5;
    timeInfo.tm_hour = 9;
    timeInfo.tm_min = 0;
    timeInfo.tm_sec = 0;
    timeInfo.tm_isdst = 0;

    std::tm utcTime = {};
#ifdef _WIN32
    std::time_t requestTime = _mkgmtime(&timeInfo);
    gmtime_s(&utcTime, &requestTime);
#else
    std::time_t requestTime = timegm(&timeInfo);
    gmtime_r(&requestTime, &utcTime);
#endif
    std::map<std::string, std::string> queryParams;
    queryParams["param1"] = "value1";
    queryParams["param2"] = "value2";

    AwsCredentials awsCredentials("testAccessKey", "testSecretKey", "testSessionToken");

    std::string awsRegion = "us-east-1";
    return SigV4Parameters(awsRegion, awsCredentials, queryParams, utcTime);

}

TEST_F(AwsSigV4UtilityTest, GIVEN_validSigV4Parameters_WHEN_generateSigV4QueryParameters_THEN_returnExpectedQueryParameters) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
    auto sigV4QueryParams = outcome.GetResult();
    EXPECT_EQ(sigV4QueryParams["Authorization"], "SigV4");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Algorithm"], "AWS4-HMAC-SHA256");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Credential"], "testAccessKey%2F20240805%2Fus-east-1%2Fgamelift%2Faws4_request");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Date"], "20240805T090000Z");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Security-Token"], "testSessionToken");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Signature"], "acd5225ad5491af728fae9e3fc93dab103bd757ff174580f6520103471595f5e");
}

    TEST_F(AwsSigV4UtilityTest, GIVEN_sigV4ParametersWithMissingAccessKey_WHEN_generateSigV4QueryParameters_THEN_returnError) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();
    sigV4Parameters.Credentials.AccessKey.clear();

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError(), "AccessKey is required");
}

    TEST_F(AwsSigV4UtilityTest, GIVEN_sigV4ParametersWithMissingSecretKey_WHEN_generateSigV4QueryParameters_THEN_returnError) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();
    sigV4Parameters.Credentials.SecretKey.clear();

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError(), "SecretKey is required");
}


    TEST_F(AwsSigV4UtilityTest, GIVEN_sigV4ParametersWithMissingSessionToken_WHEN_generateSigV4QueryParameters_THEN_returnExpectedQueryParameters) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();
    sigV4Parameters.Credentials.SessionToken.clear();

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
    auto sigV4QueryParams = outcome.GetResult();
    EXPECT_EQ(sigV4QueryParams["Authorization"], "SigV4");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Algorithm"], "AWS4-HMAC-SHA256");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Credential"], "testAccessKey%2F20240805%2Fus-east-1%2Fgamelift%2Faws4_request");
    EXPECT_EQ(sigV4QueryParams["X-Amz-Date"], "20240805T090000Z");
    EXPECT_EQ(sigV4QueryParams.count("X-Amz-Security-Token"), 0);
    EXPECT_EQ(sigV4QueryParams["X-Amz-Signature"], "acd5225ad5491af728fae9e3fc93dab103bd757ff174580f6520103471595f5e");
}


TEST_F(AwsSigV4UtilityTest, GIVEN_sigV4ParametersWithMissingQueryParams_WHEN_generateSigV4QueryParameters_THEN_returnError) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();
    sigV4Parameters.QueryParams.clear();

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError(), "QueryParams is required");
    }

    TEST_F(AwsSigV4UtilityTest, GIVEN_sigV4ParametersWithMissingRequestTime_WHEN_generateSigV4QueryParameters_THEN_returnError) {
    // GIVEN
    auto sigV4Parameters = CreateSigV4Parameters();
    sigV4Parameters.RequestTime = {};

    // WHEN
    auto outcome = AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Parameters);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError(), "RequestTime is required");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
