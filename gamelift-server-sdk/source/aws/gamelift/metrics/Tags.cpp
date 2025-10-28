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
#include <aws/gamelift/metrics/Tags.h>
#include <aws/gamelift/metrics/DynamicTag.h>

#include <algorithm>
#include <cassert>

void Tags::Handle(MetricMessage &message) {
  assert(message.Type == MetricMessageType::TagSet ||
         message.Type == MetricMessageType::TagRemove);

  if (message.Type == MetricMessageType::TagSet) {
    HandleSet(message.Metric, message.SetTag);
  } else if (message.Type == MetricMessageType::TagRemove) {
    HandleRemove(message.Metric, message.SetTag);
  }
}

void Tags::HandleSet(const IMetric *metric, MetricSetTag &message) {
  // Create tag map for current metric if not exist
  auto it = m_tags.find(metric);
  if (it == std::end(m_tags)) {
    it = m_tags.emplace(metric, std::unordered_map<std::string, std::string>())
             .first;
  }
  assert(it != std::end(m_tags));
  auto &metricTags = it->second;

  auto tagIt = metricTags.find(message.Ptr->Key);
  if (tagIt != std::end(metricTags)) {
    // Swap-in the new value to avoid copy
    // Old value gets deleted below.
    using std::swap;
    swap(tagIt->second, message.Ptr->Value);
  } else {
    // Move key and value into dictionary to avoid copy. (We delete empty shells
    // below.)
    metricTags.emplace(std::move(message.Ptr->Key),
                       std::move(message.Ptr->Value));
  }

  delete message.Ptr;
  message.Ptr = nullptr;
}

void Tags::HandleRemove(const IMetric *metric, MetricSetTag &message) {
  auto it = m_tags.find(metric);
  if (it != std::end(m_tags)) {
    it->second.erase(message.Ptr->Key);
  }
  delete message.Ptr;
  message.Ptr = nullptr;
}
