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
#include <aws/gamelift/internal/network/IWebSocketClientWrapper.h>
#include <aws/gamelift/internal/network/WebSocketppClientWrapper.h>
#include <aws/gamelift/server/GameLiftServerAPI.h>
#include <aws/gamelift/server/ProcessParameters.h>

#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <spdlog/spdlog.h>

using namespace Aws::GameLift;

static const std::string sdkVersion = "5.2.0";

#ifdef GAMELIFT_USE_STD
Aws::GameLift::AwsStringOutcome Server::GetSdkVersion() { return AwsStringOutcome(sdkVersion); }

Server::InitSDKOutcome Server::InitSDK() { return InitSDK(Aws::GameLift::Server::Model::ServerParameters()); }

Server::InitSDKOutcome Server::InitSDK(const Aws::GameLift::Server::Model::ServerParameters &serverParameters) {
    Internal::LoggerHelper::InitializeLogger(serverParameters.GetProcessId());
    spdlog::info("Initializing GameLift SDK");
    // Initialize the WebSocketWrapper
    std::shared_ptr<Internal::IWebSocketClientWrapper> webSocketClientWrapper;
    std::shared_ptr<Internal::WebSocketppClientType> wsClientPointer = std::make_shared<Internal::WebSocketppClientType>();
    webSocketClientWrapper = std::make_shared<Internal::WebSocketppClientWrapper>(wsClientPointer);

    InitSDKOutcome initOutcome = InitSDKOutcome(Internal::GameLiftServerState::CreateInstance(webSocketClientWrapper));
    if (initOutcome.IsSuccess()) {
        spdlog::info("Created Instance");
        GenericOutcome networkingOutcome = initOutcome.GetResult()->InitializeNetworking(serverParameters);
        if (!networkingOutcome.IsSuccess()) {
            spdlog::error("Networking outcome failure when init SDK");
            return InitSDKOutcome(networkingOutcome.GetError());
        }
        spdlog::info("Networking outcome success. Init SDK success");
    }
    return initOutcome;
}

GenericOutcome Server::ProcessReady(const Aws::GameLift::Server::ProcessParameters &processParameters) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->ProcessReady(processParameters);
}

GenericOutcome Server::ProcessEnding() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->ProcessEnding();
}

GenericOutcome Server::ActivateGameSession() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return serverState->ActivateGameSession();
}

StartMatchBackfillOutcome Server::StartMatchBackfill(const Aws::GameLift::Server::Model::StartMatchBackfillRequest &request) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return StartMatchBackfillOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->StartMatchBackfill(request);
}

GenericOutcome Server::StopMatchBackfill(const Aws::GameLift::Server::Model::StopMatchBackfillRequest &request) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->StopMatchBackfill(request);
}

GenericOutcome Server::UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy newPlayerSessionPolicy) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return serverState->UpdatePlayerSessionCreationPolicy(newPlayerSessionPolicy);
}

Aws::GameLift::AwsStringOutcome Server::GetGameSessionId() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return AwsStringOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return AwsStringOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return AwsStringOutcome(serverState->GetGameSessionId());
}

Aws::GameLift::AwsLongOutcome Server::GetTerminationTime() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return AwsLongOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return AwsLongOutcome(serverState->GetTerminationTime());
}

GenericOutcome Server::AcceptPlayerSession(const std::string &playerSessionId) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return serverState->AcceptPlayerSession(playerSessionId);
}

GenericOutcome Server::RemovePlayerSession(const std::string &playerSessionId) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return serverState->RemovePlayerSession(playerSessionId);
}

#else
Aws::GameLift::AwsStringOutcome Server::GetSdkVersion() { return AwsStringOutcome(sdkVersion.c_str()); }

GenericOutcome Server::InitSDK() { return InitSDK(Aws::GameLift::Server::Model::ServerParameters()); }

GenericOutcome Server::InitSDK(const Aws::GameLift::Server::Model::ServerParameters &serverParameters) {
    Internal::LoggerHelper::InitializeLogger(serverParameters.GetProcessId());
    spdlog::info("Initializing GameLift SDK");
    // Initialize the WebSocketWrapper
    Internal::InitSDKOutcome initOutcome =
        Internal::InitSDKOutcome(Internal::GameLiftServerState::CreateInstance<Internal::WebSocketppClientWrapper, Internal::WebSocketppClientType>());
    if (initOutcome.IsSuccess()) {
        spdlog::info("Created Instance");
        GenericOutcome networkingOutcome = initOutcome.GetResult()->InitializeNetworking(serverParameters);
        if (!networkingOutcome.IsSuccess()) {
            spdlog::error("Networking outcome failure when init SDK");
            return GenericOutcome(networkingOutcome.GetError());
        }
        spdlog::info("Networking outcome success. Init SDK success");
    }
    return GenericOutcome(nullptr);
}

GenericOutcome Server::ProcessReady(const Aws::GameLift::Server::ProcessParameters &processParameters) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->ProcessReady(processParameters);
}

GenericOutcome Server::ProcessEnding() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->ProcessEnding();
}

GenericOutcome Server::ActivateGameSession() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return serverState->ActivateGameSession();
}

StartMatchBackfillOutcome Server::StartMatchBackfill(const Aws::GameLift::Server::Model::StartMatchBackfillRequest &request) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return StartMatchBackfillOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->StartMatchBackfill(request);
}

GenericOutcome Server::StopMatchBackfill(const Aws::GameLift::Server::Model::StopMatchBackfillRequest &request) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    return serverState->StopMatchBackfill(request);
}

GenericOutcome Server::UpdatePlayerSessionCreationPolicy(Aws::GameLift::Server::Model::PlayerSessionCreationPolicy newPlayerSessionPolicy) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return serverState->UpdatePlayerSessionCreationPolicy(newPlayerSessionPolicy);
}

Aws::GameLift::AwsStringOutcome Server::GetGameSessionId() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return AwsStringOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return AwsStringOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return AwsStringOutcome(serverState->GetGameSessionId());
}

Aws::GameLift::AwsLongOutcome Server::GetTerminationTime() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return AwsLongOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    return AwsLongOutcome(serverState->GetTerminationTime());
}

GenericOutcome Server::AcceptPlayerSession(const char *playerSessionId) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return serverState->AcceptPlayerSession(playerSessionId);
}

GenericOutcome Server::RemovePlayerSession(const char *playerSessionId) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GenericOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return serverState->RemovePlayerSession(playerSessionId);
}
#endif

DescribePlayerSessionsOutcome Server::DescribePlayerSessions(const Aws::GameLift::Server::Model::DescribePlayerSessionsRequest &describePlayerSessionsRequest) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return DescribePlayerSessionsOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());

    if (!serverState->IsProcessReady()) {
        return DescribePlayerSessionsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY));
    }

    return serverState->DescribePlayerSessions(describePlayerSessionsRequest);
}

GenericOutcome Server::Destroy() { return Internal::GameLiftCommonState::DestroyInstance(); }

GetComputeCertificateOutcome Server::GetComputeCertificate() {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GetComputeCertificateOutcome(giOutcome.GetError());
    }

    Internal::GameLiftServerState *serverState = static_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    if (serverState != NULL) {
        return serverState->GetComputeCertificate();
    }

    return GetComputeCertificateOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::NOT_INITIALIZED));
}

GetFleetRoleCredentialsOutcome Server::GetFleetRoleCredentials(const Aws::GameLift::Server::Model::GetFleetRoleCredentialsRequest &request) {
    Internal::GetInstanceOutcome giOutcome = Internal::GameLiftCommonState::GetInstance(Internal::GAMELIFT_INTERNAL_STATE_TYPE::SERVER);

    if (!giOutcome.IsSuccess()) {
        return GetFleetRoleCredentialsOutcome(giOutcome.GetError());
    }

    auto *serverState = dynamic_cast<Internal::GameLiftServerState *>(giOutcome.GetResult());
    if (serverState != nullptr) {
        return serverState->GetFleetRoleCredentials(request);
    }

    return GetFleetRoleCredentialsOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::NOT_INITIALIZED));
}
