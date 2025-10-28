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

#include <aws/gamelift/metrics/KeySuffix.h>
#include <aws/gamelift/metrics/ReduceMetric.h>
#include <aws/gamelift/metrics/TypeTraits.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class T> struct OpLatest final {
  T operator()(T current, T newValue) { return newValue; }
};

} // namespace Internal

/**
 * Keeps only the latest value of a metric.
 */
class Latest : public Reduce<Internal::OpLatest<double>> {
public:
  /**
   * Keeps only the latest value.
   *
   * If ADD supported, this is added to latest.
   */
  Latest() : Reduce(".latest") {}

  /**
   * Keeps only the latest value.
   *
   * If ADD supported, this is added to latest.
   *
   * @param suffix Suffix to add to metric key
   */
  explicit Latest(KeySuffix &&suffix)
      : Reduce(Internal::Forward<KeySuffix>(suffix)) {}

  /**
   * @brief Gets the suffix
   * @return The current suffix
   */
  const KeySuffix &GetSuffix() const { return m_suffix; }

  /**
   * @brief Sets the suffix
   * @param suffix Suffix to add to metric key
   */
  void SetSuffix(KeySuffix &&suffix) {
    m_suffix = Internal::Forward<KeySuffix>(suffix);
  }

  /**
   * @brief Fluent setter for the suffix
   * @param suffix Suffix to add to metric key
   * @return Reference to this object for method chaining
   */
  Latest &WithSuffix(KeySuffix &&suffix) {
    SetSuffix(Internal::Forward<KeySuffix>(suffix));
    return *this;
  }
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws