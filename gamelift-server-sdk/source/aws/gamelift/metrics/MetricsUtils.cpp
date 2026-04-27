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
#include <aws/gamelift/metrics/MetricsUtils.h>
#include <aws/gamelift/metrics/GlobalMetricsProcessor.h>
#include <aws/gamelift/metrics/MetricsSettings.h>
#include <aws/gamelift/common/GameLiftErrors.h>
#include <aws/gamelift/internal/util/EnvironmentUtils.h>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>

using namespace Aws::GameLift::Server;

namespace Aws {
namespace GameLift {
namespace Metrics {

MetricsParameters CreateMetricsParametersFromEnvironmentOrDefault() {
    // Start with default values
    std::string statsdHost = DEFAULT_STATSD_HOST;
    int statsdPort = DEFAULT_STATSD_PORT;
    std::string crashReporterHost = DEFAULT_CRASH_REPORTER_HOST;
    int crashReporterPort = DEFAULT_CRASH_REPORTER_PORT;
    int flushIntervalMs = DEFAULT_FLUSH_INTERVAL_MS;
    int maxPacketSize = DEFAULT_MAX_PACKET_SIZE;

    // Check environment variables and override defaults
    using Aws::GameLift::Internal::Utils::SafeGetenv;

    std::string envStatsdHost = SafeGetenv(ENV_VAR_STATSD_HOST);
    if (!envStatsdHost.empty()) {
        statsdHost = envStatsdHost;
        spdlog::info("Env override for statsdHost: {}", statsdHost);
    }

    std::string envStatsdPort = SafeGetenv(ENV_VAR_STATSD_PORT);
    if (!envStatsdPort.empty()) {
        statsdPort = std::atoi(envStatsdPort.c_str());
        spdlog::info("Env override for statsdPort: {}", statsdPort);
    }

    std::string envCrashReporterHost = SafeGetenv(ENV_VAR_CRASH_REPORTER_HOST);
    if (!envCrashReporterHost.empty()) {
        crashReporterHost = envCrashReporterHost;
        spdlog::info("Env override for crashReporterHost: {}", crashReporterHost);
    }

    std::string envCrashReporterPort = SafeGetenv(ENV_VAR_CRASH_REPORTER_PORT);
    if (!envCrashReporterPort.empty()) {
        crashReporterPort = std::atoi(envCrashReporterPort.c_str());
        spdlog::info("Env override for crashReporterPort: {}", crashReporterPort);
    }

    std::string envFlushInterval = SafeGetenv(ENV_VAR_FLUSH_INTERVAL_MS);
    if (!envFlushInterval.empty()) {
        flushIntervalMs = std::atoi(envFlushInterval.c_str());
        spdlog::info("Env override for flushIntervalMs: {}", flushIntervalMs);
    }

    std::string envMaxPacketSize = SafeGetenv(ENV_VAR_MAX_PACKET_SIZE);
    if (!envMaxPacketSize.empty()) {
        maxPacketSize = std::atoi(envMaxPacketSize.c_str());
        spdlog::info("Env override for maxPacketSize: {}", maxPacketSize);
    }

#ifdef GAMELIFT_USE_STD
    return MetricsParameters(statsdHost, statsdPort, crashReporterHost, crashReporterPort, flushIntervalMs, maxPacketSize);
#else
    return MetricsParameters(statsdHost.c_str(), statsdPort, crashReporterHost.c_str(), crashReporterPort, flushIntervalMs, maxPacketSize);
#endif
}

MetricsSettings FromMetricsParameters(const Aws::GameLift::Server::MetricsParameters &params) {
    MetricsSettings settings;
    settings.StatsDClientHost = params.GetStatsDHost();
    settings.StatsDClientPort = params.GetStatsDPort();
    settings.CrashReporterHost = params.GetCrashReporterHost();
    settings.CrashReporterPort = params.GetCrashReporterPort();
    settings.MaxPacketSizeBytes = params.GetMaxPacketSize();
    settings.CaptureIntervalSec = params.GetFlushIntervalMs() / 1000.0f;
    return settings;
}

Aws::GameLift::GenericOutcome ValidateMetricsParameters(const Aws::GameLift::Server::MetricsParameters &params) {
    // Validate StatsD host
#ifdef GAMELIFT_USE_STD
    if (params.GetStatsDHost().empty()) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "StatsDHost cannot be empty"));
    }
#else
    const char* statsdHost = params.GetStatsDHost();
    if (statsdHost == nullptr || statsdHost[0] == '\0') {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "StatsDHost cannot be empty"));
    }
#endif

    // Validate StatsD port
    int statsdPort = params.GetStatsDPort();
    if (statsdPort < PORT_MIN || statsdPort > PORT_MAX) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "StatsDPort must be between 1 and 65535"));
    }

    // Validate CrashReporter host
#ifdef GAMELIFT_USE_STD
    if (params.GetCrashReporterHost().empty()) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "CrashReporterHost cannot be empty"));
    }
#else
    const char* crashReporterHost = params.GetCrashReporterHost();
    if (crashReporterHost == nullptr || crashReporterHost[0] == '\0') {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "CrashReporterHost cannot be empty"));
    }
#endif

    // Validate CrashReporter port
    int crashReporterPort = params.GetCrashReporterPort();
    if (crashReporterPort < PORT_MIN || crashReporterPort > PORT_MAX) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "CrashReporterPort must be between 1 and 65535"));
    }

    // Validate FlushIntervalMs
    int flushIntervalMs = params.GetFlushIntervalMs();
    if (flushIntervalMs < 0) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "FlushIntervalMs must be non-negative"));
    }

    // Validate MaxPacketSize
    int maxPacketSize = params.GetMaxPacketSize();
    if (maxPacketSize < 0) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::VALIDATION_EXCEPTION, "MaxPacketSize must be non-negative"));
    }

    return Aws::GameLift::GenericOutcome(nullptr);
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
