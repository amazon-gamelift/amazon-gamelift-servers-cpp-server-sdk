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
#include <aws/gamelift/internal/network/MockGameLiftMessageHandler.h>
#include <aws/gamelift/internal/network/callback/UpdateGameSessionCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class UpdateGameSessionCallbackTest : public ::testing::Test {
protected:
    MockGameLiftMessageHandler *mockGameLiftMessageHandler;
    UpdateGameSessionCallback *updateGameSessionCallback;
    UpdateGameSession capturedUpdateGameSession = UpdateGameSession(GameSession(), UpdateReason::UNKNOWN, "dummy");

    void SetUp() override {
        mockGameLiftMessageHandler = new MockGameLiftMessageHandler();
        updateGameSessionCallback = new UpdateGameSessionCallback(mockGameLiftMessageHandler);
    }

    void TearDown() override {
        if (mockGameLiftMessageHandler) {
            delete mockGameLiftMessageHandler;
            mockGameLiftMessageHandler = nullptr;
        }

        if (updateGameSessionCallback) {
            delete updateGameSessionCallback;
            updateGameSessionCallback = nullptr;
        }
    }

public:
    void captureUpdateGameSessionMessage(UpdateGameSession &msg) { capturedUpdateGameSession = msg; }
};

TEST_F(UpdateGameSessionCallbackTest, GIVEN_updateEmptyMessage_WHEN_onUpdateGameSession_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    EXPECT_CALL(*mockGameLiftMessageHandler, OnUpdateGameSession(testing::_))
        .WillOnce(testing::Invoke(this, &UpdateGameSessionCallbackTest::captureUpdateGameSessionMessage));

    // WHEN
    updateGameSessionCallback->OnUpdateGameSession(jsonMessage);

    // THEN
#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(capturedUpdateGameSession.GetBackfillTicketId(), "");
    EXPECT_EQ(UpdateReasonMapper::GetNameForUpdateReason(capturedUpdateGameSession.GetUpdateReason()), "UNKNOWN");
#else
    EXPECT_STREQ(capturedUpdateGameSession.GetBackfillTicketId(), "");
    EXPECT_STREQ(UpdateReasonMapper::GetNameForUpdateReason(capturedUpdateGameSession.GetUpdateReason()), "UNKNOWN");
#endif
}

TEST_F(UpdateGameSessionCallbackTest, GIVEN_updateMessage_WHEN_onUpdateGameSession_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"BackfillTicketId\":\"backfillTicketId\",\"UpdateReason\":\"MATCHMAKING_DATA_UPDATED\"}";
    EXPECT_CALL(*mockGameLiftMessageHandler, OnUpdateGameSession(testing::_))
        .WillOnce(testing::Invoke(this, &UpdateGameSessionCallbackTest::captureUpdateGameSessionMessage));

    // WHEN
    updateGameSessionCallback->OnUpdateGameSession(jsonMessage);

    // THEN
#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(capturedUpdateGameSession.GetBackfillTicketId(), "backfillTicketId");
    EXPECT_EQ(UpdateReasonMapper::GetNameForUpdateReason(capturedUpdateGameSession.GetUpdateReason()), "MATCHMAKING_DATA_UPDATED");
#else
    EXPECT_STREQ(capturedUpdateGameSession.GetBackfillTicketId(), "backfillTicketId");
    EXPECT_STREQ(UpdateReasonMapper::GetNameForUpdateReason(capturedUpdateGameSession.GetUpdateReason()), "MATCHMAKING_DATA_UPDATED");
#endif
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws