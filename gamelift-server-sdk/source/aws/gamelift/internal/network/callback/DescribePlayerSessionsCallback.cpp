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

#include <aws/gamelift/internal/model/response/WebSocketDescribePlayerSessionsResponse.h>
#include <aws/gamelift/internal/network/callback/DescribePlayerSessionsCallback.h>
#include <spdlog/spdlog.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
GenericOutcome DescribePlayerSessionsCallback::OnDescribePlayerSessions(const std::string &data) {
    spdlog::info("OnDescribePlayerSessions Received with raw data: {}", data);
    WebSocketDescribePlayerSessionsResponse *describePlayerSessionsResponse = new WebSocketDescribePlayerSessionsResponse();
    Message *message = describePlayerSessionsResponse;
    message->Deserialize(data);

    return GenericOutcome(describePlayerSessionsResponse);
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws