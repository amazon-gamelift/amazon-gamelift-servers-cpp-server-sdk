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

#include <asio.hpp>

#ifdef GAMELIFT_USE_STD
#include <string>
#endif

namespace Aws {
namespace GameLift {
namespace Metrics {

class StatsDClient {
public:
#ifdef GAMELIFT_USE_STD
    StatsDClient(const std::string& host, int port);
#else
    StatsDClient(const char* host, int port);
#endif
    ~StatsDClient() = default;

    void Send(const char* data, int size);

private:
    asio::io_service m_io_service;
    asio::ip::udp::socket m_socket;
    asio::ip::udp::endpoint m_endpoint;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
