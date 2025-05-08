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
#include <aws/gamelift/server/model/PlayerSessionStatus.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

TEST(PlayerSessionStatusTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN
    PlayerSessionStatus status{};
    // THEN
    ASSERT_EQ(status, PlayerSessionStatus::NOT_SET);
}

TEST(PlayerSessionStatusTest, GIVEN_policy_WHEN_getNameForPlayerSessionCreationPolicy_THEN_success) {
    // GIVEN
    PlayerSessionStatus status1 = PlayerSessionStatus::RESERVED;
    PlayerSessionStatus status2 = PlayerSessionStatus::ACTIVE;
    PlayerSessionStatus status3 = PlayerSessionStatus::COMPLETED;
    PlayerSessionStatus status4 = PlayerSessionStatus::TIMEDOUT;
    PlayerSessionStatus status5 = PlayerSessionStatus::NOT_SET;
    // WHEN
    auto status1Name = PlayerSessionStatusMapper::GetNameForPlayerSessionStatus(status1);
    auto status2Name = PlayerSessionStatusMapper::GetNameForPlayerSessionStatus(status2);
    auto status3Name = PlayerSessionStatusMapper::GetNameForPlayerSessionStatus(status3);
    auto status4Name = PlayerSessionStatusMapper::GetNameForPlayerSessionStatus(status4);
    auto status5Name = PlayerSessionStatusMapper::GetNameForPlayerSessionStatus(status5);
    // THEN
    Utility::TestHelper::AssertStringsEqual(status1Name, "RESERVED");
    Utility::TestHelper::AssertStringsEqual(status2Name, "ACTIVE");
    Utility::TestHelper::AssertStringsEqual(status3Name, "COMPLETED");
    Utility::TestHelper::AssertStringsEqual(status4Name, "TIMEDOUT");
    Utility::TestHelper::AssertStringsEqual(status5Name, "NOT_SET");
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST(PlayerSessionStatusTest, GIVEN_name_WHEN_getPlayerSessionStatusForName_std_THEN_success) {
    // GIVEN
    std::string name1 = "RESERVED";
    std::string name2 = "ACTIVE";
    std::string name3 = "COMPLETED";
    std::string name4 = "TIMEDOUT";
    std::string name5 = "BLAH";
    std::string name6 = "RANDOM STATUS";
    // WHEN
    PlayerSessionStatus status1 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name1);
    PlayerSessionStatus status2 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name2);
    PlayerSessionStatus status3 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name3);
    PlayerSessionStatus status4 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name4);
    PlayerSessionStatus status5 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name5);
    PlayerSessionStatus status6 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name6);
    // THEN
    ASSERT_EQ(status1, PlayerSessionStatus::RESERVED);
    ASSERT_EQ(status2, PlayerSessionStatus::ACTIVE);
    ASSERT_EQ(status3, PlayerSessionStatus::COMPLETED);
    ASSERT_EQ(status4, PlayerSessionStatus::TIMEDOUT);
    ASSERT_EQ(status5, PlayerSessionStatus::NOT_SET);
    ASSERT_EQ(status6, PlayerSessionStatus::NOT_SET);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST(PlayerSessionStatusTest, GIVEN_name_WHEN_getPlayerSessionStatusForName_no_std_THEN_success) {
    // GIVEN
    const char *name1 = "RESERVED";
    const char *name2 = "ACTIVE";
    const char *name3 = "COMPLETED";
    const char *name4 = "TIMEDOUT";
    const char *name5 = "NOT_SET";
    const char *name6 = "RANDOM REASON IDK";
    // WHEN
    PlayerSessionStatus status1 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name1);
    PlayerSessionStatus status2 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name2);
    PlayerSessionStatus status3 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name3);
    PlayerSessionStatus status4 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name4);
    PlayerSessionStatus status5 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name5);
    PlayerSessionStatus status6 = PlayerSessionStatusMapper::GetPlayerSessionStatusForName(name6);
    // THEN
    ASSERT_EQ(status1, PlayerSessionStatus::RESERVED);
    ASSERT_EQ(status2, PlayerSessionStatus::ACTIVE);
    ASSERT_EQ(status3, PlayerSessionStatus::COMPLETED);
    ASSERT_EQ(status4, PlayerSessionStatus::TIMEDOUT);
    ASSERT_EQ(status5, PlayerSessionStatus::NOT_SET);
    ASSERT_EQ(status6, PlayerSessionStatus::NOT_SET);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws