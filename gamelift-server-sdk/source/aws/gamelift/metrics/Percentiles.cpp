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
#include <aws/gamelift/metrics/Percentiles.h>

#include <aws/gamelift/metrics/LoggerMacros.h>
#include <algorithm>
#include <array>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace {
struct PercentileMetric {
  double m_percentile;
  DynamicMetric m_metric;
  bool m_metricInitialized = false;

  explicit PercentileMetric(double percentile) : m_percentile(percentile) {}

  double GetPercentile() const { return m_percentile; }
  void SetPercentile(double percentile) { m_percentile = percentile; }
  PercentileMetric &WithPercentile(double percentile) {
    SetPercentile(percentile);
    return *this;
  }
};

/**
 * INTERNAL: concrete percentile metric implementation
 */
class PercentilesImpl : public IDerivedMetric {
public:
  template <class It> PercentilesImpl(It begin, It end) {
    std::transform(begin, end, std::back_inserter(m_percentiles),
                   [](double value) { return PercentileMetric(value); });
  }

  virtual void HandleMessage(MetricMessage &message,
                             IMetricsEnqueuer &submitter) override {
    switch (message.Type) {
    case MetricMessageType::GaugeAdd:
      m_currentValue += message.SubmitDouble.Value;
      AppendValue(m_currentValue);
      break;
    case MetricMessageType::GaugeSet:
    case MetricMessageType::TimerSet:
      m_currentValue = message.SubmitDouble.Value;
      AppendValue(m_currentValue);
      break;
    case MetricMessageType::TagSet:
    case MetricMessageType::TagRemove:
      for (auto &percentile : m_percentiles) {
        CopyTagMessage(message, percentile.m_metric, submitter);
      }
      break;
    default:
      break;
    }
  }

  virtual void EmitMetrics(const IMetric *originalMetric,
                           IMetricsEnqueuer &submitter) override {
    if (m_numSeenSinceLastEmitCall == 0) {
      return;
    }

    m_numSeenSinceLastEmitCall = 0;
    std::sort(std::begin(m_values), std::end(m_values));
    for (auto &percentile : m_percentiles) {
      EmitPercentile(originalMetric, percentile, submitter);
    }
    m_values.clear();
  }

private:
  void AppendValue(double newCurrentValue) {
    m_values.emplace_back(newCurrentValue);
    m_numSeenSinceLastEmitCall++;
  }

  void EmitPercentile(const IMetric *originalMetric,
                      PercentileMetric &percentile,
                      IMetricsEnqueuer &submitter) {
    const double value = ComputePercentile(percentile.GetPercentile());

    if (!percentile.m_metricInitialized) {
      percentile.m_metric.SetMetricType(originalMetric->GetMetricType());

      std::stringstream stream;
      stream << originalMetric->GetKey() << ".p" << std::setw(2)
             << std::setfill('0')
             << static_cast<int>(percentile.GetPercentile() * 100.0);

#ifdef GAMELIFT_USE_STD
      percentile.m_metric.SetKey(stream.str());
#else
      percentile.m_metric.SetKey(stream.str().c_str());
#endif
      percentile.m_metricInitialized = true;
    }

    switch (percentile.m_metric.GetMetricType()) {
    case MetricType::Gauge:
      submitter.Enqueue(MetricMessage::GaugeSet(percentile.m_metric, value));
      break;
    case MetricType::Timer:
      submitter.Enqueue(MetricMessage::TimerSet(percentile.m_metric, value));
      break;
    default:
      break;
    }
  }

  double ComputePercentile(double percentile) const {
    const double startIndexFloat = percentile * (m_values.size() - 1);
    const size_t startIndex = static_cast<size_t>(startIndexFloat);
    const double fractionalPart = startIndexFloat - startIndex;

    const bool isBetweenValues = fractionalPart != 0;
    if (isBetweenValues) {
      const double start = m_values[startIndex];
      const double end = m_values[startIndex + 1];
      return start + fractionalPart * (end - start);
    } else {
      return m_values[startIndex];
    }
  }

private:
  std::vector<PercentileMetric> m_percentiles;

  size_t m_numSeenSinceLastEmitCall = 0;
  double m_currentValue = 0;
  std::vector<double> m_values;
};

/**
 * INTERNAL: validate percentiles and report errors / warnings for debug builds
 *
 * @param begin The begin iterator of percentile collection
 * @param end The past-the-end iterator of percentile collection
 */
template <class InputIt>
inline void ValidatePercentiles(InputIt begin, InputIt end) {
  if (begin == end) {
    GAMELIFT_METRICS_LOG_CRITICAL("Percentile list is empty.");
  }

  for (auto it = begin + 1; it != end; ++it) {
    if (*(it - 1) == *it) {
      GAMELIFT_METRICS_LOG_CRITICAL("Duplicate percentiles detected.", *it);
    }
  }

  for (auto it = begin; it != end; ++it) {
    const auto &value = *it;
    if (value < 0 || value > 1) {
      GAMELIFT_METRICS_LOG_CRITICAL("Percentiles must be in the [0, 1] range.",
                                    value);
    }

    const auto percentileFloat = value * 100;
    const auto percentileInteger = static_cast<int>(percentileFloat);
    const double fractionalPart = percentileFloat - percentileInteger;
    const double absDistance = 0.0001;
    if (std::abs(fractionalPart) >= absDistance) {
      // Warn about the user entering too many digits past decimal point.
      // Diff might be a tiny value due to float rounding, so we check against
      // distance to ignore it.
      GAMELIFT_METRICS_LOG_WARN(
          "Percentile {} ({}) has too many digits past the decimal "
          "point. It will be truncated to {}",
          percentileFloat, value, percentileInteger);
    }
  }
}
} // anonymous namespace

namespace Internal {
PercentilesWrapper PercentilesWrapper::Create(double *begin, double *end) {
  std::sort(begin, end); // sort percentiles (mostly for validation but might
                         // as well keep them in this order)

#ifndef NDEBUG
  ValidatePercentiles(begin, end);
#endif

  return PercentilesWrapper(new PercentilesImpl(begin, end));
}
} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
