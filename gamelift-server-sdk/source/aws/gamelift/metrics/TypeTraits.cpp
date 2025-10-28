/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates
 * or its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution (the "License"). All use of this software is governed by
 * the License, or, if provided, by the license below or the license
 * accompanying this file. Do not remove or modify any license notices. This
 * file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied.
 *
 */
#include <aws/gamelift/metrics/TypeTraits.h>

#ifndef GAMELIFT_USE_STD
namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {
const bool TrueType::value;
const bool FalseType::value;
} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#endif // !GAMELIFT_USE_STD
