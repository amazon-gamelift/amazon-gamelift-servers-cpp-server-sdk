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
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/DynamicTag.h>

namespace Aws {
namespace GameLift {
namespace Metrics {

IMetricsEnqueuer::~IMetricsEnqueuer() {}

IMetricsProcessor::~IMetricsProcessor() {}

#ifdef GAMELIFT_USE_STD
MetricMessage MetricMessage::TagSet(IMetric &metric, std::string key,
                                    std::string value) {
  DynamicTag *tag = new DynamicTag(std::move(key), std::move(value));
  return MetricMessage(MetricMessageType::TagSet, &metric, MetricSetTag(tag));
}

MetricMessage MetricMessage::TagRemove(Aws::GameLift::Metrics::IMetric &metric,
                                       std::string key) {
  DynamicTag *tag = new DynamicTag(std::move(key), "");
  return MetricMessage(MetricMessageType::TagRemove, &metric,
                       MetricSetTag(tag));
}
#else
MetricMessage MetricMessage::TagSet(IMetric &metric, const char *key,
                                    const char *value) {
  DynamicTag *tag = new DynamicTag(key, value);
  return MetricMessage(MetricMessageType::TagSet, &metric, MetricSetTag(tag));
}

MetricMessage MetricMessage::TagRemove(IMetric &metric, const char *key) {
  DynamicTag *tag = new DynamicTag(key, "");
  return MetricMessage(MetricMessageType::TagRemove, &metric,
                       MetricSetTag(tag));
}
#endif

bool operator==(const MetricSetTag &a, const MetricSetTag &b) {
  return a.Ptr->Key == b.Ptr->Key && a.Ptr->Value == b.Ptr->Value;
}

void CopyTagMessage(const MetricMessage &original, IMetric &destMetric,
                    IMetricsEnqueuer &enqueuer) {
  switch (original.Type) {
  case MetricMessageType::TagSet:
    enqueuer.Enqueue(MetricMessage::TagSet(destMetric,
                                           original.SetTag.Ptr->Key.c_str(),
                                           original.SetTag.Ptr->Value.c_str()));
    break;
  case MetricMessageType::TagRemove:
    enqueuer.Enqueue(
        MetricMessage::TagRemove(destMetric, original.SetTag.Ptr->Key.c_str()));
    break;
  default:
    break;
  }
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
