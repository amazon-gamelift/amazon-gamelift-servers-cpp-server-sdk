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
#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class LoggerHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state: drop any previously registered multi_sink logger
        spdlog::drop("multi_sink");
    }

    void TearDown() override {
        // Clean up: drop the multi_sink logger so it doesn't affect other tests
        spdlog::drop("multi_sink");
    }
};

TEST_F(LoggerHelperTest, GIVEN_ValidLogDirectory_WHEN_InitializeLogger_THEN_LoggerHasTwoSinks) {
    // GIVEN - ensure logs/ directory exists
    mkdir("logs", 0755);

    // WHEN
    LoggerHelper::InitializeLogger("test-valid");

    // THEN - a multi_sink logger should be registered with console + file sinks
    auto logger = spdlog::get("multi_sink");
    ASSERT_NE(logger, nullptr);
    ASSERT_EQ(logger->sinks().size(), 2u);
}

TEST_F(LoggerHelperTest, GIVEN_InvalidLogPath_WHEN_InitializeLogger_THEN_FallsBackToConsoleOnly) {
    // GIVEN - The test runner runs as a non-root user (testrunner) and the
    // "logs/" directory is owned by root with mode 000. spdlog cannot create
    // or open any file inside it, which forces the rotating_file_sink_mt
    // constructor to throw spdlog_ex.

    // WHEN - should not throw, should fall back to console-only
    ASSERT_NO_THROW(LoggerHelper::InitializeLogger("test-noperm"));

    // THEN - logger should exist with only the console sink
    auto logger = spdlog::get("multi_sink");
    ASSERT_NE(logger, nullptr);
    ASSERT_EQ(logger->sinks().size(), 1u);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
