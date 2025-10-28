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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <aws/gamelift/metrics/GameLiftMetrics.h>
#include <aws/gamelift/metrics/MetricsSettings.h>

#include "MetricMacrosTests.h"
#include "MockSampleEveryOther.h"

namespace {
GAMELIFT_METRICS_DECLARE_TIMER(MetricEnabledTimer, "glork", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricEnabledTimer);

GAMELIFT_METRICS_DECLARE_TIMER(MetricSampledTimer, "glork_sampled", MockEnabled,
                               MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_TIMER(MetricSampledTimer);

GAMELIFT_METRICS_DECLARE_TIMER(MetricDisabledTimer, "nlork", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricDisabledTimer);
} // namespace

class ScopedTimerCompilesTests : public ::testing::Test {
protected:
  virtual void SetUp() override {
    Aws::GameLift::Metrics::MetricsSettings Settings;
    Settings.SendPacketCallback = [](const char *, int) {};
    Settings.MaxPacketSizeBytes = 1000;
    Settings.CaptureIntervalSec = 10;
    // Disable StatsDClient for tests
    Settings.StatsDClientHost = "";
    // Disable crash reporter for tests
    Settings.CrashReporterHost = "";
    Settings.StatsDClientPort = 0;
    Settings.CrashReporterPort = 0;
    Aws::GameLift::Metrics::MetricsInitialize(Settings);
  }

  virtual void TearDown() override {
    Aws::GameLift::Metrics::MetricsTerminate();
  }
};

/**
 * These tests merely ensure everything compiles and does not crash.
 * Internals are tested in TestScopedTimer.cpp
 */
TEST_F(ScopedTimerCompilesTests, GivenEnabledMetric_ScopedTimerCompiles) {
  Aws::GameLift::Metrics::ScopedTimer<MetricEnabledTimer> Test;
}

TEST_F(ScopedTimerCompilesTests, GivenSampledMetric_ScopedTimerCompiles) {
  Aws::GameLift::Metrics::ScopedTimer<MetricSampledTimer> Test;
}

TEST_F(ScopedTimerCompilesTests, GivenDisabledMetric_ScopedTimerCompiles) {
  Aws::GameLift::Metrics::ScopedTimer<MetricDisabledTimer> Test;
}

TEST_F(ScopedTimerCompilesTests, GivenEnabledMetric_TimeScopeMacroCompiles) {
  GAMELIFT_METRICS_TIME_SCOPE(MetricEnabledTimer);
}

TEST_F(ScopedTimerCompilesTests, GivenSampledMetric_TimeScopeMacroCompiles) {
  GAMELIFT_METRICS_TIME_SCOPE(MetricSampledTimer);
}

TEST_F(ScopedTimerCompilesTests, GivenDisabledMetric_TimeScopeMacroCompiles) {
  GAMELIFT_METRICS_TIME_SCOPE(MetricDisabledTimer);
}

TEST_F(ScopedTimerCompilesTests,
       GivenEnabledMetric_TimeExpressionMacroCompiles) {
  const int Value = GAMELIFT_METRICS_TIME_EXPR(MetricEnabledTimer, 10 + 10);
}

TEST_F(ScopedTimerCompilesTests,
       GivenSampledMetric_TimeExpressionMacroCompiles) {
  const int Value = GAMELIFT_METRICS_TIME_EXPR(MetricSampledTimer, 10 + 10);
}

TEST_F(ScopedTimerCompilesTests,
       GivenDisabledMetric_TimeExpressionMacroCompiles) {
  const int Value = GAMELIFT_METRICS_TIME_EXPR(MetricDisabledTimer, 10 + 10);
}
