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
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/LoggerMacros.h>

/**
 * @brief Sets a gauge to a value.
 *
 * Always executes expression.
 *
 * @param metric The gauge to set.
 * @param expr The value to set the gauge to.
 *
 * @returns the result of expr
 */
#define GAMELIFT_METRICS_SET(metric, expr)                                     \
  [&]() {                                                                      \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric                                                     \
        "' is not a gauge. GAMELIFT_METRICS_SET only supports gauges.");       \
    auto Value = (expr);                                                       \
    IF_CONSTEXPR(metric::Platform::bEnabled) {                                 \
      if (metric::Instance().GetSampler().ShouldTakeSample()) {                \
        auto *Processor = GameLiftMetricsGlobalProcessor();                    \
        if (Processor) {                                                       \
          Processor->Enqueue(                                                  \
              ::Aws::GameLift::Metrics::MetricMessage::GaugeSet(               \
                  metric::Instance(), Value));                                 \
        } else {                                                               \
          GAMELIFT_METRICS_LOG_CRITICAL(                                       \
              "Global metrics processor is not initialized");                  \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    return Value;                                                              \
  }()

/**
 * @brief Sets a gauge to a value.
 *
 * Only executes expression when sampled.
 *
 * @param metric The gauge to set.
 * @param expr The value to set the gauge to.
 */
#define GAMELIFT_METRICS_SET_SAMPLED(metric, expr)                             \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric "' is not a gauge. GAMELIFT_METRICS_SET_SAMPLED "   \
        "only supports gauges.");                                              \
    IF_CONSTEXPR(metric::Platform::bEnabled) {                                 \
      if (metric::Instance().GetSampler().ShouldTakeSample()) {                \
        auto *Processor = GameLiftMetricsGlobalProcessor();                    \
        if (Processor) {                                                       \
          Processor->Enqueue(                                                  \
              ::Aws::GameLift::Metrics::MetricMessage::GaugeSet(               \
                  metric::Instance(), (expr)));                                \
        } else {                                                               \
          GAMELIFT_METRICS_LOG_CRITICAL(                                       \
              "Global metrics processor is not initialized");                  \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

/**
 * @brief Resets a gauge back to zero.
 *
 * @param metric The gauge to set.
 */
#define GAMELIFT_METRICS_RESET(metric)                                         \
  do {                                                                         \
    static_assert(                                                             \
        Aws::GameLift::Metrics::IsSupported<                                   \
            metric::MetricType, Aws::GameLift::Metrics::Gauge>::value,         \
        "Metric '" #metric                                                     \
        "' is not a gauge. GAMELIFT_METRICS_RESET only supports gauges.");     \
    IF_CONSTEXPR(metric::Platform::bEnabled) {                                 \
      auto *Processor = GameLiftMetricsGlobalProcessor();                      \
      if (Processor) {                                                         \
        Processor->Enqueue(::Aws::GameLift::Metrics::MetricMessage::GaugeSet(  \
            metric::Instance(), 0));                                           \
      } else {                                                                 \
        GAMELIFT_METRICS_LOG_CRITICAL(                                         \
            "Global metrics processor is not initialized");                    \
      }                                                                        \
    }                                                                          \
  } while (0)
