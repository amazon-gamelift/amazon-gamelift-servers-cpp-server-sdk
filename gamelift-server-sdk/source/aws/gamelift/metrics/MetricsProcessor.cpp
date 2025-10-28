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
#include <aws/gamelift/metrics/MetricsProcessor.h>
#include <aws/gamelift/metrics/DerivedMetric.h>
#include <aws/gamelift/metrics/GaugeMacros.h>
#include <iterator>
#include <unordered_set>

void MetricsProcessor::ProcessMetrics() {
  const auto now = ClockT::now();
  if (now < m_nextCaptureTime) {
    return;
  }

  // The server_up metric needs to be set on each flush period, as an untouched
  // metric will automatically expire in many metrics backends (e.g. Prometheus)
  GAMELIFT_METRICS_SET(ServerUpGauge, 1);

  ProcessMetricsNow();
}

void MetricsProcessor::ProcessMetricsNow() {
  if (m_preProcessCallback) {
    m_preProcessCallback();
  }

  const auto messageCount = m_messageQueueMPSC.size_approx();
  m_messageQueueMPSC.try_dequeue_bulk(std::back_inserter(m_processQueue),
                                      messageCount);

  m_combinedMetrics.Clear();
  ProcessMessages(m_processQueue);
  m_processQueue.clear();
  m_enqueuer.Clear();

  m_nextCaptureTime = ClockT::now() + m_captureInterval;
}

namespace {
void UpdateDerivedMetrics(std::vector<MetricMessage> &messages,
                          IMetricsEnqueuer &enqueuer) {
  for (auto &message : messages) {
    struct HandleMessageVisitor final
        : public Aws::GameLift::Metrics::IDerivedMetricVisitor {
      MetricMessage &m_message;
      IMetricsEnqueuer &m_enqueuer;

      explicit HandleMessageVisitor(MetricMessage &message,
                                    IMetricsEnqueuer &enqueuer) noexcept
          : m_message(message), m_enqueuer(enqueuer) {}

      virtual void VisitDerivedMetric(
          Aws::GameLift::Metrics::IDerivedMetric &metric) override {
        metric.HandleMessage(m_message, m_enqueuer);
      }
    };

    HandleMessageVisitor visitor(message, enqueuer);
    message.Metric->GetDerivedMetrics().Visit(visitor);
  }
}

void SubmitDerivedMetrics(std::vector<MetricMessage> &messages,
                          IMetricsEnqueuer &enqueuer) {
  std::unordered_set<const Aws::GameLift::Metrics::IMetric *> submittedMetrics;

  for (auto &message : messages) {
    const bool derivativeMetricsSubmitted =
        submittedMetrics.find(message.Metric) != std::end(submittedMetrics);
    if (derivativeMetricsSubmitted) {
      continue;
    }

    submittedMetrics.emplace(message.Metric);

    class EmitMessageVisitor final
        : public Aws::GameLift::Metrics::IDerivedMetricVisitor {
    public:
      EmitMessageVisitor(const Aws::GameLift::Metrics::IMetric *originalMetric,
                         IMetricsEnqueuer &enqueuer) noexcept
          : m_originalMetric(originalMetric), m_enqueuer(enqueuer) {}

      virtual void VisitDerivedMetric(
          Aws::GameLift::Metrics::IDerivedMetric &metric) override {
        metric.EmitMetrics(m_originalMetric, m_enqueuer);
      }

    private:
      const Aws::GameLift::Metrics::IMetric *m_originalMetric;
      IMetricsEnqueuer &m_enqueuer;
    };

    EmitMessageVisitor visitor(message.Metric, enqueuer);
    message.Metric->GetDerivedMetrics().Visit(visitor);
  }
}
} // namespace

void MetricsProcessor::ProcessMessages(std::vector<MetricMessage> &messages) {
  // Compute derived metrics and appends their messages to the end
  UpdateDerivedMetrics(messages, m_enqueuer);
  SubmitDerivedMetrics(messages, m_enqueuer);
  std::copy(std::begin(m_enqueuer.m_messages), std::end(m_enqueuer.m_messages),
            std::back_inserter(messages));

  // Combine all the metrics
  for (auto &message : messages) {
    if (message.IsTag()) {
      m_metricTags.Handle(message);
    } else {
      m_combinedMetrics.Add(message);
    }
  }

  if (m_combinedMetrics.IsEmpty()) {
    return;
  }

  // Build & send packets
  for (const auto &message : m_combinedMetrics) {
    m_packet.Append(message, m_globalTags, m_metricTags.GetTags(message.Metric),
                    m_sendPacket);
  }
  m_packet.Flush(m_sendPacket);
}

void MetricsProcessor::OnStartGameSession(
    const Aws::GameLift::Server::Model::GameSession &session) {
#ifdef GAMELIFT_USE_STD
  const char *sessionId = session.GetGameSessionId().c_str();
#else
  const char *sessionId = session.GetGameSessionId();
#endif

  if (sessionId != nullptr && sessionId[0] != '\0') {
    SetGlobalTag("session_id", sessionId);
  }
}
