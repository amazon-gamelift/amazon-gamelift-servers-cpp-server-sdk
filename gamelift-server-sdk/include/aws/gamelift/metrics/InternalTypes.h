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
#include <aws/gamelift/metrics/TypeTraits.h>

#ifdef GAMELIFT_USE_STD
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace Aws {
namespace GameLift {
namespace Metrics {
using UInt8 = uint8_t;
using Int64 = int64_t;

struct MetricMessage;
class IMetricsProcessor;
struct ISampler;

/*
 * Marker types.
 */
class Gauge {};
class Counter {};
class Timer {};

struct IDerivedMetricCollection;

enum class MetricType { Gauge, Counter, Timer };

template <class MarkerType> inline MetricType GetMetricType();

template <> inline MetricType GetMetricType<Gauge>() {
  return MetricType::Gauge;
}

template <> inline MetricType GetMetricType<Counter>() {
  return MetricType::Counter;
}

template <> inline MetricType GetMetricType<Timer>() {
  return MetricType::Timer;
}

/**
 * INTERNAL: Interface for all user-defined metrics.
 */
struct GAMELIFT_METRICS_API IMetric {
  virtual ~IMetric();

  virtual MetricType GetMetricType() const = 0;
  virtual const char *GetKey() const = 0;
  virtual IDerivedMetricCollection &GetDerivedMetrics() = 0;
  virtual ISampler &GetSampler() = 0;
};

/**
 * Utility class to verify a metric type is supported in static_asserts within
 * macros.
 *
 * For IsSupported::value to evaluate to true, Type must be one of the
 * subsequent types in the template params.
 */
template <class Type, class First, class... Rest>
struct IsSupported : Internal::Conditional<Internal::IsSame<Type, First>::value,
                                           Internal::TrueType,
                                           IsSupported<Type, Rest...>>::type {};

/**
 * Base case: only two types to compare.
 */
template <class Type, class First>
struct IsSupported<Type, First> : Internal::IsSame<Type, First> {};
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
