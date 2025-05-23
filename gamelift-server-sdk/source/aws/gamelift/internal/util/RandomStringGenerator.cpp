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

#include <aws/gamelift/internal/util/RandomStringGenerator.h>
#include <random>
#include <sstream>

namespace Aws {
namespace GameLift {
namespace Internal {

std::string RandomStringGenerator::GenerateRandomAlphaNumericString(const int stringLength) {
    std::stringstream ss;
    static const char alphaNumChars[] = "0123456789"
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz";
    static thread_local std::mt19937 randomNumberGenerator(std::random_device{}());
    // Distribution has an inclusive upper bound. "alphaNumChars" will end in a null terminator, so
    // we use "sizeof() - 2" to avoid out of bounds errors, and also to avoid including the null
    // terminator.
    std::uniform_int_distribution<int> distribution(0, sizeof(alphaNumChars) - 2);

    for (int i = 0; i < stringLength; i++) {
        ss << alphaNumChars[distribution(randomNumberGenerator)];
    }
    return ss.str();
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws