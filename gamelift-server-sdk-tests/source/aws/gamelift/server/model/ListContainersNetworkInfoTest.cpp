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
#include <cstring>
#include <aws/gamelift/server/GameLiftServerAPI.h>
#include <aws/gamelift/common/GameLiftErrors.h>
#include <aws/gamelift/common/Outcome.h>
#include <aws/gamelift/server/model/ListContainersNetworkInfoResult.h>

using namespace Aws::GameLift;
using namespace Aws::GameLift::Server::Model;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class ListContainersNetworkInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
#ifdef _WIN32
        _putenv_s("GAMELIFT_COMPUTE_TYPE", "");
        _putenv_s("GAMELIFT_CONTAINER_DISCOVERY_SERVER_ENDPOINT", "");
        _putenv_s("ECS_CONTAINER_METADATA_URI_V4", "");
#else
        unsetenv("GAMELIFT_COMPUTE_TYPE");
        unsetenv("GAMELIFT_CONTAINER_DISCOVERY_SERVER_ENDPOINT");
        unsetenv("ECS_CONTAINER_METADATA_URI_V4");
#endif
    }
};

TEST_F(ListContainersNetworkInfoTest, GIVEN_sdkNotInitialized_WHEN_listContainersNetworkInfo_THEN_returnsError) {
    // GIVEN - SDK not initialized

    // WHEN
    auto outcome = Server::ListContainersNetworkInfo();

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::NOT_INITIALIZED);
}

// Model tests
TEST(ContainerNetworkInfoModelTest, GIVEN_defaultConstructor_WHEN_created_THEN_hasDefaultValues) {
    ContainerNetworkInfo info;
    EXPECT_EQ(info.GetContainerGroupType(), ContainerGroupType::GAME_SERVER);
}

TEST(ContainerNetworkInfoModelTest, GIVEN_values_WHEN_setAndGet_THEN_returnsCorrectValues) {
    ContainerNetworkInfo info;
    info.SetContainerName("otel-collector");
    info.SetContainerId("abc123def456");
    info.SetIpAddress("172.17.0.2");
    info.SetContainerGroupType(ContainerGroupType::PER_INSTANCE);

#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(info.GetContainerName(), "otel-collector");
    EXPECT_EQ(info.GetContainerId(), "abc123def456");
    EXPECT_EQ(info.GetIpAddress(), "172.17.0.2");
#else
    EXPECT_STREQ(info.GetContainerName(), "otel-collector");
    EXPECT_STREQ(info.GetContainerId(), "abc123def456");
    EXPECT_STREQ(info.GetIpAddress(), "172.17.0.2");
#endif
    EXPECT_EQ(info.GetContainerGroupType(), ContainerGroupType::PER_INSTANCE);
}

// The discovery server returns the full 64-char Docker SHA-256 container ID rather than the
// truncated 12-char short form. Verify the buffer holds it without truncation.
TEST(ContainerNetworkInfoModelTest, GIVEN_fullDockerContainerId_WHEN_setAndGet_THEN_preservesAll64Chars) {
    const char *fullContainerId =
        "9d97c0b58d18a3f2e4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7";
    ASSERT_EQ(strlen(fullContainerId), 64u);

    ContainerNetworkInfo info;
    info.SetContainerId(fullContainerId);

#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(info.GetContainerId(), fullContainerId);
    EXPECT_EQ(info.GetContainerId().size(), 64u);
#else
    EXPECT_STREQ(info.GetContainerId(), fullContainerId);
    EXPECT_EQ(strlen(info.GetContainerId()), 64u);
#endif
}

TEST(ContainerNetworkInfoModelTest, GIVEN_result_WHEN_addContainerNetworkInfo_THEN_containsEntry) {
    ListContainersNetworkInfoResult result;

    ContainerNetworkInfo info;
    info.SetContainerName("game-server");
    info.SetContainerId("9d97c0b58d18");
    info.SetIpAddress("172.17.0.4");
    info.SetContainerGroupType(ContainerGroupType::GAME_SERVER);

    result.AddContainerNetworkInfo(info);

#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(result.GetContainersNetworkInfo().size(), 1);
    EXPECT_EQ(result.GetContainersNetworkInfo()[0].GetContainerName(), "game-server");
    EXPECT_EQ(result.GetContainersNetworkInfo()[0].GetIpAddress(), "172.17.0.4");
#else
    EXPECT_EQ(result.GetContainersNetworkInfoCount(), 1);
    EXPECT_STREQ(result.GetContainersNetworkInfo()[0].GetContainerName(), "game-server");
    EXPECT_STREQ(result.GetContainersNetworkInfo()[0].GetIpAddress(), "172.17.0.4");
#endif
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
