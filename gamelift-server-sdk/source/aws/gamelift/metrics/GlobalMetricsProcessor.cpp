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
#include <aws/gamelift/metrics/GlobalMetricsProcessor.h>
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/MetricsProcessor.h>
#include <aws/gamelift/metrics/LoggerMacros.h>
#include <aws/gamelift/metrics/CrashReporterClient.h>
#include <aws/gamelift/metrics/StatsDClient.h>
#include <aws/gamelift/server/model/GameSession.h>

#include <cassert>
#include <cstdlib>
#include <string>
#ifdef __linux__
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

GAMELIFT_METRICS_DEFINE_GAUGE(ServerUpGauge);

namespace {
std::unique_ptr<::Aws::GameLift::Metrics::IMetricsProcessor>
    GlobalProcessor(nullptr);
std::shared_ptr<::Aws::GameLift::Metrics::StatsDClient>
    GlobalStatsDClient(nullptr);
std::shared_ptr<::Aws::GameLift::Metrics::CrashReporterClient>
    GlobalCrashReporter(nullptr);

void InitializeCrashReporter(const MetricsSettings &settings) {
    std::string crashReporterHost;
#ifdef GAMELIFT_USE_STD
    crashReporterHost = settings.CrashReporterHost;
#else
    crashReporterHost = (settings.CrashReporterHost != nullptr) ? std::string(settings.CrashReporterHost) : "";
#endif
    // Skip crash reporter initialization if host is empty
    if (crashReporterHost.empty()) {
        GAMELIFT_METRICS_LOG_INFO("Crash reporter disabled - host not set");
        return;
    }
    int crashReporterPort = settings.CrashReporterPort;
    GlobalCrashReporter = std::make_shared<CrashReporterClient>(crashReporterHost, crashReporterPort);
    GlobalCrashReporter->RegisterProcess();
}

void InitializeStatsDClient(const MetricsSettings &settings) {
    std::string statsdHost;
#ifdef GAMELIFT_USE_STD
    statsdHost = settings.StatsDClientHost;
#else
    statsdHost = (settings.StatsDClientHost != nullptr) ? std::string(settings.StatsDClientHost) : "";
#endif
    // Skip crash reporter initialization if host is empty
    if (statsdHost.empty()) {
        GAMELIFT_METRICS_LOG_INFO("StatsDClient disabled - host not set");
        return;
    }
    int statsdPort = settings.StatsDClientPort;
    GlobalStatsDClient = std::make_shared<StatsDClient>(statsdHost.c_str(), statsdPort);
    GAMELIFT_METRICS_LOG_INFO("Created StatsD client for {}:{}", statsdHost, statsdPort);
}

void InitializeDefaultGlobalTags() {
  if (GlobalProcessor) {
    // Check if GAMELIFT_SDK_PROCESS_ID environment variable is set
    const char *processId = std::getenv(ENV_VAR_PROCESS_ID);
    if (processId != nullptr && processId[0] != '\0') {
      GlobalProcessor->SetGlobalTag("gamelift_process_id", processId);

      GAMELIFT_METRICS_LOG_INFO("Set global tag gamelift_process_id: {}",
                                processId);
    }

    // Set the OS process ID (Linux and Windows).
#if defined(_WIN32) || defined(_WIN64)
    DWORD pid = GetCurrentProcessId();
    std::string pidStr = std::to_string(pid);
#else
    pid_t pid = getpid();
    std::string pidStr = std::to_string(pid);
#endif
    GlobalProcessor->SetGlobalTag("process_pid", pidStr.c_str());
    GAMELIFT_METRICS_LOG_INFO("Set global tag process_pid: {}", pidStr);
  }
}
} // namespace

namespace Aws {
namespace GameLift {
namespace Metrics {

void MetricsInitialize(const MetricsSettings &settings) {
  GAMELIFT_METRICS_LOG_INFO("Initializing GameLift Servers Metrics");

  assert(!GlobalProcessor);
  InitializeCrashReporter(settings);
  InitializeStatsDClient(settings);

  // Create settings with StatsD callback only if no callback is already set
  MetricsSettings settingsWithCallbackOverride = settings;
  if (!settings.SendPacketCallback) {
    settingsWithCallbackOverride.SendPacketCallback = [](const char* data, int size) {
      if (GlobalStatsDClient) {
        GlobalStatsDClient->Send(data, size);
      } else {
        GAMELIFT_METRICS_LOG_ERROR("StatsDClient is not initialized. Cannot send metrics data.");
      }
    };
  }

  GlobalProcessor.reset(new MetricsProcessor(settingsWithCallbackOverride));
  InitializeDefaultGlobalTags();

  GAMELIFT_METRICS_LOG_INFO(
      "GameLift Servers Metrics initialized successfully");
}

void MetricsTerminate() {
  GAMELIFT_METRICS_SET(ServerUpGauge, 0);

  // Process the final metrics before shutting down
  if (GlobalProcessor) {
    GlobalProcessor->ProcessMetricsNow();
  }

  if (GlobalCrashReporter) {
    GlobalCrashReporter->DeregisterProcess();
    GlobalCrashReporter.reset();
  }

  if (GlobalStatsDClient) {
    GlobalStatsDClient.reset();
  }

  GlobalProcessor.reset();
}

void MetricsProcess() {
  assert(GlobalProcessor);
  if (GlobalProcessor) {
    GlobalProcessor->ProcessMetrics();
  }
}

void OnGameSessionStarted(
    const Aws::GameLift::Server::Model::GameSession &session) {
  assert(GlobalProcessor);
  if (GlobalProcessor) {
    GlobalProcessor->OnStartGameSession(session);
  }
  if (GlobalCrashReporter) {
#ifdef GAMELIFT_USE_STD
    GlobalCrashReporter->TagGameSession(session.GetGameSessionId());
#else
    GlobalCrashReporter->TagGameSession(std::string(session.GetGameSessionId()));
#endif
  }
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws

::Aws::GameLift::Metrics::IMetricsProcessor *GameLiftMetricsGlobalProcessor() {
  return GlobalProcessor.get();
}
