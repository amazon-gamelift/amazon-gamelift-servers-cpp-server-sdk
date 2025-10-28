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
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/Samplers.h>

#ifdef GAMELIFT_USE_STD
#include <string>

namespace Aws {
namespace GameLift {
namespace Metrics {

class DynamicMetric : public IMetric {
public:
  virtual const char *GetKey() const override { return m_key.c_str(); }
  virtual MetricType GetMetricType() const override { return m_type; }
  virtual IDerivedMetricCollection &GetDerivedMetrics() override {
    static DerivedMetricCollection<> Collection;
    return Collection;
  }
  virtual ISampler &GetSampler() override {
    static SampleAll DefaultSampler;
    return DefaultSampler;
  }

  void SetKey(const char *key) { this->m_key = key; }
  void SetKey(std::string &&key) {
    this->m_key = std::forward<std::string>(key);
  }
  void SetMetricType(MetricType newType) { m_type = newType; }

  /**
   * @brief Fluent setter for the key
   * @param key The new key string
   * @return Reference to this object for method chaining
   */
  DynamicMetric &WithKey(const char *key) {
    SetKey(key);
    return *this;
  }

  /**
   * @brief Fluent setter for the key
   * @param key The new key string
   * @return Reference to this object for method chaining
   */
  DynamicMetric &WithKey(std::string &&key) {
    SetKey(std::forward<std::string>(key));
    return *this;
  }

  /**
   * @brief Fluent setter for the metric type
   * @param newType The new metric type
   * @return Reference to this object for method chaining
   */
  DynamicMetric &WithMetricType(MetricType newType) {
    SetMetricType(newType);
    return *this;
  }

private:
  std::string m_key;
  MetricType m_type;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#else
namespace Aws {
namespace GameLift {
namespace Metrics {

class GAMELIFT_METRICS_API DynamicMetric : public IMetric {
public:
  static constexpr int MAXIMUM_KEY_LENGTH = 1024;

  DynamicMetric() = default;
  DynamicMetric(const DynamicMetric &) = default;

  virtual const char *GetKey() const override { return m_key; }
  virtual MetricType GetMetricType() const override { return m_type; }
  virtual IDerivedMetricCollection &GetDerivedMetrics() override {
    static DerivedMetricCollection<> Collection;
    return Collection;
  }
  virtual ISampler &GetSampler() override {
    static SampleAll DefaultSampler;
    return DefaultSampler;
  }

  void SetKey(const char *key);

  void SetMetricType(MetricType newType) { m_type = newType; }

  /**
   * @brief Fluent setter for the key
   * @param key The new key string
   * @return Reference to this object for method chaining
   */
  DynamicMetric &WithKey(const char *key) {
    SetKey(key);
    return *this;
  }

  /**
   * @brief Fluent setter for the metric type
   * @param newType The new metric type
   * @return Reference to this object for method chaining
   */
  DynamicMetric &WithMetricType(MetricType newType) {
    SetMetricType(newType);
    return *this;
  }

  char m_key[MAXIMUM_KEY_LENGTH];
  MetricType m_type;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#endif // GAMELIFT_USE_STD
