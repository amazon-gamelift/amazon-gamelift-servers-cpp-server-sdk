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
#include <aws/gamelift/metrics/Combiner.h>

#include <cassert>

void Combiner::Add(MetricMessage message) {
  if (message.IsGauge()) {
    // We have to do a bit more work for gauges to convert GaugeAdd to GaugeSet
    UpdateGauge(message);
  } else {
    auto it = m_combinedMessages.find(message.Metric);
    if (it == std::end(m_combinedMessages)) {
      m_combinedMessages.emplace(message.Metric, message);
      return;
    }

    assert(message.IsCounter() || message.IsTimer());
    if (message.IsCounter()) {
      UpdateCounter(it->second, message);
    } else if (message.IsTimer()) {
      UpdateTimer(it->second, message);
    }
  }
}

void Combiner::UpdateGauge(const MetricMessage &message) {
  assert(message.Type == MetricMessageType::GaugeSet ||
         message.Type == MetricMessageType::GaugeAdd);

  if (message.Type == MetricMessageType::GaugeSet) {
    // If setting - we just use the new value
    m_combinedMessages[message.Metric] = message;
    m_gaugeHistory[message.Metric] = message;
  } else if (message.Type == MetricMessageType::GaugeAdd) {
    // If adding - we get historic value (if available) and add to it
    //             if there's no historic value, we just add to 0
    double currentValue = 0;
    auto it = m_gaugeHistory.find(message.Metric);
    if (it != std::end(m_gaugeHistory)) {
      currentValue = it->second.SubmitDouble.Value;
    }
    currentValue += message.SubmitDouble.Value;

    auto newMessage = MetricMessage::GaugeSet(*message.Metric, currentValue);
    m_combinedMessages[message.Metric] = newMessage;
    m_gaugeHistory[message.Metric] = newMessage;
  }
}

void Combiner::UpdateCounter(MetricMessage &current,
                             const MetricMessage &newMessage) {
  assert(newMessage.Type == MetricMessageType::CounterAdd);

  // Just sum the values.
  current.SubmitDouble.Value += newMessage.SubmitDouble.Value;
}

void Combiner::UpdateTimer(MetricMessage &current,
                           const MetricMessage &newMessage) {
  assert(newMessage.Type == MetricMessageType::TimerSet);

  // Grab or init sample count
  auto sampleCountIt = m_timerSampleCount.find(current.Metric);
  if (sampleCountIt != std::end(m_timerSampleCount)) {
    ++(sampleCountIt->second);
  } else {
    // This is the second sample being added, hence we initialize it to 2.
    auto pair = m_timerSampleCount.emplace(current.Metric, 2);

    const bool insertSuccess = pair.second;
    assert(insertSuccess);

    sampleCountIt = pair.first;
  }

  // Welford's algorithm
  //     numerically stable mean
  //
  // Knuth - TACOP Vol 2 pg. 216
  //
  // https://nullbuffer.com/articles/welford_algorithm.html
  const double update =
      (newMessage.SubmitDouble.Value - current.SubmitDouble.Value) /
      sampleCountIt->second;
  current.SubmitDouble.Value += update;
}

void Combiner::Clear() {
  m_combinedMessages.clear();
  m_timerSampleCount.clear();
}
