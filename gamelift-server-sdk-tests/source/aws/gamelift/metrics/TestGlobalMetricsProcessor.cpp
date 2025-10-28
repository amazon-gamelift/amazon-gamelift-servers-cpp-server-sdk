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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>

#include <aws/gamelift/metrics/GlobalMetricsProcessor.h>
#include <aws/gamelift/metrics/MetricsSettings.h>
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Platform.h>
#include <aws/gamelift/metrics/Samplers.h>

using namespace ::testing;
using namespace Aws::GameLift::Metrics;

// Mock platform
GAMELIFT_METRICS_DEFINE_PLATFORM(MockEnabled, true);

// Define a test metric for our tests
namespace
{
    GAMELIFT_METRICS_DECLARE_GAUGE(TestMetric, "test_metric", MockEnabled,
                                   Aws::GameLift::Metrics::SampleAll());
    GAMELIFT_METRICS_DEFINE_GAUGE(TestMetric);
}

class GlobalMetricsProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Save original environment variable if it exists
        const char *originalProcessId = std::getenv(ENV_VAR_PROCESS_ID);
        if (originalProcessId)
        {
            originalProcessIdValue = originalProcessId;
            hadOriginalProcessId = true;
        }
        else
        {
            hadOriginalProcessId = false;
        }
    }

    void TearDown() override
    {
        // Restore original environment variable
        if (hadOriginalProcessId)
        {
            SetEnv(ENV_VAR_PROCESS_ID, originalProcessIdValue.c_str());
        }
        else
        {
            UnsetEnv(ENV_VAR_PROCESS_ID);
        }
    }

    void SetProcessIdEnvVar(const std::string &value)
    {
        SetEnv(ENV_VAR_PROCESS_ID, value.c_str());
    }

    void UnsetProcessIdEnvVar()
    {
        UnsetEnv(ENV_VAR_PROCESS_ID);
    }

    void SetEnv(const char *key, const char *val) {
        if (val == nullptr) {
#ifdef WIN32
            _putenv_s(key, "");
#else
            unsetenv(key);
#endif
        } else {
#ifdef WIN32
            _putenv_s(key, val);
#else
            setenv(key, val, 1);
#endif
        }
    }

    void UnsetEnv(const char *key) {
#ifdef WIN32
        _putenv_s(key, "");
#else
        unsetenv(key);
#endif
    }

    MetricsSettings CreateTestMetricsSettings() {
        MetricsSettings settings;
        // Disable StatsDClient for tests
        settings.StatsDClientHost = "";
        // Disable crash reporter for tests
        settings.CrashReporterHost = "";
        settings.StatsDClientPort = 0;
        settings.CrashReporterPort = 0;
        return settings;
    }

private:
    std::string originalProcessIdValue;
    bool hadOriginalProcessId = false;
};

TEST_F(GlobalMetricsProcessorTest, InitializeDefaultGlobalTags_WithProcessIdEnvVar_SetsGameliftProcessIdTag)
{
    // Arrange
    const std::string testProcessId = "test-process-123";
    SetProcessIdEnvVar(testProcessId);

    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    // Capture the packet that is sent - use a larger max packet size to ensure everything fits
    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0; // Process immediately
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    // Get the global processor and submit a metric to trigger packet generation
    auto *processor = GameLiftMetricsGlobalProcessor();
    ASSERT_NE(processor, nullptr);

    processor->Enqueue(MetricMessage::GaugeSet(TestMetric::Instance(), 42.0));

    // Process metrics until we get a packet
    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Check if the gamelift_process_id tag appears in the packet
    EXPECT_THAT(capturedPacket, HasSubstr("gamelift_process_id:" + testProcessId))
        << "Expected gamelift_process_id tag with value '" << testProcessId
        << "' not found in packet: " << capturedPacket;

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}

TEST_F(GlobalMetricsProcessorTest, InitializeDefaultGlobalTags_WithoutProcessIdEnvVar_DoesNotSetGameliftProcessIdTag)
{
    // Arrange
    UnsetProcessIdEnvVar();

    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    auto *processor = GameLiftMetricsGlobalProcessor();
    ASSERT_NE(processor, nullptr);

    processor->Enqueue(MetricMessage::GaugeSet(TestMetric::Instance(), 42.0));

    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Check that gamelift_process_id tag does not appear in the packet
    EXPECT_THAT(capturedPacket, Not(HasSubstr("gamelift_process_id")))
        << "gamelift_process_id tag should not be present in packet: " << capturedPacket;

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}

#ifdef __linux__
TEST_F(GlobalMetricsProcessorTest, InitializeDefaultGlobalTags_OnLinux_SetsProcessPidTag)
{
    // Arrange
    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    auto *processor = GameLiftMetricsGlobalProcessor();
    ASSERT_NE(processor, nullptr);

    processor->Enqueue(MetricMessage::GaugeSet(TestMetric::Instance(), 42.0));

    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Check if the process_pid tag appears in the packet
    EXPECT_THAT(capturedPacket, HasSubstr("process_pid:"))
        << "process_pid tag not found in packet: " << capturedPacket;

    // Also verify the PID value is numeric
    size_t pidPos = capturedPacket.find("process_pid:");
    if (pidPos != std::string::npos)
    {
        size_t valueStart = pidPos + 12; // length of "process_pid:"
        size_t valueEnd = capturedPacket.find_first_of(",|#\n", valueStart);
        if (valueEnd == std::string::npos)
            valueEnd = capturedPacket.length();

        std::string pidValue = capturedPacket.substr(valueStart, valueEnd - valueStart);
        // PID should be a positive integer
        int pid = std::stoi(pidValue);
        EXPECT_GT(pid, 0) << "PID should be positive, got: " << pidValue;
    }

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}
#endif

TEST_F(GlobalMetricsProcessorTest, InitializeDefaultGlobalTags_WithBothConditions_SetsBothTags)
{
    // Arrange
    const std::string testProcessId = "test-process-456";
    SetProcessIdEnvVar(testProcessId);

    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    auto *processor = GameLiftMetricsGlobalProcessor();
    ASSERT_NE(processor, nullptr);

    processor->Enqueue(MetricMessage::GaugeSet(TestMetric::Instance(), 42.0));

    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Check that both tags appear in the packet
    EXPECT_THAT(capturedPacket, HasSubstr("gamelift_process_id:" + testProcessId))
        << "gamelift_process_id tag not found in packet: " << capturedPacket;

#ifdef __linux__
    EXPECT_THAT(capturedPacket, HasSubstr("process_pid:"))
        << "process_pid tag not found in packet: " << capturedPacket;
#endif

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}

TEST_F(GlobalMetricsProcessorTest, InitializeDefaultGlobalTags_WithEmptyProcessIdEnvVar_DoesNotSetGameliftProcessIdTag)
{
    // Arrange
    SetProcessIdEnvVar(""); // Empty string

    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    auto *processor = GameLiftMetricsGlobalProcessor();
    ASSERT_NE(processor, nullptr);

    processor->Enqueue(MetricMessage::GaugeSet(TestMetric::Instance(), 42.0));

    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Empty string should not set the tag
    EXPECT_THAT(capturedPacket, Not(HasSubstr("gamelift_process_id")))
        << "gamelift_process_id tag should not be present with empty env var in packet: " << capturedPacket;

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}

TEST_F(GlobalMetricsProcessorTest, MetricsInitialize_SetsServerUpGaugeToOne)
{
    // Arrange
    MetricsSettings settings = CreateTestMetricsSettings();
    std::string capturedPacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&capturedPacket](const char *packet, int size)
    {
        capturedPacket = std::string(packet, size);
    };

    // Act
    MetricsInitialize(settings);

    // Process metrics to capture the ServerUpGauge
    while (capturedPacket.empty())
    {
        MetricsProcess();
    }

    // Assert - Check that ServerUpGauge is set to 1
    EXPECT_THAT(capturedPacket, HasSubstr("server_up:1"))
        << "ServerUpGauge should be set to 1 in packet: " << capturedPacket;

    // Manually terminate rather than in TearDown to avoid race conditions
    // between tests.
    MetricsTerminate();
}

TEST_F(GlobalMetricsProcessorTest, MetricsTerminate_SetsServerUpGaugeToZero)
{
    // Arrange
    MetricsSettings settings = CreateTestMetricsSettings();
    std::string initPacket, terminatePacket;

    settings.MaxPacketSizeBytes = 4000;
    settings.CaptureIntervalSec = 0;
    settings.SendPacketCallback = [&initPacket, &terminatePacket](const char *packet, int size)
    {
        std::string packetStr(packet, size);
        if (initPacket.empty())
        {
            initPacket = packetStr;
        }
        else
        {
            terminatePacket = packetStr;
        }
    };

    // Act
    MetricsInitialize(settings);

    // Process to get initial packet
    while (initPacket.empty())
    {
        MetricsProcess();
    }

    // Manually terminate rather than in TearDown to ensure we can capture
    // the termination packet and avoid race conditions between tests.
    MetricsTerminate();

    // Assert
    EXPECT_THAT(initPacket, HasSubstr("server_up:1"))
        << "ServerUpGauge should be set to 1 initially in packet: " << initPacket;

    EXPECT_THAT(terminatePacket, HasSubstr("server_up:0"))
        << "ServerUpGauge should be set to 0 on termination in packet: " << terminatePacket;
}
