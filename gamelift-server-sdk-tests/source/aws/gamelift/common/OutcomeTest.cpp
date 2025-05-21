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
#include <aws/gamelift/common/Outcome.h>

namespace Aws {
namespace GameLift {
namespace Common {
namespace Test {

TEST(OutcomeTest, GIVEN_noInput_WHEN_defaultConstructor_THEN_successIsFalse) {
    // GIVEN / WHEN
    Outcome<long, int> outcome;
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
}

TEST(OutcomeTest, GIVEN_resultInput_WHEN_constructOutcome_THEN_successIsTrue) {
    // GIVEN / WHEN
    long result = 123;
    Outcome<long, int> outcome(result);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetResult(), result);
}

TEST(OutcomeTest, GIVEN_errorInput_WHEN_constructOutcome_THEN_successIsFalse) {
    // GIVEN / WHEN
    int error = 123;
    Outcome<long, int> outcome(error);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError(), error);
}

TEST(OutcomeTest, GIVEN_resultAndError_WHEN_copyConstructor_THEN_success) {
    // GIVEN
    long result = 123;
    int error = 456;
    Outcome<long, int> successOutcome(result);
    Outcome<long, int> errorOutcome(error);
    // WHEN
    Outcome<long, int> successOutcomeCopy(successOutcome);
    Outcome<long, int> errorOutcomeCopy(errorOutcome);
    // THEN
    ASSERT_TRUE(successOutcomeCopy.IsSuccess());
    ASSERT_FALSE(errorOutcomeCopy.IsSuccess());
    ASSERT_EQ(errorOutcomeCopy.GetError(), error);
    ASSERT_EQ(successOutcomeCopy.GetResult(), result);
}

TEST(OutcomeTest, GIVEN_resultAndError_WHEN_operatorEquals_THEN_success) {
    // GIVEN
    long result = 123;
    int error = 456;
    Outcome<long, int> successOutcome(result);
    Outcome<long, int> errorOutcome(error);
    // WHEN
    Outcome<long, int> successOutcomeCopy = successOutcome;
    Outcome<long, int> errorOutcomeCopy = errorOutcome;
    // THEN
    ASSERT_TRUE(successOutcomeCopy.IsSuccess());
    ASSERT_FALSE(errorOutcomeCopy.IsSuccess());
    ASSERT_EQ(errorOutcomeCopy.GetError(), error);
    ASSERT_EQ(successOutcomeCopy.GetResult(), result);
}

} // namespace Test
} // namespace Common
} // namespace GameLift
} // namespace Aws