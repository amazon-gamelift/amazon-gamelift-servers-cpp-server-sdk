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
#include <aws/gamelift/internal/GameLiftCommonState.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class GameLiftCommonStateTest : public ::testing::Test {
public:
    // Derived class in order to test the Abstract Singleton
    class TestCommonState : public Internal::GameLiftCommonState {
    public:
        TestCommonState() {}

        Internal::GAMELIFT_INTERNAL_STATE_TYPE GetStateType() override { return Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER; }
    };

protected:
    void SetUp() override {
        instance = new TestCommonState();
        TestCommonState::SetInstance(instance);
    }

    void TearDown() override { TestCommonState::DestroyInstance(); }

    TestCommonState *instance;
};

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceNotInitialized_WHEN_GetInstanceWithType_THEN_Error) {
    // GIVEN (make sure no instance initialized)
    TestCommonState::DestroyInstance();
    // WHEN
    Internal::GetInstanceOutcome outcome = TestCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::NOT_INITIALIZED);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceNotInitialized_WHEN_GetInstance_THEN_Error) {
    // GIVEN (make sure no instance initialized)
    TestCommonState::DestroyInstance();
    // WHEN
    Internal::GetInstanceOutcome outcome = TestCommonState::GetInstance();
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::NOT_INITIALIZED);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceNotInitialized_WHEN_DestroyInstance_THEN_Error) {
    // GIVEN (make sure no instance initialized)
    TestCommonState::DestroyInstance();
    // WHEN
    GenericOutcome outcome = TestCommonState::DestroyInstance();
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::NOT_INITIALIZED);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceInitialized_WHEN_GetInstanceWithType_THEN_Success) {
    // GIVEN - Instance Initialized
    // WHEN
    Internal::GetInstanceOutcome outcome = TestCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetResult(), instance);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceInitialized_WHEN_GetInstance_THEN_Success) {
    // GIVEN - Instance Initialized
    // WHEN
    Internal::GetInstanceOutcome outcome = TestCommonState::GetInstance();
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetResult(), instance);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceInitialized_WHEN_DestroyInstance_THEN_Success) {
    // GIVEN - Instance Initialized
    // WHEN
    GenericOutcome outcome = TestCommonState::DestroyInstance();
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetResult(), nullptr);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceAlreadyInitialized_WHEN_SetInstance_THEN_Error) {
    // GIVEN - Instance Already Initialized
    // WHEN
    GenericOutcome outcome = TestCommonState::SetInstance(instance);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED);
}

TEST_F(GameLiftCommonStateTest, GIVEN_InstanceInitialized_WHEN_GetInstanceOfDifferentType_THEN_Error) {
    // GIVEN - Instance Initialized
    // WHEN
    Internal::GetInstanceOutcome outcome = TestCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::CLIENT);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    ASSERT_EQ(outcome.GetError().GetErrorType(), GAMELIFT_ERROR_TYPE::INITIALIZATION_MISMATCH);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws