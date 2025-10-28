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

#ifdef GAMELIFT_USE_STD
#include <chrono>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

/**
 * INTERNAL: High resolution clock used for user-facing ScopedTimer.
 */
struct HighResolutionClock {
  using Time = std::chrono::high_resolution_clock::time_point;
  using Duration = std::chrono::high_resolution_clock::duration;

  static Time Now() { return std::chrono::high_resolution_clock::now(); }

  static double ToMilliseconds(Duration Duration) {
    using Milliseconds = std::chrono::duration<double, std::milli>;
    return std::chrono::duration_cast<Milliseconds>(Duration).count();
  }
};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#else

#include <aws/gamelift/metrics/InternalTypes.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

/**
 * INTERNAL: High resolution clock used for user-facing ScopedTimer.
 */
struct GAMELIFT_METRICS_API HighResolutionClock {
  using Time = Int64;
  using Duration = Int64;

  static Int64 Now();
  static double ToMilliseconds(Int64 Duration);
};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#endif
