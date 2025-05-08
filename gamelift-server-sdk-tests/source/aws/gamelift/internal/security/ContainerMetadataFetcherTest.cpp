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
#include <aws/gamelift/internal/security/ContainerMetadataFetcher.h>
#include <aws/gamelift/internal/util/MockHttpClient.h>
#include "rapidjson/document.h"


using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class ContainerMetadataFetcherTest : public ::testing::Test {
public:

    std::shared_ptr<MockHttpClient> mockHttpClient;
    ContainerMetadataFetcher *containerMetadataFetcher;

    void SetUp() override {
        mockHttpClient = std::make_shared<::testing::NiceMock<MockHttpClient>>();
        containerMetadataFetcher = new ContainerMetadataFetcher(*mockHttpClient);
    }

    void TearDown() override {
        mockHttpClient = nullptr;
        if (containerMetadataFetcher) {
            delete containerMetadataFetcher;
            containerMetadataFetcher = nullptr;
        }
    }
};

TEST_F(ContainerMetadataFetcherTest,
       GIVEN_validJsonResponse_WHEN_fetchingMetadata_THEN_returnContainerMetadata) {
    // GIVEN
    const std::string jsonResponse = R"(
    {
        "Cluster": "default",
        "TaskARN": "arn:aws:ecs:us-west-2:211125306013:task/HelloWorldCluster/5c1a9b3178434e158ed1f2c16be69d14"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_TRUE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetResult().TaskId, "5c1a9b3178434e158ed1f2c16be69d14");
}

TEST_F(ContainerMetadataFetcherTest, GIVEN_noEnvironmentVariable_WHEN_fetchingMetadata_THEN_throwsRuntimeError) {
    // GIVEN
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "");
#else
    unsetenv("ECS_CONTAINER_METADATA_URI_V4");
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "The environment variable ECS_CONTAINER_METADATA_URI_V4 is not set.");
}

TEST_F(ContainerMetadataFetcherTest,
       GIVEN_failureResponseFromContainerMetadataService_WHEN_fetchingMetadata_THEN_returnError) {
    // GIVEN
    const std::string response = "HTTP/1.1 500 Internal Server Error";

    HttpResponse httpResponse;
    httpResponse.body = response;
    httpResponse.statusCode = 500;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "Failed to get Container Task Metadata from Container Metadata Service. HTTP Response Status Code is 500");
}

TEST_F(ContainerMetadataFetcherTest, GIVEN_invalidJsonResponse_WHEN_fetchingMetadata_THEN_returnError) {
    // GIVEN
    const std::string invalidJsonResponse = R"({ "InvalidJson": })";

    HttpResponse httpResponse;
    httpResponse.body = invalidJsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "Error parsing Container Metadata Service JSON response");
}

TEST_F(ContainerMetadataFetcherTest, GIVEN_jsonMissingTaskArn_WHEN_fetchingMetadata_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
    {
        "Cluster": "default"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "TaskArn is not found in Container Metadata Service response");
}

TEST_F(ContainerMetadataFetcherTest, GIVEN_jsonWithEmptyTaskArn_WHEN_fetchingMetadata_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
    {
        "Cluster": "default",
        "TaskARN": ""
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "Invalid TaskARN, value is empty");
}

TEST_F(ContainerMetadataFetcherTest, GIVEN_jsonWithInvalidTaskArn_WHEN_fetchingMetadata_THEN_returnError) {
    //GIVEN
    const std::string jsonResponse = R"(
    {
        "Cluster": "default",
        "TaskARN": "RANDOM_VALUE"
    }
    )";

    HttpResponse httpResponse;
    httpResponse.body = jsonResponse;
    httpResponse.statusCode = 200;

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::_))
            .WillOnce(testing::Return(httpResponse));
#ifdef _WIN32
    _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4");
#else
    setenv("ECS_CONTAINER_METADATA_URI_V4", "http://169.254.170.2/v4", 1);
#endif

    // WHEN
    Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher->FetchContainerTaskMetadata();

    // THEN
    EXPECT_FALSE(containerMetadataFetcherOutcome.IsSuccess());
    EXPECT_EQ(containerMetadataFetcherOutcome.GetError(),
              "Failed to extract Task ID from container TaskArn with value RANDOM_VALUE");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
