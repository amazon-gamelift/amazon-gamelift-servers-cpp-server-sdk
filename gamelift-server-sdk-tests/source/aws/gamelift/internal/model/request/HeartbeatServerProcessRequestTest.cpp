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
#include <aws/gamelift/internal/model/request/HeartbeatServerProcessRequest.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class HeartbeatServerProcessRequestTest : public ::testing::Test {
protected:
    const std::string testRequestId = "testRequestId";
    const bool testHealth = true;

    HeartbeatServerProcessRequest testRequest;
    std::string serializedTestRequest;

    void SetUp() override {
        testRequest.SetRequestId(testRequestId);
        testRequest.SetHealthy(testHealth);

        std::stringstream ss;
        ss << "{\"Action\":\"HeartbeatServerProcess\""
           << ",\"RequestId\":\"" << testRequestId << "\""
           << ",\"HealthStatus\":true}";
        serializedTestRequest = ss.str();
    }

    void AssertRequestEqualsTestRequest(const HeartbeatServerProcessRequest &request) {
        ASSERT_EQ(request.GetAction(), "HeartbeatServerProcess");
        ASSERT_EQ(request.GetRequestId(), testRequestId);
        ASSERT_EQ(request.GetHealthy(), testHealth);
    }
};

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    HeartbeatServerProcessRequest request;
    // THEN
    ASSERT_EQ(request.GetAction(), "HeartbeatServerProcess");
    ASSERT_NE(request.GetRequestId(), "");
    ASSERT_EQ(request.GetHealthy(), true);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_request_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    HeartbeatServerProcessRequest request(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_request_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    HeartbeatServerProcessRequest request(std::move(testRequest));
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_request_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    HeartbeatServerProcessRequest request;
    // WHEN
    request = testRequest;
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_request_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    HeartbeatServerProcessRequest request;
    // WHEN
    request = std::move(testRequest);
    // THEN
    AssertRequestEqualsTestRequest(request);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_validInputs_WHEN_withRequestValues_THEN_success) {
    // GIVEN
    HeartbeatServerProcessRequest request;
    // WHEN
    Message &requestRef = request.WithHealthy(testHealth).WithRequestId(testRequestId);
    // THEN
    AssertRequestEqualsTestRequest(request);
    ASSERT_EQ(&request, &requestRef);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_validInput_WHEN_serialize_THEN_success) {
    // GIVEN
    const Message *request = &testRequest;
    // WHEN
    std::string serializedRequest = request->Serialize();
    // THEN
    ASSERT_EQ(serializedRequest, serializedTestRequest);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_validInput_WHEN_deserialize_THEN_success) {
    // GIVEN
    HeartbeatServerProcessRequest heartbeatServerProcessRequest;
    Message &request = heartbeatServerProcessRequest;
    // WHEN
    request.Deserialize(serializedTestRequest);
    // THEN
    AssertRequestEqualsTestRequest(heartbeatServerProcessRequest);
}

TEST_F(HeartbeatServerProcessRequestTest, GIVEN_requestWithNullValues_WHEN_deserialize_THEN_success) {
    // GIVEN
    HeartbeatServerProcessRequest heartbeatServerProcessRequest;
    Message &request = heartbeatServerProcessRequest;

    std::stringstream ss;
    ss << "{\"Action\": null"
       << ",\"RequestId\": null"
       << ",\"HealthStatus\": null }";
    std::string nullStringRequest = ss.str();

    // WHEN
    request.Deserialize(nullStringRequest);

    // THEN
    ASSERT_EQ(heartbeatServerProcessRequest.GetAction(), "");
    ASSERT_EQ(heartbeatServerProcessRequest.GetRequestId(), "");
    ASSERT_EQ(heartbeatServerProcessRequest.GetHealthy(), false);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws