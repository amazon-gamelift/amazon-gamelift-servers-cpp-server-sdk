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
#include <aws/gamelift/internal/network/callback/TerminateProcessCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class TerminateProcessCallbackTest : public ::testing::Test {
protected:
    MockGameLiftMessageHandler *mockGameLiftMessageHandler;
    TerminateProcessCallback *terminateProcessCallback;
    long capturedTerminationTime;

    void SetUp() override {
        mockGameLiftMessageHandler = new MockGameLiftMessageHandler();
        terminateProcessCallback = new TerminateProcessCallback(mockGameLiftMessageHandler);
    }

    void TearDown() override {
        if (mockGameLiftMessageHandler) {
            delete mockGameLiftMessageHandler;
            mockGameLiftMessageHandler = nullptr;
        }
    }

public:
    void captureTerminationTime(long terminationTime) { capturedTerminationTime = terminationTime; }
};

TEST_F(TerminateProcessCallbackTest, GIVEN_createEmptyMessage_WHEN_onTerminateProcess_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    EXPECT_CALL(*mockGameLiftMessageHandler, OnTerminateProcess(testing::_))
        .WillOnce(testing::Invoke(this, &TerminateProcessCallbackTest::captureTerminationTime));

    // WHEN
    terminateProcessCallback->OnTerminateProcess(jsonMessage);

    // THEN
    EXPECT_THAT(capturedTerminationTime, -1);
}

TEST_F(TerminateProcessCallbackTest, GIVEN_createMessage_WHEN_onTerminateProcess_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{\"TerminationTime\":10000}";
    EXPECT_CALL(*mockGameLiftMessageHandler, OnTerminateProcess(testing::_))
        .WillOnce(testing::Invoke(this, &TerminateProcessCallbackTest::captureTerminationTime));

    // WHEN
    terminateProcessCallback->OnTerminateProcess(jsonMessage);

    // THEN
    EXPECT_THAT(capturedTerminationTime, 10000);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws