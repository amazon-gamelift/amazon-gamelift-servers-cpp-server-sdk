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
#include <aws/gamelift/metrics/Samplers.h>

#include <chrono>
#include <mutex>
#include <random>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {
/**
 * Basic wrapper around seed_seq so we don't have to leak it in the header
 */
struct SeedState {
  std::seed_seq m_seed;
  // std::seed_seq has internal state that is updated when we first initialize
  // the sampler per-thread
  std::mutex m_mutex;

  explicit SeedState(Int64 seed) : m_seed{seed} {}
};
} // namespace Internal

namespace {
/**
 * Per thread sampler with own RNG.
 *
 * We use this to avoid synchronization in multithreaded scenarios.
 * This way we only have to lock when first initializing the thread.
 */
class PerThreadSampler {
public:
  explicit PerThreadSampler(Internal::SeedState &state) noexcept
      : m_distribution(0.0, 1.0) {
    const std::lock_guard<std::mutex> guard(state.m_mutex);
    if (state.m_seed.size() > 0) {
      m_engine = std::mt19937(state.m_seed);
    }
  }

  bool ShouldTakeSample(float fraction) {
    const float randomValue = m_distribution(m_engine);
    return randomValue <= fraction;
  }

private:
  std::uniform_real_distribution<float> m_distribution;
  std::mt19937 m_engine;
};
} // namespace

ISampler::~ISampler() {}

SampleFraction::SampleFraction(float fractionToSample, Int64 seed)
    : m_fractionToSample(fractionToSample),
      m_seed(new Internal::SeedState(seed)) {}

SampleFraction::~SampleFraction() { delete m_seed; }

bool SampleFraction::ShouldTakeSample() {
  // We use per-thread RNG to avoid synchronization.
  static thread_local PerThreadSampler sampler(*m_seed);
  return sampler.ShouldTakeSample(m_fractionToSample);
}

Int64 SampleFraction::DefaultSeed() {
  using Nanoseconds = std::chrono::duration<Int64, std::nano>;
  return std::chrono::duration_cast<Nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
