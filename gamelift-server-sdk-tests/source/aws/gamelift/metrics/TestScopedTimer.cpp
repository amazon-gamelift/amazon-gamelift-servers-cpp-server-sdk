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

#include "Common.h"
#include "MetricMacrosTests.h"
#include "MockSampleEveryOther.h"

/**
 * We include the headers separately vs including GameLiftMetrics.h
 *
 * This allows us to test the timing macros with a mocked MacroScopedTimer
 */
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Samplers.h>
#include <aws/gamelift/metrics/ScopedTimer.h>

using namespace ::testing;

namespace {
MockMetricsProcessor *MockProcessor;

struct GetMockProcessor {
  IMetricsProcessor *operator()() { return MockProcessor; }
};

struct MockClock {
  using Time = double;
  using Duration = double;

  static double CurrentTime;

  static Time Now() { return CurrentTime; }

  static double ToMilliseconds(Duration Duration) { return Duration; }

  static void Reset() { CurrentTime = 0; }

  static void Advance(Duration Duration) { CurrentTime += Duration; }
};

double MockClock::CurrentTime = 0;

/*
 * Timer we'll be testing.
 */
template <class Metric>
class TestTimer
    : public Aws::GameLift::Metrics::Internal::ScopedTimer<Metric, MockClock,
                                                           GetMockProcessor> {};
} // namespace

/*
 * Mock MacroScopedTimer then include the .inl header
 */
namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class Metric> class MacroScopedTimer : public TestTimer<Metric> {};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#include <aws/gamelift/metrics/ScopedTimerMacros.inl>

class TestScopedTimer : public ::testing::Test {
protected:
  virtual void SetUp() override {
    MockProcessor = new MockMetricsProcessor;
    MockClock::Reset();
  }

  virtual void TearDown() override { delete MockProcessor; }
};

class TestScopedTimerMacros : public TestScopedTimer {};

namespace {
GAMELIFT_METRICS_DECLARE_TIMER(MetricScopedTimer, "glork", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricScopedTimer);

GAMELIFT_METRICS_DECLARE_TIMER(MetricScopedTimerSampled, "glork-sampled",
                               MockEnabled, MockSampleEveryOther());
GAMELIFT_METRICS_DEFINE_TIMER(MetricScopedTimerSampled);

GAMELIFT_METRICS_DECLARE_TIMER(MetricScopedTimerDisabled, "nlork", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(MetricScopedTimerDisabled);
} // namespace

TEST_F(TestScopedTimer,
       GivenEnabledMetric_WhenScopedTimersDefined_ThenMeasuresScopes) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 40)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 30)))
      .Times(Exactly(1));

  {
    TestTimer<MetricScopedTimer> Test;
    MockClock::Advance(40);
  }

  {
    TestTimer<MetricScopedTimer> Test;
    MockClock::Advance(30);
  }
}

TEST_F(
    TestScopedTimer,
    GivenEnabledMetric_WhenScopedTimersDefinedInOverlappingScopes_ThenMeasuresScopes) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 20)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 70)))
      .Times(Exactly(1));

  {
    TestTimer<MetricScopedTimer> Test;
    MockClock::Advance(20);

    {
      TestTimer<MetricScopedTimer> TestTwo;
      MockClock::Advance(20);
    }

    MockClock::Advance(30);
  }
}

TEST_F(TestScopedTimer,
       GivenSampledMetric_WhenScopedTimersDefined_ThenMeasuresFirstScope) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimerSampled::Instance(), 40)))
      .Times(Exactly(1));

  {
    TestTimer<MetricScopedTimerSampled> Test;
    MockClock::Advance(40);
  }

  {
    TestTimer<MetricScopedTimerSampled> Test;
    MockClock::Advance(30);
  }
}

TEST_F(TestScopedTimer,
       GivenDisabledMetric_WhenScopedTimersDefined_ThenNothingHappens) {
  EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(Exactly(0));

  {
    TestTimer<MetricScopedTimerDisabled> Test;
    MockClock::Advance(40);
  }

  {
    TestTimer<MetricScopedTimerDisabled> Test;
    MockClock::Advance(30);
  }
}

TEST_F(TestScopedTimerMacros,
       GivenEnabledMetric_WhenTimeScopeCalled_ThenMeasuresScopes) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 40)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 30)))
      .Times(Exactly(1));

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimer);
    MockClock::Advance(40);
  }

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimer);
    MockClock::Advance(30);
  }
}

TEST_F(
    TestScopedTimerMacros,
    GivenEnabledMetric_WhenTimeScopeCalledInOverlappingScopes_ThenMeasuresScopes) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 20)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 70)))
      .Times(Exactly(1));

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimer);
    MockClock::Advance(20);

    {
      GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimer);
      MockClock::Advance(20);
    }

    MockClock::Advance(30);
  }
}

TEST_F(TestScopedTimerMacros,
       GivenSampledMetric_WhenTimeScopeCalled_ThenMeasuresFirstScope) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimerSampled::Instance(), 40)))
      .Times(Exactly(1));

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimerSampled);
    MockClock::Advance(40);
  }

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimerSampled);
    MockClock::Advance(30);
  }
}

TEST_F(TestScopedTimerMacros,
       GivenDisabledMetric_WhenTimeScopeCalled_ThenNothingHappens) {
  EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(Exactly(0));

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimerDisabled);
    MockClock::Advance(40);
  }

  {
    GAMELIFT_METRICS_TIME_SCOPE(MetricScopedTimerDisabled);
    MockClock::Advance(30);
  }
}

namespace {
int MockReturns4After40Ms() {
  MockClock::Advance(40);
  return 4;
}

struct MockReturns2After20MsFunctor {
  int operator()() {
    MockClock::Advance(20);
    return 2;
  }
};
} // namespace

TEST_F(TestScopedTimerMacros,
       GivenEnabledMetric_WhenTimeExpressionCalled_ThenMeasuresExpressions) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 40)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 20)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 15)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimer::Instance(), 0)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, MockReturns4After40Ms());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer,
                             MockReturns2After20MsFunctor()());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, []() {
    MockClock::Advance(15);
    return 1;
  }());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, 10 + 11);
}

TEST_F(TestScopedTimerMacros,
    GivenEnabledMetric_WhenTimeExpressionCalled_ThenReturnsExpression) {
    // Suppress uninteresting Enqueue calls warning
    EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(AnyNumber());

    const int A =
        GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, MockReturns4After40Ms());
    const int B = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer,
        MockReturns2After20MsFunctor()());
    const int C = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, []() {
        MockClock::Advance(15);
        return 1;
        }());
    const int D = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimer, 10 + 11);

    EXPECT_EQ(A, 4);
    EXPECT_EQ(B, 2);
    EXPECT_EQ(C, 1);
    EXPECT_EQ(D, 21);
}

TEST_F(
    TestScopedTimerMacros,
    GivenSampledMetric_WhenTimeExpressionCalled4Times_ThenMeasures2Expressions) {
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimerSampled::Instance(), 40)))
      .Times(Exactly(1));
  EXPECT_CALL(*MockProcessor, Enqueue(MetricMessage::TimerSet(
                                  MetricScopedTimerSampled::Instance(), 15)))
      .Times(Exactly(1));

  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, MockReturns4After40Ms());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled,
                             MockReturns2After20MsFunctor()());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, []() {
    MockClock::Advance(15);
    return 1;
  }());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, 10 + 11);
}

TEST_F(TestScopedTimerMacros,
       GivenSampledMetric_WhenTimeExpressionCalled_ThenReturnsExpression) {
  
  // Suppress uninteresting Enqueue calls warning
  EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(AnyNumber());
  
  const int A = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled,
                                           MockReturns4After40Ms());
  const int B = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled,
                                           MockReturns2After20MsFunctor()());
  const int C = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, []() {
    MockClock::Advance(15);
    return 1;
  }());
  const int D = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, 10 + 11);

  EXPECT_EQ(A, 4);
  EXPECT_EQ(B, 2);
  EXPECT_EQ(C, 1);
  EXPECT_EQ(D, 21);
}

TEST_F(TestScopedTimerMacros,
       GivenDisabledMetric_WhenTimeExpressionCalled_ThenNothingHappens) {
  EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled,
                             MockReturns4After40Ms());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled,
                             MockReturns2After20MsFunctor()());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, []() {
    MockClock::Advance(15);
    return 1;
  }());
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, 10 + 11);
}

TEST_F(TestScopedTimerMacros,
       GivenDisabledMetric_WhenTimeExpressionCalled_ThenReturnsExpression) {
  const int A = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled,
                                           MockReturns4After40Ms());
  const int B = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled,
                                           MockReturns2After20MsFunctor()());
  const int C = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, []() {
    MockClock::Advance(15);
    return 1;
  }());
  const int D = GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, 10 + 11);

  EXPECT_EQ(A, 4);
  EXPECT_EQ(B, 2);
  EXPECT_EQ(C, 1);
  EXPECT_EQ(D, 21);
}

TEST_F(
    TestScopedTimerMacros,
    GivenSampledMetric_WhenTimeExpressionCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));
  
  // Suppress uninteresting Enqueue calls warning
  EXPECT_CALL(*MockProcessor, Enqueue(_)).Times(AnyNumber());

  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, Object.Func(10));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, Object.Func(44));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, Object.Func(22));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerSampled, Object.Func(72));
}

TEST_F(
    TestScopedTimerMacros,
    GivenDisabledMetric_WhenTimeExpressionCalled_ThenAlwaysExecutesExpression) {
  MockObject Object;
  ON_CALL(Object, Func(_)).WillByDefault(Return(5));
  EXPECT_CALL(Object, Func(10)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(44)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(22)).Times(Exactly(1));
  EXPECT_CALL(Object, Func(72)).Times(Exactly(1));

  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, Object.Func(10));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, Object.Func(44));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, Object.Func(22));
  GAMELIFT_METRICS_TIME_EXPR(MetricScopedTimerDisabled, Object.Func(72));
}
