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

#include <aws/gamelift/metrics/DerivedMetric.h>
#include <aws/gamelift/metrics/DynamicMetric.h>
#include <aws/gamelift/metrics/KeySuffix.h>
#include <aws/gamelift/metrics/TypeTraits.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
/**
 * @brief Applies a reduction on a metric.
 *
 * @param Op Functor taking a pair of doubles - current, new and returning the
 * modified value.
 */
template <class Op> class Reduce : public IDerivedMetric {
public:
  /**
   * @param suffix Suffix to add to the modified key.
   */
  Reduce(KeySuffix &&suffix)
      : m_suffix(Internal::Forward<KeySuffix>(suffix)), m_result(0),
        m_metricInitialized(false) {}

  /**
   * @param suffix Suffix to add to the modified key.
   * @param initialValue Value to initialize the reducer with.
   */
  Reduce(KeySuffix &&suffix, double initialValue)
      : m_suffix(Internal::Forward<KeySuffix>(suffix)), m_result(initialValue),
        m_metricInitialized(false) {}

  /**
   * @param suffix Suffix to add to the modified key.
   * @param initialValue Value to initialize the reducer with.
   */
  Reduce(KeySuffix &&suffix, double initialValue, Op &&operation)
      : m_suffix(Internal::Forward<KeySuffix>(suffix)), m_result(initialValue),
        operation(Internal::Forward<Op>(operation)),
        m_metricInitialized(false) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to the modified key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to the modified key
   * @return Reference to this object for method chaining
   */
  Reduce &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }

  virtual void HandleMessage(MetricMessage &message,
                             IMetricsEnqueuer &submitter) override {
    switch (message.Type) {
    case MetricMessageType::GaugeAdd:
      m_currentValue += message.SubmitDouble.Value;
      UpdateResult();
      break;
    case MetricMessageType::GaugeSet:
    case MetricMessageType::TimerSet:
      m_currentValue = message.SubmitDouble.Value;
      UpdateResult();
      break;
    case MetricMessageType::TagSet:
    case MetricMessageType::TagRemove:
      CopyTagMessage(message, m_metric, submitter);
      break;
    default:
      break;
    }
  }

  virtual void EmitMetrics(const IMetric *originalMetric,
                           IMetricsEnqueuer &submitter) override {
    if (m_numSeenSinceLastEmitCall == 0) {
      return;
    }
    m_numSeenSinceLastEmitCall = 0;

    if (!m_metricInitialized) {
      m_suffix.Apply(*originalMetric, m_metric);
      m_metric.SetMetricType(originalMetric->GetMetricType());
      m_metricInitialized = true;
    }

    switch (m_metric.GetMetricType()) {
    case MetricType::Gauge:
      submitter.Enqueue(MetricMessage::GaugeSet(m_metric, m_result));
      break;
    case MetricType::Timer:
      submitter.Enqueue(MetricMessage::TimerSet(m_metric, m_result));
      break;
    default:
      break;
    }
  }

protected:
  void UpdateResult() {
    m_result = operation(m_result, m_currentValue);
    m_numSeenSinceLastEmitCall++;
  }

protected:
  KeySuffix m_suffix;

  Op operation; // Op needs to be protected for Mean to work properly
  double m_result = 0;

  size_t m_numSeenSinceLastEmitCall = 0;
  double m_currentValue = 0;
  DynamicMetric m_metric;

  bool m_metricInitialized = false;
};

namespace Internal {
template <class T> struct OpMax final {
  T operator()(T Current, T New) { return New >= Current ? New : Current; }
};

template <class T> struct OpMin final {
  T operator()(T Current, T New) { return New <= Current ? New : Current; }
};

template <class T> struct OpSum final {
  T operator()(T Current, T New) { return Current + New; }
};

template <class T> struct OpCount final {
  T operator()(T Current, T New) { return ++Current; }
};
} // namespace Internal

/**
 * @brief Logs the maximum of all values seen during the capture period.
 */
struct Max : public Reduce<Internal::OpMax<double>> {
  /**
   * Initializes a Max metric that logs the maximum value during the capture
   * period.
   */
  Max() : Reduce(".max", -Internal::NumericLimits<double>::max()) {}

  /**
   * @param suffix Suffix to add to the modified key.
   */
  explicit Max(KeySuffix &&suffix)
      : Reduce(Internal::Forward<KeySuffix>(suffix),
               -Internal::NumericLimits<double>::max()) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to the modified key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to the modified key
   * @return Reference to this object for method chaining
   */
  Max &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }
};

/**
 * @brief Logs the minimum of all values seen during the capture period.
 */
struct Min : public Reduce<Internal::OpMin<double>> {
  /**
   * Initializes a Min metric that logs the minimum value during the capture
   * period.
   */
  Min() : Reduce(".min", Internal::NumericLimits<double>::max()) {}

  /**
   * @param suffix Suffix to add to the modified key.
   */
  explicit Min(KeySuffix &&suffix)
      : Reduce(Internal::Forward<KeySuffix>(suffix),
               Internal::NumericLimits<double>::max()) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to the modified key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to the modified key
   * @return Reference to this object for method chaining
   */
  Min &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }
};

/**
 * @brief Logs the sum of all values seen during the capture period.
 */
struct Sum : public Reduce<Internal::OpSum<double>> {
  /**
   * Initializes a Sum metric that logs the sum of values during the capture
   * period.
   */
  Sum() : Reduce(".sum", 0) {}

  /**
   * @param suffix Suffix to add to the modified key.
   */
  explicit Sum(KeySuffix &&suffix)
      : Reduce(Internal::Forward<KeySuffix>(suffix), 0) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to the modified key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to the modified key
   * @return Reference to this object for method chaining
   */
  Sum &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }
};

/**
 * @brief Logs the number of metrics seen.
 */
struct Count : public Reduce<Internal::OpCount<double>> {
  /**
   * Initializes a Count metric that logs the number of metrics during the
   * capture period.
   */
  Count() : Reduce(".count", 0) {}

  /**
   * @param suffix Suffix to add to the modified key.
   */
  explicit Count(KeySuffix &&suffix)
      : Reduce(Internal::Forward<KeySuffix>(suffix), 0) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to the modified key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to the modified key
   * @return Reference to this object for method chaining
   */
  Count &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }
};
} // namespace Metrics
} // namespace GameLift
} // namespace Aws