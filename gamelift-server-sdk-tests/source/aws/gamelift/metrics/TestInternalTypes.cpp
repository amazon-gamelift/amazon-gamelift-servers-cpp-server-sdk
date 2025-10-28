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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <aws/gamelift/metrics/InternalTypes.h>

struct A {};
struct B {};
struct C {};

struct D {};
struct E {};
struct F {};

TEST(InternalTypesTests,
     GivenThreeSupportedTypes_WhenUsedWithSupportedTypes_ThenValuesAreTrue) {
  EXPECT_TRUE((Aws::GameLift::Metrics::IsSupported<A, A, B, C>::value));
  EXPECT_TRUE((Aws::GameLift::Metrics::IsSupported<B, A, B, C>::value));
  EXPECT_TRUE((Aws::GameLift::Metrics::IsSupported<C, A, B, C>::value));
}

TEST(InternalTypesTests,
     GivenThreeSupportedTypes_WhenUsedWithUnsupportedTypes_ThenValuesAreFalse) {
  EXPECT_FALSE((Aws::GameLift::Metrics::IsSupported<D, A, B, C>::value));
  EXPECT_FALSE((Aws::GameLift::Metrics::IsSupported<E, A, B, C>::value));
  EXPECT_FALSE((Aws::GameLift::Metrics::IsSupported<F, A, B, C>::value));
}
