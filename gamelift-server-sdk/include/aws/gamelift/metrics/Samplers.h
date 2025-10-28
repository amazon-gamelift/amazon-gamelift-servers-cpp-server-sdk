/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates
 * or its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution (the "License"). All use of this software is governed by
 * the License, or, if provided, by the license below or the license
 * accompanying this file. Do not remove or modify any license notices. This
 * file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied.
 *
 */
#pragma once

#include <aws/gamelift/metrics/Defs.h>
#include <aws/gamelift/metrics/InternalTypes.h>
#include <aws/gamelift/metrics/UniquePtr.h>

namespace Aws {
namespace GameLift {
namespace Metrics {

struct GAMELIFT_METRICS_API ISampler {
  virtual ~ISampler();

  /**
   * @brief Checks if we want to take this sample and updates internal state (if
   * any).
   *
   * @return true if this sample should be recorded, false otherwise
   */
  virtual bool ShouldTakeSample() = 0;

  /**
   * @brief Gets the sample rate for this sampler
   * @return The sample rate as a float between 0.0 and 1.0 inclusive
   */
  virtual float GetSampleRate() const = 0;
};

/**
 * Takes every sample.
 */
class GAMELIFT_METRICS_API SampleAll : public ISampler {
public:
  bool ShouldTakeSample() { return true; }
  float GetSampleRate() const override { return 1.0f; }
};

/**
 * Takes no samples.
 */
class GAMELIFT_METRICS_API SampleNone : public ISampler {
public:
  bool ShouldTakeSample() { return false; }
  float GetSampleRate() const override { return 0.0f; }
};

namespace Internal {
/**
 * Opaque wrapper around seed initialization state for per-thread RNG.
 */
struct SeedState;
} // namespace Internal

/**
 * Takes a random fraction of all samples.
 */
class GAMELIFT_METRICS_API SampleFraction : public ISampler {
public:
  /**
   * @param fractionToSample The fraction of samples to take. e.g. 0.90 will
   * take 90% of all samples.
   *
   * Uses current time since epoch as default seed for the random number
   * generators.
   */
  explicit SampleFraction(double fractionToSample) noexcept
      : SampleFraction(fractionToSample, DefaultSeed()) {}

  /**
   * @param fractionToSample The fraction of samples to take. e.g. 0.90 will
   * take 90% of all samples.
   * @param seed The integer seed for random number generators.
   */
  SampleFraction(double fractionToSample, Int64 seed)
      : SampleFraction(static_cast<float>(fractionToSample), seed) {}

  /**
   * @param fractionToSample The fraction of samples to take. e.g. 0.90 will
   * take 90% of all samples.
   * @param seed The integer seed for random number generators.
   */
  SampleFraction(float fractionToSample, Int64 seed);

  SampleFraction(const SampleFraction &) = delete;
  SampleFraction &operator=(const SampleFraction &) = delete;

  SampleFraction(SampleFraction &&other)
      : m_fractionToSample(other.m_fractionToSample), m_seed(other.m_seed) {
    other.m_seed = nullptr;
  }

  SampleFraction &operator=(SampleFraction &&other) {
    m_fractionToSample = other.m_fractionToSample;
    m_seed = other.m_seed;
    other.m_seed = nullptr;
    return *this;
  }

  ~SampleFraction();

  virtual bool ShouldTakeSample() override;

  /**
   * @brief Gets the sample rate for this sampler
   * @return The fraction value between 0.0 and 1.0
   */
  float GetSampleRate() const override { return m_fractionToSample; }

  /**
   * @brief Gets the fraction of samples to take
   * @return The fraction value between 0.0 and 1.0
   */
  float GetFractionToSample() const { return m_fractionToSample; }

  /**
   * @brief Sets the fraction of samples to take
   * @param fractionToSample The fraction value between 0.0 and 1.0
   */
  void SetFractionToSample(float fractionToSample) {
    m_fractionToSample = fractionToSample;
  }

  /**
   * @brief Fluent setter for the fraction of samples to take
   * @param fractionToSample The fraction value between 0.0 and 1.0
   * @return Reference to this object for method chaining
   */
  SampleFraction &WithFractionToSample(float fractionToSample) {
    SetFractionToSample(fractionToSample);
    return *this;
  }

private:
  static Int64 DefaultSeed();

private:
  float m_fractionToSample;
  Internal::SeedState *m_seed;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
