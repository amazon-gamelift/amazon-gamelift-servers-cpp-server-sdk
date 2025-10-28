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

#include <aws/gamelift/metrics/MetricsProcessor.h>
#include <thread>
#include <vector>

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Percentiles.h>
#include <aws/gamelift/metrics/ReduceMetric.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;

struct MetricsProcessorTests : public PacketSendTest {};

TEST_F(MetricsProcessorTests, WhenEmpty_ProcessesOk) {
  MetricsProcessor Processor(Aws::GameLift::Metrics::MetricsSettings{});
  Processor.ProcessMetrics();

  EXPECT_THAT(OutputPackets, IsEmpty());
}

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(NegativeGauge, "negative_gauge", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(NegativeGauge);

GAMELIFT_METRICS_DECLARE_GAUGE(Foo, "foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(Foo);

GAMELIFT_METRICS_DECLARE_GAUGE(Bar, "bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(Bar);

GAMELIFT_METRICS_DECLARE_COUNTER(BarCount, "bar_count", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(BarCount);

GAMELIFT_METRICS_DECLARE_COUNTER(FooCount, "foo_count", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(FooCount);

GAMELIFT_METRICS_DECLARE_COUNTER(BazCount, "baz_count", MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_COUNTER(BazCount);

GAMELIFT_METRICS_DECLARE_TIMER(FooTime, "foo_time", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(FooTime);

GAMELIFT_METRICS_DECLARE_TIMER(BarTime, "bar_time", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_TIMER(BarTime);
} // namespace

TEST_F(MetricsProcessorTests, SingleProducer) {
  Aws::GameLift::Metrics::MetricsSettings Settings;
  Settings.SendPacketCallback = MockSend;
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  MetricsProcessor Processor(Settings);

  Processor.SetGlobalTag("foo", "bar");

  Processor.Enqueue(MetricMessage::GaugeSet(NegativeGauge::Instance(), -200));
  Processor.Enqueue(MetricMessage::GaugeSet(Foo::Instance(), 11));

  Processor.Enqueue(MetricMessage::GaugeAdd(Bar::Instance(), -12.21520));

  Processor.Enqueue(MetricMessage::CounterAdd(FooCount::Instance(), 5));

  Processor.Enqueue(MetricMessage::CounterAdd(BarCount::Instance(), 11));
  Processor.Enqueue(MetricMessage::CounterAdd(BarCount::Instance(), 2));

  Processor.Enqueue(MetricMessage::CounterAdd(BazCount::Instance(), 2));

  Processor.Enqueue(MetricMessage::TimerSet(FooTime::Instance(), 11));

  Processor.Enqueue(MetricMessage::TimerSet(BarTime::Instance(), 2));
  Processor.Enqueue(MetricMessage::TimerSet(BarTime::Instance(), 72));
  Processor.Enqueue(MetricMessage::TimerSet(BarTime::Instance(), 10));

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
            238); // packet size (bytes) including null terminator

  std::string PacketContents = std::get<0>(Result);
  EXPECT_THAT(
      PacketContents,
      HasSubstr(
          "negative_gauge:0|g|#foo:bar\nnegative_gauge:-200|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("foo:11|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("bar:0|g|#foo:bar\nbar:-12.21520|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("foo_count:5|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("bar_count:13|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("baz_count:2|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("foo_time:11|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("bar_time:28|ms|#foo:bar\n"));
}

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadA_Foo, "thread_a_foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadA_Bar, "thread_a_bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadA_Baz, "thread_a_baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadA_FooCount, "thread_a_foo_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadA_BarCount, "thread_a_bar_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadA_FooTime, "thread_a_foo_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadA_BarTime, "thread_a_bar_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadA_BazTime, "thread_a_baz_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadA_Foo);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadA_Bar);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadA_Baz);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadA_FooCount);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadA_BarCount);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadA_FooTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadA_BarTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadA_BazTime);

GAMELIFT_METRICS_DECLARE_GAUGE(ThreadB_Foo, "thread_b_foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadB_Bar, "thread_b_bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadB_Baz, "thread_b_baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadB_FooCount, "thread_b_foo_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadB_BarCount, "thread_b_bar_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadB_FooTime, "thread_b_foo_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadB_BarTime, "thread_b_bar_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadB_BazTime, "thread_b_baz_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadB_Foo);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadB_Bar);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadB_Baz);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadB_FooCount);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadB_BarCount);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadB_FooTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadB_BarTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadB_BazTime);

GAMELIFT_METRICS_DECLARE_GAUGE(ThreadC_Foo, "thread_c_foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadC_Bar, "thread_c_bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadC_Baz, "thread_c_baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadC_FooCount, "thread_c_foo_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadC_BarCount, "thread_c_bar_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadC_FooTime, "thread_c_foo_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadC_BarTime, "thread_c_bar_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadC_BazTime, "thread_c_baz_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadC_Foo);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadC_Bar);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadC_Baz);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadC_FooCount);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadC_BarCount);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadC_FooTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadC_BarTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadC_BazTime);

GAMELIFT_METRICS_DECLARE_GAUGE(ThreadD_Foo, "thread_d_foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadD_Bar, "thread_d_bar", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_GAUGE(ThreadD_Baz, "thread_d_baz", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadD_FooCount, "thread_d_foo_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_COUNTER(ThreadD_BarCount, "thread_d_bar_count",
                                 MockEnabled,
                                 Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadD_FooTime, "thread_d_foo_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadD_BarTime, "thread_d_bar_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DECLARE_TIMER(ThreadD_BazTime, "thread_d_baz_time",
                               MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadD_Foo);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadD_Bar);
GAMELIFT_METRICS_DEFINE_GAUGE(ThreadD_Baz);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadD_FooCount);
GAMELIFT_METRICS_DEFINE_COUNTER(ThreadD_BarCount);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadD_FooTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadD_BarTime);
GAMELIFT_METRICS_DEFINE_TIMER(ThreadD_BazTime);
} // namespace

TEST_F(MetricsProcessorTests, MultipleProducer) {
  Aws::GameLift::Metrics::MetricsSettings Settings;
  Settings.SendPacketCallback = MockSend;
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  MetricsProcessor Processor(Settings);

  Processor.SetGlobalTag("foo", "bar");

  std::vector<std::thread> Threads;
  Threads.emplace_back([&Processor]() {
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadA_Foo::Instance(), 200));
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadA_Bar::Instance(), -200));

    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadA_Baz::Instance(), -5));
    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadA_Baz::Instance(), 20));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadA_FooCount::Instance(), 11));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadA_BarCount::Instance(), 11));
    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadA_BarCount::Instance(), 2));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadA_FooTime::Instance(), 20));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadA_BarTime::Instance(), 5));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadA_BarTime::Instance(), 21));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadA_BarTime::Instance(), 82));

    Processor.Enqueue(
        MetricMessage::TimerSet(ThreadA_BazTime::Instance(), 82.12151));
  });
  Threads.emplace_back([&Processor]() {
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadB_Foo::Instance(), 200));
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadB_Bar::Instance(), -200));

    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadB_Baz::Instance(), -5));
    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadB_Baz::Instance(), 20));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadB_FooCount::Instance(), 11));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadB_BarCount::Instance(), 11));
    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadB_BarCount::Instance(), 2));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadB_FooTime::Instance(), 20));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadB_BarTime::Instance(), 5));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadB_BarTime::Instance(), 21));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadB_BarTime::Instance(), 82));

    Processor.Enqueue(
        MetricMessage::TimerSet(ThreadB_BazTime::Instance(), 82.12151));
  });
  Threads.emplace_back([&Processor]() {
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadC_Foo::Instance(), 200));
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadC_Bar::Instance(), -200));

    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadC_Baz::Instance(), -5));
    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadC_Baz::Instance(), 20));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadC_FooCount::Instance(), 11));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadC_BarCount::Instance(), 11));
    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadC_BarCount::Instance(), 2));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadC_FooTime::Instance(), 20));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadC_BarTime::Instance(), 5));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadC_BarTime::Instance(), 21));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadC_BarTime::Instance(), 82));

    Processor.Enqueue(
        MetricMessage::TimerSet(ThreadC_BazTime::Instance(), 82.12151));
  });
  Threads.emplace_back([&Processor]() {
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadD_Foo::Instance(), 200));
    Processor.Enqueue(MetricMessage::GaugeSet(ThreadD_Bar::Instance(), -200));

    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadD_Baz::Instance(), -5));
    Processor.Enqueue(MetricMessage::GaugeAdd(ThreadD_Baz::Instance(), 20));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadD_FooCount::Instance(), 11));

    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadD_BarCount::Instance(), 11));
    Processor.Enqueue(
        MetricMessage::CounterAdd(ThreadD_BarCount::Instance(), 2));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadD_FooTime::Instance(), 20));

    Processor.Enqueue(MetricMessage::TimerSet(ThreadD_BarTime::Instance(), 5));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadD_BarTime::Instance(), 21));
    Processor.Enqueue(MetricMessage::TimerSet(ThreadD_BarTime::Instance(), 82));

    Processor.Enqueue(
        MetricMessage::TimerSet(ThreadD_BazTime::Instance(), 82.12151));
  });

  for (auto &Thread : Threads) {
    Thread.join();
  }

  while (OutputPackets.empty()) {
    Processor.ProcessMetrics();
  }

  ASSERT_THAT(OutputPackets, SizeIs(1));

  auto Result = OutputPackets[0];
  EXPECT_EQ(std::get<1>(Result),
            1125); // packet size (bytes) including null terminator

  std::string PacketContents = std::get<0>(Result);
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_foo:200|g|#foo:bar\n"));
  EXPECT_THAT(
      PacketContents,
      HasSubstr("thread_a_bar:0|g|#foo:bar\nthread_a_bar:-200|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_baz:15|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_foo_count:11|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_bar_count:13|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_foo_time:20|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_a_bar_time:36|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("thread_a_baz_time:82.12151|ms|#foo:bar\n"));

  EXPECT_THAT(PacketContents, HasSubstr("thread_b_foo:200|g|#foo:bar\n"));
  EXPECT_THAT(
      PacketContents,
      HasSubstr("thread_b_bar:0|g|#foo:bar\nthread_b_bar:-200|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_b_baz:15|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_b_foo_count:11|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_b_bar_count:13|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_b_foo_time:20|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_b_bar_time:36|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("thread_b_baz_time:82.12151|ms|#foo:bar\n"));

  EXPECT_THAT(PacketContents, HasSubstr("thread_c_foo:200|g|#foo:bar\n"));
  EXPECT_THAT(
      PacketContents,
      HasSubstr("thread_c_bar:0|g|#foo:bar\nthread_c_bar:-200|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_c_baz:15|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_c_foo_count:11|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_c_bar_count:13|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_c_foo_time:20|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_c_bar_time:36|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("thread_c_baz_time:82.12151|ms|#foo:bar\n"));

  EXPECT_THAT(PacketContents, HasSubstr("thread_d_foo:200|g|#foo:bar\n"));
  EXPECT_THAT(
      PacketContents,
      HasSubstr("thread_d_bar:0|g|#foo:bar\nthread_d_bar:-200|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_d_baz:15|g|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_d_foo_count:11|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_d_bar_count:13|c|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_d_foo_time:20|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents, HasSubstr("thread_d_bar_time:36|ms|#foo:bar\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("thread_d_baz_time:82.12151|ms|#foo:bar\n"));
}

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(GaugeWithDerivs, "gauge_with_derivs",
                               MockEnabled, Aws::GameLift::Metrics::SampleAll(),
                               Aws::GameLift::Metrics::Max(),
                               Aws::GameLift::Metrics::Percentiles(0.1, 0.5,
                                                                   0.95),
                               Aws::GameLift::Metrics::Min());
GAMELIFT_METRICS_DEFINE_GAUGE(GaugeWithDerivs);

GAMELIFT_METRICS_DECLARE_TIMER(TimerWithDerivs, "timer_with_derivs",
                               MockEnabled, Aws::GameLift::Metrics::SampleAll(),
                               Aws::GameLift::Metrics::Max(),
                               Aws::GameLift::Metrics::Percentiles(0.1, 0.5,
                                                                   0.95),
                               Aws::GameLift::Metrics::Min());
GAMELIFT_METRICS_DEFINE_TIMER(TimerWithDerivs);
} // namespace

TEST_F(MetricsProcessorTests, TestDerivativeMetrics) {
  Aws::GameLift::Metrics::MetricsSettings Settings{};
  Settings.MaxPacketSizeBytes = 4000;
  Settings.CaptureIntervalSec = 0;
  Settings.SendPacketCallback = MockSend;
  MetricsProcessor Processor(Settings);

  std::vector<double> GaugeValues{
      {1559, 1045, 432,  1992, 654,  1883, 1297, 1619, 531,  826,  1481, 1576,
       13,   1712, 786,  1450, 396,  249,  1217, 1559, 1436, 127,  234,  1305,
       1391, 1213, 1636, 235,  83,   862,  1141, 199,  424,  92,   1286, 947,
       1470, 543,  1281, 626,  1045, 1294, 1527, 1467, 1752, 456,  1800, 878,
       53,   564,  1050, 896,  1415, 1903, 1123, 487,  659,  409,  487,  1243,
       1304, 228,  658,  1055, 1902, 544,  1122, 887,  1193, 330,  739,  75,
       653,  1798, 193,  1496, 1324, 1989, 83,   1236, 136,  621,  1213, 1783,
       763,  215,  330,  906,  1552, 1118, 783,  505,  1511, 1781, 498,  1033,
       1621, 1173, 1037, 1508}};
  Processor.Enqueue(
      MetricMessage::TagSet(GaugeWithDerivs::Instance(), "hello", "world"));
  for (double Value : GaugeValues) {
    Processor.Enqueue(
        MetricMessage::GaugeSet(GaugeWithDerivs::Instance(), Value));
  }

  std::vector<double> TimerValues{
      {623,  965,  843,  922,  1866, 1453, 982,  1290, 1061, 518,  1495, 899,
       411,  201,  1217, 1927, 888,  215,  1225, 1402, 1511, 699,  800,  1731,
       1079, 1720, 863,  76,   464,  1737, 2,    1091, 235,  765,  1229, 337,
       1982, 1873, 22,   452,  1257, 25,   1185, 90,   1295, 1152, 1964, 920,
       1551, 2000, 1558, 1567, 1203, 983,  771,  1562, 61,   418,  1907, 1183,
       770,  734,  1467, 74,   394,  751,  684,  614,  1169, 1349, 1024, 1398,
       813,  475,  954,  286,  1428, 442,  894,  113,  760,  925,  1651, 368,
       502,  1777, 443,  1673, 1066, 1683, 1346, 907,  646,  1772, 1218, 837,
       120,  1594, 676,  429}};
  Processor.Enqueue(
      MetricMessage::TagSet(TimerWithDerivs::Instance(), "hello", "world"));
  Processor.Enqueue(
      MetricMessage::TagSet(TimerWithDerivs::Instance(), "baz", "boz"));
  Processor.Enqueue(
      MetricMessage::TagRemove(TimerWithDerivs::Instance(), "hello"));
  for (double Value : TimerValues) {
    Processor.Enqueue(
        MetricMessage::TimerSet(TimerWithDerivs::Instance(), Value));
  }

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
            506); // packet size (bytes) including null terminator

  std::string PacketContents = std::get<0>(Result);
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs:1508|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs.max:1992|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs.min:13|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs.p10:213.40000|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs.p50:1045|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("gauge_with_derivs.p95:1804.15000|g|#hello:world\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs:979.49000|ms|#baz:boz\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs.max:2000|ms|#baz:boz\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs.min:2|ms|#baz:boz\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs.p10:213.60000|ms|#baz:boz\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs.p50:939.50000|ms|#baz:boz\n"));
  EXPECT_THAT(PacketContents,
              HasSubstr("timer_with_derivs.p95:1874.70000|ms|#baz:boz\n"));
}

// Tests for OnStartGameSession
TEST_F(MetricsProcessorTests, OnStartGameSession_WithValidSessionId_SetsGlobalTag)
{
    Aws::GameLift::Metrics::MetricsSettings Settings;
    Settings.SendPacketCallback = MockSend;
    Settings.MaxPacketSizeBytes = 4000;
    Settings.CaptureIntervalSec = 0;
    MetricsProcessor Processor(Settings);

    // Create a GameSession with a valid session ID
    Aws::GameLift::Server::Model::GameSession gameSession;
#ifdef GAMELIFT_USE_STD
    gameSession.SetGameSessionId("test-session-123");
#else
    gameSession.SetGameSessionId("test-session-123");
#endif

    // Call OnStartGameSession
    Processor.OnStartGameSession(gameSession);

    // Add a metric to trigger packet creation
    Processor.Enqueue(MetricMessage::GaugeSet(Foo::Instance(), 42));

    // Process metrics to generate packets
    while (OutputPackets.empty())
    {
        Processor.ProcessMetrics();
    }

    // Verify that the session_id global tag was set
    std::string PacketContents = std::get<0>(OutputPackets[0]);
    EXPECT_THAT(PacketContents, HasSubstr("session_id:test-session-123"));
}

TEST_F(MetricsProcessorTests, OnStartGameSession_WithEmptySessionId_DoesNotSetGlobalTag)
{
    Aws::GameLift::Metrics::MetricsSettings Settings;
    Settings.SendPacketCallback = MockSend;
    Settings.MaxPacketSizeBytes = 4000;
    Settings.CaptureIntervalSec = 0;
    MetricsProcessor Processor(Settings);

    // Create a GameSession with empty session ID
    Aws::GameLift::Server::Model::GameSession gameSession;
#ifdef GAMELIFT_USE_STD
    gameSession.SetGameSessionId("");
#else
    gameSession.SetGameSessionId("");
#endif

    // Call OnStartGameSession
    Processor.OnStartGameSession(gameSession);

    // Add a metric to trigger packet creation
    Processor.Enqueue(MetricMessage::GaugeSet(Foo::Instance(), 42));

    // Process metrics to generate packets
    while (OutputPackets.empty())
    {
        Processor.ProcessMetrics();
    }

    // Verify that the session_id global tag was NOT set
    std::string PacketContents = std::get<0>(OutputPackets[0]);
    EXPECT_THAT(PacketContents, Not(HasSubstr("session_id:")));
}

TEST_F(MetricsProcessorTests, OnStartGameSession_OverwritesPreviousSessionId)
{
    Aws::GameLift::Metrics::MetricsSettings Settings;
    Settings.SendPacketCallback = MockSend;
    Settings.MaxPacketSizeBytes = 4000;
    Settings.CaptureIntervalSec = 0;
    MetricsProcessor Processor(Settings);

    // Create first GameSession
    Aws::GameLift::Server::Model::GameSession gameSession1;
#ifdef GAMELIFT_USE_STD
    gameSession1.SetGameSessionId("first-session-123");
#else
    gameSession1.SetGameSessionId("first-session-123");
#endif

    // Call OnStartGameSession with first session
    Processor.OnStartGameSession(gameSession1);

    // Create second GameSession
    Aws::GameLift::Server::Model::GameSession gameSession2;
#ifdef GAMELIFT_USE_STD
    gameSession2.SetGameSessionId("second-session-456");
#else
    gameSession2.SetGameSessionId("second-session-456");
#endif

    // Call OnStartGameSession with second session
    Processor.OnStartGameSession(gameSession2);

    // Add a metric to trigger packet creation
    Processor.Enqueue(MetricMessage::GaugeSet(Foo::Instance(), 42));

    // Process metrics to generate packets
    while (OutputPackets.empty())
    {
        Processor.ProcessMetrics();
    }

    // Verify that only the latest session_id is present
    std::string PacketContents = std::get<0>(OutputPackets[0]);
    EXPECT_THAT(PacketContents, HasSubstr("session_id:second-session-456"));
    EXPECT_THAT(PacketContents, Not(HasSubstr("session_id:first-session-123")));
}
