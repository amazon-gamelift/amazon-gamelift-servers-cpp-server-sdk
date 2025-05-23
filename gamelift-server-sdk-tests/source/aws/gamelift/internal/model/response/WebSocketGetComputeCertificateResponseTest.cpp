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
#include <aws/gamelift/internal/model/response/WebSocketGetComputeCertificateResponse.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketGetComputeCertificateResponseTest : public ::testing::Test {
protected:
    const std::string testAction = "GetComputeCertificate";
    const std::string testRequestId = "testRequestId";

    const std::string testComputeName = "testComputeName";
    const std::string testCertificatePath = "testCertificatePath";

    WebSocketGetComputeCertificateResponse testResponse;
    std::string serializedTestResponse;

    void SetUp() override {
        testResponse.SetAction(testAction);
        testResponse.SetRequestId(testRequestId);
        testResponse.SetComputeName(testComputeName);
        testResponse.SetCertificatePath(testCertificatePath);

        std::stringstream ss;
        ss << "{\"Action\":\"" << testAction << "\",\"RequestId\":\"" << testRequestId << "\",\"ComputeName\":\"" << testComputeName
           << "\",\"CertificatePath\":\"" << testCertificatePath << "\"}";
        serializedTestResponse = ss.str();
    }

    void AssertResponseEqualsTestResponse(const WebSocketGetComputeCertificateResponse &response) {
        ASSERT_EQ(response.GetAction(), testAction);
        ASSERT_EQ(response.GetRequestId(), testRequestId);
        ASSERT_EQ(response.GetComputeName(), testComputeName);
        ASSERT_EQ(response.GetCertificatePath(), testCertificatePath);
    }
};

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateResponse response;
    // THEN
    ASSERT_EQ(response.GetAction(), "GetComputeCertificate");
    ASSERT_FALSE(response.GetRequestId().empty());
    ASSERT_EQ(response.GetComputeName(), "");
    ASSERT_EQ(response.GetCertificatePath(), "");
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_response_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateResponse response(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_response_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketGetComputeCertificateResponse response(std::move(testResponse));
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_response_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    // WHEN
    response = testResponse;
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_response_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    // WHEN
    response = std::move(testResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    // WHEN
    Message &messageRef = response.WithComputeName(testComputeName).WithCertificatePath(testCertificatePath).WithRequestId(testRequestId);
    // THEN
    AssertResponseEqualsTestResponse(response);
    ASSERT_EQ(&response, &messageRef);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *message = &testResponse;
    // WHEN
    std::string serializedMessage = message->Serialize();
    // THEN
    ASSERT_EQ(serializedMessage, serializedTestResponse);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    Message &message = response;
    // WHEN
    message.Deserialize(serializedTestResponse);
    // THEN
    AssertResponseEqualsTestResponse(response);
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_emptyMessage_WHEN_serialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    Message &message = response;
    // WHEN
    std::string serializedMessage = message.Serialize();
    // THEN
    std::stringstream ss;
    ss << "{\"Action\":\"GetComputeCertificate\",\"RequestId\":\"" << message.GetRequestId() << "\"}";
    ASSERT_EQ(serializedMessage, ss.str());
}

TEST_F(WebSocketGetComputeCertificateResponseTest, GIVEN_messageWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketGetComputeCertificateResponse response;
    Message &message = response;
    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null "
       << ",\"ComputeName\": null"
       << ",\"CertificatePath\": null"
       << "}";
    std::string serializedTestMessage = ss.str();

    // WHEN
    message.Deserialize(serializedTestMessage);
    // THEN
    ASSERT_EQ(response.GetAction(), "");
    ASSERT_EQ(response.GetRequestId(), "");
    ASSERT_EQ(response.GetComputeName(), "");
    ASSERT_EQ(response.GetCertificatePath(), "");
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws