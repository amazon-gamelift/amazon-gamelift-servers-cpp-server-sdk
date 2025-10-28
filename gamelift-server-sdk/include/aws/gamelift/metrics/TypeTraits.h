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

/**
 * @brief We define some type_traits-like types here.
 *
 * When GAMELIFT_USE_STD is on, we simply typedef them to be standard library
 * type traits to take advantage of compiler optimizations (if any).
 *
 * Otherwise, we define our own minimal implementations.
 */
#ifdef GAMELIFT_USE_STD
#include <limits>
#include <type_traits>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <bool Value, class TrueType, class FalseType>
using Conditional = std::conditional<Value, TrueType, FalseType>;

template <class T, class U> using IsSame = std::is_same<T, U>;

using TrueType = std::true_type;

template <bool Value, class T = void> using EnableIf = std::enable_if<Value, T>;

template <class T> using RemoveReference = std::remove_reference<T>;

template <class T> using NumericLimits = std::numeric_limits<T>;

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#else
#include <aws/gamelift/metrics/Defs.h>

#include <float.h>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {
struct GAMELIFT_METRICS_API TrueType {
#if GAMELIFT_METRICS_CPP_17
  static constexpr bool value = true;
#else
  static const bool value = true;
#endif // GAMELIFT_METRICS_CPP_17
};

struct GAMELIFT_METRICS_API FalseType {
#if GAMELIFT_METRICS_CPP_17
  static constexpr bool value = false;
#else
  static const bool value = false;
#endif // GAMELIFT_METRICS_CPP_17
};

template <bool Value, class TrueType, class FalseType> struct Conditional;

template <class TrueType, class FalseType>
struct Conditional<true, TrueType, FalseType> {
  using type = TrueType;
};

template <class TrueType, class FalseType>
struct Conditional<false, TrueType, FalseType> {
  using type = FalseType;
};

template <class T, class U> struct IsSame : public FalseType {};

template <class T> struct IsSame<T, T> : public TrueType {};

template <bool Value, class T = void> struct EnableIf {};

template <class T> struct EnableIf<true, T> { using type = T; };

template <class T> struct RemoveReference { using type = T; };

template <class T> struct RemoveReference<T &> { using type = T; };

template <class T> struct RemoveReference<T &&> { using type = T; };

template <class T> struct NumericLimits;

#ifdef max
#undef max
#endif

template <> struct NumericLimits<double> {
  static constexpr double max() { return DBL_MAX; }
};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#endif // GAMELIFT_USE_STD

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class T>
inline T &&Forward(typename RemoveReference<T>::type &Value) {
  return static_cast<T &&>(Value);
}

template <class T>
inline T &&Forward(typename RemoveReference<T>::type &&Value) {
  return static_cast<T &&>(Value);
}

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
