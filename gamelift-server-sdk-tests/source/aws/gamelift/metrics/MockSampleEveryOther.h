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

#include <aws/gamelift/metrics/Samplers.h>

extern thread_local int MockSamplerCounter;

class MockSampleEveryOther : public Aws::GameLift::Metrics::ISampler {
public:
  bool ShouldTakeSample() {
    ++MockSamplerCounter;
    return MockSamplerCounter % 2 == 0;
  }
  float GetSampleRate() const override {
    return 0.5f;
  }
};

void ResetMockSampler();
