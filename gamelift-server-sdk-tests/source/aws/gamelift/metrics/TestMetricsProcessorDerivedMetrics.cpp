/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */
#include "Common.h"
#include "MetricMacrosTests.h"
#include "MockDerivedMetric.h"

#include <aws/gamelift/metrics/MetricsProcessor.h>
#include <thread>
#include <vector>

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

struct MetricsProcessorDerivedMetricTests : public PacketSendTest {};

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricGauge, "gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGauge);

GAMELIFT_METRICS_DECLARE_TIMER(MetricTimer, "timer", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll(),
                               MockDerivedMetric());
GAMELIFT_METRICS_DEFINE_TIMER(MetricTimer);

struct MockMax : public Aws::GameLift::Metrics::IDerivedMetric {
  double Max = 0;

  struct Metric : public Aws::GameLift::Metrics::IMetric {
    std::string Key;

    virtual const char *GetKey() const override { return Key.c_str(); }

    virtual Aws::GameLift::Metrics::MetricType GetMetricType() const override {
      return Aws::GameLift::Metrics::MetricType::Gauge;
    }

    virtual Aws::GameLift::Metrics::IDerivedMetricCollection &
    GetDerivedMetrics() override {
      static Aws::GameLift::Metrics::DerivedMetricCollection<> Collection;
      return Collection;
    }

    virtual Aws::GameLift::Metrics::ISampler &GetSampler() override {
      static Aws::GameLift::Metrics::SampleAll DefaultSampler;
      return DefaultSampler;
    }
  };
  Metric MetricToSubmit;

  virtual void HandleMessage(MetricMessage &Message,
                             IMetricsEnqueuer &) override {
    Max = std::max(Max, Message.SubmitDouble.Value);
  }

  virtual void EmitMetrics(const IMetric *OriginalMetric,
                           IMetricsEnqueuer &Submitter) override {
    std::string NewKey = OriginalMetric->GetKey();
    NewKey += ".max";
    MetricToSubmit.Key = NewKey;

    Submitter.Enqueue(MetricMessage::GaugeSet(MetricToSubmit, Max));
  }
};

GAMELIFT_METRICS_DECLARE_GAUGE(MetricGaugeWithMax, "another_gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll(), MockMax());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricGaugeWithMax);
} // namespace

TEST_F(
    MetricsProcessorDerivedMetricTests,
    GivenGauge_When10MessagesSubmitted_ThenHandleMessageCalled10TimesAndEmitMetricsCalledOnce) {
  Aws::GameLift::Metrics::MetricsSettings Settings{};
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  Settings.SendPacketCallback = MockSend;
  MetricsProcessor Processor(Settings);
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 10));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 6));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 41));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 41));
  Processor.Enqueue(MetricMessage::GaugeAdd(MetricGauge::Instance(), -5));
  Processor.Enqueue(MetricMessage::GaugeAdd(MetricGauge::Instance(), -5));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGauge::Instance(), 52));

  // Repeated process calls to hit the SendInterval
  while (OutputPackets.empty()) {
    Processor.ProcessMetrics();
  }

  MockDerivedMetric *Metric = dynamic_cast<MockDerivedMetric *>(
      GetNthDerivedMetric(&MetricGauge::Instance(), 0));
  EXPECT_THAT(
      Metric->HandledMessages,
      ElementsAre(MetricMessage::GaugeSet(MetricGauge::Instance(), 10),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 10),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 10),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 10),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 6),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 41),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 41),
                  MetricMessage::GaugeAdd(MetricGauge::Instance(), -5),
                  MetricMessage::GaugeAdd(MetricGauge::Instance(), -5),
                  MetricMessage::GaugeSet(MetricGauge::Instance(), 52)));
  EXPECT_EQ(Metric->CallsToEmit, 1);
}

TEST_F(
    MetricsProcessorDerivedMetricTests,
    GivenTimer_When10MessagesSubmitted_ThenHandleMessageCalled10TimesAndEmitMetricsCalledOnce) {
  Aws::GameLift::Metrics::MetricsSettings Settings{};
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  Settings.SendPacketCallback = MockSend;
  MetricsProcessor Processor(Settings);
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 10));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 10));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 10));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 10));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 6));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 41));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 41));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), -5));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), -5));
  Processor.Enqueue(MetricMessage::TimerSet(MetricTimer::Instance(), 52));

  // Repeated process calls to hit the SendInterval
  while (OutputPackets.empty()) {
    Processor.ProcessMetrics();
  }

  MockDerivedMetric *Metric = dynamic_cast<MockDerivedMetric *>(
      GetNthDerivedMetric(&MetricTimer::Instance(), 0));
  EXPECT_THAT(
      Metric->HandledMessages,
      ElementsAre(MetricMessage::TimerSet(MetricTimer::Instance(), 10),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 10),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 10),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 10),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 6),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 41),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 41),
                  MetricMessage::TimerSet(MetricTimer::Instance(), -5),
                  MetricMessage::TimerSet(MetricTimer::Instance(), -5),
                  MetricMessage::TimerSet(MetricTimer::Instance(), 52)));
  EXPECT_EQ(Metric->CallsToEmit, 1);
}

TEST_F(
    MetricsProcessorDerivedMetricTests,
    GivenGaugeWithMockDerivedMetric_When10MessagesSubmitted_ThenEmitsAMaxMessage) {
  Aws::GameLift::Metrics::MetricsSettings Settings{};
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  Settings.SendPacketCallback = MockSend;
  MetricsProcessor Processor(Settings);
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 10));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 10));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 10));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 10));
  Processor.Enqueue(MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 6));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 1235));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 41));
  Processor.Enqueue(
      MetricMessage::GaugeAdd(MetricGaugeWithMax::Instance(), -5));
  Processor.Enqueue(
      MetricMessage::GaugeAdd(MetricGaugeWithMax::Instance(), -5));
  Processor.Enqueue(
      MetricMessage::GaugeSet(MetricGaugeWithMax::Instance(), 52));

  // Repeated process calls to hit the SendInterval
  while (OutputPackets.empty()) {
    Processor.ProcessMetrics();
  }

  /*
   * We cannot guarrantee the relative order of the unique keys captured during
   * the send interval. Hence we can't do a simple tuple<string, int>
   * comparison...
   */
  ASSERT_THAT(OutputPackets, SizeIs(1));

  auto Result = OutputPackets[0];
  EXPECT_EQ(std::get<1>(Result),
            45); // packet size (bytes) including null terminator

  std::string PacketContents = std::get<0>(Result);
  EXPECT_THAT(PacketContents, HasSubstr("another_gauge:52|g\n"));
  EXPECT_THAT(PacketContents, HasSubstr("another_gauge.max:1235|g\n"));
}
