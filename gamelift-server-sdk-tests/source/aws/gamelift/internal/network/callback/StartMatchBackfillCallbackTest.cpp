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
#include <aws/gamelift/internal/model/response/WebSocketStartMatchBackfillResponse.h>
#include <aws/gamelift/internal/network/MockGameLiftMessageHandler.h>
#include <aws/gamelift/internal/network/callback/StartMatchBackfillCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class StartMatchBackfillCallbackTest : public ::testing::Test {
protected:
    StartMatchBackfillCallback *startMatchBackfillCallback;

    void SetUp() override { startMatchBackfillCallback = new StartMatchBackfillCallback(); }

    void TearDown() override {
        if (startMatchBackfillCallback) {
            delete startMatchBackfillCallback;
            startMatchBackfillCallback = nullptr;
        }
    }
};

TEST_F(StartMatchBackfillCallbackTest, GIVEN_createEmptyMessage_WHEN_onStartMatchBackfill_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    // WHEN
    GenericOutcome genericOutcome = startMatchBackfillCallback->OnStartMatchBackfill(jsonMessage);
    WebSocketStartMatchBackfillResponse *response = static_cast<WebSocketStartMatchBackfillResponse *>(genericOutcome.GetResult());

    // THEN
    EXPECT_EQ(response->GetTicketId(), "");

    delete response;
}

TEST_F(StartMatchBackfillCallbackTest, GIVEN_createMessage_WHEN_onStartMatchBackfill_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"TicketId\":\"ticketId\"}";

    // WHEN
    GenericOutcome genericOutcome = startMatchBackfillCallback->OnStartMatchBackfill(jsonMessage);
    WebSocketStartMatchBackfillResponse *response = static_cast<WebSocketStartMatchBackfillResponse *>(genericOutcome.GetResult());

    // THEN
    EXPECT_EQ(response->GetTicketId(), "ticketId");

    delete response;
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws