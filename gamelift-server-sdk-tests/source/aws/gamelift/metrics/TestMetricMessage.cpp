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

#include <aws/gamelift/metrics/DynamicTag.h>
#include <aws/gamelift/metrics/Tags.h>
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/Samplers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(MetricCounter, "counter", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "timer", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());

GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);
GAMELIFT_METRICS_DEFINE_COUNTER(MetricCounter);
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);
} // namespace

TEST(MetricMessageTests, GaugeSet) {
  MetricMessage Message =
      MetricMessage::GaugeSet(MetricGauge::Instance(), 21.0);
  EXPECT_EQ(Message.Type, MetricMessageType::GaugeSet);
  EXPECT_EQ(Message.Metric, &MetricGauge::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("gauge"));
  EXPECT_EQ(Message.SubmitDouble.Value, 21.0);
}

TEST(MetricMessageTests, GaugeAdd) {
  MetricMessage Message =
      MetricMessage::GaugeAdd(MetricGauge::Instance(), 11.0);
  EXPECT_EQ(Message.Type, MetricMessageType::GaugeAdd);
  EXPECT_EQ(Message.Metric, &MetricGauge::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("gauge"));
  EXPECT_EQ(Message.SubmitDouble.Value, 11.0);
}

TEST(MetricMessageTests, CounterAdd) {
  MetricMessage Message =
      MetricMessage::CounterAdd(MetricCounter::Instance(), 7.0);
  EXPECT_EQ(Message.Type, MetricMessageType::CounterAdd);
  EXPECT_EQ(Message.Metric, &MetricCounter::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("counter"));
  EXPECT_EQ(Message.SubmitDouble.Value, 7.0);
}

TEST(MetricMessageTests, TimerSet) {
  MetricMessage Message =
      MetricMessage::TimerSet(MetricTimer::Instance(), 100.0);
  EXPECT_EQ(Message.Type, MetricMessageType::TimerSet);
  EXPECT_EQ(Message.Metric, &MetricTimer::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("timer"));
  EXPECT_EQ(Message.SubmitDouble.Value, 100.0);
}

TEST(MetricMessageTests, TagSet) {
  MetricMessage Message =
      MetricMessage::TagSet(MetricGauge::Instance(), "foo", "bar");
  EXPECT_EQ(Message.Type, MetricMessageType::TagSet);
  EXPECT_EQ(Message.Metric, &MetricGauge::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("gauge"));
  EXPECT_THAT(Message.SetTag.Ptr->Key, StrEq("foo"));
  EXPECT_THAT(Message.SetTag.Ptr->Value, StrEq("bar"));

  // We need to 'handle' the message to avoid a memory leak.
  Tags TagHandler;
  TagHandler.Handle(Message);
}

TEST(MetricMessageTests, TagRemove) {
  MetricMessage Message =
      MetricMessage::TagRemove(MetricGauge::Instance(), "foo");
  EXPECT_EQ(Message.Type, MetricMessageType::TagRemove);
  EXPECT_EQ(Message.Metric, &MetricGauge::Instance());
  EXPECT_THAT(Message.Metric->GetKey(), StrEq("gauge"));
  EXPECT_THAT(Message.SetTag.Ptr->Key, StrEq("foo"));

  // We need to 'handle' the message to avoid a memory leak.
  Tags TagHandler;
  TagHandler.Handle(Message);
}
