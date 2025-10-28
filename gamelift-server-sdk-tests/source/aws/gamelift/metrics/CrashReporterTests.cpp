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
#include <aws/gamelift/metrics/CrashReporterClient.h>
#include <aws/gamelift/internal/util/MockHttpClient.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace Aws::GameLift;
namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Test {

class CrashReporterClientTest : public ::testing::Test {
public:
    std::shared_ptr<Internal::MockHttpClient> mockHttpClient;
    CrashReporterClient *client;

    void SetUp() override {
        mockHttpClient = std::make_shared<::testing::NiceMock<Internal::MockHttpClient>>();
        client = new CrashReporterClient(mockHttpClient, "127.0.0.1", 8080);
    }

    void TearDown() override {
        mockHttpClient = nullptr;
        if (client) {
            delete client;
            client = nullptr;
        }
    }
};

TEST_F(CrashReporterClientTest, RegisterProcess_Success) {
    Internal::HttpResponse successResp;
    successResp.statusCode = 200;
    successResp.body = "OK";

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("register"))).Times(1).WillOnce(testing::Return(successResp));

    client->RegisterProcess();
}

TEST_F(CrashReporterClientTest, RegisterProcess_ConnectionError_Retries) {
    // Mock connection refused exception
    std::runtime_error connectionError("Connection failed, error number: Connection refused");

    // Expect 5 retry attempts (default JitteredGeometricBackoffRetryStrategy)
    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("register")))
        .Times(5)
        .WillRepeatedly(testing::Throw(connectionError));

    client->RegisterProcess();
}

TEST_F(CrashReporterClientTest, RegisterProcess_NonRetryableError_NoRetry) {
    std::runtime_error otherError("Some other error");

    // Should only try once for non-retryable errors
    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("register")))
        .Times(1)
        .WillOnce(testing::Throw(otherError));

    client->RegisterProcess();
}

TEST_F(CrashReporterClientTest, RegisterProcess_Failure) {
    Internal::HttpResponse failResp;
    failResp.statusCode = 500;
    failResp.body = "Internal Server Error";

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("register"))).Times(1).WillOnce(testing::Return(failResp));

    client->RegisterProcess();
}

TEST_F(CrashReporterClientTest, TagGameSession_Success) {
    Internal::HttpResponse successResp{200, "OK"};

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("update"))).Times(1).WillOnce(testing::Return(successResp));

    client->TagGameSession("session-123");
}

TEST_F(CrashReporterClientTest, DeregisterProcess_Success) {
    Internal::HttpResponse successResp{200, "OK"};

    EXPECT_CALL(*mockHttpClient, SendGetRequest(testing::HasSubstr("deregister"))).Times(1).WillOnce(testing::Return(successResp));

    client->DeregisterProcess();
}

} // namespace Test
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
