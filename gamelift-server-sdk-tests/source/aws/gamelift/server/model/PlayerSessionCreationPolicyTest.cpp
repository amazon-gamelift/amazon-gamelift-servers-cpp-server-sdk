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
#include <aws/gamelift/server/model/PlayerSessionCreationPolicy.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

TEST(PlayerSessionCreationPolicyTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN
    PlayerSessionCreationPolicy policy{};
    // THEN
    ASSERT_EQ(policy, PlayerSessionCreationPolicy::NOT_SET);
}

TEST(PlayerSessionCreationPolicyTest, GIVEN_status_WHEN_getNameForPlayerSessionStatus_THEN_success) {
    // GIVEN
    PlayerSessionCreationPolicy policy1 = PlayerSessionCreationPolicy::ACCEPT_ALL;
    PlayerSessionCreationPolicy policy2 = PlayerSessionCreationPolicy::DENY_ALL;
    PlayerSessionCreationPolicy policy3 = PlayerSessionCreationPolicy::NOT_SET;
    // WHEN
    auto policy1Name = PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(policy1);
    auto policy2Name = PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(policy2);
    auto policy3Name = PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(policy3);
    // THEN
    Utility::TestHelper::AssertStringsEqual(policy1Name, "ACCEPT_ALL");
    Utility::TestHelper::AssertStringsEqual(policy2Name, "DENY_ALL");
    Utility::TestHelper::AssertStringsEqual(policy3Name, "NOT_SET");
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST(PlayerSessionCreationPolicyTest, GIVEN_name_WHEN_getPlayerSessionCreationPolicyForName_THEN_success) {
    // GIVEN
    std::string name1 = "ACCEPT_ALL";
    std::string name2 = "DENY_ALL";
    std::string name3 = "BLAH";
    std::string name4 = "RANDOM REASON IDK";
    // WHEN
    PlayerSessionCreationPolicy creationPolicy1 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name1);
    PlayerSessionCreationPolicy creationPolicy2 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name2);
    PlayerSessionCreationPolicy creationPolicy3 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name3);
    PlayerSessionCreationPolicy creationPolicy4 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name4);
    // THEN
    ASSERT_EQ(creationPolicy1, PlayerSessionCreationPolicy::ACCEPT_ALL);
    ASSERT_EQ(creationPolicy2, PlayerSessionCreationPolicy::DENY_ALL);
    ASSERT_EQ(creationPolicy3, PlayerSessionCreationPolicy::NOT_SET);
    ASSERT_EQ(creationPolicy4, PlayerSessionCreationPolicy::NOT_SET);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST(PlayerSessionCreationPolicyTest, GIVEN_name_WHEN_getGameSessionStatusForName_THEN_success) {
    // GIVEN
    const char *name1 = "ACCEPT_ALL";
    const char *name2 = "DENY_ALL";
    const char *name3 = "BLAH";
    const char *name4 = "RANDOM POLICY";
    // WHEN
    PlayerSessionCreationPolicy creationPolicy1 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name1);
    PlayerSessionCreationPolicy creationPolicy2 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name2);
    PlayerSessionCreationPolicy creationPolicy3 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name3);
    PlayerSessionCreationPolicy creationPolicy4 = PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(name4);
    // THEN
    ASSERT_EQ(creationPolicy1, PlayerSessionCreationPolicy::ACCEPT_ALL);
    ASSERT_EQ(creationPolicy2, PlayerSessionCreationPolicy::DENY_ALL);
    ASSERT_EQ(creationPolicy3, PlayerSessionCreationPolicy::NOT_SET);
    ASSERT_EQ(creationPolicy4, PlayerSessionCreationPolicy::NOT_SET);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws