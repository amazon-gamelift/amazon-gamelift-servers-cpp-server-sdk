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
#include <aws/gamelift/internal/model/adapter/GetFleetRoleCredentialsAdapter.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {
class GetFleetRoleCredentialsAdapterTest : public ::testing::Test {};

TEST_F(GetFleetRoleCredentialsAdapterTest, GIVEN_GetFleetRoleCredentialsRequest_WHEN_convert_THEN_success) {
    // GIVEN
    Server::Model::GetFleetRoleCredentialsRequest request =
        Server::Model::GetFleetRoleCredentialsRequest().WithRoleArn("gameSessionId").WithRoleSessionName("playerId");
    // WHEN
    auto converted = Internal::GetFleetRoleCredentialsAdapter::convert(request);
    // THEN
    EXPECT_EQ(request.GetRoleArn(), converted.GetRoleArn());
    EXPECT_EQ(request.GetRoleSessionName(), converted.GetRoleSessionName());
}

TEST_F(GetFleetRoleCredentialsAdapterTest, GIVEN_WebSocketGetFleetRoleCredentialsResponse_WHEN_convert_THEN_success) {
    const int64_t testNoStdExpirationInMillis = 1640995200000;
    const time_t testNoStdExpirationInSeconds = testNoStdExpirationInMillis / 1000;
    const std::tm testStdExpiration = {
        0,   // seconds after the minute - [0, 60] including leap second
        0,   // minutes after the hour - [0, 59]
        0,   // hours since midnight - [0, 23]
        1,   // day of the month - [1, 31]
        0,   // months since January - [0, 11]
        122, // years since 1900
        6,   // days since Sunday - [0, 6]
        0,   // days since January 1 - [0, 365]
        0    // daylight savings time flag
    };

    // GIVEN
    std::unique_ptr<WebSocketGetFleetRoleCredentialsResponse> response(new WebSocketGetFleetRoleCredentialsResponse());
    response->SetAssumedRoleUserArn("assumedRoleUserArn");
    response->SetAssumedRoleId("assumedRoleId");
    response->SetAccessKeyId("accessKeyId");
    response->SetSecretAccessKey("secretAccessKey");
    response->SetSessionToken("sessionToken");
    response->SetExpiration(testNoStdExpirationInMillis);
    // WHEN
    auto converted = Internal::GetFleetRoleCredentialsAdapter::convert(response.get());

    // THEN
    EXPECT_EQ(converted.GetAssumedUserRoleArn(), response->GetAssumedRoleUserArn());
    EXPECT_EQ(converted.GetAssumedRoleId(), response->GetAssumedRoleId());
    EXPECT_EQ(converted.GetAccessKeyId(), response->GetAccessKeyId());
    EXPECT_EQ(converted.GetSecretAccessKey(), response->GetSecretAccessKey());
    EXPECT_EQ(converted.GetSessionToken(), response->GetSessionToken());
#ifdef GAMELIFT_USE_STD
    std::tm convertedExpiration = converted.GetExpiration();
    EXPECT_EQ(convertedExpiration.tm_sec, testStdExpiration.tm_sec);
    EXPECT_EQ(convertedExpiration.tm_min, testStdExpiration.tm_min);
    EXPECT_EQ(convertedExpiration.tm_hour, testStdExpiration.tm_hour);
    EXPECT_EQ(convertedExpiration.tm_mday, testStdExpiration.tm_mday);
    EXPECT_EQ(convertedExpiration.tm_mon, testStdExpiration.tm_mon);
    EXPECT_EQ(convertedExpiration.tm_year, testStdExpiration.tm_year);
    EXPECT_EQ(convertedExpiration.tm_wday, testStdExpiration.tm_wday);
    EXPECT_EQ(convertedExpiration.tm_yday, testStdExpiration.tm_yday);
    EXPECT_EQ(convertedExpiration.tm_isdst, testStdExpiration.tm_isdst);
#else
    EXPECT_EQ(converted.GetExpiration(), testNoStdExpirationInSeconds);
#endif
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws