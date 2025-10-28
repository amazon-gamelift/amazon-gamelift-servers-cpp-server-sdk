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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <type_traits>

#include "MetricMacrosTests.h"
#include "MockDerivedMetric.h"
#include "MockSampleEveryOther.h"

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/TimerMacros.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

class TimerMacrosTests : public MetricMacrosTests {};

// Define a timer
namespace {
GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "glork", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimerSampled, "glork_sampled", MockEnabled,
                               MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimerSampled);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimerDisabled, "nlork", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimerDisabled);

GAMELIFT_METRICS_DECLARE_TIMER(MetricWithDerivedMetric, "with_derived",
                               MockEnabled, Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric("foo"));
GAMELIFT_METRICS_DEFINE_TIMER(MetricWithDerivedMetric);

struct MockDerivedMetric2 : public MockDerivedMetric {
  explicit MockDerivedMetric2(std::string Name) : MockDerivedMetric(Name) {}
};

struct MockDerivedMetric3 : public MockDerivedMetric {
  explicit MockDerivedMetric3(std::string Name) : MockDerivedMetric(Name) {}
};

GAMELIFT_METRICS_DECLARE_TIMER(MetricWithManyDerivedMetrics,
                               "with_many_derived", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric("foo"),
                               MockDerivedMetric2("bar"),
                               MockDerivedMetric3("baz"));
GAMELIFT_METRICS_DEFINE_TIMER(MetricWithManyDerivedMetrics);
} // namespace

TEST_F(TimerMacrosTests, ContainsValidDetails) {
  {
    EXPECT_TRUE((std::is_same<MetricTimer::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricTimer::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_TRUE((std::is_same<MetricTimer::MetricType,
                              Aws::GameLift::Metrics::Timer>::value));
    EXPECT_THAT(MetricTimer::Instance().GetKey(), StrEq("glork"));
    EXPECT_THAT(MetricTimer::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Timer);

    MockNameVisitor Visitor;
    MetricTimer::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE(
        (std::is_same<MetricTimerDisabled::Platform, MockDisabled>::value));
    EXPECT_TRUE((std::is_same<MetricTimerDisabled::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_TRUE((std::is_same<MetricTimerDisabled::MetricType,
                              Aws::GameLift::Metrics::Timer>::value));
    EXPECT_THAT(MetricTimerDisabled::Instance().GetKey(), StrEq("nlork"));
    EXPECT_THAT(MetricTimerDisabled::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Timer);

    MockNameVisitor Visitor;
    MetricTimerDisabled::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE(
        (std::is_same<MetricTimerSampled::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricTimerSampled::SamplerType,
                              MockSampleEveryOther>::value));
    EXPECT_TRUE((std::is_same<MetricTimerSampled::MetricType,
                              Aws::GameLift::Metrics::Timer>::value));
    EXPECT_THAT(MetricTimerSampled::Instance().GetKey(),
                StrEq("glork_sampled"));
    EXPECT_THAT(MetricTimerSampled::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Timer);

    MockNameVisitor Visitor;
    MetricTimerSampled::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE(
        (std::is_same<MetricWithDerivedMetric::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricWithDerivedMetric::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_TRUE((std::is_same<MetricWithDerivedMetric::MetricType,
                              Aws::GameLift::Metrics::Timer>::value));
    EXPECT_THAT(MetricWithDerivedMetric::Instance().GetKey(),
                StrEq("with_derived"));
    EXPECT_THAT(MetricWithDerivedMetric::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Timer);

    MockNameVisitor Visitor;
    MetricWithDerivedMetric::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, ElementsAre("foo"));
  }

  {
    EXPECT_TRUE((std::is_same<MetricWithManyDerivedMetrics::Platform,
                              MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricWithManyDerivedMetrics::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_TRUE((std::is_same<MetricWithManyDerivedMetrics::MetricType,
                              Aws::GameLift::Metrics::Timer>::value));
    EXPECT_THAT(MetricWithManyDerivedMetrics::Instance().GetKey(),
                StrEq("with_many_derived"));
    EXPECT_THAT(MetricWithManyDerivedMetrics::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Timer);

    MockNameVisitor Visitor;
    MetricWithManyDerivedMetrics::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, ElementsAre("foo", "bar", "baz"));
  }
}

TEST_F(TimerMacrosTests,
       GivenSampleAll_WhenSetMsCalled_ThenEnqueuesSetMessage) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 11.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 20.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_MS(MetricTimer, 11.0);
  GAMELIFT_METRICS_SET_MS(MetricTimer, 20.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleEveryOther_WhenSetMsCalled4Times_ThenEnqueuesSetMessage2Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 11.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 20.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 11.0);
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 521.0);
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 20.0);
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 1.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleAll_WhenSetSecCalled_ThenMultipliesBy1000AndEnqueuesSetMessage) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimer::Instance(), 11000.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimer::Instance(), 20000.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_SEC(MetricTimer, 11.0);
  GAMELIFT_METRICS_SET_SEC(MetricTimer, 20.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleEveryOther_WhenSetSecCalled4Times_ThenEnqueuesSetMessage2Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 11000.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 20000.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 11.0);
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 85.0);
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 20.0);
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 1.0);
}

TEST_F(TimerMacrosTests,
       GivenSampleAll_WhenSetMsSampledCalled_ThenEnqueuesSetMessage) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 11.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 20.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimer, 11.0);
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimer, 20.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleEveryOther_WhenSetMsSampledCalled4Times_ThenEnqueuesSetMessage2Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 11.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 20.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, 11.0);
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, 521.0);
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, 20.0);
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, 1.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleAll_WhenSetSecSampledCalled_ThenMultipliesBy1000AndEnqueuesSetMessage) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimer::Instance(), 11000.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimer::Instance(), 20000.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimer, 11.0);
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimer, 20.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampleEveryOther_WhenSetSecSampledCalled4Times_ThenEnqueuesSetMessage2Times) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 11000.0)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(
                                 MetricTimerSampled::Instance(), 20000.0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, 11.0);
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, 85.0);
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, 20.0);
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, 1.0);
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenAnySetFunctionCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, 11.0);
  GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, 20.0);
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerDisabled, 11.0);
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerDisabled, 20.0);
}

namespace {
double Add(double A, double B) { return A + B; }
} // namespace

TEST_F(TimerMacrosTests, WhenSetMsCalled_ThenReturnsExpressionResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());
  
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimer, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimer, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimer, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimer, SomeLambda(20, 30)), 50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimer, Add(10, 20)), 30.0);
}

TEST_F(TimerMacrosTests, WhenSetSecCalled_ThenReturnsExpressionResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimer, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimer, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimer, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimer, SomeLambda(20, 30)), 50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimer, Add(10, 20)), 30.0);
}

TEST_F(TimerMacrosTests,
       GivenSampledMetric_WhenSetMsCalled_ThenReturnsExpressionResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 10.0 + 10.0), 20.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, SomeLambda(20, 30)),
            50.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, SomeLambda(20, 30)),
            50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Add(10, 20)), 30.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Add(10, 20)), 30.0);
}

TEST_F(TimerMacrosTests,
       GivenSampledMetric_WhenSetSecCalled_ThenReturnsExpressionResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 10.0 + 10.0), 20.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, SomeLambda(20, 30)),
            50.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, SomeLambda(20, 30)),
            50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Add(10, 20)), 30.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Add(10, 20)), 30.0);
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenSetMsCalled_ThenReturnsExpressionResult) {
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, SomeLambda(20, 30)),
            50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, Add(10, 20)), 30.0);
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenSetSecCalled_ThenReturnsExpressionResult) {
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, SomeLambda(20, 30)),
            50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, Add(10, 20)), 30.0);
}

TEST_F(
    TimerMacrosTests,
    GivenSampledMetric_WhenTimeExpressionSampledCalled_ThenExecutesExpressionOnlyWhenSampled) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));

  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerSampled, Object.Func(72));
}

TEST_F(
    TimerMacrosTests,
    GivenDisabledMetric_WhenTimeExpressionSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_TIME_EXPR_SAMPLED(MetricTimerDisabled, Object.Func(72));
}

TEST_F(
    TimerMacrosTests,
    GivenSampledMetric_WhenSetMsSampledCalled_ThenExecutesExpressionOnlyWhenSampled) {
  MockObject Object;

  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));

  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerSampled, Object.Func(72));
}

TEST_F(TimerMacrosTests,
       GivenSampledMetric_WhenSetMsCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));

  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_SET_MS(MetricTimerSampled, Object.Func(72));
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenSetMsSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_SET_MS_SAMPLED(MetricTimerDisabled, Object.Func(72));
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenSetMsCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));

  GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_SET_MS(MetricTimerDisabled, Object.Func(72));
}

TEST_F(
    TimerMacrosTests,
    GivenSampledMetric_WhenSetSecSampledCalled_ThenExecutesExpressionOnlyWhenSampled) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));

  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(MetricTimerSampled::Instance(), 5000.0)))
      .Times(AnyNumber());

  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerSampled, Object.Func(72));
}

TEST_F(TimerMacrosTests,
       GivenSampledMetric_WhenSetSecCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));

  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::TimerSet(MetricTimerSampled::Instance(), 5000.0)))
      .Times(AnyNumber());

  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_SET_SEC(MetricTimerSampled, Object.Func(72));
}

TEST_F(
    TimerMacrosTests,
    GivenDisabledMetric_WhenSetSecSampledCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_SET_SEC_SAMPLED(MetricTimerDisabled, Object.Func(72));
}

TEST_F(TimerMacrosTests,
       GivenDisabledMetric_WhenSetSecCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));

  GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_SET_SEC(MetricTimerDisabled, Object.Func(72));
}
