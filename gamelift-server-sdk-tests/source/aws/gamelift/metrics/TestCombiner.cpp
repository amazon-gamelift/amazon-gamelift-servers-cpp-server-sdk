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

#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>

#include <aws/gamelift/metrics/Combiner.h>
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge2, "gauge_2", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge2);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounter, "counter", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounter2, "counter_2", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounter);
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounter2);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "timer", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer2, "timer_2", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer2);
} // namespace

TEST(CombinerTests, ItemsAddedAndCleared) {
  Combiner MetricsCombiner;
  EXPECT_THAT(MetricsCombiner, IsEmpty());

  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge2::Instance(), 12.0));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), 4.0));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 16.0));

  EXPECT_THAT(MetricsCombiner,
              UnorderedElementsAre(
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 10.0),
                  MetricMessage::GaugeSet(MetricGauge2::Instance(), 12.0),
                  MetricMessage::CounterAdd(MetricCounter::Instance(), 4.0),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 16.0)));

  MetricsCombiner.Clear();

  EXPECT_THAT(MetricsCombiner, IsEmpty());
}

TEST(CombinerGaugeTests, WhenGaugeSetTwice_ThenLatestGaugeKept) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 42));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 42)));
}

TEST(CombinerGaugeTests, WhenGaugeSetAndValueAdded_ThenValueAddedToSetCommand) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 5));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), -1));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 14)));
}

TEST(CombinerGaugeTests, WhenGaugeValuesAdded_ThenValuesSummedIntoASetCommand) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 5));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 5));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 2));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 12)));
}

TEST(CombinerGaugeTests,
     WhenAddedAndSubtracted_ThenValuesSummedIntoNegativeSetCommand) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 5));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), -10));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), -5)));
}

TEST(CombinerGaugeTests,
     GivenCombinerWithGauge_WhenCombinerCleared_ThenCombinerSeenAsEmpty) {
  Combiner MetricsCombiner;
  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), -5));

  ASSERT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), -5)));
  MetricsCombiner.Clear();

  /*
   * MetricsCombiner should be empty now because we haven't submitted any
   * metrics since clearing it. However, it should still remember what state the
   * gauge is in.
   */
  EXPECT_THAT(MetricsCombiner, IsEmpty());
}

TEST(
    CombinerGaugeTests,
    GivenCombinerWithGauge_WhenCombinerClearedAndGaugeIsAdded_ThenValueAddedToExistingGaugeValue) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 7));
  ASSERT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 7)));
  MetricsCombiner.Clear();

  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 5));
  ASSERT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 5)));
  MetricsCombiner.Clear();

  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 10));
  MetricsCombiner.Add(MetricMessage::GaugeAdd(MetricGauge::Instance(), 11));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 26)));
}

TEST(
    CombinerGaugeTests,
    GivenCombinerWithGauge_WhenCombinerClearedAndGaugeIsSet_ThenLatestValueKept) {
  Combiner MetricsCombiner;
  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 65));

  ASSERT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 65)));
  MetricsCombiner.Clear();
  MetricsCombiner.Add(MetricMessage::GaugeSet(MetricGauge::Instance(), 17));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::GaugeSet(
                                   MetricGauge::Instance(), 17)));
}

TEST(CombinerCounterTests, WhenCounterIsAddedOnce_ThenCounterIsKept) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 11));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::CounterAdd(
                                   MetricCounter::Instance(), 11)));
}

TEST(CombinerCounterTests, WhenCounterIsAddedTwice_ThenAdditionsAreSummed) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 11));
  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 24));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::CounterAdd(
                                   MetricCounter::Instance(), 35)));
}

TEST(
    CombinerCounterTests,
    GivenTwoCounters_WhenCountersAreAddedManyTimes_ThenCountersIndividuallySummed) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 39));
  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 76));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -16));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -13));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -41));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 94));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), -79));
  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 39));
  MetricsCombiner.Add(MetricMessage::CounterAdd(MetricCounter::Instance(), 22));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 20));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 16));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), -96));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -95));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -31));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), -74));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 54));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -50));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), -97));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter::Instance(), -72));
  MetricsCombiner.Add(
      MetricMessage::CounterAdd(MetricCounter2::Instance(), 65));

  EXPECT_THAT(MetricsCombiner,
              UnorderedElementsAre(
                  MetricMessage::CounterAdd(MetricCounter::Instance(), -181),
                  MetricMessage::CounterAdd(MetricCounter2::Instance(), -58)));
}

TEST(CombinerTimerTests, WhenTimerIsSetOnce_ThenOriginalValueIsKept) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 10));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::TimerSet(
                                   MetricTimer::Instance(), 10)));
}

TEST(CombinerTimerTests, WhenTimerIsSetTwice_ThenMeanIsKept) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 10));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 30));

  EXPECT_THAT(MetricsCombiner, UnorderedElementsAre(MetricMessage::TimerSet(
                                   MetricTimer::Instance(), 20)));
}

TEST(CombinerTimerTests,
     GivenTwoTimers_WhenTimersAreSetNTimes_ThenMeansAreKept) {
  Combiner MetricsCombiner;

  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 60));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 25));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 68));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 89));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 90));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 97));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 42));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 77));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 9));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 15));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 82));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 95));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 30));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 66));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 20));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 9));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 14));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 36));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer::Instance(), 35));
  MetricsCombiner.Add(MetricMessage::TimerSet(MetricTimer2::Instance(), 48));

  ASSERT_THAT(MetricsCombiner, SizeIs(2));

  std::unordered_map<const Aws::GameLift::Metrics::IMetric *, MetricMessage>
      Results;
  std::transform(std::begin(MetricsCombiner), std::end(MetricsCombiner),
                 std::inserter(Results, std::end(Results)), [](const Aws::GameLift::Metrics::MetricMessage &Message) {
                   return std::make_pair(Message.Metric, Message);
                 });

  EXPECT_EQ(Results[&MetricTimer::Instance()].Type,
            MetricMessageType::TimerSet);
  EXPECT_EQ(Results[&MetricTimer2::Instance()].Type,
            MetricMessageType::TimerSet);

  EXPECT_THAT(Results[&MetricTimer::Instance()].SubmitDouble.Value,
              DoubleEq(46.3));
  EXPECT_THAT(Results[&MetricTimer2::Instance()].SubmitDouble.Value,
              DoubleEq(54.4));
}
