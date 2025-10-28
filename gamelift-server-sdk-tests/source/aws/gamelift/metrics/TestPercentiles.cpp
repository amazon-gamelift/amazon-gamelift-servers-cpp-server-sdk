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

#include "MetricMacrosTests.h"
#include "MockDerivedMetric.h"

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Percentiles.h>
#include <aws/gamelift/metrics/Samplers.h>

#include <vector>

using namespace ::testing;

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "timer", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);
} // namespace

TEST(PercentilesTests,
     WhenGaugeSetMessagesHandled_ThenEmitsGaugeSetForEachPercentile) {
  std::vector<double> Values{
      {90, 25, 73, 22, 34, 46, 3,  53, 71, 80, 54, 56, 53, 6,  56, 44, 94,
       19, 30, 17, 78, 47, 71, 9,  38, 63, 75, 46, 56, 71, 36, 3,  34, 96,
       30, 53, 19, 54, 97, 67, 65, 4,  28, 71, 61, 41, 96, 31, 67, 71}};

  auto Percentiles =
      Aws::GameLift::Metrics::Percentiles(0.05, 0.25, 0.5, 0.75, 0.95);

  MockVectorEnqueuer Results;
  for (double Value : Values) {
    auto SetMessage = MetricMessage::GaugeSet(MetricGauge::Instance(), Value);
    Percentiles.HandleMessage(SetMessage, Results);
  }
  Percentiles.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(5));

  MetricMessage P05 = Results.Values[0];
  EXPECT_THAT(P05.Metric->GetKey(), StrEq("gauge.p05"));
  EXPECT_EQ(P05.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P05.SubmitDouble.Value, 4.9, 0.001);

  MetricMessage P25 = Results.Values[1];
  EXPECT_THAT(P25.Metric->GetKey(), StrEq("gauge.p25"));
  EXPECT_EQ(P25.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P25.SubmitDouble.Value, 30.25, 0.001);

  MetricMessage P50 = Results.Values[2];
  EXPECT_THAT(P50.Metric->GetKey(), StrEq("gauge.p50"));
  EXPECT_EQ(P50.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P50.SubmitDouble.Value, 53, 0.001);

  MetricMessage P75 = Results.Values[3];
  EXPECT_THAT(P75.Metric->GetKey(), StrEq("gauge.p75"));
  EXPECT_EQ(P75.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P75.SubmitDouble.Value, 71, 0.001);

  MetricMessage P95 = Results.Values[4];
  EXPECT_THAT(P95.Metric->GetKey(), StrEq("gauge.p95"));
  EXPECT_EQ(P95.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P95.SubmitDouble.Value, 95.1, 0.001);
}

TEST(PercentilesTests,
     WhenMessageHandledWithTags_ThenComputesCorrectPercentiles) {
  std::vector<double> Values{
      {1559, 1045, 432,  1992, 654,  1883, 1297, 1619, 531,  826,  1481, 1576,
       13,   1712, 786,  1450, 396,  249,  1217, 1559, 1436, 127,  234,  1305,
       1391, 1213, 1636, 235,  83,   862,  1141, 199,  424,  92,   1286, 947,
       1470, 543,  1281, 626,  1045, 1294, 1527, 1467, 1752, 456,  1800, 878,
       53,   564,  1050, 896,  1415, 1903, 1123, 487,  659,  409,  487,  1243,
       1304, 228,  658,  1055, 1902, 544,  1122, 887,  1193, 330,  739,  75,
       653,  1798, 193,  1496, 1324, 1989, 83,   1236, 136,  621,  1213, 1783,
       763,  215,  330,  906,  1552, 1118, 783,  505,  1511, 1781, 498,  1033,
       1621, 1173, 1037, 1508}};

  // A partition so we can insert a tag message in the middle of our data.
  auto Partition = std::begin(Values) + 42;

  auto Percentiles = Aws::GameLift::Metrics::Percentiles(0.1, 0.5, 0.95);
  MockVectorEnqueuer Results;

  auto FirstTagMessage =
      MetricMessage::TagSet(MetricGauge::Instance(), "foo", "bar");
  auto SecondTagMessage =
      MetricMessage::TagSet(MetricGauge::Instance(), "hello", "world");
  Percentiles.HandleMessage(FirstTagMessage, Results);
  Percentiles.HandleMessage(SecondTagMessage, Results);
  for (auto It = std::begin(Values); It != Partition; ++It) {
    auto SetMessage = MetricMessage::GaugeSet(MetricGauge::Instance(), *It);
    Percentiles.HandleMessage(SetMessage, Results);
  }
  auto RemoveTagMessage =
      MetricMessage::TagRemove(MetricGauge::Instance(), "hello");
  Percentiles.HandleMessage(RemoveTagMessage, Results);
  for (auto It = Partition; It != std::end(Values); ++It) {
    auto SetMessage = MetricMessage::GaugeSet(MetricGauge::Instance(), *It);
    Percentiles.HandleMessage(SetMessage, Results);
  }
  Percentiles.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(12));

  ASSERT_EQ(Results.Values[0].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[0].Metric->GetKey(), StrEq("gauge.p10"));
  ASSERT_EQ(Results.Values[1].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[1].Metric->GetKey(), StrEq("gauge.p50"));
  ASSERT_EQ(Results.Values[2].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[2].Metric->GetKey(), StrEq("gauge.p95"));

  ASSERT_EQ(Results.Values[3].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[3].Metric->GetKey(), StrEq("gauge.p10"));
  ASSERT_EQ(Results.Values[4].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[4].Metric->GetKey(), StrEq("gauge.p50"));
  ASSERT_EQ(Results.Values[5].Type, MetricMessageType::TagSet);
  EXPECT_THAT(Results.Values[5].Metric->GetKey(), StrEq("gauge.p95"));

  ASSERT_EQ(Results.Values[6].Type, MetricMessageType::TagRemove);
  EXPECT_THAT(Results.Values[6].Metric->GetKey(), StrEq("gauge.p10"));
  ASSERT_EQ(Results.Values[7].Type, MetricMessageType::TagRemove);
  EXPECT_THAT(Results.Values[7].Metric->GetKey(), StrEq("gauge.p50"));
  ASSERT_EQ(Results.Values[8].Type, MetricMessageType::TagRemove);
  EXPECT_THAT(Results.Values[8].Metric->GetKey(), StrEq("gauge.p95"));

  MetricMessage P10 = Results.Values[9];
  EXPECT_THAT(P10.Metric->GetKey(), StrEq("gauge.p10"));
  EXPECT_EQ(P10.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P10.SubmitDouble.Value, 213.4, 0.001);

  MetricMessage P50 = Results.Values[10];
  EXPECT_THAT(P50.Metric->GetKey(), StrEq("gauge.p50"));
  EXPECT_EQ(P50.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P50.SubmitDouble.Value, 1045, 0.001);

  MetricMessage P95 = Results.Values[11];
  EXPECT_THAT(P95.Metric->GetKey(), StrEq("gauge.p95"));
  EXPECT_EQ(P95.Type, MetricMessageType::GaugeSet);
  EXPECT_NEAR(P95.SubmitDouble.Value, 1804.15, 0.001);

  // Free tags
  Tags TagHandler;
  TagHandler.Handle(FirstTagMessage);
  TagHandler.Handle(SecondTagMessage);
  TagHandler.Handle(RemoveTagMessage);
  for (auto &Message : Results.Values) {
    if (Message.IsTag()) {
      TagHandler.Handle(Message);
    }
  }
}

TEST(PercentilesTests,
     WhenTimerSetMessagesHandled_ThenEmitsTimerSetForEachPercentile) {
  std::vector<double> Values{{
      7901, 8119, 3189, 6244, 2526, 4869, 9574, 2763, 3794, 3609, 4526, 2573,
      170,  1932, 419,  6800, 6502, 2430, 5491, 8909, 3111, 1438, 7400, 6396,
      8044, 4163, 6913, 5147, 565,  9947, 5607, 9074, 6426, 3956, 1040, 1260,
      1775, 7013, 5663, 7958, 9565, 2940, 8122, 9169, 8259, 9503, 1284, 9827,
      7598, 9331, 949,  1133, 163,  994,  3482, 1334, 5915, 4539, 9560, 8228,
      8679, 37,   991,  2123, 2520, 6585, 2286, 1076, 9425, 518,  2694, 8381,
      6162, 6024, 8352, 8329, 6067, 2187, 266,  9514, 2530, 950,  9250, 3583,
      3280, 2641, 9523, 2306, 7449, 9238, 1622, 3984, 8370, 3488, 6236, 7277,
      3337, 5011, 590,  6658, 9689, 1013, 4592, 1702, 6950, 370,  3622, 9497,
      9119, 1281, 5200, 7824, 2558, 3028, 4206, 4136, 4533, 2343, 5387, 3098,
      5083, 3016, 8595, 8915, 7527, 6885, 1799, 2802, 1727, 6902, 6246, 1618,
      5092, 90,   9264, 1595, 2257, 4585, 8567, 1915, 8493, 9320, 1148, 5836,
      4578, 9155, 5089, 6777, 3286, 8190, 1610, 6687, 9526, 8351, 2341, 7252,
      4138, 9505, 1942, 6109, 6453, 1821, 9657, 8236, 5305, 3951, 8017, 2083,
      9294, 5443, 377,  7882, 1646, 485,  3326, 191,  4234, 1871, 9104, 8938,
      8289, 4780, 3751, 3415, 9361, 7981, 6719, 4402, 22,   2997, 7922, 7618,
      9709, 1436, 8648, 8927, 1977, 2614, 1248, 915,  7991, 7579, 5015, 4623,
      1660, 8181, 3543, 6110, 6553, 9847, 7579, 7288, 872,  5289, 3928, 4474,
      848,  8848, 1021, 4693, 1368, 5025, 3823, 9615, 4843, 1257, 6266, 5299,
      6866, 430,  3776, 9240, 249,  1385, 8176, 5514, 5455, 1576, 9466, 7067,
      6011, 5270, 9039, 977,  1313, 627,  5965, 6753, 6067, 450,  1554, 9202,
      557,  2571, 6293, 3042, 8915, 9583, 6953, 7335, 4003, 2754, 9937, 6900,
      5103, 7002, 5113, 3168, 9295, 3033, 6334, 1199, 350,  8791, 884,  5531,
      7531, 7086, 1615, 79,   6442, 3064, 499,  7491, 1381, 5712, 2722, 3350,
      4274, 3634, 4093, 9238, 8192, 4207, 1153, 2377, 9974, 2410, 6644, 699,
  }};

  auto Percentiles = Aws::GameLift::Metrics::Percentiles(0.01, 0.1, 0.25, 0.5,
                                                         0.8, 0.9, 0.95, 0.98);

  MockVectorEnqueuer Results;
  for (double Value : Values) {
    auto SetMessage = MetricMessage::TimerSet(MetricTimer::Instance(), Value);
    Percentiles.HandleMessage(SetMessage, Results);
  }
  Percentiles.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(8));

  MetricMessage P01 = Results.Values[0];
  EXPECT_THAT(P01.Metric->GetKey(), StrEq("timer.p01"));
  EXPECT_EQ(P01.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P01.SubmitDouble.Value, 89.89, 0.001);

  MetricMessage P10 = Results.Values[1];
  EXPECT_THAT(P10.Metric->GetKey(), StrEq("timer.p10"));
  EXPECT_EQ(P10.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P10.SubmitDouble.Value, 989.6, 0.001);

  MetricMessage P25 = Results.Values[2];
  EXPECT_THAT(P25.Metric->GetKey(), StrEq("timer.p25"));
  EXPECT_EQ(P25.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P25.SubmitDouble.Value, 2278.75, 0.001);

  MetricMessage P50 = Results.Values[3];
  EXPECT_THAT(P50.Metric->GetKey(), StrEq("timer.p50"));
  EXPECT_EQ(P50.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P50.SubmitDouble.Value, 5013, 0.001);

  MetricMessage P80 = Results.Values[4];
  EXPECT_THAT(P80.Metric->GetKey(), StrEq("timer.p80"));
  EXPECT_EQ(P80.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P80.SubmitDouble.Value, 8190.4, 0.001);

  MetricMessage P90 = Results.Values[5];
  EXPECT_THAT(P90.Metric->GetKey(), StrEq("timer.p90"));
  EXPECT_EQ(P90.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P90.SubmitDouble.Value, 9238, 0.001);

  MetricMessage P95 = Results.Values[6];
  EXPECT_THAT(P95.Metric->GetKey(), StrEq("timer.p95"));
  EXPECT_EQ(P95.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P95.SubmitDouble.Value, 9514.45, 0.001);

  MetricMessage P98 = Results.Values[7];
  EXPECT_THAT(P98.Metric->GetKey(), StrEq("timer.p98"));
  EXPECT_EQ(P98.Type, MetricMessageType::TimerSet);
  EXPECT_NEAR(P98.SubmitDouble.Value, 9689.4, 0.001);
}
