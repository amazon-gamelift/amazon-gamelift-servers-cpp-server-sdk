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

#include <aws/gamelift/metrics/Defs.h>
#include <aws/gamelift/metrics/GlobalMetricsProcessor.h>
#include <aws/gamelift/metrics/HighResolutionClock.h>
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/LoggerMacros.h>

/**
 * INTERNAL: sets timer to value in milliseconds
 */
#define _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, expr)                        \
  IF_CONSTEXPR(metric::Platform::bEnabled) {                                   \
    if (metric::Instance().GetSampler().ShouldTakeSample()) {                  \
      auto *processor = GameLiftMetricsGlobalProcessor();                      \
      if (processor) {                                                         \
        processor->Enqueue(::Aws::GameLift::Metrics::MetricMessage::TimerSet(  \
            metric::Instance(), expr));                                        \
      } else {                                                                 \
        GAMELIFT_METRICS_LOG_CRITICAL(                                         \
            "Global metrics processor is not initialized");                    \
      }                                                                        \
    }                                                                          \
  }

/**
 * @brief Sets a timer to a value in milliseconds.
 *
 * Always executes expression.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Value in milliseconds.
 *
 * @returns Result of expr.
 */
#define GAMELIFT_METRICS_SET_MS(metric, expr)                                  \
  [&]() {                                                                      \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Timer>::value,         \
        "Metric '" #metric                                                     \
        "' is not a timer. GAMELIFT_METRICS_SET_MS only supports timers.");    \
    auto value = (expr);                                                       \
    _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, value)                           \
    return value;                                                              \
  }()

/**
 * @brief Sets a timer to a value in seconds.
 *
 * Always executes expression.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Value in seconds.
 *
 * @returns Result of expr.
 */
#define GAMELIFT_METRICS_SET_SEC(metric, expr)                                 \
  [&]() {                                                                      \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Timer>::value,         \
        "Metric '" #metric                                                     \
        "' is not a timer. GAMELIFT_METRICS_SET_SEC only supports timers.");   \
    auto value = (expr);                                                       \
    _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, value * 1000.0)                  \
    return value;                                                              \
  }()

/**
 * @brief Sets a timer to a value in milliseconds.
 *
 * Only executes expression when sampled.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Value in milliseconds.
 */
#define GAMELIFT_METRICS_SET_MS_SAMPLED(metric, expr)                          \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Timer>::value,         \
        "Metric '" #metric                                                     \
        "' is not a timer. GAMELIFT_METRICS_SET_MS_SAMPLED only supports "     \
        "timers.");                                                            \
    _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, expr)                            \
  } while (0)

/**
 * @brief Sets a timer to a value in seconds.
 *
 * Only executes expression when sampled.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Value in seconds.
 */
#define GAMELIFT_METRICS_SET_SEC_SAMPLED(metric, expr)                         \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Timer>::value,         \
        "Metric '" #metric                                                     \
        "' is not a timer. GAMELIFT_METRICS_SET_SEC_SAMPLED only supports "    \
        "timers.");                                                            \
    _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, (expr)*1000.0)                   \
  } while (0)

#include <aws/gamelift/metrics/ScopedTimer.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class Metric>
class MacroScopedTimer
    : public ::Aws::GameLift::Metrics::Internal::ScopedTimer<
          Metric, ::Aws::GameLift::Metrics::Internal::HighResolutionClock,
          ::Aws::GameLift::Metrics::Internal::DefaultGetProcessor> {};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#include <aws/gamelift/metrics/ScopedTimerMacros.inl>

/**
 * @brief Times an expression. For example, a function call.
 *
 * Only executes expression if sampled.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Expression to time
 */
#define GAMELIFT_METRICS_TIME_EXPR_SAMPLED(metric, expr)                       \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Timer>::value,         \
        "Metric '" #metric                                                     \
        "' is not a timer. GAMELIFT_METRICS_TIME_EXPR_SAMPLED only supports "  \
        "timers.");                                                            \
    _GAMELIFT_METRICS_INTERNAL_SET_MS(metric, [&]() {                          \
      const auto Start =                                                       \
          ::Aws::GameLift::Metrics::Internal::HighResolutionClock::Now();      \
      (expr);                                                                  \
      const auto End =                                                         \
          ::Aws::GameLift::Metrics::Internal::HighResolutionClock::Now();      \
      return ::Aws::GameLift::Metrics::Internal::HighResolutionClock::         \
          ToMilliseconds(End - Start);                                         \
    }())                                                                       \
  } while (0)