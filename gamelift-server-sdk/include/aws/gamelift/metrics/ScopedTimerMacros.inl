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

/**
 * Times a scope.
 *
 * Timing begins when called and ends when the scope exits.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 */
#define GAMELIFT_METRICS_TIME_SCOPE(metric)                                                                                                                                                                 \
    static_assert(Aws::GameLift::Metrics::IsSupported<metric::MetricType, Aws::GameLift::Metrics::Timer>::value, "Metric '" #metric "' is not a timer. GAMELIFT_METRICS_TIME_SCOPE only supports timers."); \
    ::Aws::GameLift::Metrics::Internal::MacroScopedTimer<metric> _GAMELIFT_INTERNAL_TIME_SCOPE_##metric_##__LINE__

/**
 * Times an expression. For example, a function call.
 *
 * @param metric Metric declared with GAMELIFT_METRICS_DECLARE_TIMER
 * @param expr Expression to time
 * @returns Result of `expr`.
 */
#define GAMELIFT_METRICS_TIME_EXPR(metric, expr)                                                                                                                                                               \
    [&]()                                                                                                                                                                                                      \
    {                                                                                                                                                                                                          \
        static_assert(Aws::GameLift::Metrics::IsSupported<metric::MetricType, Aws::GameLift::Metrics::Timer>::value, "Metric '" #metric "' is not a timer. GAMELIFT_METRICS_TIME_EXPR only supports timers."); \
        ::Aws::GameLift::Metrics::Internal::MacroScopedTimer<metric> _GAMELIFT_INTERNAL_TIME_EXPR_##metric_##__LINE__;                                                                                         \
        return (expr);                                                                                                                                                                                         \
    }()
