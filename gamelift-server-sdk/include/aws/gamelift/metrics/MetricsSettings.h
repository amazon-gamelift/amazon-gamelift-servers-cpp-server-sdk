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

#include <aws/gamelift/metrics/Defs.h>
#include <aws/gamelift/metrics/Function.h>
#include <string>

namespace Aws {
namespace GameLift {
namespace Metrics {

struct GAMELIFT_METRICS_API MetricsSettings {
  using SendPacketFunc = Function<void(const char *, int)>;
  using PreProcessingFunc = Function<void()>;

  /**
   * Callback used to send UDP packets.
   * Use this to integrate with engine native sockets.
   */
  SendPacketFunc SendPacketCallback;

  /**
   * Callback before the metrics are processed for current collection period.
   */
  PreProcessingFunc PreProcessingCallback;

  /**
   * Maximum packet size in bytes.
   *
   * Typical MTU is 1500 so 1472 maximizes size while leaving room for UDP
   * header.
   */
  int MaxPacketSizeBytes = 1472;

  /**
   * Metric capture interval.
   *
   * Metrics are collected and sent off every CaptureInterval seconds.
   */
  float CaptureIntervalSec = 5.0f;

  /**
   * Floating point precision.
   *
   * How many digits past the decimal point should we log?
   */
  int FloatPrecision = 5;

   /**
    * Crash reporter host.
    */
#ifdef GAMELIFT_USE_STD
  std::string CrashReporterHost;
#else
  const char* CrashReporterHost = nullptr;
#endif

  /**
   * Crash reporter port.
   */
  int CrashReporterPort = 0;

  /**
   * StatsD client host.
   */
#ifdef GAMELIFT_USE_STD
  std::string StatsDClientHost;
#else
  const char* StatsDClientHost = nullptr;
#endif

  /**
   * StatsD client port.
   */
  int StatsDClientPort = 0;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
