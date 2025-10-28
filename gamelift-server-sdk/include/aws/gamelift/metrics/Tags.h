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
#include <string>
#include <unordered_map>

class DynamicTag;
using namespace ::Aws::GameLift::Metrics;

class Tags {
public:
  /**
   * @brief Handles a Tag message, updating current tags for metrics.
   *
   * Cleans up any dynamic allocations.
   */
  void Handle(MetricMessage &message);

  /**
   * @brief Retrieves the current tags for a particular metric.
   *
   * @param metricKey - the (static) key for the metric we're interested in.
   *
   * @return map of metrics for the metric or empty map if no tags are set.
   */
  const std::unordered_map<std::string, std::string> &
  GetTags(const IMetric *metric) const {
    auto it = m_tags.find(metric);
    if (it == std::end(m_tags)) {
      static const std::unordered_map<std::string, std::string> emptyMap;
      return emptyMap;
    }
    return it->second;
  }

private:
  void HandleSet(const IMetric *metric, MetricSetTag &message);
  void HandleRemove(const IMetric *metric, MetricSetTag &message);

private:
  std::unordered_map<const IMetric *,
                     std::unordered_map<std::string, std::string>>
      m_tags;
};