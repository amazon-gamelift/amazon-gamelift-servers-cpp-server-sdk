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
#include <aws/gamelift/server/model/GameSessionStatus.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

TEST(GameSessionStatusTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN
    GameSessionStatus status{};
    // THEN
    ASSERT_EQ(status, GameSessionStatus::NOT_SET);
}

TEST(GameSessionStatusTest, GIVEN_status_WHEN_getNameForGameSessionStatus_THEN_success) {
    // GIVEN
    GameSessionStatus status1 = GameSessionStatus::ACTIVE;
    GameSessionStatus status2 = GameSessionStatus::ACTIVATING;
    GameSessionStatus status3 = GameSessionStatus::TERMINATING;
    GameSessionStatus status4 = GameSessionStatus::TERMINATED;
    GameSessionStatus status5 = GameSessionStatus::NOT_SET;
    // WHEN
    auto name1 = GameSessionStatusMapper::GetNameForGameSessionStatus(status1);
    auto name2 = GameSessionStatusMapper::GetNameForGameSessionStatus(status2);
    auto name3 = GameSessionStatusMapper::GetNameForGameSessionStatus(status3);
    auto name4 = GameSessionStatusMapper::GetNameForGameSessionStatus(status4);
    auto name5 = GameSessionStatusMapper::GetNameForGameSessionStatus(status5);
    // THEN
    Utility::TestHelper::AssertStringsEqual(name1, "ACTIVE");
    Utility::TestHelper::AssertStringsEqual(name2, "ACTIVATING");
    Utility::TestHelper::AssertStringsEqual(name3, "TERMINATING");
    Utility::TestHelper::AssertStringsEqual(name4, "TERMINATED");
    Utility::TestHelper::AssertStringsEqual(name5, "NOT_SET");
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST(GameSessionStatusTest, GIVEN_name_WHEN_getGameSessionStatusForName_std_THEN_success) {
    // GIVEN
    std::string name1 = "ACTIVE";
    std::string name2 = "ACTIVATING";
    std::string name3 = "TERMINATING";
    std::string name4 = "TERMINATED";
    std::string name5 = "BLAH";
    std::string name6 = "RANDOM STATUS";
    // WHEN
    GameSessionStatus status1 = GameSessionStatusMapper::GetGameSessionStatusForName(name1);
    GameSessionStatus status2 = GameSessionStatusMapper::GetGameSessionStatusForName(name2);
    GameSessionStatus status3 = GameSessionStatusMapper::GetGameSessionStatusForName(name3);
    GameSessionStatus status4 = GameSessionStatusMapper::GetGameSessionStatusForName(name4);
    GameSessionStatus status5 = GameSessionStatusMapper::GetGameSessionStatusForName(name5);
    GameSessionStatus status6 = GameSessionStatusMapper::GetGameSessionStatusForName(name6);
    // THEN
    ASSERT_EQ(status1, GameSessionStatus::ACTIVE);
    ASSERT_EQ(status2, GameSessionStatus::ACTIVATING);
    ASSERT_EQ(status3, GameSessionStatus::TERMINATING);
    ASSERT_EQ(status4, GameSessionStatus::TERMINATED);
    ASSERT_EQ(status5, GameSessionStatus::NOT_SET);
    ASSERT_EQ(status6, GameSessionStatus::NOT_SET);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST(GameSessionStatusTest, GIVEN_name_WHEN_getGameSessionStatusForName_no_std_THEN_success) {
    // GIVEN
    const char *name1 = "ACTIVE";
    const char *name2 = "ACTIVATING";
    const char *name3 = "TERMINATING";
    const char *name4 = "TERMINATED";
    const char *name5 = "BLAH";
    const char *name6 = "RANDOM STATUS";
    // WHEN
    GameSessionStatus status1 = GameSessionStatusMapper::GetGameSessionStatusForName(name1);
    GameSessionStatus status2 = GameSessionStatusMapper::GetGameSessionStatusForName(name2);
    GameSessionStatus status3 = GameSessionStatusMapper::GetGameSessionStatusForName(name3);
    GameSessionStatus status4 = GameSessionStatusMapper::GetGameSessionStatusForName(name4);
    GameSessionStatus status5 = GameSessionStatusMapper::GetGameSessionStatusForName(name5);
    GameSessionStatus status6 = GameSessionStatusMapper::GetGameSessionStatusForName(name6);
    // THEN
    ASSERT_EQ(status1, GameSessionStatus::ACTIVE);
    ASSERT_EQ(status2, GameSessionStatus::ACTIVATING);
    ASSERT_EQ(status3, GameSessionStatus::TERMINATING);
    ASSERT_EQ(status4, GameSessionStatus::TERMINATED);
    ASSERT_EQ(status5, GameSessionStatus::NOT_SET);
    ASSERT_EQ(status6, GameSessionStatus::NOT_SET);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws