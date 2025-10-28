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
#include <aws/gamelift/metrics/PacketBuilder.h>
#include <aws/gamelift/metrics/Samplers.h>
#include <aws/gamelift/metrics/LoggerMacros.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace {
static constexpr int NullTerminator = 1;
}

PacketBuilder::PacketBuilder(size_t packetSize, int floatPrecision)
    : m_packetSize(packetSize), m_floatPrecision(floatPrecision) {
  m_formatBuffer << std::setprecision(floatPrecision)
                 << std::setiosflags(std::ios_base::fixed);
}

void PacketBuilder::Append(
    const MetricMessage &message, const TagMap &globalTags,
    const TagMap &metricTags,
    Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc sendPacketFunc) {
  const auto startPosition = m_formatBuffer.tellp();
  AppendToStream(message, m_floatPrecision, globalTags, metricTags,
                 m_formatBuffer);
  const auto endPosition = m_formatBuffer.tellp();

  const size_t messageLength = endPosition - startPosition;
  if (messageLength > GetPacketSize() - NullTerminator) {
    // If there's no way a message can fit the packet size, we drop it and log
    // an error.

    try {
      // Use a simple message that doesn't reference message.Metric
      // which could be null in tests
      GAMELIFT_METRICS_LOG_WARN(
          "Message length ({}) exceeds packet size ({}), message has "
          "been dropped.",
          messageLength, GetPacketSize() - NullTerminator);
    } catch (...) {
      // Silently continue if logging fails - don't break tests
    }

    // Reset the stream to the start position so we can continue appending
    m_formatBuffer.seekp(startPosition);
    return;
  }

  const size_t formatBufferLength = endPosition;
  if (formatBufferLength > GetPacketSize() - NullTerminator) {
    // Likely case:
    //     this message caused us to exceed packet size
    //
    //     1. Seek back to the end of the previous message (cut the new message
    //     out of the stream).
    //     2. Flush (sending packet).
    //     3. Re-append the message to the (now) empty stream.
    m_formatBuffer.seekp(startPosition);
    Flush(sendPacketFunc);
    AppendToStream(message, m_floatPrecision, globalTags, metricTags,
                   m_formatBuffer);
  } else if (formatBufferLength == GetPacketSize() - NullTerminator) {
    // Unlikely case:
    //      we hit the packet size exactly
    //      so we can just flush
    Flush(sendPacketFunc);
  }
}

void PacketBuilder::Flush(
    Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc sendPacketFunc) {
  m_sendBuffer = m_formatBuffer.str();
  m_sendBuffer.resize(m_formatBuffer.tellp());

  sendPacketFunc(m_sendBuffer.c_str(),
                 static_cast<int>(m_sendBuffer.size() + NullTerminator));

  m_formatBuffer.seekp(0);
}

namespace {
void WriteTags(const PacketBuilder::TagMap &tags, std::ostream &stream) {
  if (tags.size() == 0) {
    return;
  }

  auto it = std::begin(tags);
  stream << it->first << ':' << it->second;
  for (++it; it != std::end(tags); ++it) {
    stream << ',' << it->first << ':' << it->second;
  }
}

void WriteTags(const PacketBuilder::TagMap &globalTags,
               const PacketBuilder::TagMap &metricTags, std::ostream &stream) {
  if (globalTags.size() > 0 || metricTags.size() > 0) {
    stream << "|#";
    WriteTags(globalTags, stream);
    if (globalTags.size() > 0 && metricTags.size() > 0) {
      stream << ',';
    }
    WriteTags(metricTags, stream);
  }
}

void WriteValue(double value, int floatPrecision, std::ostream &stream) {
  const auto valueAsInteger = static_cast<int64_t>(value);
  const double fractionalPart = value - valueAsInteger;

  /*
   * We check if the fractional part of our value is 0 when rounded for display.
   *
   * We configured ostream to round to a specific float precision earlier.
   * Ie with a precision = 2, 1.425 becomes 1.43
   *
   * Due to floating point imprecision, we may, at some point, end up with a
   * value like 1.0000002. When rounded this ends up as 1.00 (with precision = 2
   * as before) and we'd prefer to print it as an integer.
   *
   * The check is simple: we multiply the fraction by 10^precision and see if
   * when rounded the value equals to 0.
   *
   * 0.425 * 10^2 = 42.5 ~= 43           => not zero so we print 1.425 as a real
   * 0.0000002 * 10^2 = 0.00002 ~= 0     => zero so we can print 1.0000002 as an
   * integer
   *
   * This also takes care of integer valued doubles in general. (Fractional part
   * is already zero in that case.)
   */
  const double thousandths = std::pow(10, floatPrecision);
  const bool hasFractionalPart = std::round(fractionalPart * thousandths) != 0;

  if (hasFractionalPart) {
    stream << value;
  } else {
    stream << valueAsInteger;
  }
}

void WriteSampleRate(const MetricMessage &message, std::ostream &stream) {
  // Get the sample rate from the sampler
  if (message.Metric) {
    Aws::GameLift::Metrics::ISampler &sampler = message.Metric->GetSampler();
    float rate = sampler.GetSampleRate();

    // Only add sample rate if it's less than 1.0 (1.0 is implicit/default in
    // StatsD). Packets with a 0.0 sample rate are not sent.
    if (rate < 1.0f) {
      // Format using a separate stringstream with default formatting
      std::ostringstream rateStream;
      rateStream << rate;

      stream << "|@" << rateStream.str();
    }
  }
}

void WriteMessage(const MetricMessage &message, int floatPrecision,
                  std::ostream &stream) {
  static constexpr auto showPositiveSign = std::ios_base::showpos;

  stream << message.Metric->GetKey() << ':';

  if (message.Type == MetricMessageType::GaugeSet) {
    WriteValue(message.SubmitDouble.Value, floatPrecision, stream);
    stream << "|g";
  } else if (message.Type == MetricMessageType::GaugeAdd) {
    stream << std::setiosflags(showPositiveSign);
    WriteValue(message.SubmitDouble.Value, floatPrecision, stream);
    stream << "|g" << std::resetiosflags(showPositiveSign);
  } else if (message.Type == MetricMessageType::CounterAdd) {
    WriteValue(message.SubmitDouble.Value, floatPrecision, stream);
    stream << "|c" << std::resetiosflags(showPositiveSign);
  } else if (message.Type == MetricMessageType::TimerSet) {
    WriteValue(message.SubmitDouble.Value, floatPrecision, stream);
    stream << "|ms";
  }

  // Add sample rate if using SampleFraction
  WriteSampleRate(message, stream);
}
} // namespace

void AppendToStream(const MetricMessage &message, int floatPrecision,
                    const PacketBuilder::TagMap &globalTags,
                    const PacketBuilder::TagMap &metricTags,
                    std::ostream &stream) {
  if (message.Type == MetricMessageType::GaugeSet &&
      message.SubmitDouble.Value < 0) {
    /*
    * There's an intentional amgibuity in `gaugor:-10|g`

    * It means 'subtract 10 from gaugor'. So the only way to set a
    * gauge to a negative value is by setting it to zero first and
    * subtracting:
    *      gaugor:0|g
    *      gaugor:-10|g
    *
    * See:
    https://github.com/statsd/statsd/blob/master/docs/metric_types.md#gauges
    */
    WriteMessage(MetricMessage::GaugeSet(*message.Metric, 0), floatPrecision,
                 stream);
    WriteTags(globalTags, metricTags, stream);
    stream << '\n';
    WriteMessage(
        MetricMessage::GaugeAdd(*message.Metric, message.SubmitDouble.Value),
        floatPrecision, stream);
    WriteTags(globalTags, metricTags, stream);
    stream << '\n';
  } else if (message.IsCounter() && message.SubmitDouble.Value <= 0) {
    // Skip non-positive or negative counters
  } else {
    WriteMessage(message, floatPrecision, stream);
    WriteTags(globalTags, metricTags, stream);
    stream << '\n';
  }
}
