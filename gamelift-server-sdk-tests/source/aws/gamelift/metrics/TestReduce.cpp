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
#include <aws/gamelift/metrics/ReduceMetric.h>
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

TEST(
    ReducerTests,
    GivenMaxReducer_WhenGivenGaugeAddMessages_ThenEmitsCorrectMaxGaugeMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 30),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 20), // value is now 50
       MetricMessage::GaugeSet(MetricGauge::Instance(), 40),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 21), // value is now 61
       MetricMessage::GaugeSet(MetricGauge::Instance(), 5)}};

  Aws::GameLift::Metrics::Max MaxReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MaxReducer.HandleMessage(Message, Results);
  }
  MaxReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 61);
}

TEST(ReducerTests,
     GivenMaxReducer_WhenGivenTags_ThenAppliesTagsToReducedMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 30),
       MetricMessage::TagSet(MetricGauge::Instance(), "hello", "world"),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 20), // value is now 50
       MetricMessage::TagSet(MetricGauge::Instance(), "foo", "bar"),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 40),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 21), // value is now 61
       MetricMessage::TagRemove(MetricGauge::Instance(), "foo"),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 5)}};

  Aws::GameLift::Metrics::Max MaxReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MaxReducer.HandleMessage(Message, Results);
  }
  MaxReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(4));

  MetricMessage FirstTag = Results.Values[0];
  EXPECT_THAT(FirstTag.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_EQ(FirstTag.Type, MetricMessageType::TagSet);
  EXPECT_THAT(FirstTag.SetTag.Ptr->Key, StrEq("hello"));
  EXPECT_THAT(FirstTag.SetTag.Ptr->Value, StrEq("world"));

  MetricMessage SecondTag = Results.Values[1];
  EXPECT_THAT(SecondTag.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_EQ(SecondTag.Type, MetricMessageType::TagSet);
  EXPECT_THAT(SecondTag.SetTag.Ptr->Key, StrEq("foo"));
  EXPECT_THAT(SecondTag.SetTag.Ptr->Value, StrEq("bar"));

  MetricMessage RemoveTag = Results.Values[2];
  EXPECT_THAT(RemoveTag.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_EQ(RemoveTag.Type, MetricMessageType::TagRemove);
  EXPECT_THAT(RemoveTag.SetTag.Ptr->Key, StrEq("foo"));

  MetricMessage ResultMessage = Results.Values[3];
  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 61);

  // Free tags
  Tags TagHandler;
  for (auto &Message : Messages) {
    if (Message.IsTag()) {
      TagHandler.Handle(Message);
    }
  }
  for (auto &Message : Results.Values) {
    if (Message.IsTag()) {
      TagHandler.Handle(Message);
    }
  }
}

TEST(ReducerTests,
     GivenMaxReducer_WhenGivenGaugeMessages_ThenEmitsMaxGaugeMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Max MaxReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MaxReducer.HandleMessage(Message, Results);
  }
  MaxReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.max"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 56);
}

TEST(ReducerTests,
     GivenMinReducer_WhenGivenGaugeMessages_ThenEmitsMinGaugeMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Min MinReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MinReducer.HandleMessage(Message, Results);
  }
  MinReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.min"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, -12);
}

TEST(
    ReducerTests,
    GivenMaxReducerWithCustomSuffix_WhenGivenGaugeMessages_ThenEmitsMaxGaugeMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Max MaxReducer(".foo");

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MaxReducer.HandleMessage(Message, Results);
  }
  MaxReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.foo"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 56);
}

TEST(
    ReducerTests,
    GivenMinReducerWithCustomSuffix_WhenGivenGaugeMessages_ThenEmitsMinGaugeMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Min MinReducer(".bar");

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MinReducer.HandleMessage(Message, Results);
  }
  MinReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.bar"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, -12);
}

TEST(ReducerTests,
     GivenMaxReducer_WhenGivenTimerMessages_ThenEmitsMaxTimerMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 20),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), -12)}};

  Aws::GameLift::Metrics::Max MaxReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MaxReducer.HandleMessage(Message, Results);
  }
  MaxReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.max"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 56);
}

TEST(ReducerTests,
     GivenMinReducer_WhenGivenTimerMessages_ThenEmitsMinTimerMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 20),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), -12)}};

  Aws::GameLift::Metrics::Min MinReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    MinReducer.HandleMessage(Message, Results);
  }
  MinReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.min"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, -12);
}

namespace {
struct OpMockSum final {
  double operator()(double Current, double New) { return Current + New; }
};
} // namespace

TEST(ReducerTests,
     GivenMockSumReducer_WhenGivenGaugeMessages_ThenEmitsGaugeSumMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Reduce<OpMockSum> SumReducer(".sum");

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    SumReducer.HandleMessage(Message, Results);
  }
  SumReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.sum"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 101);
}

TEST(ReducerTests,
     GivenMockSumReducer_WhenGivenTimerMessages_ThenEmitsTimerSumMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 20),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), -12)}};

  Aws::GameLift::Metrics::Reduce<OpMockSum> SumReducer(".sum");

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    SumReducer.HandleMessage(Message, Results);
  }
  SumReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.sum"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 101);
}

TEST(ReducerTests,
     GivenSumReducer_WhenGivenGaugeMessages_ThenEmitsGaugeSumMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 20),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12)}};

  Aws::GameLift::Metrics::Sum SumReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    SumReducer.HandleMessage(Message, Results);
  }
  SumReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.sum"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 101);
}

TEST(ReducerTests,
     GivenSumReducer_WhenGivenTimerMessages_ThenEmitsTimerSumMessage) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 20),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), -12)}};

  Aws::GameLift::Metrics::Sum SumReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    SumReducer.HandleMessage(Message, Results);
  }
  SumReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.sum"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 101);
}

TEST(ReducerTests,
     GivenCountReducer_WhenGiven6GaugeMessages_ThenEmitsGaugeCountOf6) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), -1),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 22),
       MetricMessage::GaugeSet(MetricGauge::Instance(), -12),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), 10)}};

  Aws::GameLift::Metrics::Count CountReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    CountReducer.HandleMessage(Message, Results);
  }
  CountReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.count"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 6);
}

TEST(ReducerTests,
     GivenCountReducer_WhenGiven3GaugeMessages_ThenEmitsGaugeCountOf3) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::GaugeSet(MetricGauge::Instance(), 15),
       MetricMessage::GaugeAdd(MetricGauge::Instance(), -1),
       MetricMessage::GaugeSet(MetricGauge::Instance(), 56)}};

  Aws::GameLift::Metrics::Count CountReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    CountReducer.HandleMessage(Message, Results);
  }
  CountReducer.EmitMetrics(&MetricGauge::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("gauge.count"));
  ASSERT_TRUE(ResultMessage.IsGauge());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 3);
}

TEST(ReducerTests,
     GivenCountReducer_WhenGiven4TimerMessages_ThenEmitsTimerCountOf4) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), 10)}};

  Aws::GameLift::Metrics::Count CountReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    CountReducer.HandleMessage(Message, Results);
  }
  CountReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.count"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 4);
}

TEST(ReducerTests,
     GivenCountReducer_WhenGiven7TimerMessages_ThenEmitsTimerCountOf7) {
  std::vector<MetricMessage> Messages{
      {MetricMessage::TimerSet(MetricTimer::Instance(), 15),
       MetricMessage::TimerSet(MetricTimer::Instance(), 56),
       MetricMessage::TimerSet(MetricTimer::Instance(), 22),
       MetricMessage::TimerSet(MetricTimer::Instance(), 100),
       MetricMessage::TimerSet(MetricTimer::Instance(), 501),
       MetricMessage::TimerSet(MetricTimer::Instance(), 82),
       MetricMessage::TimerSet(MetricTimer::Instance(), 10)}};

  Aws::GameLift::Metrics::Count CountReducer;

  MockVectorEnqueuer Results;
  for (auto &Message : Messages) {
    CountReducer.HandleMessage(Message, Results);
  }
  CountReducer.EmitMetrics(&MetricTimer::Instance(), Results);

  ASSERT_THAT(Results.Values, SizeIs(1));
  MetricMessage ResultMessage = Results.Values[0];

  EXPECT_THAT(ResultMessage.Metric->GetKey(), StrEq("timer.count"));
  ASSERT_TRUE(ResultMessage.IsTimer());
  EXPECT_EQ(ResultMessage.SubmitDouble.Value, 7);
}
