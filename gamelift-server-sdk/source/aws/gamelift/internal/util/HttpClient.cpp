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
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    using ssize_t = SSIZE_T;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>
#endif
#include <stdexcept>
#include <aws/gamelift/internal/util/HttpClient.h>
#include <spdlog/spdlog.h>

using namespace Aws::GameLift::Internal;

bool HttpResponse::IsSuccessfulStatusCode() {
    return statusCode >= 200 && statusCode <= 299;
}

HttpResponse HttpClient::SendGetRequest(const std::string &url) {
    const std::tuple<std::string, int, std::string> hostAndPortAndPath = GetHostAndPortAndPath(url);
    const std::string host = std::get<0>(hostAndPortAndPath);
    const int port = std::get<1>(hostAndPortAndPath);
    const std::string path = std::get<2>(hostAndPortAndPath);
    const std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    int sock = -1;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed, error number: " + std::to_string(WSAGetLastError()));
    }
#endif

    try {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
#ifdef _WIN32
            throw std::runtime_error("Socket creation failed, error number: " + std::to_string(WSAGetLastError()));
#else
            throw std::runtime_error("Socket creation failed, error number: " + std::string(strerror(errno)));
#endif
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

#ifdef _WIN32
        if (InetPton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address or address not supported, error number: " + std::to_string(WSAGetLastError()));
        }
#else
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address or address not supported, error number: " + std::string(strerror(errno)));
        }
#endif

        if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
            throw std::runtime_error("Connection failed, error number: " + std::to_string(WSAGetLastError()));
#else
            throw std::runtime_error("Connection failed, error number: " + std::string(strerror(errno)));
#endif
        }

        ssize_t sent = send(sock, request.c_str(), request.length(), 0);
        if (sent < 0) {
#ifdef _WIN32
            throw std::runtime_error("Send failed, error number: " + std::to_string(WSAGetLastError()));
#else
            throw std::runtime_error("Send failed, error number: " + std::string(strerror(errno)));
#endif
        } else if (sent != (ssize_t) request.length()) {
            throw std::runtime_error("Send incomplete, only " + std::to_string(sent) + " bytes sent.");
        }

        std::string fullResponse;
        char buffer[1024] = {0};
        ssize_t bytesReceived;
        while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            fullResponse.append(buffer, bytesReceived);
        }

        if (bytesReceived < 0) {
#ifdef _WIN32
            throw std::runtime_error("Receive failed, error number: " + std::to_string(WSAGetLastError()));
#else
            throw std::runtime_error("Receive failed, error number: " + std::string(strerror(errno)));
#endif
        }

#ifdef _WIN32
        if (closesocket(sock) < 0) {
            spdlog::warn("Socket close failed, error number: {}", WSAGetLastError());
        }
        WSACleanup();
#else
        if (close(sock) < 0) {
            spdlog::warn("Socket close failed, error number: {}", errno);
        }
#endif
        sock = -1;

        HttpResponse httpResponse = ParseHttpResponse(fullResponse);
        return httpResponse;
    } catch (const std::runtime_error& e) {
        if (sock >= 0) {
#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
        }
        throw e;
    }
}

std::tuple<std::string, int, std::string> HttpClient::GetHostAndPortAndPath(const std::string &url) {
    std::string host;
    std::string path;
    int port = 80;

    std::size_t protocolPos = url.find("://");
    if (protocolPos == std::string::npos) {
        throw std::runtime_error("Invalid URL: Missing protocol");
    }

    std::size_t hostStart = protocolPos + 3;

    if (hostStart >= url.size()) {
        throw std::runtime_error("Invalid URL: Host is missing");
    }

    std::size_t pathPos = url.find('/', hostStart);
    std::size_t portPos = url.find(':', hostStart);

    if (portPos != std::string::npos && (pathPos == std::string::npos || portPos < pathPos)) {
        host = url.substr(hostStart, portPos - hostStart);
        port = std::stoi(
                url.substr(portPos + 1, (pathPos == std::string::npos ? url.size() : pathPos) - portPos - 1));
    } else {
        if (pathPos == std::string::npos) {
            host = url.substr(hostStart);
            path = "/";
        } else {
            host = url.substr(hostStart, pathPos - hostStart);
            path = url.substr(pathPos);
        }
    }

    return std::make_tuple(host, port, path);
}

HttpResponse HttpClient::ParseHttpResponse(const std::string &response) {
    HttpResponse httpResponse;
    std::size_t statusCodeStart = response.find("HTTP/1.1 ") + 9;
    std::size_t statusCodeEnd = response.find(" ", statusCodeStart);
    if (statusCodeStart != std::string::npos && statusCodeEnd != std::string::npos) {
        httpResponse.statusCode = std::stoi(response.substr(statusCodeStart, statusCodeEnd - statusCodeStart));
    }
    std::size_t bodyStart = response.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        httpResponse.body = response.substr(bodyStart + 4);
    }
    if (response.find("Transfer-Encoding: chunked") != std::string::npos
        && response.find("Content-Type: application/json") != std::string::npos) {
        int jsonStart = httpResponse.body.find("{");
        int jsonEnd = httpResponse.body.find_last_of("}");
        if (jsonStart != std::string::npos && jsonEnd != std::string::npos) {
            httpResponse.body = httpResponse.body.substr(jsonStart, jsonEnd - jsonStart + 2);
        }
    }
    return httpResponse;
}
