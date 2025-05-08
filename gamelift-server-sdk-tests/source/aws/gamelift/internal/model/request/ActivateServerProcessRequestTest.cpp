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
#include <aws/gamelift/internal/model/request/ActivateServerProcessRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class ActivateServerProcessRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "testAction";
    const std::string testRequestId = "testRequestId";
    const std::string testSdkVersion = "testSdkVersion";
    const std::string testSdkLanguage = "testSdkLanguage";
    const int testPort = 4444;
#ifdef GAMELIFT_USE_STD
    const std::vector<std::string> testLogPaths = {"/game/logs", "/game/logs/1234"};
#else
    static const int testLogPathsSize = 2;
    const char *const testLogPaths[testLogPathsSize] = {"/game/logs", "/game/logs/1234"};
#endif

    ActivateServerProcessRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);
        testRequest.SetSdkVersion(testSdkVersion);
        testRequest.SetSdkLanguage(testSdkLanguage);
        testRequest.SetPort(testPort);
#ifdef GAMELIFT_USE_STD
        testRequest.SetLogParameters(Aws::GameLift::Server::LogParameters(testLogPaths));
#else
        testRequest.SetLogParameters(Aws::GameLift::Server::LogParameters(testLogPaths, testLogPathsSize));
#endif

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"SdkVersion\":\"" << testSdkVersion << "\",\"SdkLanguage\":\""
           << testSdkLanguage << "\",\"Port\":" << testPort << ",\"LogPaths\":[";

#ifdef GAMELIFT_USE_STD
        for (size_t i = 0; i < testLogPaths.size(); i++) {
#else
        for (size_t i = 0; i < testLogPathsSize; i++) {
#endif
            if (i != 0) {
                ss << ",";
            }

            ss << "\"" << testLogPaths[i] << "\"";
        }

        ss << "]}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const ActivateServerProcessRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetSdkVersion(), testSdkVersion);
        ASSERT_EQ(request.GetSdkLanguage(), testSdkLanguage);
        ASSERT_EQ(request.GetPort(), testPort);
        ASSERT_EQ(request.GetLogParameters(), testRequest.GetLogParameters());
    }
};

TEST_F(ActivateServerProcessRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateServerProcessRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "ActivateServerProcess");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetSdkLanguage(), "");
    ASSERT_EQ(request.GetSdkVersion(), "");
    ASSERT_EQ(request.GetPort(), -1);
    ASSERT_EQ(request.GetLogParameters(), Aws::GameLift::Server::LogParameters());
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_args_WHEN_allArgConstructor_THEN_success) {
    // GIVEN / WHEN
    ActivateServerProcessRequest request(testSdkVersion, testSdkLanguage, testPort, testRequest.GetLogParameters());
    // THEN
    ASSERT_EQ(request.GetAction(), "ActivateServerProcess");
    ASSERT_FALSE(request.GetRequestId().empty());
    ASSERT_EQ(request.GetSdkVersion(), testSdkVersion);
    ASSERT_EQ(request.GetSdkLanguage(), testSdkLanguage);
    ASSERT_EQ(request.GetPort(), testPort);
    ASSERT_EQ(request.GetLogParameters(), testRequest.GetLogParameters());
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateServerProcessRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    ActivateServerProcessRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    // WHEN
    Message &messageRef = request.WithSdkVersion(testSdkVersion)
                              .WithSdkLanguage(testSdkLanguage)
                              .WithPort(testPort)
                              .WithLogParameters(testRequest.GetLogParameters())
                              .WithRequestId(testRequestId)
                              .WithAction(testAction);

    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"ActivateServerProcess\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(ActivateServerProcessRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    ActivateServerProcessRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"SdkVersion\": null"
       << ",\"SdkLanguage\": null"
       << ",\"Port\": null"
       << ",\"LogPaths\": null}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
    ASSERT_EQ(request.GetSdkVersion(), "");
    ASSERT_EQ(request.GetSdkLanguage(), "");
    ASSERT_EQ(request.GetPort(), -1);
    ASSERT_EQ(request.GetLogParameters(), Aws::GameLift::Server::LogParameters());
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws