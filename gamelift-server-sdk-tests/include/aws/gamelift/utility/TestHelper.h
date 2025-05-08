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

#pragma once

#include "gtest/gtest.h"
#include <stdlib.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Utility {

class TestHelper {
public:
    static void AssertStringsEqual(const std::string &string1, const std::string &string2) { ASSERT_EQ(string1, string2); }

    static void AssertStringsEqual(const char *string1, const char *string2) { ASSERT_STREQ(string1, string2); }

    static void AssertStringsEqual(const std::string &string1, const char *string2) { ASSERT_STREQ(string1.c_str(), string2); }

    static void AssertStringsEqual(const char *string1, const std::string &string2) { ASSERT_STREQ(string1, string2.c_str()); }

    static std::string GenerateRandomAlphaNumericString(int stringLength) {
        std::string s;
        static const char alphaNumChars[] = "0123456789"
                                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < stringLength; i++) {
            s += alphaNumChars[rand() % (sizeof(alphaNumChars) - 1)];
        }
        return s;
    }
};

} // namespace Utility
} // namespace Server
} // namespace GameLift
} // namespace Aws