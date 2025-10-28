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

#include <aws/gamelift/common/GameLift_EXPORTS.h>
#include <string.h>

#ifndef GAMELIFT_USE_STD
#ifndef MAX_STATSD_HOST_LENGTH
#define MAX_STATSD_HOST_LENGTH 256
#endif
#ifndef MAX_CRASH_REPORTER_HOST_LENGTH
#define MAX_CRASH_REPORTER_HOST_LENGTH 256
#endif
#endif

namespace Aws {
namespace GameLift {
namespace Server {

class AWS_GAMELIFT_API MetricsParameters {
#ifdef GAMELIFT_USE_STD
public:
    MetricsParameters(const std::string &statsDHost, int statsDPort, const std::string &crashReporterHost, 
                     int crashReporterPort, int flushIntervalMs, int maxPacketSize)
        : m_statsDHost(statsDHost), m_statsDPort(statsDPort), m_crashReporterHost(crashReporterHost), 
          m_crashReporterPort(crashReporterPort), m_flushIntervalMs(flushIntervalMs), m_maxPacketSize(maxPacketSize) {}

    MetricsParameters(const MetricsParameters &) = default;
    MetricsParameters(MetricsParameters &&) = default;
    MetricsParameters &operator=(const MetricsParameters &) = default;
    MetricsParameters &operator=(MetricsParameters &&) = default;

    inline const std::string &GetStatsDHost() const { return m_statsDHost; }
    inline int GetStatsDPort() const { return m_statsDPort; }
    inline int GetFlushIntervalMs() const { return m_flushIntervalMs; }
    inline int GetMaxPacketSize() const { return m_maxPacketSize; }
    inline const std::string &GetCrashReporterHost() const { return m_crashReporterHost; }
    inline int GetCrashReporterPort() const { return m_crashReporterPort; }

    inline void SetStatsDHost(const std::string &statsDHost) { m_statsDHost = statsDHost; }
    inline void SetStatsDPort(int statsDPort) { m_statsDPort = statsDPort; }
    inline void SetFlushIntervalMs(int flushIntervalMs) { m_flushIntervalMs = flushIntervalMs; }
    inline void SetMaxPacketSize(int maxPacketSize) { m_maxPacketSize = maxPacketSize; }
    inline void SetCrashReporterHost(const std::string &crashReporterHost) { m_crashReporterHost = crashReporterHost; }
    inline void SetCrashReporterPort(int crashReporterPort) { m_crashReporterPort = crashReporterPort; }

private:
    std::string m_statsDHost;
    int m_statsDPort;
    std::string m_crashReporterHost;
    int m_crashReporterPort;
    int m_flushIntervalMs;
    int m_maxPacketSize;

#else
public:
    MetricsParameters(const char *statsDHost, int statsDPort, const char *crashReporterHost, 
                     int crashReporterPort, int flushIntervalMs, int maxPacketSize) 
        : m_statsDPort(statsDPort), m_crashReporterPort(crashReporterPort), m_flushIntervalMs(flushIntervalMs), m_maxPacketSize(maxPacketSize) {
        if (statsDHost != nullptr) {
            strncpy(m_statsDHost, statsDHost, MAX_STATSD_HOST_LENGTH - 1);
            m_statsDHost[MAX_STATSD_HOST_LENGTH - 1] = '\0';
        }
        if (crashReporterHost != nullptr) {
            strncpy(m_crashReporterHost, crashReporterHost, MAX_CRASH_REPORTER_HOST_LENGTH - 1);
            m_crashReporterHost[MAX_CRASH_REPORTER_HOST_LENGTH - 1] = '\0';
        }
    }

    MetricsParameters(const MetricsParameters &other) : m_statsDPort(other.m_statsDPort), m_crashReporterPort(other.m_crashReporterPort), 
                                                       m_flushIntervalMs(other.m_flushIntervalMs), m_maxPacketSize(other.m_maxPacketSize) {
        strncpy(m_statsDHost, other.m_statsDHost, MAX_STATSD_HOST_LENGTH - 1);
        m_statsDHost[MAX_STATSD_HOST_LENGTH - 1] = '\0';
        strncpy(m_crashReporterHost, other.m_crashReporterHost, MAX_CRASH_REPORTER_HOST_LENGTH - 1);
        m_crashReporterHost[MAX_CRASH_REPORTER_HOST_LENGTH - 1] = '\0';
    }

    MetricsParameters &operator=(const MetricsParameters &other) {
        if (this != &other) {
            m_statsDPort = other.m_statsDPort;
            m_flushIntervalMs = other.m_flushIntervalMs;
            m_maxPacketSize = other.m_maxPacketSize;
            m_crashReporterPort = other.m_crashReporterPort;
            strncpy(m_statsDHost, other.m_statsDHost, MAX_STATSD_HOST_LENGTH - 1);
            m_statsDHost[MAX_STATSD_HOST_LENGTH - 1] = '\0';
            strncpy(m_crashReporterHost, other.m_crashReporterHost, MAX_CRASH_REPORTER_HOST_LENGTH - 1);
            m_crashReporterHost[MAX_CRASH_REPORTER_HOST_LENGTH - 1] = '\0';
        }
        return *this;
    }

    inline const char *GetStatsDHost() const { return m_statsDHost; }
    inline int GetStatsDPort() const { return m_statsDPort; }
    inline int GetFlushIntervalMs() const { return m_flushIntervalMs; }
    inline int GetMaxPacketSize() const { return m_maxPacketSize; }
    inline const char *GetCrashReporterHost() const { return m_crashReporterHost; }
    inline int GetCrashReporterPort() const { return m_crashReporterPort; }

    inline void SetStatsDHost(const char *statsDHost) {
        if (statsDHost != nullptr) {
            strncpy(m_statsDHost, statsDHost, MAX_STATSD_HOST_LENGTH - 1);
            m_statsDHost[MAX_STATSD_HOST_LENGTH - 1] = '\0';
        }
    }
    inline void SetStatsDPort(int statsDPort) { m_statsDPort = statsDPort; }
    inline void SetFlushIntervalMs(int flushIntervalMs) { m_flushIntervalMs = flushIntervalMs; }
    inline void SetMaxPacketSize(int maxPacketSize) { m_maxPacketSize = maxPacketSize; }
    inline void SetCrashReporterHost(const char *crashReporterHost) {
        if (crashReporterHost != nullptr) {
            strncpy(m_crashReporterHost, crashReporterHost, MAX_CRASH_REPORTER_HOST_LENGTH - 1);
            m_crashReporterHost[MAX_CRASH_REPORTER_HOST_LENGTH - 1] = '\0';
        }
    }
    inline void SetCrashReporterPort(int crashReporterPort) { m_crashReporterPort = crashReporterPort; }

private:
    char m_statsDHost[MAX_STATSD_HOST_LENGTH];
    int m_statsDPort;
    char m_crashReporterHost[MAX_CRASH_REPORTER_HOST_LENGTH];
    int m_crashReporterPort;
    int m_flushIntervalMs;
    int m_maxPacketSize;
#endif
};

} // namespace Server
} // namespace GameLift
} // namespace Aws
