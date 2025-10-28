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
#include "MockSampleEveryOther.h"
#include "MockDerivedMetric.h"

#include <aws/gamelift/metrics/GameLiftMetrics.h>
#include <aws/gamelift/metrics/DerivedMetric.h>

using namespace ::testing;
using namespace Aws::GameLift::Metrics;

class ArithmeticMacrosTests : public MetricMacrosTests {};

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricFooGauge, "foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFooGauge);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricFooDisabled, "no-foo", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFooDisabled);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricFooSampled, "foo_with_sampler",
                               MockEnabled, MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFooSampled);
} // namespace

namespace {
GAMELIFT_METRICS_DECLARE_COUNTER(MetricBarCounter, "bar", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricBarCounter);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricBarDisabled, "no-bar", MockDisabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricBarDisabled);

GAMELIFT_METRICS_DECLARE_COUNTER(MetricBarSampled, "bar_with_sampler",
                                 MockEnabled, MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_COUNTER(MetricBarSampled);
} // namespace

namespace {
int Returns4() { return 4; }

struct Returns6Functor {
  int operator()() { return 6; }
};
} // namespace

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenAddCalled_ThenAddMessageEnqueued) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooGauge::Instance(), 42)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricFooGauge, 42);
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetricWithSampler_WhenAddCalled_ThenAlwaysExecutesExpression) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
  
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_ADD(MetricFooSampled, Object.Func(20));
  GAMELIFT_METRICS_ADD(MetricFooSampled, Object.Func(30));
  GAMELIFT_METRICS_ADD(MetricFooSampled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenAddSampledCalled_ThenOnlyExecutesExpressionWhenSampled) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, Object.Func(20));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, Object.Func(30));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, Object.Func(40));
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenSubtractCalled_ThenNegativeAddMessageEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooGauge::Instance(), -121)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SUBTRACT(MetricFooGauge, 121);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenSubtractCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, Object.Func(20));
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, Object.Func(30));
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenSubtractSampledCalled_ThenOnlyExecutesExpressionWhenSampled) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
    
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));

  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, Object.Func(20));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, Object.Func(30));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, Object.Func(40));
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenIncrementCalled_ThenAddOneMessageEnqueued) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooGauge::Instance(), 1)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_INCREMENT(MetricFooGauge);
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenDecrementCalled_ThenNegativeAddOneMessageEnqueued) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooGauge::Instance(), -1)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_DECREMENT(MetricFooGauge);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenAddCalled6Times_ThenAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 42)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 22)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 91)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricFooSampled, 42);
  GAMELIFT_METRICS_ADD(MetricFooSampled, 11);
  GAMELIFT_METRICS_ADD(MetricFooSampled, 22);
  GAMELIFT_METRICS_ADD(MetricFooSampled, 182);
  GAMELIFT_METRICS_ADD(MetricFooSampled, 91);
  GAMELIFT_METRICS_ADD(MetricFooSampled, 1);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenSubtractCalled6Times_ThenNegativeAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -121)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -10)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -94)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 121);
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 52);
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 10);
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 111);
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 94);
  GAMELIFT_METRICS_SUBTRACT(MetricFooSampled, 101);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenAddSampledCalled6Times_ThenAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 42)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 22)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), 91)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 42);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 11);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 22);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 182);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 91);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooSampled, 1);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenSubtractSampledCalled6Times_ThenNegativeAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -121)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -10)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -94)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 121);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 52);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 10);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 111);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 94);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooSampled, 101);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenIncrementCalled6Times_ThenAddOneMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooSampled::Instance(), 1)))
      .Times(Exactly(3));

  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
  GAMELIFT_METRICS_INCREMENT(MetricFooSampled);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenDecrementCalled6Times_ThenNegativeAddOneMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeAdd(
                                 MetricFooSampled::Instance(), -1)))
      .Times(Exactly(3));

  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
  GAMELIFT_METRICS_DECREMENT(MetricFooSampled);
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenCountHitCalled_ThenAddOneMessageEnqueued) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooGauge::Instance(), 1)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_COUNT_HIT(MetricFooGauge);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenCountHitCalled6Times_ThenAddOneMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooSampled::Instance(), 1)))
      .Times(Exactly(3));

  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooSampled);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenCountExprCalledFourTimes_ThenExpressionExecutedFourTimes) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

    MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(4));

  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Object.Func(10));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenCountExprCalledFourTimes_ThenExpressionExecutedFourTimes) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(4));

  GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Object.Func(10));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenCountExprCalledFourTimes_ThenAddOneMessageEnqueuedTwoTimes) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeAdd(MetricFooSampled::Instance(), 1)))
      .Times(Exactly(2));

  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, 10 + 10);
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Returns4());
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Returns6Functor()());
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, []() { return 8; }());
}

TEST_F(ArithmeticMacrosTests,
       GivenGaugeMetric_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
  
  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricFooGauge, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricFooGauge, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooGauge, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooGauge, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenGaugeMetricWithSampler_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
  
  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooSampled, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenAnyArithmeticFunctionCalled_ThenNothingIsEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_ADD(MetricFooDisabled, 42);
  GAMELIFT_METRICS_SUBTRACT(MetricFooDisabled, 121);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooDisabled, 42);
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooDisabled, 121);
  GAMELIFT_METRICS_INCREMENT(MetricFooDisabled);
  GAMELIFT_METRICS_DECREMENT(MetricFooDisabled);
  GAMELIFT_METRICS_COUNT_HIT(MetricFooDisabled);
  GAMELIFT_METRICS_COUNT_EXPR(MetricFooDisabled, 10 + 11);
}

TEST_F(ArithmeticMacrosTests,
       GivenDisabledGaugeMetric_WhenAddCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_ADD(MetricFooDisabled, Object.Func(20));
  GAMELIFT_METRICS_ADD(MetricFooDisabled, Object.Func(30));
  GAMELIFT_METRICS_ADD(MetricFooDisabled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenSubtractCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_SUBTRACT(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_SUBTRACT(MetricFooDisabled, Object.Func(20));
  GAMELIFT_METRICS_SUBTRACT(MetricFooDisabled, Object.Func(30));
  GAMELIFT_METRICS_SUBTRACT(MetricFooDisabled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenAddSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooDisabled, Object.Func(20));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooDisabled, Object.Func(30));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricFooDisabled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledGaugeMetric_WhenSubtractSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooDisabled, Object.Func(10));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooDisabled, Object.Func(20));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooDisabled, Object.Func(30));
  GAMELIFT_METRICS_SUBTRACT_SAMPLED(MetricFooDisabled, Object.Func(40));
}

TEST_F(ArithmeticMacrosTests,
       GivenCounterMetric_WhenAddCalled_ThenAddMessageEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarCounter::Instance(), 42)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricBarCounter, 42);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenAddCalled_ThenAlwaysExecutesExpression) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricBarSampled, Object.Func(10));
  GAMELIFT_METRICS_ADD(MetricBarSampled, Object.Func(20));
  GAMELIFT_METRICS_ADD(MetricBarSampled, Object.Func(30));
  GAMELIFT_METRICS_ADD(MetricBarSampled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenAddSampledCalled_ThenOnlyExecutesExpressionWhenSampled) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
    
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, Object.Func(10));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, Object.Func(20));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, Object.Func(30));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, Object.Func(40));
}

TEST_F(ArithmeticMacrosTests,
       GivenCounterMetric_WhenIncrementCalled_ThenAddOneMessageEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarCounter::Instance(), 1)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_INCREMENT(MetricBarCounter);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenAddCalled6Times_ThenAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 42)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 22)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 91)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricBarSampled, 42);
  GAMELIFT_METRICS_ADD(MetricBarSampled, 11);
  GAMELIFT_METRICS_ADD(MetricBarSampled, 22);
  GAMELIFT_METRICS_ADD(MetricBarSampled, 182);
  GAMELIFT_METRICS_ADD(MetricBarSampled, 91);
  GAMELIFT_METRICS_ADD(MetricBarSampled, 1);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenAddSampledCalled6Times_ThenAddMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 42)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 22)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 91)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 42);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 11);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 22);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 182);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 91);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarSampled, 1);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenIncrementCalled6Times_ThenAddOneMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 1)))
      .Times(Exactly(3));

  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
  GAMELIFT_METRICS_INCREMENT(MetricBarSampled);
}

TEST_F(ArithmeticMacrosTests,
       GivenCounterMetric_WhenCountHitCalled_ThenAddOneMessageEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarCounter::Instance(), 1)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_COUNT_HIT(MetricBarCounter);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenCountHitCalled6Times_ThenAddOneMessageEnqueued3Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 1)))
      .Times(Exactly(3));

  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarSampled);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenCountExprCalledFourTimes_ThenExpressionExecutedFourTimes) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(4));

  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Object.Func(10));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledCounterMetric_WhenCountExprCalledFourTimes_ThenExpressionExecutedFourTimes) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(4));

  GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Object.Func(10));
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Object.Func(10));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetric_WhenCountExprCalledFourTimes_ThenAddOneMessageEnqueuedFourTimes) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarCounter::Instance(), 1)))
      .Times(Exactly(4));

  GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, 10 + 10);
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, Returns4());
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, Returns6Functor()());
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, []() { return 8; }());
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenCountExprCalledFourTimes_ThenAddOneMessageEnqueuedTwoTimes) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::CounterAdd(
                                 MetricBarSampled::Instance(), 1)))
      .Times(Exactly(2));

  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, 10 + 10);
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Returns4());
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Returns6Functor()());
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, []() { return 8; }());
}

TEST_F(ArithmeticMacrosTests,
       GivenCounterMetric_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarCounter, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledCounterMetric_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenCounterMetricWithSampler_WhenCountExprCalledFourTimes_ThenReturnsValues) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  const double A = GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, 10 + 10);
  const double B = GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Returns4());
  const double C =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, Returns6Functor()());
  const double D =
      GAMELIFT_METRICS_COUNT_EXPR(MetricBarSampled, []() { return 8; }());

  EXPECT_EQ(A, 20.0);
  EXPECT_EQ(B, 4.0);
  EXPECT_EQ(C, 6.0);
  EXPECT_EQ(D, 8.0);
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledCounterMetric_WhenAnyArithmeticFunctionCalled_ThenNothingIsEnqueued) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_ADD(MetricBarDisabled, 42);
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarDisabled, 42);
  GAMELIFT_METRICS_INCREMENT(MetricBarDisabled);
  GAMELIFT_METRICS_COUNT_HIT(MetricBarDisabled);
  GAMELIFT_METRICS_COUNT_EXPR(MetricBarDisabled, 11 + 10);
}

TEST_F(ArithmeticMacrosTests,
       GivenDisabledCounterMetric_WhenAddCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_ADD(MetricBarDisabled, Object.Func(10));
  GAMELIFT_METRICS_ADD(MetricBarDisabled, Object.Func(20));
  GAMELIFT_METRICS_ADD(MetricBarDisabled, Object.Func(30));
  GAMELIFT_METRICS_ADD(MetricBarDisabled, Object.Func(40));
}

TEST_F(
    ArithmeticMacrosTests,
    GivenDisabledCounterMetric_WhenAddSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarDisabled, Object.Func(10));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarDisabled, Object.Func(20));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarDisabled, Object.Func(30));
  GAMELIFT_METRICS_ADD_SAMPLED(MetricBarDisabled, Object.Func(40));
}
