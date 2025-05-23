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
#include <aws/gamelift/internal/model/request/WebSocketGetComputeCertificateRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketGetComputeCertificateRequestTest : public ::testing::Test {
protected:
    const std::string testAction = "GetComputeCertificate";
    const std::string testRequestId = "testRequestId";

    WebSocketGetComputeCertificateRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetAction(testAction);
        testRequest.SetRequestId(testRequestId);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\"}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const WebSocketGetComputeCertificateRequest &request) {
        ASSERT_EQ(request.GetAction(), testAction);
        ASSERT_EQ(request.GetRequestId(), testRequestId);
    }
};

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "GetComputeCertificate");
    ASSERT_FALSE(request.GetRequestId().empty());
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    // WHEN
    Message &messageRef = request.WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &messageRef);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testRequest;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestRequest);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    Message &message = request;
    // WHEN
    message.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    request.SetRequestId(testRequestId);
    Message &message = request;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, "{\"Action\":\"GetComputeCertificate\",\"RequestId\":\"" + testRequestId + "\"}");
}

TEST_F(WebSocketGetComputeCertificateRequestTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateRequest request;
    Message &message = request;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null }";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(request.GetAction(), "");
    ASSERT_EQ(request.GetRequestId(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws