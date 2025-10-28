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

#include <spdlog/spdlog.h>
#include <cstdlib>

#define GAMELIFT_METRICS_LOG_INFO(format, ...)                                 \
  spdlog::info("[METRICS] " format, ##__VA_ARGS__)

#define GAMELIFT_METRICS_LOG_WARN(format, ...)                                 \
  spdlog::warn("[METRICS] " format, ##__VA_ARGS__)

#define GAMELIFT_METRICS_LOG_ERROR(format, ...)                                \
  spdlog::error("[METRICS] " format, ##__VA_ARGS__)

#if defined(GAMELIFT_METRICS_DEBUG) && GAMELIFT_METRICS_DEBUG
#define GAMELIFT_METRICS_LOG_CRITICAL(format, ...)                             \
  do {                                                                         \
    spdlog::critical("[METRICS] " format, ##__VA_ARGS__);                      \
    std::abort();                                                              \
  } while (0)
#else
#define GAMELIFT_METRICS_LOG_CRITICAL(format, ...)                             \
  spdlog::critical("[METRICS] " format, ##__VA_ARGS__)
#endif
