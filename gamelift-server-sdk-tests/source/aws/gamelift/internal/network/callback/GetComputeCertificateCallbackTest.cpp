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
#include <aws/gamelift/internal/network/callback/GetComputeCertificateCallback.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class GetComputeCertificateCallbackTest : public ::testing::Test {
protected:
    GetComputeCertificateCallback getComputeCertificateCallback;
};

TEST_F(GetComputeCertificateCallbackTest, GIVEN_createEmptyMessage_WHEN_onGetComputeCertificate_THEN_success) {
    // GIVEN / EXPECT
    std::string jsonMessage = "{}";

    // WHEN
    GenericOutcome genericOutcome = getComputeCertificateCallback.OnGetComputeCertificateCallback(jsonMessage);
    std::unique_ptr<WebSocketGetComputeCertificateResponse> response(static_cast<WebSocketGetComputeCertificateResponse *>(genericOutcome.GetResult()));

    // THEN
    EXPECT_EQ(response->GetComputeName(), "");
    EXPECT_EQ(response->GetCertificatePath(), "");
}

TEST_F(GetComputeCertificateCallbackTest, GIVEN_createMessage_WHEN_onGetComputeCertificate_THEN_success) {
    // GIVEN / EXPECT
    std::stringstream ss;
    ss << "{\"ComputeName\":\"ComputeName\","
       << "\"CertificatePath\":\"CertificatePath\"}";
    std::string jsonMessage = ss.str();

    // WHEN
    GenericOutcome genericOutcome = getComputeCertificateCallback.OnGetComputeCertificateCallback(jsonMessage);
    std::unique_ptr<WebSocketGetComputeCertificateResponse> response(static_cast<WebSocketGetComputeCertificateResponse *>(genericOutcome.GetResult()));

    // THEN
    EXPECT_EQ(response->GetComputeName(), "ComputeName");
    EXPECT_EQ(response->GetCertificatePath(), "CertificatePath");
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws