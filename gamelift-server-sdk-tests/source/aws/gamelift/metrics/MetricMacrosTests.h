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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <aws/gamelift/metrics/Tags.h>
#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/Platform.h>

#include "MockSampleEveryOther.h"

// Mock platform
GAMELIFT_METRICS_DEFINE_PLATFORM(MockEnabled, true);
GAMELIFT_METRICS_DEFINE_PLATFORM(MockDisabled, false);

#define GAMELIFT_METRICS_PLATFORM_IS_ENABLED(platform) platform::bEnabled

// Mock processor
class MockMetricsProcessor : public IMetricsProcessor {
public:
  MOCK_METHOD(void, Enqueue, (MetricMessage Message), (override));
  MOCK_METHOD(void, ProcessMetrics, (), (override));
  MOCK_METHOD(void, ProcessMetricsNow, (), (override));
  MOCK_METHOD(void, OnStartGameSession, (const Aws::GameLift::Server::Model::GameSession &), (override));

#ifdef GAMELIFT_USE_STD
  MOCK_METHOD(void, SetGlobalTag, (const std::string &, const std::string &),
              (override));
  MOCK_METHOD(void, RemoveGlobalTag, (const std::string &), (override));
#else
  MOCK_METHOD(void, SetGlobalTag, (const char *, const char *), (override));
  MOCK_METHOD(void, RemoveGlobalTag, (const char *), (override));
#endif
};

/**
 * Base fixture for macro tests.
 */
class MetricMacrosTests : public ::testing::Test {
public:
  MockMetricsProcessor MockProcessor;

  virtual void SetUp() override {
    using namespace ::testing;

    /* If we're running in a single process, the counter won't be cleared between tests
        so we're clearing it in advance. Note that if the tests are run parallel this
        might result in the counter clearing in the middle of a test. */
    ResetMockSampler();

    ON_CALL(MockProcessor, Enqueue(_)).WillByDefault([this](MetricMessage Message) {
      if (Message.IsTag()) {
        // We handle 'tags' to ensure they're deleted.
        static Tags Handler;
        Handler.Handle(Message);
      }
    });
  }

  /*
   * This function mocks the global GameLiftMetricsGlobalProcessor function
   * called internally by the macros.
   */
  virtual IMetricsProcessor *GameLiftMetricsGlobalProcessor() {
    return &MockProcessor;
  }
};
