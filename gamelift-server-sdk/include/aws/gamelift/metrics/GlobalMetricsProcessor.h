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

#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Defs.h>
#include <aws/gamelift/metrics/GaugeMacros.h>
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/MetricsSettings.h>
#include <aws/gamelift/metrics/Platform.h>
#include <aws/gamelift/metrics/Samplers.h>
#include <aws/gamelift/server/model/GameSession.h>

// Internal platform definition always enabled to guarantee shipping critical
// metrics such as server_up.
GAMELIFT_METRICS_DEFINE_PLATFORM(GlobalMetricsProcessor, true);

// Indicates the server is running and the metrics system is initialized.
GAMELIFT_METRICS_DECLARE_GAUGE(ServerUpGauge, "server_up",
                               GlobalMetricsProcessor,
                               Aws::GameLift::Metrics::SampleAll());

namespace Aws {
namespace GameLift {
namespace Metrics {

static constexpr const char *ENV_VAR_PROCESS_ID = "GAMELIFT_SDK_PROCESS_ID";

/**
 * **Must** be called before any other metrics APIs are used.
 */
extern GAMELIFT_METRICS_API void
MetricsInitialize(const MetricsSettings &settings);
extern GAMELIFT_METRICS_API void MetricsInitialize();

/**
 * @brief Cleans up global metrics capture state.
 *
 * Metrics APIs **must not** be used after this function is called.
 */
extern GAMELIFT_METRICS_API void MetricsTerminate();

/**
 * @brief Processes metrics if enough time has elapsed since the last capture.
 */
extern GAMELIFT_METRICS_API void MetricsProcess();

/**
 * @brief Called when a game session is started
 * @param session The game session that was started
 */
extern GAMELIFT_METRICS_API void
OnGameSessionStarted(const Aws::GameLift::Server::Model::GameSession &session);

} // namespace Metrics
} // namespace GameLift
} // namespace Aws

/*
 * Defining GameLiftMetricsGlobalProcessor as a function instead of
 * a static method like IMetricProcessor::Get() allows us to mock the
 * processor in testing. Same goes for not using namespaces.
 */

/**
 * @returns The global metrics processor instance used to log stats.
 */
extern GAMELIFT_METRICS_API ::Aws::GameLift::Metrics::IMetricsProcessor *
GameLiftMetricsGlobalProcessor();
