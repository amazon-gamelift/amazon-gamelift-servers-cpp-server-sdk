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

#include <aws/gamelift/internal/retry/RetryingCallable.h>
#include <thread>

namespace Aws {
namespace GameLift {
namespace Internal {

RetryingCallable::Builder &RetryingCallable::Builder::WithCallable(const std::function<bool(void)> &callable) {
    m_callable = callable;
    return *this;
}

RetryingCallable::Builder &RetryingCallable::Builder::WithRetryStrategy(RetryStrategy *retryStrategy) {
    m_retryStrategy = retryStrategy;
    return *this;
}

RetryingCallable RetryingCallable::Builder::Build() const { return RetryingCallable(*m_retryStrategy, m_callable); }

RetryingCallable::RetryingCallable(RetryStrategy &retryStrategy, std::function<bool(void)> callable) : m_retryStrategy(retryStrategy), m_callable(callable) {}

void RetryingCallable::call() { m_retryStrategy.apply(m_callable); }

} // namespace Internal
} // namespace GameLift
} // namespace Aws