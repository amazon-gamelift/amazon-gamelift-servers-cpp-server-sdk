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

#include <thread>
#include <vector>

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Mean.h>
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

TEST(MeanTests, WhenGivenGaugeSetValues_ThenEmitsMean) {
  std::vector<double> Values{{
      1526, 1505, 1446, 922,  1927, 1338, 725,  1621, 1262, 1546, 1102, 821,
      511,  1483, 654,  237,  89,   1903, 423,  1248, 486,  1477, 1277, 1694,
      413,  1355, 1330, 1882, 1889, 1301, 1535, 325,  279,  1647, 900,  1445,
      1601, 675,  986,  147,  1831, 1905, 1276, 1768, 950,  200,  1106, 480,
      227,  1966, 252,  1699, 310,  883,  700,  476,  60,   545,  185,  1755,
      1364, 874,  1989, 227,  556,  1934, 1374, 1157, 463,  1160, 607,  1763,
      840,  742,  1568, 263,  1106, 1859, 554,  151,  1740, 1803, 1510, 1486,
      621,  534,  1890, 397,  1698, 1937, 679,  1347, 1650, 345,  1536, 425,
      1937, 577,  1779, 70,
  }};

  Aws::GameLift::Metrics::Mean MeanReducer;

  MockVectorEnqueuer Results;
  for (double Value : Values) {
    auto SetMessage = MetricMessage::GaugeSet(MetricGauge::Instance(), Value);
    MeanReducer.HandleMessage(SetMessage, Results);
  }
  MeanReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.mean"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_NEAR(ResultMessage.SubmitDouble.Value, 1080.19, 0.001);
}

TEST(MeanTests, WhenGivenGaugeSetAndAddMessages_ThenEmitsMean) {
  std::vector<MetricMessage> Messages{{
      MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
      MetricMessage::GaugeSet(MetricGauge::Instance(), 30),
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 20), // value is now 50
      MetricMessage::GaugeSet(MetricGauge::Instance(), 40),
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 21), // value is now 61
      MetricMessage::GaugeSet(MetricGauge::Instance(), 5),
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 16), // value now 21
      MetricMessage::GaugeSet(MetricGauge::Instance(), 42)  // value now 21
  }};

  Aws::GameLift::Metrics::Mean MeanReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MeanReducer.HandleMessage(Message, Results);
  }
  MeanReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.mean"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 33);
}

TEST(MeanTests, WhenGivenTimerSetValues_ThenEmitsMean) {
  std::vector<double> Values{
      {315, 547, 468, 22,  853,  417, 291, 394, 106, 83,  418, 373, 127,
       737, 287, 5,   464, 588,  649, 661, 726, 619, 654, 507, 502, 241,
       644, 797, 257, 514, 716,  477, 168, 634, 318, 350, 95,  437, 60,
       278, 226, 397, 419, 922,  37,  551, 440, 637, 359, 886, 680, 282,
       301, 851, 850, 450, 719,  92,  519, 380, 942, 929, 133, 318, 114,
       970, 105, 205, 33,  497,  13,  478, 863, 855, 213, 353, 139, 912,
       947, 603, 550, 914, 658,  314, 873, 771, 188, 912, 348, 938, 229,
       777, 126, 407, 290, 1000, 492, 142, 643, 244}};

  Aws::GameLift::Metrics::Mean MeanReducer;

  MockVectorEnqueuer Results;
  for (double Value : Values) {
    auto SetMessage = MetricMessage::TimerSet(MetricTimer::Instance(), Value);
    MeanReducer.HandleMessage(SetMessage, Results);
  }
  MeanReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.mean"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_NEAR(ResultMessage.SubmitDouble.Value, 472.35, 0.001);
}
