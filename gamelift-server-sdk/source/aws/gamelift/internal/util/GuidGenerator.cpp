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

#include <aws/gamelift/internal/util/GuidGenerator.h>
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

using namespace Aws::GameLift::Internal;

std::string GuidGenerator::GenerateGuid() {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 15);
    std::uniform_int_distribution<int> dist2(8, 11);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 8; ++i) ss << dist(rd);
    ss << "-";

    for (int i = 0; i < 4; ++i) ss << dist(rd);
    ss << "-";

    ss << "4";
    for (int i = 0; i < 3; ++i) ss << dist(rd);
    ss << "-";

    ss << dist2(rd);
    for (int i = 0; i < 3; ++i) ss << dist(rd);
    ss << "-";

    for (int i = 0; i < 12; ++i) ss << dist(rd);

    return ss.str();
}
