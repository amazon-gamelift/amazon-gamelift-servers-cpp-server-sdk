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

#if defined(_MSC_VER) && !defined(GAMELIFT_USE_STD)
#pragma warning(push)           // Save warning settings.
#pragma warning(disable : 4996) // Disable deprecated warning for strncpy
#endif

#include <aws/gamelift/common/GameLift_EXPORTS.h>

#ifndef GAMELIFT_USE_STD
#include <string.h>
#include <algorithm>
#endif

#ifdef GAMELIFT_USE_STD
#include <string>
#include <vector>
#endif

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {

enum class ContainerGroupType { GAME_SERVER, PER_INSTANCE };

#ifdef GAMELIFT_USE_STD

class AWS_GAMELIFT_API ContainerNetworkInfo {
public:
    ContainerNetworkInfo() : m_containerGroupType(ContainerGroupType::GAME_SERVER) {}
    ~ContainerNetworkInfo() {}

    ContainerNetworkInfo(const ContainerNetworkInfo &other)
        : m_containerName(other.m_containerName), m_containerId(other.m_containerId),
          m_ipAddress(other.m_ipAddress), m_containerGroupType(other.m_containerGroupType) {}

    ContainerNetworkInfo(ContainerNetworkInfo &&other) { *this = std::move(other); }

    ContainerNetworkInfo &operator=(const ContainerNetworkInfo &other) {
        m_containerName = other.m_containerName;
        m_containerId = other.m_containerId;
        m_ipAddress = other.m_ipAddress;
        m_containerGroupType = other.m_containerGroupType;
        return *this;
    }

    ContainerNetworkInfo &operator=(ContainerNetworkInfo &&other) {
        m_containerName = std::move(other.m_containerName);
        m_containerId = std::move(other.m_containerId);
        m_ipAddress = std::move(other.m_ipAddress);
        m_containerGroupType = other.m_containerGroupType;
        return *this;
    }

    inline const std::string &GetContainerName() const { return m_containerName; }
    inline void SetContainerName(const std::string &value) { m_containerName = value; }
    inline ContainerNetworkInfo &WithContainerName(const std::string &value) {
        SetContainerName(value);
        return *this;
    }

    inline const std::string &GetContainerId() const { return m_containerId; }
    inline void SetContainerId(const std::string &value) { m_containerId = value; }
    inline ContainerNetworkInfo &WithContainerId(const std::string &value) {
        SetContainerId(value);
        return *this;
    }

    inline const std::string &GetIpAddress() const { return m_ipAddress; }
    inline void SetIpAddress(const std::string &value) { m_ipAddress = value; }
    inline ContainerNetworkInfo &WithIpAddress(const std::string &value) {
        SetIpAddress(value);
        return *this;
    }

    inline ContainerGroupType GetContainerGroupType() const { return m_containerGroupType; }
    inline void SetContainerGroupType(ContainerGroupType value) { m_containerGroupType = value; }
    inline ContainerNetworkInfo &WithContainerGroupType(ContainerGroupType value) {
        SetContainerGroupType(value);
        return *this;
    }

private:
    std::string m_containerName;
    std::string m_containerId;
    std::string m_ipAddress;
    ContainerGroupType m_containerGroupType;
};

class AWS_GAMELIFT_API ListContainersNetworkInfoResult {
public:
    ListContainersNetworkInfoResult() {}
    ~ListContainersNetworkInfoResult() {}

    ListContainersNetworkInfoResult(const ListContainersNetworkInfoResult &other)
        : m_containersNetworkInfo(other.m_containersNetworkInfo) {}

    ListContainersNetworkInfoResult(ListContainersNetworkInfoResult &&other) { *this = std::move(other); }

    ListContainersNetworkInfoResult &operator=(const ListContainersNetworkInfoResult &other) {
        m_containersNetworkInfo = other.m_containersNetworkInfo;
        return *this;
    }

    ListContainersNetworkInfoResult &operator=(ListContainersNetworkInfoResult &&other) {
        m_containersNetworkInfo = std::move(other.m_containersNetworkInfo);
        return *this;
    }

    inline const std::vector<ContainerNetworkInfo> &GetContainersNetworkInfo() const { return m_containersNetworkInfo; }
    inline void SetContainersNetworkInfo(const std::vector<ContainerNetworkInfo> &value) { m_containersNetworkInfo = value; }
    inline void AddContainerNetworkInfo(const ContainerNetworkInfo &value) { m_containersNetworkInfo.push_back(value); }
    inline ListContainersNetworkInfoResult &WithContainersNetworkInfo(const std::vector<ContainerNetworkInfo> &value) {
        SetContainersNetworkInfo(value);
        return *this;
    }

private:
    std::vector<ContainerNetworkInfo> m_containersNetworkInfo;
};

#else

#ifndef MAX_CONTAINER_NAME_LENGTH
#define MAX_CONTAINER_NAME_LENGTH 128
#endif
#ifndef MAX_CONTAINER_ID_LENGTH
// Docker container IDs are SHA-256 digests rendered as 64 hex chars; +1 for null terminator.
#define MAX_CONTAINER_ID_LENGTH 65
#endif
#ifndef MAX_IP_ADDRESS_LENGTH
#define MAX_IP_ADDRESS_LENGTH 46
#endif
#ifndef MAX_CONTAINER_NETWORK_INFOS
#define MAX_CONTAINER_NETWORK_INFOS 1527
#endif

class AWS_GAMELIFT_API ContainerNetworkInfo {
public:
    ContainerNetworkInfo() : m_containerGroupType(ContainerGroupType::GAME_SERVER) {
        memset(m_containerName, 0, sizeof(m_containerName));
        memset(m_containerId, 0, sizeof(m_containerId));
        memset(m_ipAddress, 0, sizeof(m_ipAddress));
    }

    ~ContainerNetworkInfo() {}

    ContainerNetworkInfo(const ContainerNetworkInfo &other) : m_containerGroupType(other.m_containerGroupType) {
        strncpy(m_containerName, other.m_containerName, sizeof(m_containerName));
        strncpy(m_containerId, other.m_containerId, sizeof(m_containerId));
        strncpy(m_ipAddress, other.m_ipAddress, sizeof(m_ipAddress));
    }

    ContainerNetworkInfo &operator=(const ContainerNetworkInfo &other) {
        strncpy(m_containerName, other.m_containerName, sizeof(m_containerName));
        strncpy(m_containerId, other.m_containerId, sizeof(m_containerId));
        strncpy(m_ipAddress, other.m_ipAddress, sizeof(m_ipAddress));
        m_containerGroupType = other.m_containerGroupType;
        return *this;
    }

    inline const char *GetContainerName() const { return m_containerName; }
    inline void SetContainerName(const char *value) {
        strncpy(m_containerName, value, sizeof(m_containerName));
        m_containerName[sizeof(m_containerName) - 1] = '\0';
    }
    inline ContainerNetworkInfo &WithContainerName(const char *value) {
        SetContainerName(value);
        return *this;
    }

    inline const char *GetContainerId() const { return m_containerId; }
    inline void SetContainerId(const char *value) {
        strncpy(m_containerId, value, sizeof(m_containerId));
        m_containerId[sizeof(m_containerId) - 1] = '\0';
    }
    inline ContainerNetworkInfo &WithContainerId(const char *value) {
        SetContainerId(value);
        return *this;
    }

    inline const char *GetIpAddress() const { return m_ipAddress; }
    inline void SetIpAddress(const char *value) {
        strncpy(m_ipAddress, value, sizeof(m_ipAddress));
        m_ipAddress[sizeof(m_ipAddress) - 1] = '\0';
    }
    inline ContainerNetworkInfo &WithIpAddress(const char *value) {
        SetIpAddress(value);
        return *this;
    }

    inline ContainerGroupType GetContainerGroupType() const { return m_containerGroupType; }
    inline void SetContainerGroupType(ContainerGroupType value) { m_containerGroupType = value; }
    inline ContainerNetworkInfo &WithContainerGroupType(ContainerGroupType value) {
        SetContainerGroupType(value);
        return *this;
    }

private:
    char m_containerName[MAX_CONTAINER_NAME_LENGTH];
    char m_containerId[MAX_CONTAINER_ID_LENGTH];
    char m_ipAddress[MAX_IP_ADDRESS_LENGTH];
    ContainerGroupType m_containerGroupType;
};

class AWS_GAMELIFT_API ListContainersNetworkInfoResult {
public:
    ListContainersNetworkInfoResult() : m_containersNetworkInfo_count(0) {}
    ~ListContainersNetworkInfoResult() {}

    ListContainersNetworkInfoResult(const ListContainersNetworkInfoResult &other)
        : m_containersNetworkInfo_count(other.m_containersNetworkInfo_count) {
        std::copy(std::begin(other.m_containersNetworkInfo),
                  std::begin(other.m_containersNetworkInfo) + other.m_containersNetworkInfo_count,
                  std::begin(m_containersNetworkInfo));
    }

    ListContainersNetworkInfoResult &operator=(const ListContainersNetworkInfoResult &other) {
        m_containersNetworkInfo_count = other.m_containersNetworkInfo_count;
        std::copy(std::begin(other.m_containersNetworkInfo),
                  std::begin(other.m_containersNetworkInfo) + other.m_containersNetworkInfo_count,
                  std::begin(m_containersNetworkInfo));
        return *this;
    }

    inline const ContainerNetworkInfo *GetContainersNetworkInfo() const { return m_containersNetworkInfo; }
    inline int GetContainersNetworkInfoCount() const { return m_containersNetworkInfo_count; }
    inline void AddContainerNetworkInfo(const ContainerNetworkInfo &value) {
        if (m_containersNetworkInfo_count < MAX_CONTAINER_NETWORK_INFOS) {
            m_containersNetworkInfo[m_containersNetworkInfo_count++] = value;
        }
    }

private:
    ContainerNetworkInfo m_containersNetworkInfo[MAX_CONTAINER_NETWORK_INFOS];
    int m_containersNetworkInfo_count;
};

#endif

} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws

#if defined(_MSC_VER) && !defined(GAMELIFT_USE_STD)
#pragma warning(pop) // Restore warnings to previous state.
#endif
