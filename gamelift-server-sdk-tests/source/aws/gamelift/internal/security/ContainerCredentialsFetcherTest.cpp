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
#include <gmock/gmock.h>
#include <aws/gamelift/internal/security/ContainerCredentialsFetcher.h>
#include <aws/gamelift/internal/util/MockHttpClient.h>
#include "rapidjson/document.h"

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class ContainerCredentialsFetcherTest : public ::testing::Test {
public:

    std::shared_ptr<MockHttpClient> mockHttpClient;
    ContainerCredentialsFetcher *containerCredentialsFetcher;

    void SetUp() override {
        mockHttpClient = std::make_shared<::testing::NiceMock<MockHttpClient>>();
        containerCredentialsFetcher = new ContainerCredentialsFetcher(*mockHttpClient);
    }

    void TearDown() override {
        mockHttpClient = nullptr;
        if (containerCredentialsFetcher) {
            delete containerCredentialsFetcher;
            containerCredentialsFetcher = nullptr;
        }
    }
};

TEST_F(ContainerCredentialsFetcherTest, GIVEN_validJsonResponse_WHEN_fetchingCredentials_THEN_returnAwsCredentials) {
    // GIVEN
    const std::string jsonResponse = R"(
    {
        "AccessKeyId": "testAccessKeyId",
        "SecretAccessKey": "testSecretAccessKey",
        "Token": "testToken"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_TRUE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetResult().AccessKey, "testAccessKeyId");
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetResult().SecretKey, "testSecretAccessKey");
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetResult().SessionToken, "testToken");
}

TEST_F(ContainerCredentialsFetcherTest, GIVEN_noEnvironmentVariable_WHEN_fetchingCredentials_THEN_returnError) {
    // GIVEN
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "");
#else
    unsetenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI");
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "The environment variable AWS_CONTAINER_CREDENTIALS_RELATIVE_URI is not set.");
}

TEST_F(ContainerCredentialsFetcherTest, GIVEN_failureResponseFromContainerCredentialsProvider_WHEN_fetchingCredentials_THEN_returnError) {
    // GIVEN
    const std::string response = "HTTP/1.1 500 Internal Server Error";

    HttpResponse httpResponse;
    httpResponse.body = response;
    httpResponse.statusCode = 500;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "Failed to get Container Credentials from Container Credential Provider. HTTP Response Status Code is 500");
}

TEST_F(ContainerCredentialsFetcherTest, GIVEN_invalidJsonResponse_WHEN_fetchingCredentials_THEN_returnError) {
    // GIVEN
    const std::string invalidJsonResponse = R"({ "InvalidJson": })";

    HttpResponse httpResponse;
    httpResponse.body = invalidJsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "Error parsing Container Credential Provider JSON response");
}

TEST_F(ContainerCredentialsFetcherTest, GIVEN_jsonMissingAccessKeyId_WHEN_fetchingCredentials_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
        {
            "SecretAccessKey": "testSecretAccessKey",
            "Token": "testToken"
        }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "AccessKeyId is not found in Container Credential Provider response");
}


TEST_F(ContainerCredentialsFetcherTest, GIVEN_jsonMissingSecretAccessKey_WHEN_fetchingCredentials_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
    {
        "AccessKeyId": "testAccessKeyId",
        "Token": "testToken"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "SecretAccessKey is not found in Container Credential Provider response");
}


TEST_F(ContainerCredentialsFetcherTest, GIVEN_jsonMissingToken_WHEN_fetchingCredentials_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
    {
        "AccessKeyId": "testAccessKeyId",
        "SecretAccessKey": "testSecretAccessKey"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
        .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/");
#else
    setenv("AWS_CONTAINER_CREDENTIALS_RELATIVE_URI", "/v2/credentials/", 1);
#endif

    // WHEN
    Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher->FetchContainerCredentials();

    // THEN
    EXPECT_FALSE(containerCredentialsFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerCredentialsFetcherOutcome.GetError(),
              "Token is not found in Container Credential Provider response");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
