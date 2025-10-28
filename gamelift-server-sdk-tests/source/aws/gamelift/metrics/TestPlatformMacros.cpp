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

#include <aws/gamelift/metrics/GameLiftMetrics.h>

/*
 * Some build-config dependent defs
 */
#define MOCK_SERVER_BUILD 1
#define MOCK_DEVELOPMENT_BUILD 0
#define MOCK_CLIENT_BUILD 0

/*
 * Define the platforms.
 */
namespace {
GAMELIFT_METRICS_DEFINE_PLATFORM(Server, MOCK_SERVER_BUILD);
GAMELIFT_METRICS_DEFINE_PLATFORM(ServerDevelopment,
                                 MOCK_SERVER_BUILD &&MOCK_DEVELOPMENT_BUILD);
GAMELIFT_METRICS_DEFINE_PLATFORM(Client, MOCK_CLIENT_BUILD);
} // namespace

TEST(PlatformMacroTests, ServerBuildIsEnabled) {
  const bool Result = GAMELIFT_METRICS_PLATFORM_IS_ENABLED(Server);
  EXPECT_TRUE(Result);
}

TEST(PlatformMacroTests, ServerDevelopmentBuildIsDisabled) {
  const bool Result = GAMELIFT_METRICS_PLATFORM_IS_ENABLED(ServerDevelopment);
  EXPECT_FALSE(Result);
}

TEST(PlatformMacroTests, ClientBuildIsDisabled) {
  const bool Result = GAMELIFT_METRICS_PLATFORM_IS_ENABLED(Client);
  EXPECT_FALSE(Result);
}

/*
 * Works with constexpr too
 */
constexpr bool bConstexprTest = true;

GAMELIFT_METRICS_DEFINE_PLATFORM(Constexpr, bConstexprTest);

TEST(PlatformMacroTests, WorksWithConstexpr) {
  const bool Result = GAMELIFT_METRICS_PLATFORM_IS_ENABLED(Constexpr);
  EXPECT_TRUE(Result);
}
