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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <aws/gamelift/metrics/MetricsUtils.h>
#include <aws/gamelift/server/MetricsParameters.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Test {

TEST(MetricsUtilsTest, GIVEN_ValidParameters_WHEN_ValidateMetricsParameters_THEN_Success) {
    // GIVEN
#ifdef GAMELIFT_USE_STD
    Server::MetricsParameters params("localhost", 8125, "crash-host", 9125, 1000, 1024);
#else
    Server::MetricsParameters params("localhost", 8125, "crash-host", 9125, 1000, 1024);
#endif

    // WHEN
    GenericOutcome outcome = ValidateMetricsParameters(params);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST(MetricsUtilsTest, GIVEN_EmptyStatsDHost_WHEN_ValidateMetricsParameters_THEN_ValidationError) {
    // GIVEN
#ifdef GAMELIFT_USE_STD
    Server::MetricsParameters params("", 8125, "crash-host", 9125, 1000, 1024);
#else
    Server::MetricsParameters params("", 8125, "crash-host", 9125, 1000, 1024);
#endif

    // WHEN
    GenericOutcome outcome = ValidateMetricsParameters(params);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION);
}

TEST(MetricsUtilsTest, GIVEN_NoEnvironment_WHEN_CreateMetricsParametersFromEnvironmentOrDefault_THEN_UsesDefaults) {
    // WHEN
    Server::MetricsParameters params = CreateMetricsParametersFromEnvironmentOrDefault();

    // THEN
#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(params.GetStatsDHost(), "127.0.0.1");
    EXPECT_EQ(params.GetCrashReporterHost(), "127.0.0.1");
#else
    EXPECT_STREQ(params.GetStatsDHost(), "127.0.0.1");
    EXPECT_STREQ(params.GetCrashReporterHost(), "127.0.0.1");
#endif
    EXPECT_EQ(params.GetStatsDPort(), 8125);
    EXPECT_EQ(params.GetCrashReporterPort(), 8126);
    EXPECT_EQ(params.GetFlushIntervalMs(), 10000);
    EXPECT_EQ(params.GetMaxPacketSize(), 512);
}

} // namespace Test
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
