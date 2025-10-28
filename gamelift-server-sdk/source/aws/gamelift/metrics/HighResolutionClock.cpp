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
#include <aws/gamelift/metrics/HighResolutionClock.h>

#ifndef GAMELIFT_USE_STD
#include <aws/gamelift/metrics/InternalTypes.h>

#include <chrono>

/**
 *  We hide the internal chrono use and report a time as int64 nanoseconds
 * publicly.
 *
 *  Then convert back to chrono internally.
 */

namespace {
using Nanoseconds =
    std::chrono::duration<Aws::GameLift::Metrics::Int64, std::nano>;
}

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {
Int64 HighResolutionClock::Now() {
  const auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::time_point_cast<Nanoseconds>(now)
      .time_since_epoch()
      .count();
}

double HighResolutionClock::ToMilliseconds(Int64 duration) {
  using Milliseconds = std::chrono::duration<double, std::milli>;
  return std::chrono::duration_cast<Milliseconds>(Nanoseconds(duration))
      .count();
}
} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#endif // !GAMELIFT_USE_STD
