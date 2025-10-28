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

#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/TypeTraits.h>

namespace Aws {
namespace GameLift {
namespace Metrics {

/**
 * Metric computed from an existing metric.
 */
struct GAMELIFT_METRICS_API IDerivedMetric {
  virtual ~IDerivedMetric();

  /**
   * @brief Handles metric messages.
   *
   * This is the place to intercept them and update internal state.
   *
   * @param message The metric message to handle.
   * @param submitter The enqueuer to submit the metric to.
   */
  virtual void HandleMessage(MetricMessage &message,
                             IMetricsEnqueuer &submitter) = 0;

  /**
   * @brief Emits metrics before sending.
   *
   * Called once per parent metric per capture period.
   *
   * @param originalMetric The original metric that this derived metric is based
   * on.
   * @param submitter The enqueuer to submit the metric to.
   */
  virtual void EmitMetrics(const IMetric *originalMetric,
                           IMetricsEnqueuer &submitter) = 0;
};

struct GAMELIFT_METRICS_API IDerivedMetricVisitor {
  virtual ~IDerivedMetricVisitor();

  virtual void VisitDerivedMetric(IDerivedMetric &metric) = 0;
};

struct GAMELIFT_METRICS_API IDerivedMetricCollection {
  virtual ~IDerivedMetricCollection();

  /**
   * @brief Visits all derived metrics in the collection.
   *
   * Use visitor pattern to add a visitor from the processor which
   * will handle message and then later call emit metrics!
   *
   * @param visitor The visitor to call for each derived metric.
   */
  virtual void Visit(IDerivedMetricVisitor &visitor) = 0;
};

/**
 * INTERNAL: Statically holds the derivative metric types
 */
template <class... DerivedMetric> struct DerivedMetricCollection;

/**
 * Specialization for empty collection.
 *
 * No-op visitor.
 */
template <> struct DerivedMetricCollection<> : public IDerivedMetricCollection {
  DerivedMetricCollection() = default;

  virtual void Visit(IDerivedMetricVisitor &visitor) override {
    (void)visitor; // Suppress unused parameter warning
  }
};

/**
 * Recursive specialization for collections with multiple values.
 */
template <class First, class... Rest>
struct DerivedMetricCollection<First, Rest...>
    : public DerivedMetricCollection<Rest...> {
  using Base = DerivedMetricCollection<Rest...>;

  DerivedMetricCollection(First &&firstMetric, Rest &&...otherMetrics)
      : Base(Internal::Forward<Rest>(otherMetrics)...),
        m_metric(Internal::Forward<First>(firstMetric)) {}

  virtual void Visit(IDerivedMetricVisitor &visitor) override {
    visitor.VisitDerivedMetric(m_metric);
    Base::Visit(visitor);
  }

private:
  First m_metric;
};

/**
 * Base case: just one derived metric.
 */
template <class Single>
struct DerivedMetricCollection<Single> : public IDerivedMetricCollection {
  DerivedMetricCollection(Single &&singleMetric)
      : m_metric(Internal::Forward<Single>(singleMetric)) {}

  virtual void Visit(IDerivedMetricVisitor &visitor) override {
    visitor.VisitDerivedMetric(m_metric);
  }

private:
  Single m_metric;
};

/**
 * INTERNAL: factory for DerivedMetrics - used to infer the full
 * DerivedMetricCollection<A, B, C, ...> type
 */
template <class... Types>
inline DerivedMetricCollection<Types...>
CollectDerivedMetrics(Types &&...Metrics) {
  return DerivedMetricCollection<Types...>(
      Internal::Forward<Types>(Metrics)...);
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
