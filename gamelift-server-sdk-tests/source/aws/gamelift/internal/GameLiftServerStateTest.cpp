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
#include <aws/gamelift/common/GameLiftErrors.h>
#include <aws/gamelift/common/Outcome.h>
#include <aws/gamelift/internal/GameLiftServerState.h>
#include <aws/gamelift/internal/model/Message.h>
#include <aws/gamelift/internal/model/request/ActivateGameSessionRequest.h>
#include <aws/gamelift/internal/model/request/ActivateServerProcessRequest.h>
#include <aws/gamelift/internal/model/request/HeartbeatServerProcessRequest.h>
#include <aws/gamelift/internal/model/request/RemovePlayerSessionRequest.h>
#include <aws/gamelift/internal/model/request/WebSocketGetFleetRoleCredentialsRequest.h>
#include <aws/gamelift/internal/model/response/WebSocketGetComputeCertificateResponse.h>
#include <aws/gamelift/internal/model/response/WebSocketGetFleetRoleCredentialsResponse.h>
#include <aws/gamelift/internal/model/response/WebSocketStartMatchBackfillResponse.h>
#include <aws/gamelift/internal/network/MockWebSocketClientWrapper.h>
#include <aws/gamelift/server/model/Player.h>
#include <aws/gamelift/server/model/ServerParameters.h>
#include <chrono>
#include <future>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class GameLiftServerStateTest : public ::testing::Test {
protected:
    const char *websocketUrl = "wss://n1myab2jn9.execute-api.us-west-2.amazonaws.com/alpha";
    const char *fleetId = "fleet-123";
    const char *hostId = "i-123";
    const char *processId = "process-123123123";
    const char *authToken = "AuthToken";
    std::string capturedSocketMessage;
    std::shared_ptr<MockWebSocketClientWrapper> mockWebSocketClientWrapper;
    GameLiftServerState *serverState;
    Aws::GameLift::Server::Model::GameSession gameSession = Aws::GameLift::Server::Model::GameSession().WithGameSessionId("gameSessionId");

    struct MessageCaptor {
        std::string received_message;

        Aws::GameLift::GenericOutcome SendSocketMessage(const std::string &requestId, const std::string &message) {
            received_message = message;
            return Aws::GameLift::GenericOutcome(nullptr);
        }
    };

    struct MessageCaptorAsync {
        std::promise<std::string> received_message;

        Aws::GameLift::GenericOutcome SendSocketMessage(const std::string &requestId, const std::string &message) {
            received_message.set_value(message);
            return Aws::GameLift::GenericOutcome(nullptr);
        }
    };

    void SetUp() override {
#ifdef GAMELIFT_USE_STD
        mockWebSocketClientWrapper = std::make_shared<::testing::NiceMock<MockWebSocketClientWrapper>>();
        Server::InitSDKOutcome initOutcome = GameLiftServerState::CreateInstance(mockWebSocketClientWrapper);
#else
        Internal::InitSDKOutcome initOutcome = GameLiftServerState::CreateInstance<::testing::NiceMock<MockWebSocketClientWrapper>>();
        if (initOutcome.IsSuccess()) {
            mockWebSocketClientWrapper = std::dynamic_pointer_cast<MockWebSocketClientWrapper>(initOutcome.GetResult()->GetWebSocketClientWrapper());
        }
#endif
        ASSERT_TRUE(initOutcome.IsSuccess());
        serverState = initOutcome.GetResult();

        Aws::GameLift::Server::Model::ServerParameters serverParameters(websocketUrl, authToken, fleetId, hostId, processId);
        GenericOutcome outcome = serverState->InitializeNetworking(serverParameters);
    }

    void TearDown() override {
        mockWebSocketClientWrapper = nullptr;

        if (serverState) {
            serverState->DestroyInstance();
            serverState = nullptr;
        }
    }

    GenericOutcome CallProcessReady() {

#ifdef GAMELIFT_USE_STD
        Aws::GameLift::Server::ProcessParameters processParams =
            Aws::GameLift::Server::ProcessParameters(nullptr, nullptr, nullptr, nullptr, 1001, Aws::GameLift::Server::LogParameters());
#else
        Aws::GameLift::Server::ProcessParameters processParams = Aws::GameLift::Server::ProcessParameters(
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 1001, Aws::GameLift::Server::LogParameters());
#endif

        return serverState->ProcessReady(processParams);
    }

public:
    Aws::GameLift::GenericOutcome captureSocketMessage(const std::string &requestId, const std::string &msg) {
        capturedSocketMessage = msg;
        return Aws::GameLift::GenericOutcome(nullptr);
    }
};

MATCHER_P(HasAction, action, "") {
    Message message;
    message.Deserialize(arg);
    return message.GetAction() == action;
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedWebSocketClient_WHEN_processReady_THEN_success) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));

    // WHEN
    GenericOutcome outcome = CallProcessReady();

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_nonConnectedWebSocketClient_WHEN_processReady_THEN_notInitializedError) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(false));

    // WHEN
    GenericOutcome outcome = CallProcessReady();

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    GameLiftError expected = GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED);
    EXPECT_EQ(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED), outcome.GetError());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedWebSocketClient_WHEN_acceptPlayerSession_THEN_success) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("AcceptPlayerSession")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    std::string playerSessionId = "playerSessionId";

    // WHEN
    CallProcessReady();
    serverState->OnStartGameSession(gameSession);
    GenericOutcome outcome = serverState->AcceptPlayerSession(playerSessionId);

    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoSession_WHEN_acceptPlayerSession_THEN_fail) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("AcceptPlayerSession"))).Times(0);
    std::string playerSessionId = "playerSessionId";

    // WHEN
    CallProcessReady();
    GenericOutcome outcome = serverState->AcceptPlayerSession(playerSessionId);

    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET), outcome.GetError());
}

TEST_F(GameLiftServerStateTest, GIVEN_processNotReady_WHEN_acceptPlayerSession_THEN_fail) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("AcceptPlayerSession"))).Times(0);
    std::string playerSessionId = "playerSessionId";

    // WHEN
    GenericOutcome outcome = serverState->AcceptPlayerSession(playerSessionId);

    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED), outcome.GetError());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedWebSocketClient_WHEN_updatePlayerSessionCreationPolicy_THEN_success) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("UpdatePlayerSessionCreationPolicy")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));

    // WHEN
    CallProcessReady();
    serverState->OnStartGameSession(gameSession);
    GenericOutcome outcome = serverState->UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy::ACCEPT_ALL);

    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoSession_WHEN_updatePlayerSessionCreationPolicy_THEN_fail) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("UpdatePlayerSessionCreationPolicy"))).Times(0);

    // WHEN
    CallProcessReady();
    GenericOutcome outcome = serverState->UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy::ACCEPT_ALL);

    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET), outcome.GetError());
}

TEST_F(GameLiftServerStateTest, GIVEN_processNotReady_WHEN_updatePlayerSessionCreationPolicy_THEN_fail) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("UpdatePlayerSessionCreationPolicy"))).Times(0);

    // WHEN
    GenericOutcome outcome = serverState->UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy::ACCEPT_ALL);

    // THEN
    ASSERT_FALSE(outcome.IsSuccess());
    EXPECT_EQ(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED), outcome.GetError());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_ActivateGameSession_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateGameSession")))
        .WillOnce(testing::Invoke(this, &GameLiftServerStateTest::captureSocketMessage));

    // WHEN
    CallProcessReady();
    GenericOutcome outcome = serverState->ActivateGameSession();

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());

    ActivateGameSessionRequest activateGameSessionRequest;
    Message &message = activateGameSessionRequest;
    message.Deserialize(capturedSocketMessage);

    // GameSession ID is empty unless we call OnStartGameSession which would then require us to
    // setup m_onStartGameSession callback
    EXPECT_EQ(activateGameSessionRequest.GetGameSessionId(), "");
}

TEST_F(GameLiftServerStateTest, GIVEN_noProcessReady_WHEN_ActivateGameSession_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateGameSession"))).Times(0);
    // No Process Ready
    // WHEN
    GenericOutcome outcome = serverState->ActivateGameSession();

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoLongerConnected_WHEN_ActivateGameSession_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));

    // WHEN
    CallProcessReady();
    GenericOutcome outcome = serverState->ActivateGameSession();

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_validState_WHEN_processReady_THEN_registerAndHealthMessagesSent) {
    // GIVEN
    MessageCaptor registerHost;
    MessageCaptorAsync processHealth;

    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        // First message to register with websocket
        .WillOnce(testing::Invoke(&registerHost, &MessageCaptor::SendSocketMessage));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        // Second message for first process health message
        .WillOnce(testing::Invoke(&processHealth, &MessageCaptorAsync::SendSocketMessage));

    // WHEN
    GenericOutcome outcome = CallProcessReady();

    // THEN
    rapidjson::Document registerHostJson;
    registerHostJson.Parse(registerHost.received_message.c_str());

    rapidjson::Document processHealthJson;
    std::future<std::string> processHealthFuture = processHealth.received_message.get_future();

    EXPECT_TRUE(outcome.IsSuccess());
    // Health reporting spins a thread up. Giving time to wait for result. Typically a few ms.
    ASSERT_EQ(std::future_status::ready, processHealthFuture.wait_for(std::chrono::milliseconds(200)));
    processHealthJson.Parse(processHealthFuture.get().c_str());

    // Validate that proper actions for the process id were sent. Cast to string to ensure proper
    // comparison is used.
    EXPECT_EQ("ActivateServerProcess", (std::string)registerHostJson["Action"].GetString());
    EXPECT_EQ("HeartbeatServerProcess", (std::string)processHealthJson["Action"].GetString());
}

TEST_F(GameLiftServerStateTest, GIVEN_wait71Seconds_WHEN_processReady_THEN_reportsHealthTwice) {
    // GIVEN
    MessageCaptorAsync processHealth1;
    MessageCaptorAsync processHealth2;

    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        // First message to register with websocket
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillOnce(testing::Invoke(&processHealth1, &MessageCaptorAsync::SendSocketMessage))
        .WillOnce(testing::Invoke(&processHealth2, &MessageCaptorAsync::SendSocketMessage));

    // WHEN
    GenericOutcome outcome = CallProcessReady();

    // THEN
    rapidjson::Document processHealth1Json;
    std::future<std::string> processHealth1Future = processHealth1.received_message.get_future();
    EXPECT_TRUE(outcome.IsSuccess());
    // Health reporting spins a thread up. Giving time to wait for result. Typically a few ms.
    ASSERT_EQ(std::future_status::ready, processHealth1Future.wait_for(std::chrono::milliseconds(200)));
    processHealth1Json.Parse(processHealth1Future.get().c_str());
    EXPECT_EQ("HeartbeatServerProcess", (std::string)processHealth1Json["Action"].GetString());

    rapidjson::Document processHealth2Json;
    std::future<std::string> processHealth2Future = processHealth2.received_message.get_future();
    EXPECT_TRUE(outcome.IsSuccess());
    // Second health check call should occur within 71MS after the first call.
    ASSERT_EQ(std::future_status::ready, processHealth2Future.wait_for(std::chrono::milliseconds(71000)));
    processHealth2Json.Parse(processHealth2Future.get().c_str());
    EXPECT_EQ("HeartbeatServerProcess", (std::string)processHealth2Json["Action"].GetString());
}

TEST_F(GameLiftServerStateTest, GIVEN_validStopMatchBackfillRequest_WHEN_callingStopMatchBackfill_THEN_returnSuccessOutcome) {
    // GIVEN
    MessageCaptor stopMatchBackfill;
    const char *ticketId = "testId";
    const char *gameSessionArn = "testGameSession";
    const char *matchmakingConfigurationArn = "testConfig";
    Aws::GameLift::Server::Model::StopMatchBackfillRequest stopRequest = Aws::GameLift::Server::Model::StopMatchBackfillRequest()
                                                                             .WithGameSessionArn(gameSessionArn)
                                                                             .WithMatchmakingConfigurationArn(matchmakingConfigurationArn)
                                                                             .WithTicketId(ticketId);

    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("StopMatchBackfill")))
        .WillOnce(testing::Invoke(&stopMatchBackfill, &MessageCaptor::SendSocketMessage));
    // WHEN
    Aws::GameLift::GenericOutcome outcome = serverState->StopMatchBackfill(stopRequest);
    // THEN
    rapidjson::Document stopMatchBackfillJson;
    stopMatchBackfillJson.Parse(stopMatchBackfill.received_message.c_str());

    EXPECT_EQ(outcome.IsSuccess(), true);
    EXPECT_EQ(ticketId, (std::string)stopMatchBackfillJson["TicketId"].GetString());
    EXPECT_EQ(gameSessionArn, (std::string)stopMatchBackfillJson["GameSessionArn"].GetString());
    EXPECT_EQ(matchmakingConfigurationArn, (std::string)stopMatchBackfillJson["MatchmakingConfigurationArn"].GetString());
    EXPECT_EQ("StopMatchBackfill", (std::string)stopMatchBackfillJson["Action"].GetString());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_RemovePlayerSession_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("RemovePlayerSession")))
        .WillOnce(testing::Invoke(this, &GameLiftServerStateTest::captureSocketMessage));

    // WHEN
    CallProcessReady();
    serverState->OnStartGameSession(gameSession);
    GenericOutcome outcome = serverState->RemovePlayerSession("testPlayerId");

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());

    RemovePlayerSessionRequest removePlayerSessionRequest;
    Message &message = removePlayerSessionRequest;
    message.Deserialize(capturedSocketMessage);

    EXPECT_EQ(removePlayerSessionRequest.GetGameSessionId(), "gameSessionId");
}

TEST_F(GameLiftServerStateTest, GIVEN_noProcessReady_WHEN_RemovePlayerSession_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("RemovePlayerSession"))).Times(0);
    // No Process Ready

    // WHEN
    GenericOutcome outcome = serverState->RemovePlayerSession("testPlayerId");

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoLongerConnected_WHEN_RemovePlayerSession_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("RemovePlayerSession"))).Times(0);

    // WHEN
    CallProcessReady();
    GenericOutcome outcome = serverState->RemovePlayerSession("testPlayerId");

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_GetComputeCertificate_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    WebSocketGetComputeCertificateResponse *response = new WebSocketGetComputeCertificateResponse();
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("GetComputeCertificate")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    // WHEN
    CallProcessReady();
    GetComputeCertificateOutcome outcome = serverState->GetComputeCertificate();

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoLongerConnected_WHEN_GetComputeCertificate_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("GetComputeCertificate"))).Times(0);

    // WHEN
    CallProcessReady();
    GetComputeCertificateOutcome outcome = serverState->GetComputeCertificate();

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

// Matcher for GetFleetRoleCredentials() tests
MATCHER_P(HasRoleSessionName, expectedName, "") {
    std::unique_ptr<WebSocketGetFleetRoleCredentialsRequest> request(new WebSocketGetFleetRoleCredentialsRequest());
    Message *message = request.get();
    message->Deserialize(arg);
    return request->GetRoleSessionName() == expectedName;
}

MATCHER_P2(HasUrlAndAuthToken, expectedUrl, expectedAuthToken, "") {
    auto params = arg.GetQueryMap();
    auto authToken = params.find("Authorization");
    return arg.GetBaseUriString() == expectedUrl && authToken != params.end() && authToken->second == expectedAuthToken;
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_GetFleetRoleCredentials_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    auto response = new WebSocketGetFleetRoleCredentialsResponse();
    response->SetAccessKeyId("AccessKeyId");
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasRoleSessionName("fleet-123-i-123")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_StartMatchBackfill_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response = new WebSocketStartMatchBackfillResponse();
    response->SetTicketId("TicketId");
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("StartMatchBackfill")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    // Don't allocate StartMatchBackfillRequests on the stack, it will overflow on Windows
    std::unique_ptr<Aws::GameLift::Server::Model::StartMatchBackfillRequest> startMatchBackfillRequest(
        new Aws::GameLift::Server::Model::StartMatchBackfillRequest());
    startMatchBackfillRequest->SetTicketId("TicketId");
    startMatchBackfillRequest->SetMatchmakingConfigurationArn("MatchmakingConfigurationArn");
    startMatchBackfillRequest->SetGameSessionArn("GameSessionArn");
    Player player;
    startMatchBackfillRequest->AddPlayer(player);

    // WHEN
    CallProcessReady();
    StartMatchBackfillOutcome outcome = serverState->StartMatchBackfill(*startMatchBackfillRequest);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_noProcessReady_WHEN_StartMatchBackfill_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("StartMatchBackfill"))).Times(0);
    // Don't allocate StartMatchBackfillRequests on the stack, it will overflow on Windows
    std::unique_ptr<Aws::GameLift::Server::Model::StartMatchBackfillRequest> startMatchBackfillRequest(
        new Aws::GameLift::Server::Model::StartMatchBackfillRequest());

    // No Process Ready
    // WHEN
    StartMatchBackfillOutcome outcome = serverState->StartMatchBackfill(*startMatchBackfillRequest);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoLongerConnected_WHEN_StartMatchBackfill_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("StartMatchBackfill"))).Times(0);

    // Don't allocate StartMatchBackfillRequests on the stack, it will overflow on Windows
    std::unique_ptr<Aws::GameLift::Server::Model::StartMatchBackfillRequest> startMatchBackfillRequest(
        new Aws::GameLift::Server::Model::StartMatchBackfillRequest());

    // WHEN
    CallProcessReady();
    StartMatchBackfillOutcome outcome = serverState->StartMatchBackfill(*startMatchBackfillRequest);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_processReadyButNoLongerConnected_WHEN_GetFleetRoleCredentials_THEN_outcomeFailed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("GetFleetRoleCredentials")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome = serverState->GetFleetRoleCredentials(Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest());

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_readyWithRoleSessionName_WHEN_GetFleetRoleCredentials_THEN_messageSent) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response = new WebSocketGetFleetRoleCredentialsResponse();
    response->SetAccessKeyId("AccessKeyId");
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasRoleSessionName("customRoleSessionName")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");
    request.SetRoleSessionName("customRoleSessionName");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_readyWithLongRoleSessionName_WHEN_GetFleetRoleCredentials_THEN_returnsFailureOutcome) {
    // GIVEN
    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");
    request.SetRoleSessionName("AVeryLongRoleSessionNameThatWouldIamWouldntAssumeSinceItsOverSixtyFourCharacters");

    // WHEN
    GetFleetRoleCredentialsOutcome outcome = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_readyWithLongComputeName_WHEN_GetFleetRoleCredentials_THEN_messageSent) {
    // GIVEN
    // Kind of hacky, re-initialize the networking with a new hostId for this test
    mockWebSocketClientWrapper = nullptr;
    serverState->DestroyInstance();
    serverState = nullptr;
#ifdef GAMELIFT_USE_STD
    mockWebSocketClientWrapper = std::make_shared<::testing::NiceMock<MockWebSocketClientWrapper>>();
    Server::InitSDKOutcome initOutcome = GameLiftServerState::CreateInstance(mockWebSocketClientWrapper);
#else
    Internal::InitSDKOutcome initOutcome = GameLiftServerState::CreateInstance<::testing::NiceMock<MockWebSocketClientWrapper>>();
    if (initOutcome.IsSuccess()) {
        mockWebSocketClientWrapper = std::dynamic_pointer_cast<MockWebSocketClientWrapper>(initOutcome.GetResult()->GetWebSocketClientWrapper());
    }
#endif
    ASSERT_TRUE(initOutcome.IsSuccess());
    serverState = initOutcome.GetResult();

    Aws::GameLift::Server::Model::ServerParameters serverParameters(
        websocketUrl, authToken, "AVeryLongRoleSessionNameThatWouldIamWouldntAssumeSinceItsOverSixtyFourCharacters", hostId, processId);
    serverState->InitializeNetworking(serverParameters);

    // Initialize the test
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response = new WebSocketGetFleetRoleCredentialsResponse();
    response->SetAccessKeyId("AccessKeyId");
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper,
                SendSocketMessage(testing::_, HasRoleSessionName("AVeryLongRoleSessionNameThatWouldIamWouldntAssumeSinceItsOverSix")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_validOnPremRequest_WHEN_GetFleetRoleCredentials_THEN_returnsCachedErrorOutcome) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response = new WebSocketGetFleetRoleCredentialsResponse();
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasRoleSessionName("fleet-123-i-123")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome1 = serverState->GetFleetRoleCredentials(request);
    GetFleetRoleCredentialsOutcome outcome2 = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_FALSE(outcome1.IsSuccess());
    EXPECT_FALSE(outcome2.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_readyWithMultipleRequests_WHEN_GetFleetRoleCredentials_THEN_cachesAndReturnsSuccess) {
    const int64_t expirationInMillis = (time(nullptr) + GameLiftServerState::INSTANCE_ROLE_CREDENTIAL_TTL_MIN + 60) * 1000;

    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response = new WebSocketGetFleetRoleCredentialsResponse();
    response->SetAccessKeyId("AccessKeyId");
    response->SetExpiration(expirationInMillis);
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("GetFleetRoleCredentials")))
        .WillOnce(testing::Return(GenericOutcome(response)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome1 = serverState->GetFleetRoleCredentials(request);
    GetFleetRoleCredentialsOutcome outcome2 = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_TRUE(outcome1.IsSuccess());
    EXPECT_TRUE(outcome2.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_validRequestsAfterTimeout_WHEN_GetFleetRoleCredentials_THEN_doesNotCacheAndReturnsSuccessOutcomes) {
    const int64_t expirationInMillis = time(nullptr) * 1000;
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    auto response1 = new WebSocketGetFleetRoleCredentialsResponse();
    response1->SetAccessKeyId("AccessKeyId");
    response1->SetExpiration(expirationInMillis);
    auto response2 = new WebSocketGetFleetRoleCredentialsResponse();
    response2->SetAccessKeyId("AccessKeyId");
    response2->SetExpiration(expirationInMillis);
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("GetFleetRoleCredentials")))
        .WillOnce(testing::Return(GenericOutcome(response1)))
        .WillOnce(testing::Return(GenericOutcome(response2)));

    Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest request;
    request.SetRoleArn("roleArn");

    // WHEN
    CallProcessReady();
    GetFleetRoleCredentialsOutcome outcome1 = serverState->GetFleetRoleCredentials(request);
    GetFleetRoleCredentialsOutcome outcome2 = serverState->GetFleetRoleCredentials(request);

    // THEN
    EXPECT_TRUE(outcome1.IsSuccess());
    EXPECT_TRUE(outcome2.IsSuccess());
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReady_WHEN_OnRefreshConnection_THEN_connectionRefreshed) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(HasUrlAndAuthToken("newEndpoint/", "newToken"))).WillOnce(testing::Return(GenericOutcome()));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));

    // WHEN
    CallProcessReady();
    serverState->OnRefreshConnection("newEndpoint", "newToken");

    // THEN - No errors
}

TEST_F(GameLiftServerStateTest, GIVEN_processNotReady_WHEN_OnRefreshConnection_THEN_noOp) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(HasUrlAndAuthToken("newEndpoint/", "newToken"))).Times(0);

    // WHEN
    serverState->OnRefreshConnection("newEndpoint", "newToken");

    // THEN - No errors
}

TEST_F(GameLiftServerStateTest, GIVEN_connectedProcessAndReadyButRefreshFails_WHEN_OnRefreshConnection_THEN_noOp) {
    // GIVEN
    EXPECT_CALL(*mockWebSocketClientWrapper, IsConnected()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(HasUrlAndAuthToken("newEndpoint/", "newToken")))
        .WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("ActivateServerProcess")))
        .WillOnce(testing::Return(GenericOutcome(nullptr)));
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(testing::_, HasAction("HeartbeatServerProcess")))
        .WillRepeatedly(testing::Return(GenericOutcome(nullptr)));

    // WHEN
    CallProcessReady();
    serverState->OnRefreshConnection("newEndpoint", "newToken");

    // THEN - No errors
}

TEST_F(GameLiftServerStateTest, GIVEN_retriable_failure_WHEN_sendMessage_THEN_retry) {
    // GIVEN
    Message message;
    std::stringstream ss;
    ss << "{\"RequestId\":\"" << message.GetRequestId() << "\"}";
    std::string expectedJsonMessage = ss.str();
    // EXPECT
    EXPECT_CALL(*mockWebSocketClientWrapper, SendSocketMessage(message.GetRequestId(), expectedJsonMessage))
    .WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE)))
    .WillOnce(testing::Return(GenericOutcome(nullptr)));
    // WHEN
    GenericOutcome outcome = serverState->SendSocketMessageWithRetries(message);
    // THEN
    ASSERT_TRUE(outcome.IsSuccess());
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws