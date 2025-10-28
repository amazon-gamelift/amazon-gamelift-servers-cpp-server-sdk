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
#include <aws/gamelift/metrics/StatsDClient.h>
#include <aws/gamelift/metrics/LoggerMacros.h>

using namespace Aws::GameLift::Metrics;

#ifdef GAMELIFT_USE_STD
StatsDClient::StatsDClient(const std::string& host, int port) 
#else
StatsDClient::StatsDClient(const char* host, int port) 
#endif
    : m_socket(m_io_service)
    , m_endpoint(asio::ip::address::from_string(host), port) {
    
    try {
        m_socket.open(asio::ip::udp::v4());
    } catch (const std::exception& e) {
        GAMELIFT_METRICS_LOG_ERROR("Failed to open StatsD socket: {}", e.what());
    }
}

void StatsDClient::Send(const char* data, int size) {
    try {
        m_socket.send_to(asio::buffer(data, size), m_endpoint);
    } catch (const std::exception& e) {
        GAMELIFT_METRICS_LOG_ERROR("Failed to send StatsD packet: {}", e.what());
    }
}
