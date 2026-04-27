/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
#pragma once

#include <cstdlib>
#include <string>
#ifdef _MSC_VER
#include <memory>
#endif

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Utils {

/**
 * Platform-safe wrapper for reading environment variables.
 * Uses _dupenv_s on MSVC (avoids C4996 warning) with RAII cleanup.
 * Uses std::getenv on other platforms.
 * Returns empty string if the variable is not set.
 */
inline std::string SafeGetenv(const char* name) {
#ifdef _MSC_VER
    char* val = nullptr;
    size_t len = 0;
    _dupenv_s(&val, &len, name);
    std::unique_ptr<char, decltype(&free)> guard(val, free);
    return val ? std::string(val) : std::string();
#else
    const char* val = std::getenv(name);
    return val ? std::string(val) : std::string();
#endif
}

} // namespace Utils
} // namespace Internal
} // namespace GameLift
} // namespace Aws
