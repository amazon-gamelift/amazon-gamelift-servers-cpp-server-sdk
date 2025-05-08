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
#include <aws/gamelift/internal/retry/JitteredGeometricBackoffRetryStrategy.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

TEST(JitteredGeometricBackoffRetryStrategyTest, GIVEN_DefaultSettings_WHEN_callWithEventuallySucceedingCallable_THEN_succeedWithRetries) {
    // GIVEN
    static int calls = 0;
    const std::function<bool(void)> &eventuallySucceedCallable = [] {
        calls++;
        return calls > 1;
    };
    JitteredGeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder().WithRetryStrategy(&retryStrategy).WithCallable(eventuallySucceedCallable).Build();
    // WHEN
    callable.call();
    // THEN
    ASSERT_EQ(calls, 2);
}

TEST(JitteredGeometricBackoffRetryStrategyTest, GIVEN_DefaultSettings_WHEN_callWithAlwaysSucceedingCallable_THEN_succeeds) {
    // GIVEN
    static int calls = 0;
    const std::function<bool(void)> &alwaysSucceedCallable = [] {
        calls++;
        return true;
    };
    JitteredGeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder().WithRetryStrategy(&retryStrategy).WithCallable(alwaysSucceedCallable).Build();
    // WHEN
    callable.call();
    // THEN
    ASSERT_EQ(calls, 1);
}

TEST(JitteredGeometricBackoffRetryStrategyTest, GIVEN_DefaultSettings_WHEN_callWithAFailCallable_THEN_stops) {
    // GIVEN
    static int calls = 0;
    const std::function<bool(void)> &alwaysFailCallable = [] {
        calls++;
        return false;
    };
    JitteredGeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder().WithRetryStrategy(&retryStrategy).WithCallable(alwaysFailCallable).Build();
    // WHEN
    callable.call();
    // THEN
    ASSERT_TRUE(calls > 1);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
