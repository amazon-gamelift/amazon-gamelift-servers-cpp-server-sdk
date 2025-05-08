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

#include <aws/gamelift/internal/model/response/WebSocketStartMatchBackfillResponse.h>
#include <aws/gamelift/internal/network/callback/StartMatchBackfillCallback.h>

using namespace Aws::GameLift;

namespace Aws {
namespace GameLift {
namespace Internal {
GenericOutcome StartMatchBackfillCallback::OnStartMatchBackfill(const std::string &data) {
    WebSocketStartMatchBackfillResponse *startMatchBackfillResponse = new WebSocketStartMatchBackfillResponse();
    Message *message = startMatchBackfillResponse;
    message->Deserialize(data);

    return GenericOutcome(startMatchBackfillResponse);
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws