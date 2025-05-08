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
#include <aws/gamelift/internal/network/GameLiftWebSocketClientManager.h>
#include <aws/gamelift/internal/network/MockWebSocketClientWrapper.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

static const std::string sdkVersion = "5.2.0";

class GameLiftWebSocketClientManagerTest : public ::testing::Test {
protected:
    // private constants copied over for testing purposes
    static constexpr const char *PID_KEY = "pID";
    static constexpr const char *SDK_VERSION_KEY = "sdkVersion";
    static constexpr const char *FLAVOR_KEY = "sdkLanguage";
    static constexpr const char *AUTH_TOKEN_KEY = "Authorization";
    static constexpr const char *COMPUTE_ID_KEY = "ComputeId";
    static constexpr const char *FLEET_ID_KEY = "FleetId";

    std::shared_ptr<MockWebSocketClientWrapper> mockWebSocketClientWrapper;

    GameLiftWebSocketClientManager *clientManager;

    void SetUp() override {
        mockWebSocketClientWrapper = std::make_shared<MockWebSocketClientWrapper>();
        clientManager = new GameLiftWebSocketClientManager(mockWebSocketClientWrapper);
    }

    void TearDown() override {
        if (clientManager) {
            delete clientManager;
            clientManager = nullptr;
        }
    }
};

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_isConnected_THEN_success) {
    // GIVEN / EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true));
    // WHEN
    bool isConnected = clientManager->IsConnected();
    // THEN
    ASSERT_TRUE(isConnected);
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connect_THEN_success) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(nullptr)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_sendMessage_THEN_success) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";
    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    Message message;
    std::stringstream ss;
    ss << "{\"RequestId\":\"" << message.GetRequestId() << "\"}";
    std::string expectedJsonMessage = ss.str();
    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(message.GetRequestId(), expectedJsonMessage)).WillOnce(testing::Return(GenericOutcome(nullptr)));
    // WHEN
    // connect first to set connection variables
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    outcome = clientManager->SendSocketMessage(message);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_non_retriable_failure_WHEN_sendMessage_THEN_fail) {
    // GIVEN
    Message message;
    std::stringstream ss;
    ss << "{\"RequestId\":\"" << message.GetRequestId() << "\"}";
    std::string expectedJsonMessage = ss.str();
    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(message.GetRequestId(), expectedJsonMessage))
        .WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_SEND_MESSAGE_FAILURE)));
    // WHEN
    GenericOutcome outcome = clientManager->SendSocketMessage(message);
    // THEN
    ASSERT_TRUE(!outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_retriable_failure_WHEN_max_retries_send_message_THEN_fail) {
    // GIVEN
    Message message;
    std::stringstream ss;
    ss << "{\"RequestId\":\"" << message.GetRequestId() << "\"}";
    std::string expectedJsonMessage = ss.str();
    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(message.GetRequestId(), expectedJsonMessage))
        .WillRepeatedly(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE)));
    // WHEN
    GenericOutcome outcome = clientManager->SendSocketMessage(message);
    // THEN
    ASSERT_TRUE(!outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connectFails_THEN_fail) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connectForbidden_THEN_fail) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_FORBIDDEN)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connectInvalidURL_THEN_fail) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri))
        .WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_INVALID_URL)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connectTimeout_THEN_fail) {
    // GIVEN
    std::string testUrl = "testUrl";
    std::string testAuthToken = "testAuthToken";
    std::string testProcessId = "testProcessId";
    std::string testHostId = "testHostId";
    std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl + "/")
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE_TIMEOUT)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_disconnect_THEN_success) {
    // GIVEN
    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Disconnect());
    // WHEN / THEN
    clientManager->Disconnect();
}

TEST_F(GameLiftWebSocketClientManagerTest, GIVEN_clientManager_WHEN_connectUrlWithSlash_THEN_success) {
    // GIVEN
    const std::string testUrl = "my/testUrl/";
    const std::string testAuthToken = "testAuthToken";
    const std::string testProcessId = "testProcessId";
    const std::string testHostId = "testHostId";
    const std::string testFleetId = "testFleetId";

    Uri uri = Uri::UriBuilder()
                  .WithBaseUri(testUrl)
                  .AddQueryParam(PID_KEY, testProcessId)
                  .AddQueryParam(SDK_VERSION_KEY, sdkVersion)
                  .AddQueryParam(FLAVOR_KEY, "Cpp")
                  .AddQueryParam(AUTH_TOKEN_KEY, testAuthToken)
                  .AddQueryParam(COMPUTE_ID_KEY, testHostId)
                  .AddQueryParam(FLEET_ID_KEY, testFleetId)
                  .Build();

    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(uri)).WillOnce(testing::Return(GenericOutcome(nullptr)));
    // WHEN
    GenericOutcome outcome = clientManager->Connect(testUrl, testAuthToken, testProcessId, testHostId, testFleetId);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
