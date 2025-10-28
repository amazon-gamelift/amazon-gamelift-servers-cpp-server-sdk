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
#include <aws/gamelift/metrics/LoggerMacros.h>
#include <aws/gamelift/metrics/TypeTraits.h>

/**
 * INTERNAL: add expr to metric
 */
#define _GAMELIFT_METRICS_INTERNAL_ADD(metric, expr)                           \
  IF_CONSTEXPR(metric::Platform::bEnabled) {                                   \
    if (metric::Instance().GetSampler().ShouldTakeSample()) {                  \
      auto *Processor = GameLiftMetricsGlobalProcessor();                      \
      if (Processor) {                                                         \
        IF_CONSTEXPR(                                                          \
            ::Aws::GameLift::Metrics::Internal::IsSame<                        \
                metric::MetricType, ::Aws::GameLift::Metrics::Gauge>::value) { \
          Processor->Enqueue(                                                  \
              ::Aws::GameLift::Metrics::MetricMessage::GaugeAdd(               \
                  metric::Instance(), (expr)));                                \
        }                                                                      \
        else {                                                                 \
          Processor->Enqueue(                                                  \
              ::Aws::GameLift::Metrics::MetricMessage::CounterAdd(             \
                  metric::Instance(), (expr)));                                \
        }                                                                      \
      } else {                                                                 \
        GAMELIFT_METRICS_LOG_CRITICAL(                                         \
            "Global metrics processor is not initialized");                    \
      }                                                                        \
    }                                                                          \
  }

/**
 * @brief This macros adds value to a gauge or a counter.
 *
 * Expression always executed.
 *
 * @param metric Gauge or counter to add to.
 * @param expr Value to add.
 */
#define GAMELIFT_METRICS_ADD(metric, expr)                                     \
  do {                                                                         \
    static_assert(Aws::GameLift::Metrics::IsSupported<                         \
                      metric::MetricType, Aws::GameLift::Metrics::Gauge,       \
                      Aws::GameLift::Metrics::Counter>::value,                 \
                  "Metric '" #metric                                           \
                  "' is not a gauge or counter. GAMELIFT_METRICS_ADD only "    \
                  "supports gauges and counters.");                            \
    auto Value = (expr);                                                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, Value)                              \
  } while (0)

/**
 * @brief This macro subtracts value from a gauge.
 *
 * Expression always executed.
 *
 * @param metric Gauge to subtract from.
 * @param expr Value to subtract.
 */
#define GAMELIFT_METRICS_SUBTRACT(metric, expr)                                \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric "' is not a gauge. GAMELIFT_METRICS_SUBTRACT "      \
        "only supports gauges.");                                              \
    auto Value = (expr);                                                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, -(Value))                           \
  } while (0)

/**
 * @brief This macro adds value to a gauge or a counter.
 *
 * Only executes expression when sampled.
 *
 * @param metric Gauge or counter to add to.
 * @param expr Value to add.
 */
#define GAMELIFT_METRICS_ADD_SAMPLED(metric, expr)                             \
  do {                                                                         \
    static_assert(Aws::GameLift::Metrics::IsSupported<                         \
                      metric::MetricType, Aws::GameLift::Metrics::Gauge,       \
                      Aws::GameLift::Metrics::Counter>::value,                 \
                  "Metric '" #metric                                           \
                  "' is not a gauge or counter. GAMELIFT_METRICS_ADD_SAMPLED " \
                  "only supports gauges and counters.");                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, expr)                               \
  } while (0)

/**
 * @brief This macro subtracts value from a gauge.
 *
 * Only executes expression when sampled.
 *
 * @param metric Gauge to subtract from.
 * @param expr Value to subtract.
 */
#define GAMELIFT_METRICS_SUBTRACT_SAMPLED(metric, expr)                        \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric                                                     \
        "' is not a gauge. GAMELIFT_METRICS_SUBTRACT_SAMPLED only "            \
        "supports gauges.");                                                   \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, -(expr))                            \
  } while (0)

/**
 * @brief This macro increments a gauge or a counter.
 *
 * @param metric Gauge or counter to increment.
 */
#define GAMELIFT_METRICS_INCREMENT(metric)                                     \
  do {                                                                         \
    static_assert(Aws::GameLift::Metrics::IsSupported<                         \
                      metric::MetricType, Aws::GameLift::Metrics::Gauge,       \
                      Aws::GameLift::Metrics::Counter>::value,                 \
                  "Metric '" #metric                                           \
                  "' is not a gauge or counter. GAMELIFT_METRICS_INCREMENT "   \
                  "only supports gauges and counters.");                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, 1)                                  \
  } while (0)

/**
 * @brief This macro decrements a gauge.
 *
 * @param metric Gauge or counter to decrement.
 */
#define GAMELIFT_METRICS_DECREMENT(metric)                                     \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric "' is not a gauge. GAMELIFT_METRICS_DECREMENT "     \
        "only supports gauges.");                                              \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, -1)                                 \
  } while (0)

/**
 * @brief This macro counts a line hit.
 *
 * Alias for GAMELIFT_METRICS_INCREMENT
 *
 * @param metric Gauge or counter to increment.
 */
#define GAMELIFT_METRICS_COUNT_HIT(metric)                                     \
  do {                                                                         \
    static_assert(Aws::GameLift::Metrics::IsSupported<                         \
                      metric::MetricType, Aws::GameLift::Metrics::Gauge,       \
                      Aws::GameLift::Metrics::Counter>::value,                 \
                  "Metric '" #metric                                           \
                  "' is not a gauge or counter. GAMELIFT_METRICS_COUNT_HIT "   \
                  "only supports gauges and counters.");                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, 1)                                  \
  } while (0)

/**
 * @brief This macro counts number of times an expression runs. For example,
 *     a function call.
 *
 * @param metric Gauge or counter to increment.
 * @param expr Expression to count.
 * @returns Result of expr.
 */
#define GAMELIFT_METRICS_COUNT_EXPR(metric, expr)                              \
  [&]() {                                                                      \
    static_assert(Aws::GameLift::Metrics::IsSupported<                         \
                      metric::MetricType, Aws::GameLift::Metrics::Gauge,       \
                      Aws::GameLift::Metrics::Counter>::value,                 \
                  "Metric '" #metric                                           \
                  "' is not a gauge or counter. GAMELIFT_METRICS_COUNT_EXPR "  \
                  "only supports gauges and counters.");                       \
    _GAMELIFT_METRICS_INTERNAL_ADD(metric, 1);                                 \
    return (expr);                                                             \
  }()
