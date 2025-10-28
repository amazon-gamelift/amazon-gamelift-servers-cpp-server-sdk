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
#include <aws/gamelift/server/model/GameSession.h>

#include "Combiner.h"
#include "PacketBuilder.h"
#include "Tags.h"
#include <chrono>
#include <concurrentqueue.h>
#include <functional>
#include <vector>

class MetricsProcessor : public IMetricsProcessor {
private:
  using ClockT = std::chrono::high_resolution_clock;
  using TimeT = ClockT::time_point;
  using NativeDurationT = TimeT::duration;
  using SecondsT = std::chrono::duration<float>;

public:
  /**
   * @param settings The settings object containing configuration for the
   * metrics processor
   */
  explicit MetricsProcessor(
      const Aws::GameLift::Metrics::MetricsSettings &settings)
      : m_sendPacket(settings.SendPacketCallback),
        m_captureInterval(std::chrono::duration_cast<NativeDurationT>(
            SecondsT(settings.CaptureIntervalSec))),
        m_nextCaptureTime(ClockT::now() + m_captureInterval),
        m_packet(settings.MaxPacketSizeBytes, settings.FloatPrecision) {}

  virtual void Enqueue(MetricMessage message) override {
    m_messageQueueMPSC.enqueue(message);
  }

#ifdef GAMELIFT_USE_STD
  virtual void SetGlobalTag(const std::string &key,
                            const std::string &value) override {
    m_globalTags[key] = value;
  }

  virtual void RemoveGlobalTag(const std::string &key) override {
    auto it = m_globalTags.find(key);
    if (it != std::end(m_globalTags)) {
      m_globalTags.erase(it);
    }
  }
#else
  virtual void SetGlobalTag(const char *key, const char *value) override {
    m_globalTags[key] = value;
  }

  virtual void RemoveGlobalTag(const char *key) override {
    auto it = m_globalTags.find(key);
    if (it != std::end(m_globalTags)) {
      m_globalTags.erase(it);
    }
  }
#endif

  /**
   * @brief Gets the capture interval in native duration format
   * @return The capture interval duration
   */
  NativeDurationT GetCaptureInterval() const { return m_captureInterval; }

  /**
   * @brief Sets the capture interval
   * @param captureInterval The new capture interval
   */
  void SetCaptureInterval(const NativeDurationT &captureInterval) {
    m_captureInterval = captureInterval;
  }

  /**
   * @brief Fluent setter for capture interval
   * @param captureInterval The new capture interval
   * @return Reference to this object for method chaining
   */
  MetricsProcessor &
  WithCaptureInterval(const NativeDurationT &captureInterval) {
    SetCaptureInterval(captureInterval);
    return *this;
  }

  /**
   * @brief Gets the next scheduled capture time
   * @return The next capture time point
   */
  TimeT GetNextCaptureTime() const { return m_nextCaptureTime; }

  /**
   * @brief Sets the next capture time
   * @param nextCaptureTime The new capture time point
   */
  void SetNextCaptureTime(const TimeT &nextCaptureTime) {
    m_nextCaptureTime = nextCaptureTime;
  }

  /**
   * @brief Fluent setter for next capture time
   * @param nextCaptureTime The new capture time point
   * @return Reference to this object for method chaining
   */
  MetricsProcessor &WithNextCaptureTime(const TimeT &nextCaptureTime) {
    SetNextCaptureTime(nextCaptureTime);
    return *this;
  }

  virtual void ProcessMetrics() override;

  virtual void ProcessMetricsNow() override;

  /**
   * @brief Called when a game session is started
   * @param session The game session that was started
   */
  virtual void
  OnStartGameSession(const Aws::GameLift::Server::Model::GameSession &session);

private:
  void ProcessMessages(std::vector<MetricMessage> &messages);

  struct VectorEnqueuer : public IMetricsEnqueuer {
    std::vector<MetricMessage> m_messages;

    virtual void Enqueue(MetricMessage message) override {
      m_messages.emplace_back(message);
    }

    void Clear() { m_messages.clear(); }
  };

private:
  moodycamel::ConcurrentQueue<MetricMessage> m_messageQueueMPSC;

  Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc m_sendPacket;
  Aws::GameLift::Metrics::MetricsSettings::PreProcessingFunc
      m_preProcessCallback;

  NativeDurationT m_captureInterval;
  TimeT m_nextCaptureTime;

  std::vector<MetricMessage> m_processQueue;
  Combiner m_combinedMetrics;
  PacketBuilder m_packet;
  Tags m_metricTags;
  std::unordered_map<std::string, std::string> m_globalTags;

  VectorEnqueuer m_enqueuer;
};
