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

#ifdef GAMELIFT_USE_STD
#include <cstdint>
#include <string>
#include <utility>
#else
#include <cstring>
#include <stdint.h>
#endif

#include <aws/gamelift/metrics/Defs.h>
#include <aws/gamelift/metrics/DynamicTag.h>
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/server/model/GameSession.h>

namespace Aws {
namespace GameLift {
namespace Metrics {

enum class GAMELIFT_METRICS_API MetricMessageType : UInt8 {
  None,

  GaugeSet,
  GaugeAdd,

  CounterAdd,

  TimerSet,

  TagSet,
  TagRemove
};

struct GAMELIFT_METRICS_API MetricSubmitDouble final {
  double Value;

  /**
   * @brief Submits a double value metric.
   *
   * @param value The double value to be submitted.
   */
  constexpr MetricSubmitDouble(double value) noexcept : Value(value) {}

  /**
   * @brief Gets the double value
   * @return The stored double value
   */
  double GetValue() const { return Value; }

  /**
   * @brief Sets the double value
   * @param value The new double value
   */
  void SetValue(double value) { Value = value; }

  /**
   * @brief Fluent setter for the double value
   * @param value The new double value
   * @return Reference to this object for method chaining
   */
  MetricSubmitDouble &WithValue(double value) {
    SetValue(value);
    return *this;
  }

  friend bool operator!=(const MetricSubmitDouble &a,
                         const MetricSubmitDouble &b) {
    return !(a == b);
  }
  friend bool operator==(const MetricSubmitDouble &a,
                         const MetricSubmitDouble &b) {
    return a.Value == b.Value;
  }
};

class DynamicTag;

struct GAMELIFT_METRICS_API MetricSetTag final {
  DynamicTag *Ptr;

  /**
   * @brief Tags next submission with tag.
   *
   * @param ptr Pointer to the tag to apply to the next metrics submission.
   */
  constexpr explicit MetricSetTag(DynamicTag *ptr) noexcept : Ptr(ptr) {}

  /**
   * @brief Gets the pointer to the DynamicTag
   * @return The DynamicTag pointer
   */
  DynamicTag *GetPtr() const { return Ptr; }

  /**
   * @brief Sets the pointer to the DynamicTag
   * @param ptr The DynamicTag pointer
   */
  void SetPtr(DynamicTag *ptr) { Ptr = ptr; }

  /**
   * @brief Fluent setter for the pointer
   * @param ptr The DynamicTag pointer
   * @return Reference to this object for method chaining
   */
  MetricSetTag &WithPtr(DynamicTag *ptr) {
    SetPtr(ptr);
    return *this;
  }

  friend bool operator!=(const MetricSetTag &a, const MetricSetTag &b) {
    return !(a == b);
  }
  friend bool operator==(const MetricSetTag &a, const MetricSetTag &b);
};

/**
 * @brief Represents a metric being logged at a call site.
 */
struct GAMELIFT_METRICS_API MetricMessage final {
  MetricMessageType Type;

  IMetric *Metric;
  /*
   * This union is tagged by `Type`.
   */
  union {
    MetricSubmitDouble SubmitDouble;
    MetricSetTag SetTag;
  };

  MetricMessage() : Type(MetricMessageType::None), Metric(nullptr) {}
#ifdef GAMELIFT_USE_STD
  MetricMessage(MetricMessageType type, IMetric *metric,
                MetricSubmitDouble &&submitDouble)
      : Type(type), Metric(metric),
        SubmitDouble(std::forward<MetricSubmitDouble>(submitDouble)) {}
  MetricMessage(MetricMessageType type, IMetric *metric, MetricSetTag &&setTag)
      : Type(type), Metric(metric), SetTag(std::forward<MetricSetTag>(setTag)) {
  }
#else
  MetricMessage(MetricMessageType type, IMetric *metric,
                const MetricSubmitDouble &submitDouble)
      : Type(type), Metric(metric), SubmitDouble(submitDouble) {}
  MetricMessage(MetricMessageType type, IMetric *metric,
                const MetricSetTag &setTag)
      : Type(type), Metric(metric), SetTag(setTag) {}
#endif

  /**
   * @brief Gets the message type
   * @return The metric message type
   */
  MetricMessageType GetType() const { return Type; }

  /**
   * @brief Sets the message type
   * @param type The metric message type
   */
  void SetType(MetricMessageType type) { Type = type; }

  /**
   * @brief Fluent setter for the message type
   * @param type The metric message type
   * @return Reference to this object for method chaining
   */
  MetricMessage &WithType(MetricMessageType type) {
    SetType(type);
    return *this;
  }

  /**
   * @brief Gets the metric pointer
   * @return The metric pointer
   */
  IMetric *GetMetric() const { return Metric; }

  /**
   * @brief Sets the metric pointer
   * @param metric The metric pointer
   */
  void SetMetric(IMetric *metric) { Metric = metric; }

  /**
   * @brief Fluent setter for the metric pointer
   * @param metric The metric pointer
   * @return Reference to this object for method chaining
   */
  MetricMessage &WithMetric(IMetric *metric) {
    SetMetric(metric);
    return *this;
  }

  bool IsGauge() const noexcept {
    switch (Type) {
    case MetricMessageType::GaugeSet:
    case MetricMessageType::GaugeAdd:
      return true;
    default:
      return false;
    }
  }

  bool IsCounter() const noexcept {
    switch (Type) {
    case MetricMessageType::CounterAdd:
      return true;
    default:
      return false;
    }
  }

  bool IsTimer() const noexcept {
    switch (Type) {
    case MetricMessageType::TimerSet:
      return true;
    default:
      return false;
    }
  }

  bool IsTag() const noexcept {
    switch (Type) {
    case MetricMessageType::TagSet:
    case MetricMessageType::TagRemove:
      return true;
    default:
      return false;
    }
  }

  bool IsDouble() const noexcept { return true; }

  /*
   * METRICS
   */
  static MetricMessage GaugeSet(IMetric &metric, double value) {
    return MetricMessage(MetricMessageType::GaugeSet, &metric,
                         MetricSubmitDouble(value));
  }
  static MetricMessage GaugeAdd(IMetric &metric, double value) {
    return MetricMessage(MetricMessageType::GaugeAdd, &metric,
                         MetricSubmitDouble(value));
  }
  static MetricMessage CounterAdd(IMetric &metric, double value) {
    return MetricMessage(MetricMessageType::CounterAdd, &metric,
                         MetricSubmitDouble(value));
  }
  static MetricMessage TimerSet(IMetric &metric, double value) {
    return MetricMessage(MetricMessageType::TimerSet, &metric,
                         MetricSubmitDouble(value));
  }

  /*
   * TAGS
   */
#ifdef GAMELIFT_USE_STD
  static MetricMessage TagSet(IMetric &metric, std::string key,
                              std::string value);
  static MetricMessage TagRemove(IMetric &metric, std::string key);
#else
  static MetricMessage TagSet(IMetric &metric, const char *key,
                              const char *value);
  static MetricMessage TagRemove(IMetric &metric, const char *key);
#endif

  friend bool operator!=(const MetricMessage &a, const MetricMessage &b) {
    return !(a == b);
  }
  friend bool operator==(const MetricMessage &a, const MetricMessage &b) {
    if (a.Type != b.Type) {
      return false;
    }

    // intentional ptr comparison as metric should be a global, static value
    if (a.Metric != b.Metric) {
      return false;
    }

    if (a.IsTag()) {
      return a.SetTag == b.SetTag;
    } else {
      return a.SubmitDouble == b.SubmitDouble;
    }
  }
};

struct GAMELIFT_METRICS_API IMetricsEnqueuer {
  virtual ~IMetricsEnqueuer();

  /**
   * Enqueues message for later processing.
   *
   * Thread safe.
   */
  virtual void Enqueue(MetricMessage message) = 0;
};

/**
 * Copies a tag message for another metric
 */
extern GAMELIFT_METRICS_API void CopyTagMessage(const MetricMessage &original,
                                                IMetric &destMetric,
                                                IMetricsEnqueuer &enqueuer);

/**
 * Metrics interface to faciliate testing.
 */
class GAMELIFT_METRICS_API IMetricsProcessor : public IMetricsEnqueuer {
public:
  virtual ~IMetricsProcessor();

#ifdef GAMELIFT_USE_STD
  /**
   * Sets a key:value tag to be applied to all metrics.
   */
  virtual void SetGlobalTag(const std::string &key,
                            const std::string &value) = 0;

  /**
   * Removes a global tag by key.
   */
  virtual void RemoveGlobalTag(const std::string &key) = 0;
#else
  /**
   * Sets a key:value tag to be applied to all metrics.
   */
  virtual void SetGlobalTag(const char *key, const char *value) = 0;

  /**
   * Removes a global tag by key.
   */
  virtual void RemoveGlobalTag(const char *key) = 0;
#endif // GAMELIFT_USE_STD

  /**
   * Processes all the current metrics in the queue if the next capture time has
   * been reached.
   *
   * This includes:
   * - Summing and averaging counters or timers logged during the capture
   * period.
   * - Computing derived stats.
   * - Assembling a statsd packet.
   * - Sending the packet off.
   */
  virtual void ProcessMetrics() = 0;

  /**
   * Processes all the current metrics in the queue irrespective of the next
   * capture time. Intended for use on terminate or for a time critical flush.
   *
   * This includes:
   * - Summing and averaging counters or timers logged during the capture
   * period.
   * - Computing derived stats.
   * - Assembling a statsd packet.
   * - Sending the packet off.
   */
  virtual void ProcessMetricsNow() = 0;

  /**
   * @brief Called when a game session is started
   * @param session The game session that was started
   */
  virtual void OnStartGameSession(
      const Aws::GameLift::Server::Model::GameSession &session) = 0;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
