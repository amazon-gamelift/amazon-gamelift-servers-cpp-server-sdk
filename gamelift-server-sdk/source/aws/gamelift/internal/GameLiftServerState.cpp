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
#include <aws/gamelift/internal/GameLiftServerState.h>
#include <aws/gamelift/server/ProcessParameters.h>
#include <aws/gamelift/server/model/DescribePlayerSessionsResult.h>
#include <aws/gamelift/server/model/GetFleetRoleCredentialsRequest.h>
#include <aws/gamelift/server/model/PlayerSessionCreationPolicy.h>
#include <iostream>

#include <aws/gamelift/internal/model/request/AcceptPlayerSessionRequest.h>
#include <aws/gamelift/internal/model/request/ActivateGameSessionRequest.h>
#include <aws/gamelift/internal/model/request/ActivateServerProcessRequest.h>
#include <aws/gamelift/internal/model/request/RemovePlayerSessionRequest.h>
#include <aws/gamelift/internal/model/request/TerminateServerProcessRequest.h>
#include <aws/gamelift/internal/model/request/UpdatePlayerSessionCreationPolicyRequest.h>
#include <aws/gamelift/internal/model/request/WebSocketDescribePlayerSessionsRequest.h>
#include <aws/gamelift/internal/model/request/WebSocketGetComputeCertificateRequest.h>
#include <aws/gamelift/internal/model/request/WebSocketGetFleetRoleCredentialsRequest.h>

#include <aws/gamelift/internal/model/adapter/DescribePlayerSessionsAdapter.h>
#include <aws/gamelift/internal/model/adapter/GetFleetRoleCredentialsAdapter.h>
#include <aws/gamelift/internal/model/adapter/StartMatchBackfillAdapter.h>

#include <aws/gamelift/internal/model/request/HeartbeatServerProcessRequest.h>
#include <aws/gamelift/internal/model/request/WebSocketStopMatchBackfillRequest.h>

#include <aws/gamelift/internal/model/response/WebSocketGetComputeCertificateResponse.h>
#include <aws/gamelift/internal/model/response/WebSocketGetFleetRoleCredentialsResponse.h>

#include <aws/gamelift/internal/network/WebSocketppClientWrapper.h>
#include <aws/gamelift/server/ProcessParameters.h>
#include <cstdlib>
#include <ctime>
#include <spdlog/spdlog.h>

#include <aws/gamelift/internal/retry/JitteredGeometricBackoffRetryStrategy.h>
#include <aws/gamelift/internal/retry/RetryingCallable.h>

#include <aws/gamelift/internal/util/GuidGenerator.h>
#include <aws/gamelift/internal/util/HttpClient.h>
#include <aws/gamelift/internal/security/ContainerMetadataFetcher.h>
#include <aws/gamelift/internal/security/ContainerCredentialsFetcher.h>
#include <aws/gamelift/internal/security/AwsSigV4Utility.h>

using namespace Aws::GameLift;

#ifdef GAMELIFT_USE_STD
Internal::GameLiftServerState::GameLiftServerState()
    : m_onStartGameSession(nullptr), m_onProcessTerminate(nullptr), m_onHealthCheck(nullptr), m_processReady(false), m_terminationTime(-1),
      m_webSocketClientManager(nullptr), m_webSocketClientWrapper(nullptr), m_healthCheckThread(nullptr), m_healthCheckInterrupted(false),
      m_createGameSessionCallback(new CreateGameSessionCallback(this)), m_describePlayerSessionsCallback(new DescribePlayerSessionsCallback()),
      m_getComputeCertificateCallback(new GetComputeCertificateCallback()), m_getFleetRoleCredentialsCallback(new GetFleetRoleCredentialsCallback()),
      m_terminateProcessCallback(new TerminateProcessCallback(this)), m_updateGameSessionCallback(new UpdateGameSessionCallback(this)),
      m_startMatchBackfillCallback(new StartMatchBackfillCallback()), m_refreshConnectionCallback(new RefreshConnectionCallback(this)) {}

Internal::GameLiftServerState::~GameLiftServerState() {
    m_processReady = false;
    if (m_healthCheckThread && m_healthCheckThread->joinable()) {
        {
            std::unique_lock<std::mutex> lock(m_healthCheckMutex);
            m_healthCheckInterrupted = true;
            // "Interrupts" the thread's sleep and causes it to evaluate its "wait_for" predicate.
            // Since "m_healthCheckInterrupted = true" now the predicate will evaluate to "true" and
            // the thread will continue without waiting the full ~60 second interval.
            m_healthCheckConditionVariable.notify_all();
        }
        m_healthCheckThread->join();
    }

    Internal::GameLiftCommonState::SetInstance(nullptr);
    m_onStartGameSession = nullptr;
    m_onUpdateGameSession = nullptr;
    m_onProcessTerminate = nullptr;
    m_onHealthCheck = nullptr;
    m_terminationTime = -1;

    // Tell the webSocketClientManager to disconnect and delete the websocket
    if (m_webSocketClientManager) {
        m_webSocketClientManager->Disconnect();
        delete m_webSocketClientManager;
        m_webSocketClientManager = nullptr;
    }

    m_webSocketClientWrapper = nullptr;
}

GenericOutcome Internal::GameLiftServerState::ProcessReady(const Aws::GameLift::Server::ProcessParameters &processParameters) {
    spdlog::info("Calling ProcessReady");

    m_onStartGameSession = processParameters.getOnStartGameSession();
    m_onUpdateGameSession = processParameters.getOnUpdateGameSession();
    m_onProcessTerminate = processParameters.getOnProcessTerminate();
    m_onHealthCheck = processParameters.getOnHealthCheck();

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::ActivateServerProcessRequest activateServerProcessRequest(Server::GetSdkVersion().GetResult(), Internal::GameLiftServerState::LANGUAGE,
                                                                        processParameters.getPort(), processParameters.getLogParameters());
    Internal::Message &request = activateServerProcessRequest;

    // SendSocketMessageWithRetries makes sync call
    GenericOutcome result = SendSocketMessageWithRetries(request);

    if (result.IsSuccess()) {
        spdlog::info("Successfully executed ActivateServerProcess. Marked m_processReady as true and starting m_healthCheckThread().");
        m_processReady = true;
        m_healthCheckThread = std::unique_ptr<std::thread>(new std::thread([this] { HealthCheck(); }));
    } else {
        spdlog::info("Error while executing ActivateServerProcess. See the root cause error for more information.");
    }

    return result;
}

void Internal::GameLiftServerState::ReportHealth() {
    spdlog::info("Calling ReportHealth");
    std::future<bool> future(std::async([]() { return true; }));
    if (m_onHealthCheck) {
        future = std::async(std::launch::async, m_onHealthCheck);
    }

    // Static variable not guaranteed to be defined (location in memory) at this point unless C++
    // 17+. Creating temp int to pass by reference.
    std::chrono::system_clock::time_point timeoutSeconds = std::chrono::system_clock::now() + std::chrono::milliseconds(int(HEALTHCHECK_TIMEOUT_MILLIS));
    bool health = false;

    // wait_until blocks until timeoutSeconds has been reached or the result becomes available
    if (std::future_status::ready == future.wait_until(timeoutSeconds)) {
        health = future.get();
        spdlog::info("Received Health Response: {} from Server Process: {}", health, m_processId);
    } else {
        spdlog::warn("Timed out waiting for health response from the server process {}. Reporting as unhealthy.", m_processId);
    }

    Internal::HeartbeatServerProcessRequest request = Internal::HeartbeatServerProcessRequest().WithHealthy(health);
    if (m_webSocketClientManager || m_webSocketClientWrapper) {
        spdlog::info("Trying to report process health as {} for process {}", health, m_processId);
        auto outcome = SendSocketMessageWithRetries(request);
        if (!outcome.IsSuccess()) {
	        spdlog::error("Error reporting process health for process {}.", m_processId);
        }
    } else {
	    spdlog::error("Tried to report process health for process {} with no active connection", m_processId);
    }
}

::GenericOutcome Internal::GameLiftServerState::ProcessEnding() {
    m_processReady = false;

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::TerminateServerProcessRequest terminateServerProcessRequest;
    Internal::Message &request = terminateServerProcessRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

std::string Internal::GameLiftServerState::GetGameSessionId() const { return m_gameSessionId; }

long Internal::GameLiftServerState::GetTerminationTime() const { return m_terminationTime; }

GenericOutcome Internal::GameLiftServerState::ActivateGameSession() {
    if (!m_processReady) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::ActivateGameSessionRequest activateGameSessionRequest(m_gameSessionId);
    Internal::Message &request = activateGameSessionRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

GenericOutcome
Internal::GameLiftServerState::UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy newPlayerSessionPolicy) {
    std::string newPlayerSessionPolicyInString =
        Aws::GameLift::Server::Model::PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(newPlayerSessionPolicy);

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    Internal::UpdatePlayerSessionCreationPolicyRequest updatePlayerSessionCreationPolicyRequest(
        m_gameSessionId, PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(newPlayerSessionPolicy));
    Internal::Message &request = updatePlayerSessionCreationPolicyRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

Server::InitSDKOutcome Internal::GameLiftServerState::CreateInstance(std::shared_ptr<Internal::IWebSocketClientWrapper> webSocketClientWrapper) {
    if (GameLiftCommonState::GetInstance().IsSuccess()) {
        return Server::InitSDKOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED));
    }

    GameLiftServerState *newState = new GameLiftServerState();
    newState->m_webSocketClientWrapper = webSocketClientWrapper;
    GenericOutcome setOutcome = GameLiftCommonState::SetInstance(newState);
    if (!setOutcome.IsSuccess()) {
        delete newState;
        return Server::InitSDKOutcome(setOutcome.GetError());
    }

    return newState;
}

GenericOutcome Internal::GameLiftServerState::AcceptPlayerSession(const std::string &playerSessionId) {
    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    AcceptPlayerSessionRequest request = AcceptPlayerSessionRequest().WithGameSessionId(m_gameSessionId).WithPlayerSessionId(playerSessionId);

    return SendSocketMessageWithRetries(request);
}

GenericOutcome Internal::GameLiftServerState::RemovePlayerSession(const std::string &playerSessionId) {
    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    RemovePlayerSessionRequest request = RemovePlayerSessionRequest().WithGameSessionId(m_gameSessionId).WithPlayerSessionId(playerSessionId);

    return SendSocketMessageWithRetries(request);
}

void Internal::GameLiftServerState::OnStartGameSession(Aws::GameLift::Server::Model::GameSession &gameSession) {
    // Inject data that already exists on the server
    gameSession.SetFleetId(m_fleetId);

    std::string gameSessionId = gameSession.GetGameSessionId();
    if (!m_processReady) {
        return;
    }

    m_gameSessionId = gameSessionId;

    // Invoking OnStartGameSession callback if specified by the developer.
    if (m_onStartGameSession) {
        std::thread activateGameSession(std::bind(m_onStartGameSession, gameSession));
        activateGameSession.detach();
    }
}

void Internal::GameLiftServerState::OnTerminateProcess(long terminationTime) {
    // If processReady was never invoked, the callback for processTerminate is null.
    if (!m_processReady) {
        return;
    }

    m_terminationTime = terminationTime;

    // Invoking OnProcessTerminate callback if specified by the developer.
    if (m_onProcessTerminate) {
        std::thread terminateProcess(std::bind(m_onProcessTerminate));
        terminateProcess.detach();
    }
}

void Internal::GameLiftServerState::OnUpdateGameSession(Aws::GameLift::Server::Model::UpdateGameSession &updateGameSession) {
    if (!m_processReady) {
        return;
    }

    // Invoking OnUpdateGameSession callback if specified by the developer.
    if (m_onUpdateGameSession) {
        std::thread updateGameSessionThread(std::bind(m_onUpdateGameSession, updateGameSession));
        updateGameSessionThread.detach();
    }
}

void Internal::GameLiftServerState::OnRefreshConnection(const std::string &refreshConnectionEndpoint, const std::string &authToken) {
    if (!m_processReady) {
        return;
    }
    m_connectionEndpoint = refreshConnectionEndpoint;
    m_authToken = authToken;
    spdlog::info("Refreshing Connection to ConnectionEndpoint: {} for process {}...", m_connectionEndpoint, m_processId);
    m_webSocketClientManager->Connect(m_connectionEndpoint, m_authToken, m_processId, m_hostId, m_fleetId);
}

bool Internal::GameLiftServerState::AssertNetworkInitialized() { return !m_webSocketClientManager || !m_webSocketClientManager->IsConnected(); }

#else

Internal::GameLiftServerState::GameLiftServerState()
    : m_onStartGameSession(nullptr), m_onProcessTerminate(nullptr), m_onHealthCheck(nullptr), m_processReady(false), m_terminationTime(-1),
      m_webSocketClientManager(nullptr), m_webSocketClientWrapper(nullptr), m_healthCheckThread(nullptr), m_healthCheckInterrupted(false),
      m_createGameSessionCallback(new CreateGameSessionCallback(this)), m_describePlayerSessionsCallback(new DescribePlayerSessionsCallback()),
      m_getComputeCertificateCallback(new GetComputeCertificateCallback()), m_getFleetRoleCredentialsCallback(new GetFleetRoleCredentialsCallback()),
      m_terminateProcessCallback(new TerminateProcessCallback(this)), m_updateGameSessionCallback(new UpdateGameSessionCallback(this)),
      m_startMatchBackfillCallback(new StartMatchBackfillCallback()), m_refreshConnectionCallback(new RefreshConnectionCallback(this)) {}

Internal::GameLiftServerState::~GameLiftServerState() {
    m_processReady = false;
    if (m_healthCheckThread && m_healthCheckThread->joinable()) {
        {
            std::unique_lock<std::mutex> lock(m_healthCheckMutex);
            m_healthCheckInterrupted = true;
            // "Interrupts" the thread's sleep and causes it to evaluate its "wait_for" predicate.
            // Since "m_healthCheckInterrupted = true" now the predicate will evaluate to "true" and
            // the thread will continue without waiting the full ~60 second interval.
            m_healthCheckConditionVariable.notify_all();
        }
        m_healthCheckThread->join();
    }

    Internal::GameLiftCommonState::SetInstance(nullptr);
    m_onStartGameSession = nullptr;
    m_onProcessTerminate = nullptr;
    m_onHealthCheck = nullptr;
    m_startGameSessionState = nullptr;
    m_updateGameSessionState = nullptr;
    m_processTerminateState = nullptr;
    m_healthCheckState = nullptr;
    m_terminationTime = -1;

    // Tell the webSocketClientManager to disconnect and delete the websocket
    if (m_webSocketClientManager) {
        m_webSocketClientManager->Disconnect();
        delete m_webSocketClientManager;
        m_webSocketClientManager = nullptr;
    }

    m_webSocketClientWrapper = nullptr;
}

GenericOutcome Internal::GameLiftServerState::ProcessReady(const Aws::GameLift::Server::ProcessParameters &processParameters) {
    spdlog::info("Calling ProcessReady");

    m_onStartGameSession = processParameters.getOnStartGameSession();
    m_startGameSessionState = processParameters.getStartGameSessionState();
    m_onUpdateGameSession = processParameters.getOnUpdateGameSession();
    m_updateGameSessionState = processParameters.getUpdateGameSessionState();
    m_onProcessTerminate = processParameters.getOnProcessTerminate();
    m_processTerminateState = processParameters.getProcessTerminateState();
    m_onHealthCheck = processParameters.getOnHealthCheck();
    m_healthCheckState = processParameters.getHealthCheckState();

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::ActivateServerProcessRequest activateServerProcessRequest(Server::GetSdkVersion().GetResult(), Internal::GameLiftServerState::LANGUAGE,
                                                                        processParameters.getPort(), processParameters.getLogParameters());
    Internal::Message &request = activateServerProcessRequest;

    // SendSocketMessageWithRetries makes sync call
    GenericOutcome result = SendSocketMessageWithRetries(request);

    if (result.IsSuccess()) {
        spdlog::info("Successfully executed ActivateServerProcess. Marked m_processReady as true and starting m_healthCheckThread().");
        m_processReady = true;
        m_healthCheckThread = std::unique_ptr<std::thread>(new std::thread([this] { HealthCheck(); }));
    } else {
        spdlog::info("Error while executing ActivateServerProcess. See the root cause error for more information.");
    }

    return result;
}

void Internal::GameLiftServerState::ReportHealth() {
    spdlog::info("Calling ReportHealth");
    std::future<bool> future(std::async([]() { return true; }));
    if (m_onHealthCheck) {
        future = std::async(std::launch::async, m_onHealthCheck, m_healthCheckState);
    }

    std::chrono::system_clock::time_point timeoutSeconds = std::chrono::system_clock::now() + std::chrono::milliseconds(HEALTHCHECK_TIMEOUT_MILLIS);
    bool health = false;

    // wait_until blocks until timeoutSeconds has been reached or the result becomes available
    if (std::future_status::ready == future.wait_until(timeoutSeconds)) {
        health = future.get();
        spdlog::info("Received Health Response: {} from Server Process: {}", health, m_processId);
    } else {
        spdlog::warn("Timed out waiting for health response from the server process {}. Reporting as unhealthy.", m_processId);
    }

    Internal::HeartbeatServerProcessRequest msg = Internal::HeartbeatServerProcessRequest().WithHealthy(health);
    if (m_webSocketClientManager || m_webSocketClientWrapper) {
        spdlog::info("Trying to report process health as {} for process {}", health, m_processId);
        auto outcome = SendSocketMessageWithRetries(msg);
        if (!outcome.IsSuccess()) {
            spdlog::error("Error reporting process health for process {}.", m_processId);
        }
    } else {
        spdlog::error("Tried to report process health for process {} with no active connection", m_processId);
    }
}

::GenericOutcome Internal::GameLiftServerState::ProcessEnding() {
    m_processReady = false;

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::TerminateServerProcessRequest terminateServerProcessRequest;
    Internal::Message &request = terminateServerProcessRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

const char *Internal::GameLiftServerState::GetGameSessionId() { return m_gameSessionId.c_str(); }

long Internal::GameLiftServerState::GetTerminationTime() { return m_terminationTime; }

GenericOutcome Internal::GameLiftServerState::ActivateGameSession() {
    if (!m_processReady) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::ActivateGameSessionRequest activateGameSessionRequest(m_gameSessionId);
    Internal::Message &request = activateGameSessionRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

GenericOutcome
Internal::GameLiftServerState::UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy newPlayerSessionPolicy) {
    std::string newPlayerSessionPolicyInString =
        Aws::GameLift::Server::Model::PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(newPlayerSessionPolicy);

    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    Internal::UpdatePlayerSessionCreationPolicyRequest updatePlayerSessionCreationPolicyRequest(
        m_gameSessionId, PlayerSessionCreationPolicyMapper::GetNameForPlayerSessionCreationPolicy(newPlayerSessionPolicy));
    Internal::Message &request = updatePlayerSessionCreationPolicyRequest;
    GenericOutcome result = SendSocketMessageWithRetries(request);

    return result;
}

GenericOutcome Internal::GameLiftServerState::AcceptPlayerSession(const std::string &playerSessionId) {
    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    AcceptPlayerSessionRequest request = AcceptPlayerSessionRequest().WithGameSessionId(m_gameSessionId).WithPlayerSessionId(playerSessionId);

    return SendSocketMessageWithRetries(request);
}

GenericOutcome Internal::GameLiftServerState::RemovePlayerSession(const std::string &playerSessionId) {
    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    if (m_gameSessionId.empty()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAME_SESSION_ID_NOT_SET));
    }

    RemovePlayerSessionRequest request = RemovePlayerSessionRequest().WithGameSessionId(m_gameSessionId).WithPlayerSessionId(playerSessionId);

    return SendSocketMessageWithRetries(request);
}

std::shared_ptr<Internal::IWebSocketClientWrapper> Internal::GameLiftServerState::GetWebSocketClientWrapper() const { return m_webSocketClientWrapper; }

Internal::InitSDKOutcome Internal::GameLiftServerState::ConstructInternal(std::shared_ptr<IWebSocketClientWrapper> webSocketClientWrapper) {
    if (GameLiftCommonState::GetInstance().IsSuccess()) {
        return Internal::InitSDKOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED));
    }

    GameLiftServerState *newState = new GameLiftServerState();
    newState->m_webSocketClientWrapper = webSocketClientWrapper;

    GenericOutcome setOutcome = GameLiftCommonState::SetInstance(newState);
    if (!setOutcome.IsSuccess()) {
        delete newState;
        return Outcome<GameLiftServerState *, GameLiftError>(setOutcome.GetError());
    }

    return newState;
}

void Internal::GameLiftServerState::OnStartGameSession(Aws::GameLift::Server::Model::GameSession &gameSession) {
    // Inject data that already exists on the server
    gameSession.SetFleetId(m_fleetId.c_str());

    std::string gameSessionId = gameSession.GetGameSessionId();
    if (!m_processReady) {
        return;
    }

    m_gameSessionId = gameSessionId;

    // Invoking OnStartGameSession callback if specified by the developer.
    if (m_onStartGameSession) {
        std::thread activateGameSession(std::bind(m_onStartGameSession, gameSession, m_startGameSessionState));
        activateGameSession.detach();
    }
}

void Internal::GameLiftServerState::OnUpdateGameSession(Aws::GameLift::Server::Model::UpdateGameSession &updateGameSession) {
    if (!m_processReady) {
        return;
    }

    // Invoking OnUpdateGameSession callback if specified by the developer.
    if (m_onUpdateGameSession) {
        std::thread updateGameSessionThread(std::bind(m_onUpdateGameSession, updateGameSession, m_updateGameSessionState));
        updateGameSessionThread.detach();
    }
}

void Internal::GameLiftServerState::OnTerminateProcess(long terminationTime) {
    // If processReady was never invoked, the callback for processTerminate is null.
    if (!m_processReady) {
        return;
    }

    m_terminationTime = terminationTime;

    // Invoking onProcessTerminate callback if specified by the developer.
    if (m_onProcessTerminate) {
        std::thread terminateProcess(std::bind(m_onProcessTerminate, m_processTerminateState));
        terminateProcess.detach();
    }
}

void Internal::GameLiftServerState::OnRefreshConnection(const std::string &refreshConnectionEndpoint, const std::string &authToken) {
    if (!m_processReady) {
        return;
    }
    m_connectionEndpoint = refreshConnectionEndpoint;
    m_authToken = authToken;
    spdlog::info("Refreshing Connection to ConnectionEndpoint: {} for process {}...", m_connectionEndpoint, m_processId);
    m_webSocketClientManager->Connect(m_connectionEndpoint, m_authToken, m_processId, m_hostId, m_fleetId);
}

bool Internal::GameLiftServerState::AssertNetworkInitialized() { return !m_webSocketClientManager || !m_webSocketClientManager->IsConnected(); }
#endif

GenericOutcome Internal::GameLiftServerState::InitializeNetworking(const Aws::GameLift::Server::Model::ServerParameters &serverParameters) {
    spdlog::info("Initializing Networking");

    Internal::GameLiftServerState::SetUpCallbacks();

    // Connect to websocket, use environment vars if present
    char *webSocketUrl;
    char *authToken;
    char *processId;
    char *hostId;
    char *fleetId;
    char *computeType;
    char *awsRegion;
    char *accessKey;
    char *secretKey;
    char *sessionToken;

    GetOverrideParams(&webSocketUrl, &authToken, &processId, &hostId, &fleetId, &computeType, &awsRegion, &accessKey, &secretKey, &sessionToken);
    m_connectionEndpoint = std::string(webSocketUrl == nullptr ? serverParameters.GetWebSocketUrl() : webSocketUrl);
    m_authToken = std::string(authToken == nullptr ? serverParameters.GetAuthToken() : authToken);
    m_fleetId = std::string(fleetId == nullptr ? serverParameters.GetFleetId() : fleetId);
    m_hostId = std::string(hostId == nullptr ? serverParameters.GetHostId() : hostId);
    m_processId = std::string(processId == nullptr ? serverParameters.GetProcessId() : processId);

#ifdef GAMELIFT_USE_STD
    if(!awsRegion || awsRegion[0] == '\0') {
        awsRegion = new char[serverParameters.GetAwsRegion().size() + 1];
        std::strcpy(awsRegion, serverParameters.GetAwsRegion().c_str());
    }
    if(!accessKey || accessKey[0] == '\0') {
        accessKey = new char[serverParameters.GetAccessKey().size() + 1];
        std::strcpy(accessKey, serverParameters.GetAccessKey().c_str());
    }
    if(!secretKey || secretKey[0] == '\0') {
        secretKey = new char[serverParameters.GetSecretKey().size() + 1];
        std::strcpy(secretKey, serverParameters.GetSecretKey().c_str());
    }
    if(!sessionToken || sessionToken[0] == '\0') {
        sessionToken = new char[serverParameters.GetSessionToken().size() + 1];
        std::strcpy(sessionToken, serverParameters.GetSessionToken().c_str());
    }
#else
    if(!awsRegion || awsRegion[0] == '\0') {
        awsRegion = new char[strlen(serverParameters.GetAwsRegion()) + 1];
        std::strcpy(awsRegion, serverParameters.GetAwsRegion());
    }
    if(!accessKey || accessKey[0] == '\0') {
        accessKey = new char[strlen(serverParameters.GetAccessKey()) + 1];
        std::strcpy(accessKey, serverParameters.GetAccessKey());
    }
    if(!secretKey || secretKey[0] == '\0') {
        secretKey = new char[strlen(serverParameters.GetSecretKey()) + 1];
        std::strcpy(secretKey, serverParameters.GetSecretKey());
    }
    if(!sessionToken || sessionToken[0] == '\0') {
        sessionToken = new char[strlen(serverParameters.GetSessionToken()) + 1];
        std::strcpy(sessionToken, serverParameters.GetSessionToken());
    }
#endif
    bool isContainerComputeType = computeType && std::strcmp(computeType, COMPUTE_TYPE_CONTAINER) == 0;
    bool authTokenPassed = !m_authToken.empty();
    bool sigV4ParametersPassed = awsRegion != nullptr && accessKey != nullptr && secretKey != nullptr;
    if (!authTokenPassed && !sigV4ParametersPassed && !isContainerComputeType) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }
    if (authTokenPassed) {
        GenericOutcome outcome =
            m_webSocketClientManager->Connect(m_connectionEndpoint, m_authToken, m_processId, m_hostId, m_fleetId);
        return outcome;
    } else {
        if (isContainerComputeType) {
            HttpClient httpClient;
            ContainerCredentialsFetcher containerCredentialsFetcher = ContainerCredentialsFetcher(httpClient);
            Outcome<AwsCredentials, std::string> containerCredentialsFetcherOutcome = containerCredentialsFetcher.FetchContainerCredentials();
            if(!containerCredentialsFetcherOutcome.IsSuccess()) {
                spdlog::error("Failed to get Container Credentials due to {}",
                              containerCredentialsFetcherOutcome.GetError().c_str());
                return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION));
            }
            accessKey = new char[containerCredentialsFetcherOutcome.GetResult().AccessKey.size() + 1];
            std::strcpy(accessKey, containerCredentialsFetcherOutcome.GetResult().AccessKey.c_str());

            secretKey = new char[containerCredentialsFetcherOutcome.GetResult().SecretKey.size() + 1];
            std::strcpy(secretKey, containerCredentialsFetcherOutcome.GetResult().SecretKey.c_str());

            sessionToken = new char[containerCredentialsFetcherOutcome.GetResult().SessionToken.size() + 1];
            std::strcpy(sessionToken, containerCredentialsFetcherOutcome.GetResult().SessionToken.c_str());

            ContainerMetadataFetcher containerMetadataFetcher = ContainerMetadataFetcher(httpClient);
            Outcome<ContainerTaskMetadata, std::string> containerMetadataFetcherOutcome = containerMetadataFetcher.FetchContainerTaskMetadata();
            if(!containerMetadataFetcherOutcome.IsSuccess()) {
                spdlog::error("Failed to get Container Task Metadata due to {}",
                              containerMetadataFetcherOutcome.GetError().c_str());
                return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION));
            }
            m_hostId = containerMetadataFetcherOutcome.GetResult().TaskId;
        }
        Outcome<std::map<std::string, std::string>, std::string> sigV4QueryParametersOutcome = GetSigV4QueryParameters(awsRegion, accessKey, secretKey, sessionToken);
        if (!sigV4QueryParametersOutcome.IsSuccess()) {
            spdlog::error("Failed to generate SigV4 Query Parameters due to {}",
                          sigV4QueryParametersOutcome.GetError().c_str());
            return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION));
        }
        GenericOutcome outcome =
                m_webSocketClientManager->Connect(
                        m_connectionEndpoint,
                        m_authToken,
                        m_processId,
                        m_hostId,
                        m_fleetId,
                        sigV4QueryParametersOutcome.GetResult());
        return outcome;
    }
}

void Internal::GameLiftServerState::SetUpCallbacks() {
    // Setup
    m_webSocketClientManager = new Internal::GameLiftWebSocketClientManager(m_webSocketClientWrapper);

    // Setup CreateGameSession callback
    spdlog::info("Setting Up WebSocket With default callbacks");
    // Passing callback raw pointers down is fine since m_webSocketClientWrapper won't outlive the
    // callbacks
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        CreateGameSessionCallback::CREATE_GAME_SESSION,
        std::bind(&CreateGameSessionCallback::OnStartGameSession, m_createGameSessionCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        DescribePlayerSessionsCallback::DESCRIBE_PLAYER_SESSIONS,
        std::bind(&DescribePlayerSessionsCallback::OnDescribePlayerSessions, m_describePlayerSessionsCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        GetComputeCertificateCallback::GET_COMPUTE_CERTIFICATE,
        std::bind(&GetComputeCertificateCallback::OnGetComputeCertificateCallback, m_getComputeCertificateCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        GetFleetRoleCredentialsCallback::GET_FLEET_ROLE_CREDENTIALS,
        std::bind(&GetFleetRoleCredentialsCallback::OnGetFleetRoleCredentials, m_getFleetRoleCredentialsCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        TerminateProcessCallback::TERMINATE_PROCESS,
        std::bind(&TerminateProcessCallback::OnTerminateProcess, m_terminateProcessCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        UpdateGameSessionCallback::UPDATE_GAME_SESSION,
        std::bind(&UpdateGameSessionCallback::OnUpdateGameSession, m_updateGameSessionCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        StartMatchBackfillCallback::START_MATCH_BACKFILL,
        std::bind(&StartMatchBackfillCallback::OnStartMatchBackfill, m_startMatchBackfillCallback.get(), std::placeholders::_1));
    m_webSocketClientWrapper->RegisterGameLiftCallback(
        RefreshConnectionCallback::REFRESH_CONNECTION,
        std::bind(&RefreshConnectionCallback::OnRefreshConnection, m_refreshConnectionCallback.get(), std::placeholders::_1));
}

GenericOutcome Internal::GameLiftServerState::SendSocketMessageWithRetries(Message &message) {
    spdlog::debug("Trying to send socket message for process: {}...", m_processId);
    GenericOutcome outcome;
    int resendFailureCount = 0;
    const int maxFailuresBeforeReconnect = 2;

    // Delegate to the websocketClientManager to send the request and retry if possible
    const std::function<bool(void)> &retriable = [&] {
        outcome = m_webSocketClientManager->SendSocketMessage(message);
        if (outcome.IsSuccess()) {
            spdlog::debug("Successfully send message for process: {}", m_processId);
            return true;
        }
        else if (outcome.GetError().GetErrorType() == GAMELIFT_ERROR_TYPE::WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE) {
            resendFailureCount++;
            if (resendFailureCount >= maxFailuresBeforeReconnect) {
                spdlog::warn("Max sending message failure threshold reached for process: {}. Attempting to reconnect...", m_processId);
                m_webSocketClientWrapper->Disconnect();

                spdlog::info("Attempting to create a new WebSocket connections for process: {}...", m_processId);
                // Create a completely new webSocketClientWrapper
                std::shared_ptr<Internal::WebSocketppClientType> wsClientPointer = std::make_shared<Internal::WebSocketppClientType>();
                m_webSocketClientWrapper = std::make_shared<Internal::WebSocketppClientWrapper>(wsClientPointer);

                spdlog::info("Re-establish Networking...");
                // Re-establish network with new webSocketClientWrapper
                Internal::GameLiftServerState::SetUpCallbacks();
                auto networkOutcome = m_webSocketClientManager->Connect(m_connectionEndpoint, m_authToken, m_processId, m_hostId, m_fleetId);
                if (networkOutcome.IsSuccess()) {
                    spdlog::info("Reconnected successfully. Retrying message sending...");
                    resendFailureCount = 0;
                    return false; // Force another retry sending message after successful connection
                } else {
                    spdlog::error("Reconnection failed. Aborting retries.");
                    return true; // Abort retry if connection fails
                }
            }
            return false; // Continue retrying if not reaching the threshold
        }
        else {
            return true; // Not retry sending message for unexpected errors aside from WEBSOCKET_RETRIABLE_SEND_MESSAGE_FAILURE
        }
    };

    // Jittered retry required because many requests can cause buffer to fill.
    // If retries all happpen in sync, it can cause potential delays in recovery.
    JitteredGeometricBackoffRetryStrategy retryStrategy;
    RetryingCallable callable = RetryingCallable::Builder().WithRetryStrategy(&retryStrategy).WithCallable(retriable).Build();

    callable.call();

    if (resendFailureCount > 0 && !outcome.IsSuccess()) {
        spdlog::error("Error sending socket message");
        outcome = GenericOutcome(GAMELIFT_ERROR_TYPE::WEBSOCKET_SEND_MESSAGE_FAILURE);
    }

    return outcome;
}

DescribePlayerSessionsOutcome
Internal::GameLiftServerState::DescribePlayerSessions(const Aws::GameLift::Server::Model::DescribePlayerSessionsRequest &describePlayerSessionsRequest) {
    if (AssertNetworkInitialized()) {
        return DescribePlayerSessionsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    WebSocketDescribePlayerSessionsRequest request = Internal::DescribePlayerSessionsAdapter::convert(describePlayerSessionsRequest);
    GenericOutcome rawResponse = SendSocketMessageWithRetries(request);
    if (rawResponse.IsSuccess()) {
        WebSocketDescribePlayerSessionsResponse *webSocketResponse = static_cast<WebSocketDescribePlayerSessionsResponse *>(rawResponse.GetResult());
        DescribePlayerSessionsResult result = Internal::DescribePlayerSessionsAdapter::convert(webSocketResponse);
        delete webSocketResponse;

        return DescribePlayerSessionsOutcome(result);
    } else {
        return DescribePlayerSessionsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }
}

StartMatchBackfillOutcome
Internal::GameLiftServerState::StartMatchBackfill(const Aws::GameLift::Server::Model::StartMatchBackfillRequest &startMatchBackfillRequest) {
    if (AssertNetworkInitialized()) {
        return StartMatchBackfillOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    WebSocketStartMatchBackfillRequest request = Internal::StartMatchBackfillAdapter::convert(startMatchBackfillRequest);
    GenericOutcome rawResponse = SendSocketMessageWithRetries(request);
    if (rawResponse.IsSuccess()) {
        WebSocketStartMatchBackfillResponse *webSocketResponse = static_cast<WebSocketStartMatchBackfillResponse *>(rawResponse.GetResult());
        StartMatchBackfillResult result = Internal::StartMatchBackfillAdapter::convert(webSocketResponse);
        delete webSocketResponse;

        return StartMatchBackfillOutcome(result);
    } else {
        return StartMatchBackfillOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }
}

GenericOutcome Internal::GameLiftServerState::StopMatchBackfill(const Aws::GameLift::Server::Model::StopMatchBackfillRequest &stopMatchBackfillRequest) {
    if (AssertNetworkInitialized()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    Internal::WebSocketStopMatchBackfillRequest request = Internal::WebSocketStopMatchBackfillRequest()
                                                              .WithTicketId(stopMatchBackfillRequest.GetTicketId())
                                                              .WithGameSessionArn(stopMatchBackfillRequest.GetGameSessionArn())
                                                              .WithMatchmakingConfigurationArn(stopMatchBackfillRequest.GetMatchmakingConfigurationArn());
    GenericOutcome outcome = SendSocketMessageWithRetries(request);
    if (!outcome.IsSuccess()) {
        spdlog::error("Error calling StopMatchBackfill.");
    }
    return outcome;
}

GetComputeCertificateOutcome Internal::GameLiftServerState::GetComputeCertificate() {
    if (AssertNetworkInitialized()) {
        return GetComputeCertificateOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    WebSocketGetComputeCertificateRequest request;
    GenericOutcome rawResponse = SendSocketMessageWithRetries(request);
    if (rawResponse.IsSuccess()) {
        WebSocketGetComputeCertificateResponse *webSocketResponse = static_cast<WebSocketGetComputeCertificateResponse *>(rawResponse.GetResult());
        GetComputeCertificateResult result = GetComputeCertificateResult()
                                                 .WithCertificatePath(webSocketResponse->GetCertificatePath().c_str())
                                                 .WithComputeName(webSocketResponse->GetComputeName().c_str());
        delete webSocketResponse;

        return GetComputeCertificateOutcome(result);
    } else {
        return GetComputeCertificateOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }
}

GetFleetRoleCredentialsOutcome
Internal::GameLiftServerState::GetFleetRoleCredentials(const Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest &request) {
    if (AssertNetworkInitialized()) {
        return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED));
    }

    // If we've decided we're not on managed EC2, fail without making an APIGW call
    if (!m_onManagedEC2) {
        return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }

    auto webSocketRequest = Internal::GetFleetRoleCredentialsAdapter::convert(request);

    // Check if we're cached credentials recently that still has at least 15 minutes before
    // expiration
    if (m_instanceRoleResultCache.find(webSocketRequest.GetRoleArn()) != m_instanceRoleResultCache.end()) {
        auto previousResult = m_instanceRoleResultCache[webSocketRequest.GetRoleArn()];
#ifdef GAMELIFT_USE_STD
        std::tm expiration = previousResult.GetExpiration();
#ifdef WIN32
        time_t previousResultExpiration = _mkgmtime(&expiration);
#else
        time_t previousResultExpiration = timegm(&expiration);
#endif
#else
        time_t previousResultExpiration = previousResult.GetExpiration();
#endif
        time_t currentTime = time(nullptr);

        if ((previousResultExpiration - INSTANCE_ROLE_CREDENTIAL_TTL_MIN) > currentTime) {
            return GetFleetRoleCredentialsOutcome(previousResult);
        }

        m_instanceRoleResultCache.erase(webSocketRequest.GetRoleArn());
    }

    if (webSocketRequest.GetRoleSessionName().empty()) {
        std::string generatedRoleSessionName = m_fleetId + "-" + m_hostId;
        if (generatedRoleSessionName.length() > MAX_ROLE_SESSION_NAME_LENGTH) {
            generatedRoleSessionName = generatedRoleSessionName.substr(0, MAX_ROLE_SESSION_NAME_LENGTH);
        }
        webSocketRequest.SetRoleSessionName(generatedRoleSessionName);
    }

    if (webSocketRequest.GetRoleSessionName().length() > MAX_ROLE_SESSION_NAME_LENGTH) {
        return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }

    auto rawResponse = SendSocketMessageWithRetries(webSocketRequest);
    if (!rawResponse.IsSuccess()) {
        return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }

    std::unique_ptr<WebSocketGetFleetRoleCredentialsResponse> webSocketResponse(
        static_cast<WebSocketGetFleetRoleCredentialsResponse *>(rawResponse.GetResult()));

    // If we get a success response from APIGW with empty fields we're not on managed EC2
    if (webSocketResponse->GetAccessKeyId().empty()) {
        m_onManagedEC2 = false;
        return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION));
    }

    auto result = Internal::GetFleetRoleCredentialsAdapter::convert(webSocketResponse.get());
    m_instanceRoleResultCache[webSocketRequest.GetRoleArn()] = result;
    return GetFleetRoleCredentialsOutcome(result);
}

void Internal::GameLiftServerState::GetOverrideParams(
        char **webSocketUrl,
        char **authToken,
        char **processId,
        char **hostId,
        char **fleetId,
        char **computeType,
        char **awsRegion,
        char **accessKey,
        char **secretKey,
        char **sessionToken) {
    *webSocketUrl = std::getenv(ENV_VAR_WEBSOCKET_URL);
    *authToken = std::getenv(ENV_VAR_AUTH_TOKEN);
    *processId = std::getenv(ENV_VAR_PROCESS_ID);
    *hostId = std::getenv(ENV_VAR_HOST_ID);
    *fleetId = std::getenv(ENV_VAR_FLEET_ID);
    *computeType = std::getenv(ENV_VAR_COMPUTE_TYPE);
    *awsRegion = std::getenv(ENV_VAR_AWS_REGION);
    *accessKey = std::getenv(ENV_VAR_ACCESS_KEY);
    *secretKey = std::getenv(ENV_VAR_SECRET_KEY);
    *sessionToken = std::getenv(ENV_VAR_SESSION_TOKEN);

    if (*webSocketUrl != nullptr) {
        spdlog::info("Env override for webSocketUrl: {}", *webSocketUrl);
    }

    if (*authToken != nullptr) {
        spdlog::info("Using env override for authToken");
    }

    if (*processId != nullptr) {
        spdlog::info("Env override for processId: {}", *processId);
        if (std::strcmp(*processId, AGENTLESS_CONTAINER_PROCESS_ID) == 0) {
            std::string guidValue = GuidGenerator::GenerateGuid();
            *processId = new char[guidValue.size() + 1];
            std::strcpy(*processId, guidValue.c_str());
            spdlog::info("Auto Generated ProcessId, new value is: {}", *processId);
        }
    }

    if (*hostId != nullptr) {
        spdlog::info("Env override for hostId: {}", *hostId);
    }

    if (*fleetId != nullptr) {
        spdlog::info("Env override for fleetId: {}", *fleetId);
    }

    if (*computeType != nullptr) {
        spdlog::info("Env override for computeType: {}", *computeType);
    }

    if (*awsRegion != nullptr) {
        spdlog::info("Env override for awsRegion: {}", *awsRegion);
    }
    // We are not logging AWS Credentials for security reasons.
}

Outcome<std::map<std::string, std::string>, std::string> Internal::GameLiftServerState::GetSigV4QueryParameters(
        char *awsRegion,
        char *accessKey,
        char *secretKey,
        char *sessionToken) {
    AwsCredentials awsCredentials(accessKey, secretKey, sessionToken);
    std::map <std::string, std::string> queryParamsToSign;
    queryParamsToSign["ComputeId"] = m_hostId;
    queryParamsToSign["FleetId"] = m_fleetId;
    queryParamsToSign["pID"] = m_processId;

    std::time_t requestTime = std::time(nullptr);
    std::tm utcTime = {};
#ifdef _WIN32
    gmtime_s(&utcTime, &requestTime);
#else
    gmtime_r(&requestTime, &utcTime);
#endif
    SigV4Parameters sigV4Params(awsRegion, awsCredentials, queryParamsToSign, utcTime);
    return AwsSigV4Utility::GenerateSigV4QueryParameters(sigV4Params);
}

void Internal::GameLiftServerState::HealthCheck() {
    // Seed the random number generator used to generate healthCheck interval jitters
    std::srand(std::time(0));

    while (m_processReady) {
        std::async(std::launch::async, &Internal::GameLiftServerState::ReportHealth, this);
        std::chrono::duration<long int, std::ratio<1, 1000>> time = std::chrono::milliseconds(GetNextHealthCheckIntervalMillis());
        std::unique_lock<std::mutex> lock(m_healthCheckMutex);
        // If the lambda below returns false, the thread will wait until "time" millis expires. If
        // it returns true, the thread immediately continues.
        spdlog::info("Performing HealthCheck(), processReady is true, wait for {} ms for next health check", time.count());
        m_healthCheckConditionVariable.wait_for(lock, time, [&]() { return m_healthCheckInterrupted; });
    }
}

int Internal::GameLiftServerState::GetNextHealthCheckIntervalMillis() {
    // Jitter the healthCheck interval +/- a random value between [-MAX_JITTER_SECONDS,
    // MAX_JITTER_SECONDS]
    int jitter = std::rand() % (HEALTHCHECK_MAX_JITTER_MILLIS * 2 + 1) - HEALTHCHECK_MAX_JITTER_MILLIS;
    return HEALTHCHECK_INTERVAL_MILLIS + jitter;
}
