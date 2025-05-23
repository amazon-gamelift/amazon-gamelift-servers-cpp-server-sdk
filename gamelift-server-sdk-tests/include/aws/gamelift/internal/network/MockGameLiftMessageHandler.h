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
#pragma once
#include "gmock/gmock.h"
#include <aws/gamelift/internal/network/IGameLiftMessageHandler.h>

namespace Aws {
namespace GameLift {
namespace Internal {

    class MockGameLiftMessageHandler : public IGameLiftMessageHandler {
    public:
        MOCK_METHOD(void , OnStartGameSession, (GameSession& gameSession), (override));
        MOCK_METHOD(void , OnUpdateGameSession, (UpdateGameSession& gameSession), (override));
        MOCK_METHOD(void , OnTerminateProcess, (long terminationTime), (override));
        MOCK_METHOD(void , OnRefreshConnection, (const std::string& refreshConnectionEndpoint, const std::string& authToken), (override));
    };

} //namespace Internal
} //namespace GameLift
} //namespace Aws
