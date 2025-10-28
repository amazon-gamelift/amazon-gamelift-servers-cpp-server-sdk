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

/**
 * @brief Sets a global tag applied to all metrics.
 *
 * @param platform The platform on which this tag is set. If not enabled, the
 * tag will not be added.
 * @param key Key string
 * @param expr String expression
 */
#define GAMELIFT_METRICS_GLOBAL_TAG_SET(platform, key, expr)                   \
  IF_CONSTEXPR(platform::bEnabled) {                                           \
    auto *processor = GameLiftMetricsGlobalProcessor();                        \
    if (processor) {                                                           \
      processor->SetGlobalTag(key, expr);                                      \
    } else {                                                                   \
      GAMELIFT_METRICS_LOG_CRITICAL(                                           \
          "Global metrics processor is not initialized");                      \
    }                                                                          \
  }

/**
 * @brief Removes global tag.
 *
 * @param platform The platform on which this tag is set. If not enabled, the
 * tag will not be added.
 * @param key Key string
 */
#define GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(platform, key)                      \
  IF_CONSTEXPR(platform::bEnabled) {                                           \
    auto *processor = GameLiftMetricsGlobalProcessor();                        \
    if (processor) {                                                           \
      processor->RemoveGlobalTag(key);                                         \
    } else {                                                                   \
      GAMELIFT_METRICS_LOG_CRITICAL(                                           \
          "Global metrics processor is not initialized");                      \
    }                                                                          \
  }

/**
 * @brief Sets a tag for a specific metric.
 *
 * @param metric The metric to apply the tag to. Declared with
 * GAMELIFT_METRIC_DECLARE_XXXXXX
 * @param key Key string
 * @param expr String expression
 */
#define GAMELIFT_METRICS_TAG_SET(metric, key, expr)                            \
  IF_CONSTEXPR(metric::Platform::bEnabled) {                                   \
    auto *processor = GameLiftMetricsGlobalProcessor();                        \
    if (processor) {                                                           \
      processor->Enqueue(::Aws::GameLift::Metrics::MetricMessage::TagSet(      \
          metric::Instance(), key, (expr)));                                   \
    } else {                                                                   \
      GAMELIFT_METRICS_LOG_CRITICAL(                                           \
          "Global metrics processor is not initialized");                      \
    }                                                                          \
  }

/**
 * @brief Removes a tag for a specific metric.
 *
 * @param metric The metric to apply the tag to. Declared with
 * GAMELIFT_METRIC_DECLARE_XXXXXX
 * @param key Key string
 */
#define GAMELIFT_METRICS_TAG_REMOVE(metric, key)                               \
  IF_CONSTEXPR(metric::Platform::bEnabled) {                                   \
    auto *processor = GameLiftMetricsGlobalProcessor();                        \
    if (processor) {                                                           \
      processor->Enqueue(::Aws::GameLift::Metrics::MetricMessage::TagRemove(   \
          metric::Instance(), key));                                           \
    } else {                                                                   \
      GAMELIFT_METRICS_LOG_CRITICAL(                                           \
          "Global metrics processor is not initialized");                      \
    }                                                                          \
  }