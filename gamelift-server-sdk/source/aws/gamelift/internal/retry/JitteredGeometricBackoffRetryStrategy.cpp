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

#include <aws/gamelift/internal/retry/JitteredGeometricBackoffRetryStrategy.h>
#include <random>
#include <thread>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

void JitteredGeometricBackoffRetryStrategy::apply(const std::function<bool(void)> &callable) {
    int retryIntervalMs = m_initialRetryIntervalMs;
    std::random_device rd;
    std::mt19937 randGenerator(rd());
    for (int i = 0; i < m_maxRetries; ++i) {
        bool success = callable();
        if (success) {
            break;
        } else {
            std::uniform_int_distribution<> intervalRange(m_minRetryDelayMs, retryIntervalMs);
            int currentInterval = intervalRange(randGenerator);
            spdlog::warn("Sending Message Failed. Retrying in {} milliseconds...", currentInterval);
            std::this_thread::sleep_for(std::chrono::milliseconds(currentInterval));
            retryIntervalMs *= m_retryFactor;
        }
    }
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws