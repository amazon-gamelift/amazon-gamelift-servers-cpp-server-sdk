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
#include <aws/gamelift/server/model/UpdateReason.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

TEST(UpdateReasonTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN
    UpdateReason reason{};
    // THEN
    ASSERT_EQ(reason, UpdateReason::UNKNOWN);
}

TEST(UpdateReasonTest, GIVEN_reason_WHEN_getNameForUpdateReason_THEN_success) {
    // GIVEN
    UpdateReason updateReason1 = UpdateReason::MATCHMAKING_DATA_UPDATED;
    UpdateReason updateReason2 = UpdateReason::BACKFILL_FAILED;
    UpdateReason updateReason3 = UpdateReason::BACKFILL_TIMED_OUT;
    UpdateReason updateReason4 = UpdateReason::BACKFILL_CANCELLED;
    UpdateReason updateReason5 = UpdateReason::UNKNOWN;
    // WHEN
    auto name1 = UpdateReasonMapper::GetNameForUpdateReason(updateReason1);
    auto name2 = UpdateReasonMapper::GetNameForUpdateReason(updateReason2);
    auto name3 = UpdateReasonMapper::GetNameForUpdateReason(updateReason3);
    auto name4 = UpdateReasonMapper::GetNameForUpdateReason(updateReason4);
    auto name5 = UpdateReasonMapper::GetNameForUpdateReason(updateReason5);
    // THEN
    Utility::TestHelper::AssertStringsEqual(name1, "MATCHMAKING_DATA_UPDATED");
    Utility::TestHelper::AssertStringsEqual(name2, "BACKFILL_FAILED");
    Utility::TestHelper::AssertStringsEqual(name3, "BACKFILL_TIMED_OUT");
    Utility::TestHelper::AssertStringsEqual(name4, "BACKFILL_CANCELLED");
    Utility::TestHelper::AssertStringsEqual(name5, "UNKNOWN");
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST(UpdateReasonTest, GIVEN_name_WHEN_getUpdateReasonForName_std_THEN_success) {
    // GIVEN
    std::string name1 = "MATCHMAKING_DATA_UPDATED";
    std::string name2 = "BACKFILL_FAILED";
    std::string name3 = "BACKFILL_TIMED_OUT";
    std::string name4 = "BACKFILL_CANCELLED";
    std::string name5 = "BLAH";
    std::string name6 = "RANDOM REASON";
    // WHEN
    UpdateReason reason1 = UpdateReasonMapper::GetUpdateReasonForName(name1);
    UpdateReason reason2 = UpdateReasonMapper::GetUpdateReasonForName(name2);
    UpdateReason reason3 = UpdateReasonMapper::GetUpdateReasonForName(name3);
    UpdateReason reason4 = UpdateReasonMapper::GetUpdateReasonForName(name4);
    UpdateReason reason5 = UpdateReasonMapper::GetUpdateReasonForName(name5);
    UpdateReason reason6 = UpdateReasonMapper::GetUpdateReasonForName(name6);
    // THEN
    ASSERT_EQ(reason1, UpdateReason::MATCHMAKING_DATA_UPDATED);
    ASSERT_EQ(reason2, UpdateReason::BACKFILL_FAILED);
    ASSERT_EQ(reason3, UpdateReason::BACKFILL_TIMED_OUT);
    ASSERT_EQ(reason4, UpdateReason::BACKFILL_CANCELLED);
    ASSERT_EQ(reason5, UpdateReason::UNKNOWN);
    ASSERT_EQ(reason6, UpdateReason::UNKNOWN);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST(UpdateReasonTest, GIVEN_name_WHEN_getUpdateReasonForName_no_std_THEN_success) {
    // GIVEN
    const char *name1 = "MATCHMAKING_DATA_UPDATED";
    const char *name2 = "BACKFILL_FAILED";
    const char *name3 = "BACKFILL_TIMED_OUT";
    const char *name4 = "BACKFILL_CANCELLED";
    const char *name5 = "BLAH";
    const char *name6 = "RANDOM REASON";
    // WHEN
    UpdateReason reason1 = UpdateReasonMapper::GetUpdateReasonForName(name1);
    UpdateReason reason2 = UpdateReasonMapper::GetUpdateReasonForName(name2);
    UpdateReason reason3 = UpdateReasonMapper::GetUpdateReasonForName(name3);
    UpdateReason reason4 = UpdateReasonMapper::GetUpdateReasonForName(name4);
    UpdateReason reason5 = UpdateReasonMapper::GetUpdateReasonForName(name5);
    UpdateReason reason6 = UpdateReasonMapper::GetUpdateReasonForName(name6);
    // THEN
    ASSERT_EQ(reason1, UpdateReason::MATCHMAKING_DATA_UPDATED);
    ASSERT_EQ(reason2, UpdateReason::BACKFILL_FAILED);
    ASSERT_EQ(reason3, UpdateReason::BACKFILL_TIMED_OUT);
    ASSERT_EQ(reason4, UpdateReason::BACKFILL_CANCELLED);
    ASSERT_EQ(reason5, UpdateReason::UNKNOWN);
    ASSERT_EQ(reason6, UpdateReason::UNKNOWN);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws