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
#include <aws/gamelift/internal/GameLiftServerState.h>
#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <aws/gamelift/server/CustomLoggerConfiguration.h>
#include <spdlog/spdlog.h>
#include <iostream>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Test {

static const std::string sdkVersion = "5.6.0";

TEST(GameLiftServerAPITest, GIVEN_SdkVersion_WHEN_GetSdkVersion_THEN_success) {
    // GIVEN
    // WHEN
    AwsStringOutcome outcome = Server::GetSdkVersion();
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
#ifdef GAMELIFT_USE_STD
    ASSERT_EQ(outcome.GetResult(), sdkVersion);
#else
    ASSERT_STREQ(outcome.GetResult(), sdkVersion.c_str());
#endif
}

TEST(GameLiftServerAPITest, GIVEN_nullCallback_WHEN_initCustomLogger_THEN_returnsBadRequest) {
    // Server::InitCustomLogger rejects null callbacks to prevent crashes on the first log message.
    // GIVEN
    Aws::GameLift::Server::CustomLoggerConfiguration params(nullptr, nullptr, Aws::GameLift::Server::LogLevel::Info);

    // WHEN
    GenericOutcome outcome = Server::InitCustomLogger(params);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION);
}

namespace {
void TestLogCallback(Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
    auto* counter = static_cast<int*>(userData);
    ++(*counter);
}
} // anonymous namespace

TEST(GameLiftServerAPITest, GIVEN_customLoggerAlreadyRegistered_WHEN_initCustomLoggerCalledAgain_THEN_returnsAlreadyInitialized) {
    // Ensure clean state for this test.
    Aws::GameLift::Internal::LoggerHelper::ResetCustomLoggerRegistered();

    // GIVEN - first InitCustomLogger with a valid callback succeeds
    int callCountA = 0;
    Aws::GameLift::Server::CustomLoggerConfiguration paramsA(TestLogCallback, &callCountA, Aws::GameLift::Server::LogLevel::Info);
    GenericOutcome firstOutcome = Server::InitCustomLogger(paramsA);
    ASSERT_TRUE(firstOutcome.IsSuccess());

    // WHEN - second InitCustomLogger with a different valid callback
    int callCountB = 0;
    Aws::GameLift::Server::CustomLoggerConfiguration paramsB(TestLogCallback, &callCountB, Aws::GameLift::Server::LogLevel::Warn);
    GenericOutcome secondOutcome = Server::InitCustomLogger(paramsB);

    // THEN - returns ALREADY_INITIALIZED
    EXPECT_FALSE(secondOutcome.IsSuccess());
    EXPECT_EQ(secondOutcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED);

    // The original callback remains active (second callback was never registered)
    spdlog::info("verify original callback still active");
    spdlog::default_logger()->flush();
    EXPECT_GT(callCountA, 0);
    EXPECT_EQ(callCountB, 0);

    // Cleanup
    Aws::GameLift::Internal::LoggerHelper::ResetCustomLoggerRegistered();
}

} // namespace Test
} // namespace Server
} // namespace GameLift
} // namespace Aws