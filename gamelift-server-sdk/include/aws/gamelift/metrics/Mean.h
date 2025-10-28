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

#include <aws/gamelift/metrics/ReduceMetric.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class Real> struct OpMean {
  size_t sampleCount = 0;

  Real operator()(Real current, Real next) {
    ++sampleCount;
    const double update = (next - current) / sampleCount;
    return current + update;
  }
};

using MeanReducer = Reduce<Internal::OpMean<double>>;

} // namespace Internal

struct Mean : public Internal::MeanReducer {
  /**
   * Computes the mean over capture period as `metric.mean`.
   */
  Mean() : Internal::MeanReducer(".mean", 0) {}

  /**
   * Computes the mean over capture period as `metricSUFFIX` with user specified
   * SUFFIX.
   *
   * @param keySuffix Suffix to append to the parent metric name.
   */
  explicit Mean(const char *keySuffix) : Internal::MeanReducer(keySuffix) {}

  /**
   * @brief Gets the key suffix
   * @return The current key suffix
   */
  const char *GetKeySuffix() const {
#ifdef GAMELIFT_USE_STD
    return m_keySuffix.GetSuffix().c_str();
#else
    return m_keySuffix.GetSuffix();
#endif
  }

  /**
   * @brief Sets the key suffix
   * @param keySuffix Suffix to append to the parent metric name
   */
  void SetKeySuffix(const char *keySuffix) {
    m_keySuffix = KeySuffix(keySuffix);
  }

  /**
   * @brief Fluent setter for the key suffix
   * @param keySuffix Suffix to append to the parent metric name
   * @return Reference to this object for method chaining
   */
  Mean &WithKeySuffix(const char *keySuffix) {
    SetKeySuffix(keySuffix);
    return *this;
  }

  virtual void EmitMetrics(const IMetric *originalMetric,
                           IMetricsEnqueuer &submitter) override {
    Internal::MeanReducer::EmitMetrics(originalMetric, submitter);
    operation.sampleCount = 0;
  }

private:
  KeySuffix m_keySuffix;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws