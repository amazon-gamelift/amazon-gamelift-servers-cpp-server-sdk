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
#include <type_traits>

#include "MetricMacrosTests.h"
#include "MockSampleEveryOther.h"

#include <aws/gamelift/metrics/GameLiftMetrics.h>

using namespace ::testing;

class CounterMacrosTests : public MetricMacrosTests {};

// Define a counter
namespace {
GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounter, "count", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounter);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounterDisabled, "nocount", MockDisabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounterDisabled);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricWithSampler, "counter_with_sampler",
                                 MockEnabled, MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricWithSampler);
} // namespace

TEST_F(CounterMacrosTests, ContainsValidDetails) {
  EXPECT_TRUE((std::is_same<MetricCounter::Platform, MockEnabled>::value));
  EXPECT_TRUE((std::is_same<MetricCounter::SamplerType,
                            Aws::GameLift::Metrics::SampleAll>::value));
  EXPECT_TRUE((std::is_same<MetricCounter::MetricType,
                            Aws::GameLift::Metrics::Counter>::value));
  EXPECT_THAT(MetricCounter::Instance().GetKey(), StrEq("count"));
  EXPECT_THAT(MetricCounter::Instance().GetMetricType(),
              Aws::GameLift::Metrics::MetricType::Counter);

  EXPECT_TRUE(
      (std::is_same<MetricCounterDisabled::Platform, MockDisabled>::value));
  EXPECT_TRUE((std::is_same<MetricCounterDisabled::SamplerType,
                            Aws::GameLift::Metrics::SampleAll>::value));
  EXPECT_TRUE((std::is_same<MetricCounterDisabled::MetricType,
                            Aws::GameLift::Metrics::Counter>::value));
  EXPECT_THAT(MetricCounterDisabled::Instance().GetKey(), StrEq("nocount"));
  EXPECT_THAT(MetricCounterDisabled::Instance().GetMetricType(),
              Aws::GameLift::Metrics::MetricType::Counter);

  EXPECT_TRUE((std::is_same<MetricWithSampler::Platform, MockEnabled>::value));
  EXPECT_TRUE((std::is_same<MetricWithSampler::SamplerType,
                            MockSampleEveryOther>::value));
  EXPECT_TRUE((std::is_same<MetricWithSampler::MetricType,
                            Aws::GameLift::Metrics::Counter>::value));
  EXPECT_THAT(MetricWithSampler::Instance().GetKey(),
              StrEq("counter_with_sampler"));
  EXPECT_THAT(MetricWithSampler::Instance().GetMetricType(),
              Aws::GameLift::Metrics::MetricType::Counter);
}
