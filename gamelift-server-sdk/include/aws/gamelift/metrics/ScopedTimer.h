/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates
 * or its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution (the "License"). All use of this software is governed by
 * the License, or, if provided, by the license below or the license
 * accompanying this file. Do not remove or modify any license notices. This
 * file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied.
 *
 */
#pragma once

#include <aws/gamelift/metrics/GlobalMetricsProcessor.h>
#include <aws/gamelift/metrics/HighResolutionClock.h>
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/LoggerMacros.h>
#include <aws/gamelift/metrics/TypeTraits.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

/**
 * INTERNAL: RAII scoped timer
 *
 * @param Metric Metric to record to.
 * @param Clock Clock used for measurements. (@see
 * Internal::HighResolutionClock)
 * @param GetProcessor Functor returning the global metrics processor.
 */
template <class Metric, class Clock, class GetProcessor, class Enabled = void>
class ScopedTimer {
  // This is the no-op base case for when the metric is disabled for a build.

public:
  ScopedTimer() : m_dummy(0) {}
  ~ScopedTimer() {
    /*
     * This line results in the dummy being 'used' which removes
     * 'unused variable' warnings wherever ScopedTimer is used with
     * a disabled metric.
     *
     * It is a no-op and will be optimized out by the compiler.
     *
     * If we leave ctor and dtor undefined, we get warnings because the timer
     * variable is not used by calling code and does nothing itself.
     */
    (void)m_dummy;
  }

private:
  int m_dummy;
};

template <class Metric, class Clock, class GetProcessor>
class ScopedTimer<
    Metric, Clock, GetProcessor,
    typename Internal::EnableIf<Metric::Platform::bEnabled>::type> {
  // This is the main case for enabled metrics. See EnableIf ^

private:
  using Time = typename Clock::Time;

public:
  ScopedTimer() : m_startTime(Clock::Now()) {}

  ~ScopedTimer() {
    const auto duration = Clock::Now() - m_startTime;

    if (!Metric::Instance().GetSampler().ShouldTakeSample()) {
      return;
    }

    auto *processor = GetProcessor()();
    if (processor) {
      processor->Enqueue(MetricMessage::TimerSet(
          Metric::Instance(), Clock::ToMilliseconds(duration)));
    } else {
      GAMELIFT_METRICS_LOG_CRITICAL("Metrics processor is not initialized");
    }
  }

private:
  Time m_startTime;
};

/**
 * INTERNAL: default processor
 */
struct GAMELIFT_METRICS_API DefaultGetProcessor {
  IMetricsProcessor *operator()() { return GameLiftMetricsGlobalProcessor(); }
};

} // namespace Internal

/**
 * @brief Scoped timer. Measures its own lifetime.
 *
 * Timing begins at construction and ends when destroyed.
 * Time collected in milliseconds and sent using the provided metric.
 *
 * @param Metric The name of a metric declared with
 * GAMELIFT_METRIC_DECLARE_TIMER
 */
template <class Metric>
using ScopedTimer = Internal::ScopedTimer<Metric, Internal::HighResolutionClock,
                                          Internal::DefaultGetProcessor>;
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
