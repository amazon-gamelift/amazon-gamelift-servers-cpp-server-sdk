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
#include <aws/gamelift/internal/model/Uri.h>
#include <aws/gamelift/internal/model/request/ActivateGameSessionRequest.h>
#include <aws/gamelift/internal/network/MockWebSocketClientWrapper.h>
#include <aws/gamelift/server/model/ServerParameters.h>

using namespace Aws::GameLift;
using ::testing::Return;

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

static constexpr const char *ENV_VAR_WEBSOCKET_URL = "GAMELIFT_SDK_WEBSOCKET_URL";
static constexpr const char *ENV_VAR_AUTH_TOKEN = "GAMELIFT_SDK_AUTH_TOKEN";
static constexpr const char *ENV_VAR_PROCESS_ID = "GAMELIFT_SDK_PROCESS_ID";
static constexpr const char *ENV_VAR_HOST_ID = "GAMELIFT_SDK_HOST_ID";
static constexpr const char *ENV_VAR_FLEET_ID = "GAMELIFT_SDK_FLEET_ID";

static constexpr const char *ENV_VAR_WEBSOCKET_URL_VAL = "__url";
static constexpr const char *ENV_VAR_AUTH_TOKEN_VAL = "__authToken";
static constexpr const char *ENV_VAR_PROCESS_ID_VAL = "__processId";
static constexpr const char *ENV_VAR_HOST_ID_VAL = "__hostId";
static constexpr const char *ENV_VAR_FLEET_ID_VAL = "__fleetId";

static constexpr const char *URI_PID_KEY = "pID";
static constexpr const char *URI_SDK_VERSION_KEY = "sdkVersion";
static constexpr const char *URI_FLAVOR_KEY = "sdkLanguage";
static constexpr const char *URI_AUTH_TOKEN_KEY = "Authorization";
static constexpr const char *URI_COMPUTE_ID_KEY = "ComputeId";
static constexpr const char *URI_FLEET_ID_KEY = "FleetId";

class GameLiftServerStateInitTest : public ::testing::Test {
public:
    std::shared_ptr<MockWebSocketClientWrapper> mockWebSocketClientWrapper;
    GameLiftServerState *serverState = nullptr;
    std::string capturedUriString;
    std::map<const char *, char *> envVarsSnapShot;

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

        envVarsSnapShot.clear();
        envVarsSnapShot[ENV_VAR_WEBSOCKET_URL] = GetEnv(ENV_VAR_WEBSOCKET_URL);
        envVarsSnapShot[ENV_VAR_AUTH_TOKEN] = GetEnv(ENV_VAR_AUTH_TOKEN);
        envVarsSnapShot[ENV_VAR_PROCESS_ID] = GetEnv(ENV_VAR_PROCESS_ID);
        envVarsSnapShot[ENV_VAR_HOST_ID] = GetEnv(ENV_VAR_HOST_ID);
        envVarsSnapShot[ENV_VAR_FLEET_ID] = GetEnv(ENV_VAR_FLEET_ID);
    }

    void TearDown() override {
        SetEnv(ENV_VAR_WEBSOCKET_URL, envVarsSnapShot[ENV_VAR_WEBSOCKET_URL]);
        SetEnv(ENV_VAR_AUTH_TOKEN, envVarsSnapShot[ENV_VAR_AUTH_TOKEN]);
        SetEnv(ENV_VAR_PROCESS_ID, envVarsSnapShot[ENV_VAR_PROCESS_ID]);
        SetEnv(ENV_VAR_HOST_ID, envVarsSnapShot[ENV_VAR_HOST_ID]);
        SetEnv(ENV_VAR_FLEET_ID, envVarsSnapShot[ENV_VAR_FLEET_ID]);
        envVarsSnapShot.clear();

        if (serverState) {
            serverState->DestroyInstance();
            serverState = nullptr;
        }

        mockWebSocketClientWrapper = nullptr;
    }

    GenericOutcome CaptureUri(const Uri &uri) {
        capturedUriString = uri.GetUriString();
        return GenericOutcome(nullptr);
    }

    char *GetEnv(const char *key) { return std::getenv(key); }

    void SetEnv(const char *key, const char *val) {
        if (val == nullptr) {
#ifdef WIN32
            _putenv_s(key, "");
#else
            unsetenv(key);
#endif
        } else {
#ifdef WIN32
            _putenv_s(key, val);
#else
            setenv(key, val, 1);
#endif
        }
    }
};

TEST_F(GameLiftServerStateInitTest, GIVEN_envVars_WHEN_InitializeNetworking_THEN_useEnvVars) {
    // GIVEN
    const char *websocketUrl = "wss://n1myab2jn9.execute-api.us-west-2.amazonaws.com/alpha";
    const char *fleetId = "fleet-123";
    const char *hostId = "i-123";
    const char *processId = "process-123123123";
    const char *authToken = "AuthToken";
    Aws::GameLift::Server::Model::ServerParameters serverParameters(websocketUrl, authToken, fleetId, hostId, processId);

    SetEnv(ENV_VAR_WEBSOCKET_URL, ENV_VAR_WEBSOCKET_URL_VAL);
    SetEnv(ENV_VAR_AUTH_TOKEN, ENV_VAR_AUTH_TOKEN_VAL);
    SetEnv(ENV_VAR_PROCESS_ID, ENV_VAR_PROCESS_ID_VAL);
    SetEnv(ENV_VAR_HOST_ID, ENV_VAR_HOST_ID_VAL);
    SetEnv(ENV_VAR_FLEET_ID, ENV_VAR_FLEET_ID_VAL);

    Uri expectedUri = Uri::UriBuilder()
                          .WithBaseUri(std::string(ENV_VAR_WEBSOCKET_URL_VAL) + "/")
                          .AddQueryParam(URI_PID_KEY, ENV_VAR_PROCESS_ID_VAL)
                          .AddQueryParam(URI_AUTH_TOKEN_KEY, ENV_VAR_AUTH_TOKEN_VAL)
                          .AddQueryParam(URI_COMPUTE_ID_KEY, ENV_VAR_HOST_ID_VAL)
                          .AddQueryParam(URI_FLEET_ID_KEY, ENV_VAR_FLEET_ID_VAL)
                          .AddQueryParam(URI_SDK_VERSION_KEY, Server::GetSdkVersion().GetResult())
                          .AddQueryParam(URI_FLAVOR_KEY, GameLiftServerState::LANGUAGE)
                          .Build();
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(testing::_)).Times(1).WillOnce(testing::Invoke(this, &GameLiftServerStateInitTest::CaptureUri));

    // WHEN
    GenericOutcome outcome = serverState->InitializeNetworking(serverParameters);

    // THEN
    EXPECT_EQ(expectedUri.GetUriString(), capturedUriString);
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateInitTest, GIVEN_noEnvVars_WHEN_InitializeNetworking_THEN_useParams) {
    // GIVEN
    const char *websocketUrl = "wss://n1myab2jn9.execute-api.us-west-2.amazonaws.com/alpha";
    const char *fleetId = "fleet-123";
    const char *hostId = "i-123";
    const char *processId = "process-123123123";
    const char *authToken = "AuthToken";
    Aws::GameLift::Server::Model::ServerParameters serverParameters(websocketUrl, authToken, fleetId, hostId, processId);

    SetEnv(ENV_VAR_WEBSOCKET_URL, nullptr);
    SetEnv(ENV_VAR_AUTH_TOKEN, nullptr);
    SetEnv(ENV_VAR_PROCESS_ID, nullptr);
    SetEnv(ENV_VAR_HOST_ID, nullptr);
    SetEnv(ENV_VAR_FLEET_ID, nullptr);

    Uri expectedUri = Uri::UriBuilder()
                          .WithBaseUri(std::string(websocketUrl) + "/")
                          .AddQueryParam(URI_PID_KEY, processId)
                          .AddQueryParam(URI_AUTH_TOKEN_KEY, authToken)
                          .AddQueryParam(URI_COMPUTE_ID_KEY, hostId)
                          .AddQueryParam(URI_FLEET_ID_KEY, fleetId)
                          .AddQueryParam(URI_SDK_VERSION_KEY, Server::GetSdkVersion().GetResult())
                          .AddQueryParam(URI_FLAVOR_KEY, GameLiftServerState::LANGUAGE)
                          .Build();

    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(testing::_)).Times(1).WillOnce(testing::Invoke(this, &GameLiftServerStateInitTest::CaptureUri));

    // WHEN
    GenericOutcome outcome = serverState->InitializeNetworking(serverParameters);

    // THEN
    EXPECT_EQ(expectedUri.GetUriString(), capturedUriString);
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateInitTest, GIVEN_someEnvVars_WHEN_InitializeNetworking_THEN_useEnvVarsThatWereSet) {
    // GIVEN
    const char *websocketUrl = "wss://n1myab2jn9.execute-api.us-west-2.amazonaws.com/alpha";
    const char *fleetId = "fleet-123";
    const char *hostId = "i-123";
    const char *processId = "process-123123123";
    const char *authToken = "AuthToken";
    Aws::GameLift::Server::Model::ServerParameters serverParameters(websocketUrl, authToken, fleetId, hostId, processId);

    SetEnv(ENV_VAR_WEBSOCKET_URL, ENV_VAR_WEBSOCKET_URL_VAL);
    SetEnv(ENV_VAR_AUTH_TOKEN, ENV_VAR_AUTH_TOKEN_VAL);
    SetEnv(ENV_VAR_PROCESS_ID, nullptr);
    SetEnv(ENV_VAR_HOST_ID, nullptr);
    SetEnv(ENV_VAR_FLEET_ID, nullptr);

    Uri expectedUri = Uri::UriBuilder()
                          .WithBaseUri(std::string(ENV_VAR_WEBSOCKET_URL_VAL) + "/")
                          .AddQueryParam(URI_PID_KEY, processId)
                          .AddQueryParam(URI_AUTH_TOKEN_KEY, ENV_VAR_AUTH_TOKEN_VAL)
                          .AddQueryParam(URI_COMPUTE_ID_KEY, hostId)
                          .AddQueryParam(URI_FLEET_ID_KEY, fleetId)
                          .AddQueryParam(URI_SDK_VERSION_KEY, Server::GetSdkVersion().GetResult())
                          .AddQueryParam(URI_FLAVOR_KEY, GameLiftServerState::LANGUAGE)
                          .Build();
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(testing::_)).Times(1).WillOnce(testing::Invoke(this, &GameLiftServerStateInitTest::CaptureUri));

    // WHEN
    GenericOutcome outcome = serverState->InitializeNetworking(serverParameters);

    // THEN
    EXPECT_EQ(expectedUri.GetUriString(), capturedUriString);
    EXPECT_TRUE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateInitTest, GIVEN_failureToConnect_WHEN_InitializeNetworking_THEN_returnFailure) {
    // GIVEN
    const char *websocketUrl = "wss://n1myab2jn9.execute-api.us-west-2.amazonaws.com/alpha";
    const char *fleetId = "fleet-123";
    const char *hostId = "i-123";
    const char *processId = "process-123123123";
    const char *authToken = "AuthToken";
    Aws::GameLift::Server::Model::ServerParameters serverParameters(websocketUrl, authToken, fleetId, hostId, processId);

    SetEnv(ENV_VAR_WEBSOCKET_URL, ENV_VAR_WEBSOCKET_URL_VAL);
    SetEnv(ENV_VAR_AUTH_TOKEN, ENV_VAR_AUTH_TOKEN_VAL);
    SetEnv(ENV_VAR_PROCESS_ID, ENV_VAR_PROCESS_ID_VAL);
    SetEnv(ENV_VAR_HOST_ID, ENV_VAR_HOST_ID_VAL);
    SetEnv(ENV_VAR_FLEET_ID, ENV_VAR_FLEET_ID_VAL);

    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(testing::_))
        .Times(1)
        .WillOnce(testing::Return(GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_CONNECT_FAILURE)));

    // WHEN
    GenericOutcome outcome = serverState->InitializeNetworking(serverParameters);

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
}

TEST_F(GameLiftServerStateInitTest, GIVEN_defaultServerParameters_WHEN_initializeNetworking_THEN_returnsSuccessOutcome) {
    // GIVEN
    SetEnv(ENV_VAR_WEBSOCKET_URL, ENV_VAR_WEBSOCKET_URL_VAL);
    SetEnv(ENV_VAR_AUTH_TOKEN, ENV_VAR_AUTH_TOKEN_VAL);
    SetEnv(ENV_VAR_PROCESS_ID, ENV_VAR_PROCESS_ID_VAL);
    SetEnv(ENV_VAR_HOST_ID, ENV_VAR_HOST_ID_VAL);
    SetEnv(ENV_VAR_FLEET_ID, ENV_VAR_FLEET_ID_VAL);

    Uri expectedUri = Uri::UriBuilder()
                          .WithBaseUri(std::string(ENV_VAR_WEBSOCKET_URL_VAL) + "/")
                          .AddQueryParam(URI_PID_KEY, ENV_VAR_PROCESS_ID_VAL)
                          .AddQueryParam(URI_AUTH_TOKEN_KEY, ENV_VAR_AUTH_TOKEN_VAL)
                          .AddQueryParam(URI_COMPUTE_ID_KEY, ENV_VAR_HOST_ID_VAL)
                          .AddQueryParam(URI_FLEET_ID_KEY, ENV_VAR_FLEET_ID_VAL)
                          .AddQueryParam(URI_SDK_VERSION_KEY, Server::GetSdkVersion().GetResult())
                          .AddQueryParam(URI_FLAVOR_KEY, GameLiftServerState::LANGUAGE)
                          .Build();
    EXPECT_CALL(*mockWebSocketClientWrapper, Connect(testing::_)).Times(1).WillOnce(testing::Invoke(this, &GameLiftServerStateInitTest::CaptureUri));
    //  WHEN
    GenericOutcome outcome = serverState->InitializeNetworking(Aws::GameLift::Server::Model::ServerParameters());
    // THEN
    EXPECT_EQ(expectedUri.GetUriString(), capturedUriString);
    EXPECT_TRUE(outcome.IsSuccess());
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws