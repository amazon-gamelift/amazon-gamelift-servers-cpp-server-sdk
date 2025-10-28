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

#include <aws/gamelift/metrics/GameLiftMetrics.h>

using namespace ::testing;

// Define a gauge
namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGaugor, "gaugor", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGaugor);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricGaugorious, "gaugorious", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGaugorious);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricDisabled, "no-gaugor", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricDisabled);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricWithSampler, "sampled_gauge", MockEnabled,
                               MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricWithSampler);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricWithDerivedMetric, "with_derived",
                               MockEnabled, Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric("foo"));
GAMELIFT_METRICS_DEFINE_GAUGE(MetricWithDerivedMetric);

struct MockDerivedMetric2 : public MockDerivedMetric {
  explicit MockDerivedMetric2(std::string Name) : MockDerivedMetric(Name) {}
};

struct MockDerivedMetric3 : public MockDerivedMetric {
  explicit MockDerivedMetric3(std::string Name) : MockDerivedMetric(Name) {}
};

GAMELIFT_METRICS_DECLARE_GAUGE(MetricWithManyDerivedMetrics,
                               "with_many_derived", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric("foo"),
                               MockDerivedMetric2("bar"),
                               MockDerivedMetric3("baz"));
GAMELIFT_METRICS_DEFINE_GAUGE(MetricWithManyDerivedMetrics);
} // namespace

class GaugeMacrosTests : public MetricMacrosTests {};

TEST_F(GaugeMacrosTests, ContainsValidDetails) {
  {
    EXPECT_TRUE((std::is_same<MetricGaugor::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricGaugor::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_THAT(MetricGaugor::Instance().GetKey(), StrEq("gaugor"));
    EXPECT_THAT(MetricGaugor::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Gauge);

    MockNameVisitor Visitor;
    MetricGaugor::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE((std::is_same<MetricGaugorious::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricGaugorious::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_THAT(MetricGaugorious::Instance().GetKey(), StrEq("gaugorious"));
    EXPECT_THAT(MetricGaugorious::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Gauge);

    MockNameVisitor Visitor;
    MetricGaugorious::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE(
        (std::is_same<MetricWithSampler::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricWithSampler::SamplerType,
                              MockSampleEveryOther>::value));
    EXPECT_THAT(MetricWithSampler::Instance().GetKey(), StrEq("sampled_gauge"));
    EXPECT_THAT(MetricWithSampler::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Gauge);

    MockNameVisitor Visitor;
    MetricWithSampler::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, IsEmpty());
  }

  {
    EXPECT_TRUE(
        (std::is_same<MetricWithDerivedMetric::Platform, MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricWithDerivedMetric::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_THAT(MetricWithDerivedMetric::Instance().GetKey(),
                StrEq("with_derived"));
    EXPECT_THAT(MetricWithDerivedMetric::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Gauge);

    MockNameVisitor Visitor;
    MetricWithDerivedMetric::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, ElementsAre("foo"));
  }

  {
    EXPECT_TRUE((std::is_same<MetricWithManyDerivedMetrics::Platform,
                              MockEnabled>::value));
    EXPECT_TRUE((std::is_same<MetricWithManyDerivedMetrics::SamplerType,
                              Aws::GameLift::Metrics::SampleAll>::value));
    EXPECT_THAT(MetricWithManyDerivedMetrics::Instance().GetKey(),
                StrEq("with_many_derived"));
    EXPECT_THAT(MetricWithManyDerivedMetrics::Instance().GetMetricType(),
                Aws::GameLift::Metrics::MetricType::Gauge);

    MockNameVisitor Visitor;
    MetricWithManyDerivedMetrics::Instance().GetDerivedMetrics().Visit(Visitor);
    EXPECT_THAT(Visitor.Names, ElementsAre("foo", "bar", "baz"));
  }
}

TEST_F(GaugeMacrosTests, GivenSampleAll_WhenSetIsCalled_ThenSubmitsSetMessage) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeSet(MetricGaugor::Instance(), 42)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricGaugorious::Instance(), 12)))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeSet(MetricGaugor::Instance(), -5)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_SET(MetricGaugor, 42);
  GAMELIFT_METRICS_SET(MetricGaugorious, 12);
  GAMELIFT_METRICS_SET(MetricGaugor, -5);
}

namespace {
double Add(double a, double b) { return a + b; }
} // namespace

TEST_F(GaugeMacrosTests,
       GivenSampleAll_WhenSetIsCalled_ThenSetReturnsExpressionResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricGaugor, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricGaugor, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricGaugor, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricGaugor, SomeLambda(20, 30)), 50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricGaugor, Add(10, 20)), 30.0);
}

TEST_F(GaugeMacrosTests,
       GivenSampleEveryOther_WhenSetIsCalledTwice_ThenBothCallsReturnResult) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, 15.0), 15.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, 30.0), 30.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, 10.0 + 10.0), 20.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, 20.0 + 20.0), 40.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, SomeLambda(20, 30)), 50.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, SomeLambda(11, 12)), 23.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, Add(10, 20)), 30.0);
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricWithSampler, Add(40, 20)), 60.0);
}

TEST_F(
    GaugeMacrosTests,
    GivenSampleEveryOther_WhenSetIsCalledTwice_ThenSubmitsEveryOtherSetMessage) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricWithSampler::Instance(), 15.0)));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricWithSampler::Instance(), 20.0)));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricWithSampler::Instance(), 50.0)));
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricWithSampler::Instance(), 30.0)));

  GAMELIFT_METRICS_SET(MetricWithSampler, 15.0);
  GAMELIFT_METRICS_SET(MetricWithSampler, 30.0); // skipped

  GAMELIFT_METRICS_SET(MetricWithSampler, 10.0 + 10.0);
  GAMELIFT_METRICS_SET(MetricWithSampler, 20.0 + 20.0); // skipped

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  GAMELIFT_METRICS_SET(MetricWithSampler, SomeLambda(20, 30));
  GAMELIFT_METRICS_SET(MetricWithSampler, SomeLambda(11, 12)); // skipped

  GAMELIFT_METRICS_SET(MetricWithSampler, Add(10, 20));
  GAMELIFT_METRICS_SET(MetricWithSampler, Add(40, 20)); // skipped
}

TEST_F(GaugeMacrosTests,
       GivenDisabledMetric_WhenSetIsCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SET(MetricDisabled, 222);
}

TEST_F(GaugeMacrosTests,
       GivenDisabledMetric_WhenSetIsCalled_ThenSetReturnsExpressionResult) {
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricDisabled, 15.0), 15.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricDisabled, 10.0 + 10.0), 20.0);

  const std::function<double(double, double)> SomeLambda = [](double A, double B) { return A + B; };
  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricDisabled, SomeLambda(20, 30)), 50.0);

  EXPECT_EQ(GAMELIFT_METRICS_SET(MetricDisabled, Add(10, 20)), 30.0);
}

TEST_F(GaugeMacrosTests,
       GivenSampleAll_WhenResetIsCalled_ThenSubmitsSetZeroMessage) {
  EXPECT_CALL(MockProcessor,
              Enqueue(MetricMessage::GaugeSet(MetricGaugor::Instance(), 0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_RESET(MetricGaugor);
}

TEST_F(
    GaugeMacrosTests,
    GivenSampleEveryOther_WhenResetIsCalled10Times_ThenAlwaysSubmitsSetZeroMessage) {
  EXPECT_CALL(MockProcessor, Enqueue(MetricMessage::GaugeSet(
                                 MetricWithSampler::Instance(), 0)))
      .Times(Exactly(10));

  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
  GAMELIFT_METRICS_RESET(MetricWithSampler);
}

TEST_F(GaugeMacrosTests,
       GivenDisabledMetric_WhenResetIsCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_RESET(MetricDisabled);
}

TEST_F(GaugeMacrosTests,
       GivenSampledMetric_WhenSetIsCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_SET(MetricWithSampler, Object.Func(10));
  GAMELIFT_METRICS_SET(MetricWithSampler, Object.Func(20));
  GAMELIFT_METRICS_SET(MetricWithSampler, Object.Func(30));
  GAMELIFT_METRICS_SET(MetricWithSampler, Object.Func(40));
}

TEST_F(
    GaugeMacrosTests,
    GivenSampledMetric_WhenSetSampledIsCalled_ThenOnlyExecutesExpressionWhenSampled) {
  // Suppress warnings about uninteresting mock calls to MockProcessor.Enqueue
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(AnyNumber());

  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));

  GAMELIFT_METRICS_SET_SAMPLED(MetricWithSampler, Object.Func(10));
  GAMELIFT_METRICS_SET_SAMPLED(MetricWithSampler, Object.Func(20));
  GAMELIFT_METRICS_SET_SAMPLED(MetricWithSampler, Object.Func(30));
  GAMELIFT_METRICS_SET_SAMPLED(MetricWithSampler, Object.Func(40));
}

TEST_F(GaugeMacrosTests,
       GivenDisabledMetric_WhenSetIsCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(20)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(30)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(40)).Times(Exactly(1));

  GAMELIFT_METRICS_SET(MetricDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET(MetricDisabled, Object.Func(20));
  GAMELIFT_METRICS_SET(MetricDisabled, Object.Func(30));
  GAMELIFT_METRICS_SET(MetricDisabled, Object.Func(40));
}

TEST_F(GaugeMacrosTests,
       GivenDisabledMetric_WhenSetSampledIsCalled_ThenNeverExecutesExpression) {
  MockObject Object;
  EXPECT_CALL(Object, Func(_)).Times(Exactly(0));

  GAMELIFT_METRICS_SET_SAMPLED(MetricDisabled, Object.Func(10));
  GAMELIFT_METRICS_SET_SAMPLED(MetricDisabled, Object.Func(20));
  GAMELIFT_METRICS_SET_SAMPLED(MetricDisabled, Object.Func(30));
  GAMELIFT_METRICS_SET_SAMPLED(MetricDisabled, Object.Func(40));
}
