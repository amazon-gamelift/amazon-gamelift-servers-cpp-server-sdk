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
#include <aws/gamelift/metrics/DynamicMetric.h>

#ifndef GAMELIFT_USE_STD
#include <cstring>

namespace Aws {
namespace GameLift {
namespace Metrics {
void DynamicMetric::SetKey(const char *newKey) {
#ifdef _WIN32
  strncpy_s(m_key, DynamicMetric::MAXIMUM_KEY_LENGTH, newKey,
            DynamicMetric::MAXIMUM_KEY_LENGTH);
#else
  strncpy(m_key, newKey, DynamicMetric::MAXIMUM_KEY_LENGTH);
#endif // _WIN32
}
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#endif // !GAMELIFT_USE_STD