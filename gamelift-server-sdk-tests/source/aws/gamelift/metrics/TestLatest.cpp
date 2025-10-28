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
#include "Common.h"
#include "MetricMacrosTests.h"
#include "MockDerivedMetric.h"

#include <aws/gamelift/metrics/MetricsProcessor.h>
#include <thread>
#include <vector>

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Latest.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "timer", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);
} // namespace

TEST(LatestReducerTests, GivenGauge_WhenSetCalled_ThenLatestKeepsTheLatestSet) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 30),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 40),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 5)}};

  Aws::GameLift::Metrics::Latest LatestReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    LatestReducer.HandleMessage(Message, Results);
  }
  LatestReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.latest"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 5);
}

TEST(LatestReducerTests, GivenGauge_WhenSetAndAddCalled_ThenLatestKeepsSummed) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 10),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 5)}};

  Aws::GameLift::Metrics::Latest LatestReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    LatestReducer.HandleMessage(Message, Results);
  }
  LatestReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.latest"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 30);
}

TEST(LatestReducerTests,
     GivenGauge_WhenSetAndAddAndSetCalled_ThenLatestKeepsLastSet) {
  std::vector<MetricMessage> Messages{{
      MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 10),
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 5),
      MetricMessage::GaugeSet(MetricGauge::Instance(), 9999),
  }};

  Aws::GameLift::Metrics::Latest LatestReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    LatestReducer.HandleMessage(Message, Results);
  }
  LatestReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.latest"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 9999);
}

TEST(LatestReducerTests, GivenTimer_WhenSetCalled_ThenLatestKeepsTheLatestSet) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 30),
       MetricMessage::TimerSet(MetricTimer::Instance(), 40),
       MetricMessage::TimerSet(MetricTimer::Instance(), 92)}};

  Aws::GameLift::Metrics::Latest LatestReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    LatestReducer.HandleMessage(Message, Results);
  }
  LatestReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.latest"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 92);
}
