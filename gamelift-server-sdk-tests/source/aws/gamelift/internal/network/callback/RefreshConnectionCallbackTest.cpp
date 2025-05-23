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
#include <aws/gamelift/internal/network/callback/RefreshConnectionCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class RefreshConnectionCallbackTest : public ::testing::Test {
protected:
    MockGameLiftMessageHandler *mockGameLiftMessageHandler;
    RefreshConnectionCallback *refreshConnectionCallback;
    std::string capturedRefreshConnectionEndpoint;
    std::string capturedAuthToken;

    void SetUp() override {
        mockGameLiftMessageHandler = new MockGameLiftMessageHandler();
        refreshConnectionCallback = new RefreshConnectionCallback(mockGameLiftMessageHandler);
    }

    void TearDown() override {
        if (refreshConnectionCallback) {
            delete refreshConnectionCallback;
            refreshConnectionCallback = nullptr;
        }
        if (mockGameLiftMessageHandler) {
            delete mockGameLiftMessageHandler;
            mockGameLiftMessageHandler = nullptr;
        }
    }

public:
    void captureRefreshConnectionEndpointAndAuthToken(const std::string &refreshConnectionEndpoint, const std::string &authToken) {
        capturedRefreshConnectionEndpoint = refreshConnectionEndpoint;
        capturedAuthToken = authToken;
    }
};

TEST_F(RefreshConnectionCallbackTest, GIVEN_createEmptyMessage_WHEN_onRefreshConnection_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    EXPECT_CALL(*mockGameLiftMessageHandler, OnRefreshConnection(testing::_, testing::_))
        .WillOnce(testing::Invoke(this, &RefreshConnectionCallbackTest::captureRefreshConnectionEndpointAndAuthToken));

    // WHEN
    refreshConnectionCallback->OnRefreshConnection(jsonMessage);

    // THEN
    EXPECT_THAT(capturedRefreshConnectionEndpoint, "");
    EXPECT_THAT(capturedAuthToken, "");
}

TEST_F(RefreshConnectionCallbackTest, GIVEN_createMessage_WHEN_onRefreshConnection_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"RefreshConnectionEndpoint\":\"endpoint\",\"AuthToken\":\"token\"}";

    EXPECT_CALL(*mockGameLiftMessageHandler, OnRefreshConnection(testing::_, testing::_))
        .WillOnce(testing::Invoke(this, &RefreshConnectionCallbackTest::captureRefreshConnectionEndpointAndAuthToken));

    // WHEN
    refreshConnectionCallback->OnRefreshConnection(jsonMessage);

    // THEN
    EXPECT_THAT(capturedRefreshConnectionEndpoint, "endpoint");
    EXPECT_THAT(capturedAuthToken, "token");
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws