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

#include <aws/gamelift/metrics/DynamicMetric.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
#ifdef GAMELIFT_USE_STD
/**
 * Helper class for derived metrics.
 *
 * Wraps a key suffix and applies it to dynamic metrics.
 */
struct GAMELIFT_METRICS_API KeySuffix final {
  KeySuffix() = default;
  KeySuffix(const char *suffix) : m_suffix(suffix) {}
  KeySuffix(std::string suffix) : m_suffix(suffix) {}

  std::string m_suffix;

  void Apply(const IMetric &original, DynamicMetric &target);

  /**
   * @brief Gets the suffix value
   * @return The current suffix value
   */
  const std::string &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix value
   * @param suffix The new suffix value
   */
  void SetSuffix(const std::string &suffix) { m_suffix = suffix; }

  /**
   * @brief Fluent setter for the suffix value
   * @param suffix The new suffix value
   * @return Reference to this object for method chaining
   */
  KeySuffix &WithSuffix(const std::string &suffix) {
    SetSuffix(suffix);
    return *this;
  }
};
#else
/**
 * Helper class for derived metrics.
 *
 * Wraps a key suffix and applies it to dynamic metrics.
 */
struct GAMELIFT_METRICS_API KeySuffix final {
  static constexpr int MAXIMUM_SUFFIX_LENGTH = 1024;

  KeySuffix();
  KeySuffix(const char *suffix);

  char m_suffix[MAXIMUM_SUFFIX_LENGTH];

  void Apply(const IMetric &original, DynamicMetric &target);

  /**
   * @brief Gets the suffix value
   * @return The current suffix value
   */
  const char *GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix value
   * @param suffix The new suffix value
   */
  void SetSuffix(const char *suffix);

  /**
   * @brief Fluent setter for the suffix value
   * @param suffix The new suffix value
   * @return Reference to this object for method chaining
   */
  KeySuffix &WithSuffix(const char *suffix) {
    SetSuffix(suffix);
    return *this;
  }
};
#endif
} // namespace Metrics
} // namespace GameLift
} // namespace Aws