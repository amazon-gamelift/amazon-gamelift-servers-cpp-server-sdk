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

#include <aws/gamelift/server/MetricsParameters.h>
#include <aws/gamelift/common/Outcome.h>

// Forward declarations
namespace Aws {
namespace GameLift {
namespace Metrics {
struct MetricsSettings;
}
}
}

namespace Aws {
namespace GameLift {
namespace Metrics {

// Environment variable names for metrics configuration
static constexpr const char *ENV_VAR_STATSD_HOST = "GAMELIFT_STATSD_HOST";
static constexpr const char *ENV_VAR_STATSD_PORT = "GAMELIFT_STATSD_PORT";
static constexpr const char *ENV_VAR_CRASH_REPORTER_HOST = "GAMELIFT_CRASH_REPORTER_HOST";
static constexpr const char *ENV_VAR_CRASH_REPORTER_PORT = "GAMELIFT_CRASH_REPORTER_PORT";
static constexpr const char *ENV_VAR_FLUSH_INTERVAL_MS = "GAMELIFT_FLUSH_INTERVAL_MS";
static constexpr const char *ENV_VAR_MAX_PACKET_SIZE = "GAMELIFT_MAX_PACKET_SIZE";

// Default values for metrics configuration
static constexpr const char *DEFAULT_STATSD_HOST = "127.0.0.1";
static constexpr int DEFAULT_STATSD_PORT = 8125;
static constexpr const char *DEFAULT_CRASH_REPORTER_HOST = "127.0.0.1";
static constexpr int DEFAULT_CRASH_REPORTER_PORT = 8126;
static constexpr int DEFAULT_FLUSH_INTERVAL_MS = 10000;
static constexpr int DEFAULT_MAX_PACKET_SIZE = 512;

// Port validation constants
static constexpr int PORT_MIN = 1;
static constexpr int PORT_MAX = 65535;

/**
 * Internal utility function to create MetricsParameters with default values,
 * overridden by environment variables if set.
 */
Aws::GameLift::Server::MetricsParameters CreateMetricsParametersFromEnvironmentOrDefault();

/**
 * Create MetricsSettings from MetricsParameters.
 */
MetricsSettings FromMetricsParameters(const Aws::GameLift::Server::MetricsParameters &params);

/**
 * Validates MetricsParameters for required fields and valid ranges.
 * @param params The MetricsParameters to validate
 * @return GenericOutcome indicating success or validation error
 */
Aws::GameLift::GenericOutcome ValidateMetricsParameters(const Aws::GameLift::Server::MetricsParameters &params);

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
