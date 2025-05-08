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
#include <aws/gamelift/internal/network/IWebSocketClientWrapper.h>

namespace Aws {
namespace GameLift {
namespace Internal {

    /**
     * Interface for a class that wraps a websocket implementation.
     */
    class MockWebSocketClientWrapper : public IWebSocketClientWrapper {
    public:
        MOCK_METHOD(GenericOutcome, Connect, (const Uri& uri), (override));
        MOCK_METHOD(GenericOutcome, SendSocketMessage, (const std::string& requestId, const std::string& message), (override));
        MOCK_METHOD(void, Disconnect, (), (override));
        MOCK_METHOD(void, RegisterGameLiftCallback,
                (const std::string& gameLiftEvent, const std::function<GenericOutcome(std::string)>& callback),
                (override));
        MOCK_METHOD(bool, IsConnected, (), (override));
    };

} //namespace Internal
} //namespace GameLift
} //namespace Aws
