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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include <aws/gamelift/metrics/PacketBuilder.h>
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

struct PacketBuilderTests : public PacketSendTest {};

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGaugor, "gaugor", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGaugor);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricBarGauge, "bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricBarGauge);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricFooGauge, "foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFooGauge);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricBazGauge, "baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricBazGauge);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricBarCounter, "bar", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricBarCounter);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricBazCounter, "baz", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricBazCounter);

GAMELIFT_METRICS_DECLARE_TIMER(MetricBazTimer, "baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricBazTimer);

GAMELIFT_METRICS_DECLARE_TIMER(MetricBarTimer, "bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricBarTimer);

// Metric with SampleFraction for testing sample rate
GAMELIFT_METRICS_DECLARE_GAUGE(MetricHalfSampledGauge, "half_sampled_gauge", MockEnabled,
                                Aws::GameLift::Metrics::SampleFraction(0.5));
GAMELIFT_METRICS_DEFINE_GAUGE(MetricHalfSampledGauge);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricQuarterSampledGauge, "quarter_sampled_gauge", MockEnabled,
                                Aws::GameLift::Metrics::SampleFraction(0.25));
GAMELIFT_METRICS_DEFINE_GAUGE(MetricQuarterSampledGauge);
} // namespace

TEST_F(PacketBuilderTests, WhenConstructed_ReportsCorrectValues) {
  {
    PacketBuilder A(1234);

    EXPECT_GE(A.GetFloatPrecision(), 0); // default must be greater or equal to 0
    EXPECT_EQ(A.GetPacketSize(), 1234);
  }

  {
    PacketBuilder B(1234, 10);

    EXPECT_EQ(B.GetFloatPrecision(), 10);
    EXPECT_EQ(B.GetPacketSize(), 1234);
  }
}

TEST_F(PacketBuilderTests, WhenFlushingEmptyBuilder_ThenGetsNullTerminator) {
  constexpr int PacketSize = 30;
  constexpr int Precision = 3;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Flush(MockSend);

  ASSERT_THAT(OutputPackets, ElementsAre(MakeResult("", 1)));

  ASSERT_EQ(*std::get<0>(OutputPackets[0]).data(), '\0');
}

TEST_F(
    PacketBuilderTests,
    WhenGivenManyMetrics_ThenSendsThreePacketsInAppend_ThenSendsOnePacketWhenFlushing) {
  constexpr int PacketSize = 25;
  constexpr int Precision = 3;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(MetricMessage::GaugeSet(MetricGaugor::Instance(), 10.0), {},
                 {}, MockSend); // gaugor:10|g\n
  Builder.Append(MetricMessage::GaugeSet(MetricBarGauge::Instance(), 5.0), {},
                 {}, MockSend); // bar:5|g\n
  // new packet
  Builder.Append(MetricMessage::GaugeAdd(MetricBazGauge::Instance(), 5.0),
                 {std::make_pair("abcde", "fghijk")}, {},
                 MockSend); // baz:+5|g|#abcde:fghijk\n
  // new packet
  Builder.Append(MetricMessage::CounterAdd(MetricBarCounter::Instance(), 5.0),
                 {}, {}, MockSend); // bar:5|c\n
  Builder.Append(MetricMessage::TimerSet(MetricBazTimer::Instance(), 5.0), {},
                 {}, MockSend); // baz:10|ms\n
  // new packet
  Builder.Append(MetricMessage::TimerSet(MetricBarTimer::Instance(), 121.0), {},
                 {}, MockSend); // bar:121|ms\n

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("gaugor:10|g\nbar:5|g\n", 21),
                          MakeResult("baz:+5|g|#abcde:fghijk\n", 24),
                          MakeResult("bar:5|c\nbaz:5|ms\n", 18)));

  ClearOutputPackets();
  Builder.Flush(MockSend);
  EXPECT_THAT(OutputPackets, ElementsAre(MakeResult("bar:121|ms\n", 12)));
}

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricLong,
                               "thisisaverylongmetricthatoverflowsthepacket",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricLong);
} // namespace

TEST_F(PacketBuilderTests, WhenAMetricIsTooLong_ThenMetricIsSkipped) {
  constexpr int PacketSize = 24;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(MetricMessage::GaugeSet(MetricGaugor::Instance(), 5.0), {}, {},
                 MockSend);
  Builder.Append(MetricMessage::GaugeSet(MetricLong::Instance(), 5.0), {}, {},
                 MockSend);
  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 11.0), {},
                 {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("gaugor:5|g\nfoo:11|g\n", 21)));
}

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricFits, "aaaaaaaaaaaaaaaaaaaaaaa",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFits);
} // namespace

TEST_F(PacketBuilderTests,
       WhenAMetricIsTooLongDueToValueFormatting_ThenMetricIsSkipped) {
  constexpr int PacketSize = 30;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  // The key is long enough to fit but the formatting for 5.12 will blow out the
  // packet
  Builder.Append(MetricMessage::GaugeSet(MetricFits::Instance(), 5.12), {}, {},
                 MockSend);
  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 11.0), {},
                 {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets, ElementsAre(MakeResult("foo:11|g\n", 10)));
}

TEST_F(PacketBuilderTests, WhenAMetricIsTooLongDueToTags_ThenMetricIsSkipped) {
  constexpr int PacketSize = 12;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  // The first metric won't fit due to the tags being too long
  Builder.Append(MetricMessage::GaugeSet(MetricBarGauge::Instance(), 5.0),
                 {std::make_pair("abcdef", "ghijklmopqr")}, {}, MockSend);
  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 11.0), {},
                 {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets, ElementsAre(MakeResult("foo:11|g\n", 10)));
}

TEST_F(PacketBuilderTests,
       GivenPrecisionIs3_WhenGivenFloatMessages_ThenThreeDigitsAfterDecimal) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 3;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 10.12365),
                 {}, {}, MockSend);
  Builder.Append(MetricMessage::GaugeAdd(MetricBarGauge::Instance(), 10.51423),
                 {}, {}, MockSend);
  Builder.Append(MetricMessage::GaugeAdd(MetricBazGauge::Instance(), -5.14061),
                 {}, {}, MockSend);
  Builder.Append(
      MetricMessage::CounterAdd(MetricBazCounter::Instance(), 10.51888881), {},
      {}, MockSend);
  Builder.Append(
      MetricMessage::CounterAdd(MetricBarCounter::Instance(), 5.0121241), {},
      {}, MockSend);
  Builder.Append(MetricMessage::TimerSet(MetricBarTimer::Instance(), 10.51199),
                 {}, {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(
      OutputPackets,
      ElementsAre(MakeResult("foo:10.124|g\nbar:+10.514|g\nbaz:-5.141|g\nbaz:"
                             "10.519|c\nbar:5.012|c\nbar:10.512|ms\n",
                             80)));
}

TEST_F(PacketBuilderTests,
       GivenPrecisionIs7_WhenGivenFloatMessages_ThenSevenDigitsAfterDecimal) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 7;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 10.12365),
                 {}, {}, MockSend);
  Builder.Append(MetricMessage::GaugeAdd(MetricBarGauge::Instance(), 10.51423),
                 {}, {}, MockSend);
  Builder.Append(MetricMessage::GaugeAdd(MetricBazGauge::Instance(), -5.14061),
                 {}, {}, MockSend);
  Builder.Append(
      MetricMessage::CounterAdd(MetricBazCounter::Instance(), 10.51888881), {},
      {}, MockSend);
  Builder.Append(
      MetricMessage::CounterAdd(MetricBarCounter::Instance(), 5.0121241), {},
      {}, MockSend);
  Builder.Append(MetricMessage::TimerSet(MetricBarTimer::Instance(), 10.51199),
                 {}, {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult(
                  "foo:10.1236500|g\nbar:+10.5142300|g\nbaz:-5.1406100|g\nbaz:"
                  "10.5188888|c\nbar:5.0121241|c\nbar:10.5119900|ms\n",
                  104)));
}

TEST_F(
    PacketBuilderTests,
    GivenPrecisionIs2_WhenFloatWithVerySmallFractionalPartAppended_ThenEmitsInteger) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(
      MetricMessage::GaugeSet(MetricFooGauge::Instance(), 1.00000002), {}, {},
      MockSend);
  Builder.Append(
      MetricMessage::TimerSet(MetricBarTimer::Instance(), 1.00000002), {}, {},
      MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("foo:1|g\nbar:1|ms\n", 18)));
}

TEST_F(
    PacketBuilderTests,
    GivenPrecisionIs2_WhenFloatWith3DigitFractionAppended_ThenEmitsRoundedFloat) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 1.425), {},
                 {}, MockSend);
  Builder.Append(MetricMessage::TimerSet(MetricBarTimer::Instance(), 1.425), {},
                 {}, MockSend);
  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("foo:1.43|g\nbar:1.43|ms\n", 24)));
}

TEST_F(PacketBuilderTests,
       GivenSampleFractionMetric_WhenAppended_ThenEmitsSampleRateInPacket) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  // Test with SampleFraction metric - should include |@0.5
  Builder.Append(MetricMessage::GaugeSet(MetricHalfSampledGauge::Instance(), 42.0), {},
                 {}, MockSend);

  // Test with SampleFraction metric - should include |@0.25
  Builder.Append(MetricMessage::GaugeSet(MetricQuarterSampledGauge::Instance(), 42.0), {},
                 {}, MockSend);

  // Test with SampleAll metric - should not include sample rate
  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 100.0), {},
                 {}, MockSend);

  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("half_sampled_gauge:42|g|@0.5\nquarter_sampled_gauge:42|g|@0.25\nfoo:100|g\n", 73)));
}

TEST_F(PacketBuilderTests,
       GivenSampleFractionMetricWithTags_WhenAppended_ThenEmitsBothTagsAndSampleRate) {
  constexpr int PacketSize = 10000;
  constexpr int Precision = 2;
  PacketBuilder Builder(PacketSize, Precision);

  // Test SampleFraction metric with per-metric tags - should include both tags and sample rate
  Builder.Append(MetricMessage::GaugeSet(MetricHalfSampledGauge::Instance(), 42.0),
                 {std::make_pair("env", "prod")},
                 {}, MockSend);

  // Test SampleFraction metric with global tags
  Builder.Append(MetricMessage::GaugeSet(MetricQuarterSampledGauge::Instance(), 100.0),
                 {},
                 {std::make_pair("app", "gameserver")}, MockSend);

  // Test SampleAll metric with tags - should include tags but no sample rate
  Builder.Append(MetricMessage::GaugeSet(MetricFooGauge::Instance(), 25.0),
                 {std::make_pair("metric", "test")},
                 {}, MockSend);

  Builder.Flush(MockSend);

  EXPECT_THAT(OutputPackets,
              ElementsAre(MakeResult("half_sampled_gauge:42|g|@0.5|#env:prod\nquarter_sampled_gauge:100|g|@0.25|#app:gameserver\nfoo:25|g|#metric:test\n", 112)));
}
