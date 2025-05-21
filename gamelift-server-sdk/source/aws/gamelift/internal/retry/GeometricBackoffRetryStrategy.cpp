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

#include <aws/gamelift/internal/retry/GeometricBackoffRetryStrategy.h>
#include <thread>
#include <spdlog/spdlog.h>

namespace Aws {
namespace GameLift {
namespace Internal {

void GeometricBackoffRetryStrategy::apply(const std::function<bool(void)> &callable) {
    int retryIntervalSeconds = m_initialRetryIntervalSeconds;
    for (int i = 0; i < m_maxRetries; ++i) {
        bool success = callable();
        if (success) {
            break;
        } else {
            spdlog::warn("Connection Failed. Retrying in {} seconds...", retryIntervalSeconds);
            std::this_thread::sleep_for(std::chrono::seconds(retryIntervalSeconds));
            retryIntervalSeconds *= m_retryFactor;
            retryIntervalSeconds = retryIntervalSeconds > m_maxRetryIntervalSeconds ? m_maxRetryIntervalSeconds : retryIntervalSeconds;
        }
    }
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws