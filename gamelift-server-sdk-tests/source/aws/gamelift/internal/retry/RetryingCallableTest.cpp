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

#include "gtest/gtest.h"
#include <aws/gamelift/internal/retry/MockRetryStrategy.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

TEST(RetryingCallableTest, GIVEN_retryStrategyAndCallable_WHEN_call_THEN_delegateToStrategy) {
    // GIVEN
    MockRetryStrategy strategy;
    const std::function<bool(void)> &methodToRetry = [] { return true; };
    RetryingCallable callable = RetryingCallable::Builder().WithRetryStrategy(&strategy).WithCallable(methodToRetry).Build();
    // THEN
    EXPECT_CALL(strategy, apply(testing::_));
    // WHEN
    callable.call();
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
