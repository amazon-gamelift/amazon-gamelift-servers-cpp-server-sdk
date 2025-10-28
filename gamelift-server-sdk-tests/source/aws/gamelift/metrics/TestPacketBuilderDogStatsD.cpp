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
#include "MetricMacrosTests.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iomanip>
#include <sstream>
#include <string>

#include <aws/gamelift/metrics/PacketBuilder.h>
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

struct PacketBuilderDogStatsDTests : public Test {
  std::stringstream Stream;

  PacketBuilderDogStatsDTests() {
    Stream << std::setprecision(3) << std::setiosflags(std::ios_base::fixed);
  }

  std::string Result() {
    Stream.flush();
    return Stream.str();
  }
};

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gaugor", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounter, "countor", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounter);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "glork", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);
} // namespace

TEST_F(PacketBuilderDogStatsDTests, GaugeSet) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5, {},
                 {}, Stream);
  EXPECT_EQ(Result(), "gaugor:10|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSetNegative) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), -10.0), 5, {},
                 {}, Stream);
  EXPECT_EQ(Result(), "gaugor:0|g\ngaugor:-10|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), 10.0), 5, {},
                 {}, Stream);
  EXPECT_EQ(Result(), "gaugor:+10|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSubtract) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), -10.0), 5, {},
                 {}, Stream);
  EXPECT_EQ(Result(), "gaugor:-10|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), 10.0), 5,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "countor:10|c\n");
}

TEST_F(PacketBuilderDogStatsDTests, NegativeCounterAdd) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), -10.0), 5,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "");
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet) {
  AppendToStream(MetricMessage::TimerSet(MetricTimer::Instance(), 2121), 5, {},
                 {}, Stream);
  EXPECT_EQ(Result(), "glork:2121|ms\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSet_Fraction3Digits) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 14.125), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:14.125|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSetNegative_Fraction3Digits) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), -14.125), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:0|g\ngaugor:-14.125|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd_Fraction3Digits) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), 14.125), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:+14.125|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSubtract_Fraction3Digits) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), -14.125), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:-14.125|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd_Fraction3Digits) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), 14.125),
                 3, {}, {}, Stream);
  EXPECT_EQ(Result(), "countor:14.125|c\n");
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet_Fraction3Digits) {
  AppendToStream(MetricMessage::TimerSet(MetricTimer::Instance(), 2121.125), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "glork:2121.125|ms\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSet_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 14.1256), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:14.126|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSetNegative_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), -14.1256), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:0|g\ngaugor:-14.126|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), 14.1256), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:+14.126|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSubtract_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), -14.1256), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:-14.126|g\n");
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), 14.1256),
                 3, {}, {}, Stream);
  EXPECT_EQ(Result(), "countor:14.126|c\n");
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet_Fraction3Digits_Rounded) {
  AppendToStream(MetricMessage::TimerSet(MetricTimer::Instance(), 2121.1256), 3,
                 {}, {}, Stream);
  EXPECT_EQ(Result(), "glork:2121.126|ms\n");
}

TEST_F(PacketBuilderDogStatsDTests,
       GaugeSet_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::GaugeSet(MetricGauge::Instance(), 14.0002522123), 3, {},
      {}, Stream);
  EXPECT_EQ(Result(), "gaugor:14|g\n");
}

TEST_F(
    PacketBuilderDogStatsDTests,
    GaugeSetNegative_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::GaugeSet(MetricGauge::Instance(), -14.0002522123), 3, {},
      {}, Stream);
  EXPECT_EQ(Result(), "gaugor:0|g\ngaugor:-14|g\n");
}

TEST_F(PacketBuilderDogStatsDTests,
       GaugeAdd_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 14.0002522123), 3, {},
      {}, Stream);
  EXPECT_EQ(Result(), "gaugor:+14|g\n");
}

TEST_F(PacketBuilderDogStatsDTests,
       GaugeSubtract_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::GaugeAdd(MetricGauge::Instance(), -14.0002522123), 3, {},
      {}, Stream);
  EXPECT_EQ(Result(), "gaugor:-14|g\n");
}

TEST_F(PacketBuilderDogStatsDTests,
       CounterAdd_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::CounterAdd(MetricCounter::Instance(), 14.0002522123), 3,
      {}, {}, Stream);
  EXPECT_EQ(Result(), "countor:14|c\n");
}

TEST_F(PacketBuilderDogStatsDTests,
       TimerSet_Fraction3Digits_FractionBelowDisplaySizeTurnsToInteger) {
  AppendToStream(
      MetricMessage::TimerSet(MetricTimer::Instance(), 2121.0002522123), 3, {},
      {}, Stream);
  EXPECT_EQ(Result(), "glork:2121|ms\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSet_WithGlobalTags) {
  AppendToStream(
      MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);

  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests,
       GaugeSetNegative_WithGlobalTags_BothCommandsIncludeTags) {
  AppendToStream(
      MetricMessage::GaugeSet(MetricGauge::Instance(), -10.0), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);
  const std::string result = Result();

  // Should container two metric packets, separated by newline.
  const size_t newline_pos = result.find('\n');
  ASSERT_NE(newline_pos, std::string::npos);

  const std::string line1 = result.substr(0, newline_pos);
  const std::string line2 = result.substr(newline_pos + 1, result.find('\n', newline_pos + 1) - (newline_pos + 1));

  // Static checks.
  EXPECT_THAT(line1, StartsWith("gaugor:0|g|#"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(line1, HasSubstr("hello:world"));
  EXPECT_THAT(line1, HasSubstr("tag_two:foo"));

  const std::string tags_part1 = line1.substr(line1.find('#') + 1);
  EXPECT_EQ(std::count(tags_part1.begin(), tags_part1.end(), ','), 1);

  // Static checks.
  EXPECT_THAT(line2, StartsWith("gaugor:-10|g|#"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(line2, HasSubstr("hello:world"));
  EXPECT_THAT(line2, HasSubstr("tag_two:foo"));

  const std::string tags_part2 = line2.substr(line2.find('#') + 1);
  EXPECT_EQ(std::count(tags_part2.begin(), tags_part2.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd_WithGlobalTags) {
  AppendToStream(
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 10.0), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:+10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSubtract_WithGlobalTags) {
  AppendToStream(
      MetricMessage::GaugeAdd(MetricGauge::Instance(), -10.0), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:-10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd_WithGlobalTags) {
  AppendToStream(
      MetricMessage::CounterAdd(MetricCounter::Instance(), 10.0), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("countor:10|c|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet_WithGlobalTags) {
  AppendToStream(
      MetricMessage::TimerSet(MetricTimer::Instance(), 2121), 5,
      {std::make_pair("hello", "world"), std::make_pair("tag_two", "foo")}, {},
      Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("glork:2121|ms|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSet_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:10|g|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSetNegative_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), -10.0), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:0|g|#hello:world\ngaugor:-10|g|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:+10|g|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSubtract_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), -10.0), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:-10|g|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "countor:10|c|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet_WithSingleGlobalTag) {
  AppendToStream(MetricMessage::TimerSet(MetricTimer::Instance(), 2121), 5,
                 {std::make_pair("hello", "world")}, {}, Stream);
  EXPECT_EQ(Result(), "glork:2121|ms|#hello:world\n");
}

TEST_F(PacketBuilderDogStatsDTests, GaugeSet_WithGlobalTagsAndPetMetricTags) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(
    PacketBuilderDogStatsDTests,
    GaugeSetNegative_WithGlobalTagsAndPerMetricTags_BothCommandsIncludeTags) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), -10.0), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  const size_t newline_pos = result.find('\n');
  ASSERT_NE(newline_pos, std::string::npos);

  const std::string line1 = result.substr(0, newline_pos);
  const std::string line2 = result.substr(newline_pos + 1, result.find('\n', newline_pos + 1) - (newline_pos + 1));

  // Static checks.
  EXPECT_THAT(line1, StartsWith("gaugor:0|g|#"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(line1, HasSubstr("hello:world"));
  EXPECT_THAT(line1, HasSubstr("tag_two:foo"));
  const std::string tags_part1 = line1.substr(line1.find('#') + 1);
  EXPECT_EQ(std::count(tags_part1.begin(), tags_part1.end(), ','), 1);

  // Static checks.
  EXPECT_THAT(line2, StartsWith("gaugor:-10|g|#"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(line2, HasSubstr("hello:world"));
  EXPECT_THAT(line2, HasSubstr("tag_two:foo"));
  const std::string tags_part2 = line2.substr(line2.find('#') + 1);
  EXPECT_EQ(std::count(tags_part2.begin(), tags_part2.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, GaugeAdd_WithGlobalTagsAndPerMetricTags) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:+10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests,
       GaugeSubtract_WithGlobalTagsAndPerMetricTags) {
  AppendToStream(MetricMessage::GaugeAdd(MetricGauge::Instance(), -10.0), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("gaugor:-10|g|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, CounterAdd_WithGlobalTagsAndPerMetricTags) {
  AppendToStream(MetricMessage::CounterAdd(MetricCounter::Instance(), 10.0), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("countor:10|c|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests, TimerSet_WithGlobalTagsAndPerMetricTags) {
  AppendToStream(MetricMessage::TimerSet(MetricTimer::Instance(), 2121), 5,
                 {std::make_pair("hello", "world")},
                 {std::make_pair("tag_two", "foo")}, Stream);
  const std::string result = Result();

  // Static checks.
  EXPECT_THAT(result, StartsWith("glork:2121|ms|#"));
  EXPECT_THAT(result, EndsWith("\n"));

  // Tags should be present irrespective of order, comma-separated.
  EXPECT_THAT(result, HasSubstr("hello:world"));
  EXPECT_THAT(result, HasSubstr("tag_two:foo"));

  const std::string tags_part = result.substr(result.find('#') + 1);
  EXPECT_EQ(std::count(tags_part.begin(), tags_part.end(), ','), 1);
}

TEST_F(PacketBuilderDogStatsDTests,
       WhenGaugeSetWithGlobalTags_ThenNoTrailingCommaInTags) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("foo", "bar")}, {}, Stream);
  EXPECT_EQ(Result(), "gaugor:10|g|#foo:bar\n");
}

TEST_F(
    PacketBuilderDogStatsDTests,
    WhenGaugeSetWithGlobalTagsAndPerMetricTags_ThenMetricsCombinedWithComma) {
  AppendToStream(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0), 5,
                 {std::make_pair("foo", "bar")}, {std::make_pair("a", "b")},
                 Stream);
  EXPECT_EQ(Result(), "gaugor:10|g|#foo:bar,a:b\n");
}
