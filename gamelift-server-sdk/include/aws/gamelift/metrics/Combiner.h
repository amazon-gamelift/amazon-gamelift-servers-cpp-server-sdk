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

#include <cstdint>
#include <unordered_map>
#include <vector>

using namespace ::Aws::GameLift::Metrics;

/**
 * Combines metrics logged in the current capture period.
 */
class Combiner final {
public:
  using CombinedMessagesMap =
      std::unordered_map<const IMetric *, MetricMessage>;

public:
  void Add(MetricMessage message);
  void Clear();

  bool IsEmpty() const { return GetSize() == 0; }
  std::size_t GetSize() const { return m_combinedMessages.size(); }

private:
  void UpdateGauge(const MetricMessage &newMessage);
  void UpdateCounter(MetricMessage &existing, const MetricMessage &newMessage);
  void UpdateTimer(MetricMessage &existing, const MetricMessage &newMessage);

private:
  CombinedMessagesMap m_combinedMessages;
  std::unordered_map<const IMetric *, int> m_timerSampleCount;

  /**
   * We keep track of gauge values during the lifetime of the program.
   * This allows us to detect whether a gauge has changed since the last
   * collection update. If it has, we can submit it to combined messages to be
   * emitted in the next statsd packet.
   *
   * This allows us to use the 'set gauge' syntax: foo:10|g. This allows us to
   * support sending gauges to StatsD receivers that do not have long-term
   * internal state, such as the OpenTelemetry collector.
   */
  CombinedMessagesMap m_gaugeHistory;

  /**
   * STL iterator support.
   *
   * For compatibility with STL and gtest.
   */
public:
  /**
   * @brief STL iterator supporting const and mutable iteration via ValueT
   * parameter.
   *
   * @param ValueT The value type we return. Used to customize for const and
   * mutable iteration.
   * @param BaseIterator The iterator of the underlying deduped message map.
   */
  template <class ValueT, class BaseIterator> class Iterator final {
    /**
     * STL type defs.
     */
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = ValueT;
    using pointer = value_type *;
    using reference = value_type &;

  public:
    Iterator(BaseIterator base) : m_base(base) {}

    reference operator*() { return m_base->second; }
    pointer operator->() { return &m_base->second; }

    reference operator*() const { return m_base->second; }
    pointer operator->() const { return &m_base->second; }

    // Pre-increment
    Iterator &operator++() {
      ++m_base;
      return *this;
    }
    // Post-increment
    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.m_base == b.m_base;
    };
    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return !(a == b);
    };

  private:
    BaseIterator m_base;
  };

  using value_type = MetricMessage;
  using iterator = Iterator<value_type, CombinedMessagesMap::iterator>;
  using const_iterator =
      Iterator<const value_type, CombinedMessagesMap::const_iterator>;

  iterator begin() { return iterator(std::begin(m_combinedMessages)); }
  iterator end() { return iterator(std::end(m_combinedMessages)); }

  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  const_iterator cbegin() const {
    return const_iterator(m_combinedMessages.begin());
  }
  const_iterator cend() const {
    return const_iterator(m_combinedMessages.end());
  }

  std::size_t size() const { return GetSize(); }
  bool empty() const { return IsEmpty(); }
};