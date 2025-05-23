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
#include <aws/gamelift/internal/network/callback/CreateGameSessionCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class CreateGameSessionCallbackTest : public ::testing::Test {
protected:
    GameSession capturedGameSession;
    MockGameLiftMessageHandler *mockGameLiftMessageHandler;

    CreateGameSessionCallback *createGameSessionCallback;

    void SetUp() override {
        mockGameLiftMessageHandler = new MockGameLiftMessageHandler();
        createGameSessionCallback = new CreateGameSessionCallback(mockGameLiftMessageHandler);
    }

    void TearDown() override {
        if (mockGameLiftMessageHandler) {
            delete mockGameLiftMessageHandler;
            mockGameLiftMessageHandler = nullptr;
        }

        if (createGameSessionCallback) {
            delete createGameSessionCallback;
            createGameSessionCallback = nullptr;
        }
    }

public:
    void captureGameSessionMessage(GameSession &msg) { capturedGameSession = msg; }
};

TEST_F(CreateGameSessionCallbackTest, GIVEN_createEmptyMessage_WHEN_onStartGameSession_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    EXPECT_CALL(*mockGameLiftMessageHandler, OnStartGameSession(testing::_))
        .WillOnce(testing::Invoke(this, &CreateGameSessionCallbackTest::captureGameSessionMessage));

    // WHEN
    createGameSessionCallback->OnStartGameSession(jsonMessage);

    // THEN
#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(capturedGameSession.GetGameSessionId(), "");
    EXPECT_EQ(capturedGameSession.GetName(), "");
#else
    EXPECT_STREQ(capturedGameSession.GetGameSessionId(), "");
    EXPECT_STREQ(capturedGameSession.GetName(), "");
#endif
}

TEST_F(CreateGameSessionCallbackTest, GIVEN_createMessage_WHEN_onStartGameSession_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"GameSessionId\":\"gameSessionId\",\"GameSessionName\":\"gameName\"}";
    EXPECT_CALL(*mockGameLiftMessageHandler, OnStartGameSession(testing::_))
        .WillOnce(testing::Invoke(this, &CreateGameSessionCallbackTest::captureGameSessionMessage));

    // WHEN
    createGameSessionCallback->OnStartGameSession(jsonMessage);

    // THEN
#ifdef GAMELIFT_USE_STD
    EXPECT_EQ(capturedGameSession.GetGameSessionId(), "gameSessionId");
    EXPECT_EQ(capturedGameSession.GetName(), "gameName");
#else
    EXPECT_STREQ(capturedGameSession.GetGameSessionId(), "gameSessionId");
    EXPECT_STREQ(capturedGameSession.GetName(), "gameName");
#endif
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws