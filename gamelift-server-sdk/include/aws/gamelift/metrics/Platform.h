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

/**
 * Defines a new metric logging platform.
 *
 * @param platform Platform name.
 * @param compileTimeExpr An expression evaluated at compile time. If true or
 * not 0, platform is enabled. Otherwise, platform won't log any metrics.
 */
#define GAMELIFT_METRICS_DEFINE_PLATFORM(platform, compileTimeExpr)            \
  struct platform {                                                            \
    static constexpr bool bEnabled = (compileTimeExpr);                        \
  }

/**
 * Defines a new metric logging platform.
 *
 * @param api API to use
 * @param platform Platform name.
 * @param compileTimeExpr An expression evaluated at compile time. If true or
 * not 0, platform is enabled. Otherwise, platform won't log any metrics.
 */
#define GAMELIFT_METRICS_DEFINE_PLATFORM_API(api, platform, compileTimeExpr)   \
  struct api platform {                                                        \
    static constexpr bool bEnabled = (compileTimeExpr);                        \
  }

/**
 * @returns true if platform is enabled at compile time
 */
#define GAMELIFT_METRICS_PLATFORM_IS_ENABLED(platform) platform::bEnabled
