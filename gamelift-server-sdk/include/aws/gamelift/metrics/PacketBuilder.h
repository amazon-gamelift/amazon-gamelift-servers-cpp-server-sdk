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
#include <aws/gamelift/metrics/MetricsSettings.h>

#include <sstream>
#include <unordered_map>
#include <vector>

using namespace ::Aws::GameLift::Metrics;

class PacketBuilder final {
public:
  using TagMap = std::unordered_map<std::string, std::string>;

  explicit PacketBuilder(size_t packetSize) noexcept
      : PacketBuilder(packetSize, 5) {}
  PacketBuilder(size_t packetSize, int floatPrecision);

  /**
   * @brief Appends message to the back of the packet.
   *
   * Metric is tagged with global tags first and per-metric tags second.
   *
   * @param message Message to append.
   * @param globalTags Global metric tags
   * @param metricTags Per-metric tags
   * @param sendPacketFunc Callback called with the current packet data whenever
   * the packet fills up.
   */
  void Append(
      const MetricMessage &message, const TagMap &globalTags,
      const TagMap &metricTags,
      Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc sendPacketFunc);

  /**
   * @brief Flushes the internal buffers via sendPacketFunc.
   * Resets internal state for new metrics.
   *
   * @param sendPacketFunc Callback called with the remaining packet data.
   */
  void
  Flush(Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc sendPacketFunc);

  /**
   * @brief Gets the maximum packet size in bytes.
   * @returns Maximum packet size in bytes.
   */
  size_t GetPacketSize() const noexcept { return m_packetSize; }

  /**
   * @brief Sets the maximum packet size in bytes.
   * @param packetSize The new maximum packet size in bytes
   */
  void SetPacketSize(size_t packetSize) { m_packetSize = packetSize; }

  /**
   * @brief Fluent setter for packet size
   * @param packetSize The new maximum packet size in bytes
   * @return Reference to this object for method chaining
   */
  PacketBuilder &WithPacketSize(size_t packetSize) {
    SetPacketSize(packetSize);
    return *this;
  }

  /**
   * @brief Gets how many digits floats are rounded to after the decimal point.
   * @returns Float precision value
   */
  int GetFloatPrecision() const noexcept { return m_floatPrecision; }

  /**
   * @brief Sets how many digits floats are rounded to after the decimal point.
   * @param floatPrecision The new float precision value
   */
  void SetFloatPrecision(int floatPrecision) {
    m_floatPrecision = floatPrecision;
  }

  /**
   * @brief Fluent setter for float precision
   * @param floatPrecision The new float precision value
   * @return Reference to this object for method chaining
   */
  PacketBuilder &WithFloatPrecision(int floatPrecision) {
    SetFloatPrecision(floatPrecision);
    return *this;
  }

private:
  size_t m_packetSize;
  int m_floatPrecision = 5;

  std::string m_sendBuffer;
  std::stringstream m_formatBuffer;
};

/**
 * @brief Appends Message to Stream in dogstatsd format.
 *
 * <a
 * href="https://docs.datadoghq.com/developers/dogstatsd/datagram_shell?tab=metrics">See
 * DogStatsD documentation.</a>
 *
 * @param message Message to append to the Stream.
 * @param floatPrecision Number of digits after the decimal point for floating
 * point numbers
 * @param globalTags Global metric tags
 * @param metricTags Per-metric tags.
 * @param stream An Output stream to append to.
 */
extern void AppendToStream(const MetricMessage &message, int floatPrecision,
                           const PacketBuilder::TagMap &globalTags,
                           const PacketBuilder::TagMap &metricTags,
                           std::ostream &stream);
