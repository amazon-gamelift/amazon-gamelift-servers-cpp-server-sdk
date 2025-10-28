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
#include <aws/gamelift/metrics/DerivedMetric.h>
#include <aws/gamelift/metrics/InternalTypes.h>

/**
 * INTERNAL: declares a metric class
 *
 * @brief This macro generates a forward declaration for any classes or
 * functions required by our implementation.
 *
 * Can be placed into headers safely.
 */
#define _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(api, name, key, platform,      \
                                                sampler_expr, type, ...)       \
  class api name : public ::Aws::GameLift::Metrics::IMetric {                  \
  public:                                                                      \
    using Platform = platform;                                                 \
    using MetricType = type;                                                   \
    using SamplerType = decltype(sampler_expr);                                \
    using DerivedMetricCollectionType =                                        \
        decltype(Aws::GameLift::Metrics::CollectDerivedMetrics(__VA_ARGS__));  \
                                                                               \
    static name &Instance();                                                   \
                                                                               \
    virtual ::Aws::GameLift::Metrics::MetricType                               \
    GetMetricType() const override {                                           \
      return ::Aws::GameLift::Metrics::GetMetricType<MetricType>();            \
    }                                                                          \
    virtual const char *GetKey() const override { return Key; }                \
    virtual Aws::GameLift::Metrics::IDerivedMetricCollection &                 \
    GetDerivedMetrics() override {                                             \
      return DerivedMetrics;                                                   \
    }                                                                          \
                                                                               \
    virtual Aws::GameLift::Metrics::ISampler &GetSampler() override {          \
      return Sampler;                                                          \
    }                                                                          \
                                                                               \
  private:                                                                     \
    const char *Key;                                                           \
    SamplerType Sampler;                                                       \
    DerivedMetricCollectionType DerivedMetrics;                                \
                                                                               \
    name()                                                                     \
        : Key(key), Sampler(sampler_expr),                                     \
          DerivedMetrics(                                                      \
              Aws::GameLift::Metrics::CollectDerivedMetrics(__VA_ARGS__)) {}   \
  }

/**
 * INTERNAL: defines a metric class
 *
 * This implements any methods forward-declared by
 * _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS.
 *
 * Intended for a single source file to define global state.
 */
#define _GAMELIFT_INTERNAL_DEFINE_METRIC_CLASS(metric)                         \
  metric &metric::Instance() {                                                 \
    static metric GlobalInstance;                                              \
    return GlobalInstance;                                                     \
  }

/**
 * @brief This macro declares a gauge.
 *
 * Gauges are metrics that represent a snapshot of a single value like the
 * player count or used memory. Once set, the value remains until modified
 * again.
 *
 * Once declared, the gauge must be defined with GAMELIFT_METRICS_DEFINE_GAUGE.
 *
 * @param name The name of this gauge.
 * @param key The key for the metric tied to this gauge.
 * @param platform Platform to sample this gauge on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_GAUGE(name, key, platform, sampler, ...)      \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(, name, key, platform, sampler,      \
                                          Aws::GameLift::Metrics::Gauge,       \
                                          ##__VA_ARGS__)

/**
 * @brief This macro declares a gauge.
 *
 * Gauges are metrics that represent a snapshot of a single value like the
 * player count or used memory. Once set, the value remains until modified
 * again.
 *
 * Once declared, the gauge must be defined with GAMELIFT_METRICS_DEFINE_GAUGE.
 *
 * @param api Api export for metric class.
 * @param name The name of this gauge.
 * @param key The key for the metric tied to this gauge.
 * @param platform Platform to sample this gauge on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_GAUGE_API(api, name, key, platform, sampler,  \
                                           ...)                                \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(api, name, key, platform, sampler,   \
                                          Aws::GameLift::Metrics::Gauge,       \
                                          ##__VA_ARGS__)

/**
 * @brief This macro defines a gauge.
 *
 * Prior to definition, the gauge must be declared with
 * GAMELIFT_METRICS_DECLARE_GAUGE.
 *
 * @param metric A gauge declared via `GAMELIFT_METRICS_DECLARE_GAUGE`.
 */
#define GAMELIFT_METRICS_DEFINE_GAUGE(metric)                                  \
  static_assert(Aws::GameLift::Metrics::IsSupported<                           \
                    metric::MetricType, Aws::GameLift::Metrics::Gauge>::value, \
                "Metric '" #metric "' is not a gauge.");                       \
  _GAMELIFT_INTERNAL_DEFINE_METRIC_CLASS(metric)

/**
 * @brief This macro declares a counter.
 *
 * Counters are metrics that keep track of occurences over time, for example,
 * the number of bytes sent, number of shots fired in-game, or the number of
 * times a function was called.
 *
 * Once declared, the counter must be defined with
 * GAMELIFT_METRICS_DEFINE_COUNTER.
 *
 * @param name The name of this counter.
 * @param key The key for the metric tied to this counter.
 * @param platform Platform to sample this counter on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_COUNTER(name, key, platform, sampler)         \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(, name, key, platform, sampler,      \
                                          Aws::GameLift::Metrics::Counter)

/**
 * @brief This macro declares a counter.
 *
 * Counters are metrics that keep track of occurences over time, for example,
 * the number of bytes sent, number of shots fired in-game, or the number of
 * times a function was called.
 *
 * Once declared, the counter must be defined with
 * GAMELIFT_METRICS_DEFINE_COUNTER.
 *
 * @param api Api export for metric class.
 * @param name The name of this counter.
 * @param key The key for the metric tied to this counter.
 * @param platform Platform to sample this counter on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_COUNTER_API(api, name, key, platform,         \
                                             sampler)                          \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(api, name, key, platform, sampler,   \
                                          Aws::GameLift::Metrics::Counter)

/**
 * @brief This macro defines a counter.
 *
 * The counter must be declared with GAMELIFT_METRICS_DECLARE_COUNTER first.
 *
 * @param metric A counter declared via `GAMELIFT_METRICS_DECLARE_COUNTER`.
 */
#define GAMELIFT_METRICS_DEFINE_COUNTER(metric)                                \
  static_assert(                                                               \
      Aws::GameLift::Metrics::IsSupported<                                     \
          metric::MetricType, Aws::GameLift::Metrics::Counter>::value,         \
      "Metric '" #metric "' is not a counter.");                               \
  _GAMELIFT_INTERNAL_DEFINE_METRIC_CLASS(metric)

/**
 * @brief This macro declares a timer.
 *
 * Timers are metrics that represent how long something takes, for example, how
 * long a game tick takes, the duration of a user session, or the timing of a
 * function.
 *
 * Once declared, the timer must be defined with GAMELIFT_METRICS_DEFINE_TIMER.
 *
 * @param name The name of this timer.
 * @param key The key for the metric tied to this timer.
 * @param platform Platform to sample this timer on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_TIMER(name, key, platform, sampler, ...)      \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(, name, key, platform, sampler,      \
                                          Aws::GameLift::Metrics::Timer,       \
                                          ##__VA_ARGS__)

/**
 * @brief This macro declares a timer.
 *
 * Timers are metrics that represent how long something takes, for example, how
 * long a game tick takes, the duration of a user session, or the timing of a
 * function.
 *
 * Once declared, the timer must be defined with GAMELIFT_METRICS_DEFINE_TIMER.
 *
 * @param api Api export for metric class.
 * @param name The name of this timer.
 * @param key The key for the metric tied to this timer.
 * @param platform Platform to sample this timer on.
 * @param sampler The sampling strategy to use with this metric.
 */
#define GAMELIFT_METRICS_DECLARE_TIMER_API(api, name, key, platform, sampler,  \
                                           ...)                                \
  _GAMELIFT_INTERNAL_DECLARE_METRIC_CLASS(api, name, key, platform, sampler,   \
                                          Aws::GameLift::Metrics::Timer,       \
                                          ##__VA_ARGS__)

/**
 * @brief This macro defines a timer.
 *
 * The timer must be declared with GAMELIFT_METRICS_DECLARE_TIMER first.
 *
 * @param metric A timer declared via `GAMELIFT_METRICS_DECLARE_TIMER`.
 */
#define GAMELIFT_METRICS_DEFINE_TIMER(metric)                                  \
  static_assert(Aws::GameLift::Metrics::IsSupported<                           \
                    metric::MetricType, Aws::GameLift::Metrics::Timer>::value, \
                "Metric '" #metric "' is not a timer.");                       \
  _GAMELIFT_INTERNAL_DEFINE_METRIC_CLASS(metric)
