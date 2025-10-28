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
#include <aws/gamelift/metrics/LoggerMacros.h>
#include <aws/gamelift/metrics/UniquePtr.h>
#include <vector>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {
class GAMELIFT_METRICS_API PercentilesWrapper : public IDerivedMetric {
public:
  static PercentilesWrapper Create(double *percentilesItBegin,
                                   double *percentilesItEnd);

  virtual void HandleMessage(MetricMessage &message,
                             IMetricsEnqueuer &submitter) override {
    m_impl->HandleMessage(message, submitter);
  }

  virtual void EmitMetrics(const IMetric *originalMetric,
                           IMetricsEnqueuer &submitter) override {
    m_impl->EmitMetrics(originalMetric, submitter);
  }

private:
  explicit PercentilesWrapper(IDerivedMetric *impl) : m_impl(impl) {}

private:
  UniquePtr<IDerivedMetric> m_impl;
};
} // namespace Internal

/**
 * Emit a series of percentiles.
 */
template <class... Real>
inline Internal::PercentilesWrapper Percentiles(Real... values) {
  std::vector<double> valuesVec = {static_cast<double>(values)...};
  double *valuesBegin = valuesVec.data();
  double *valuesEnd = valuesBegin + valuesVec.size();
  return Internal::PercentilesWrapper::Create(valuesBegin, valuesEnd);
}

/**
 * Compute the median as .p50
 */
inline Internal::PercentilesWrapper Median() { return Percentiles(0.5); }

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
