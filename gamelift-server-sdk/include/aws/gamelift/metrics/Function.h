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

#ifdef GAMELIFT_USE_STD
#include <functional>

namespace Aws {
namespace GameLift {
namespace Metrics {

/**
 * We still want to use std::function here as we benefit
 * from zero-alloc specializations for func pointers or stateless lambdas.
 */
template <class Signature> using Function = std::function<Signature>;

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#else
#include <aws/gamelift/metrics/UniquePtr.h>

namespace Aws {
namespace GameLift {
namespace Metrics {

template <class Signature> class GAMELIFT_METRICS_API Function;

template <class RetType, class... ArgTypes>
class GAMELIFT_METRICS_API Function<RetType(ArgTypes...)> {
private:
  /**
   * Base class for our type-erasure construct.
   *
   * Unlike std::function, we do not currently implement the zero-allocation
   * optimization for function pointers or stateless/small lambdas
   */
  struct WrapperBase {
    virtual ~WrapperBase() = default;

    virtual RetType Invoke(ArgTypes &&...args) = 0;
    virtual Internal::UniquePtr<WrapperBase> Clone() const = 0;
  };

  template <class T> struct FunctionWrapper : public WrapperBase {
    T m_state;

    explicit FunctionWrapper(T &&state)
        : m_state(Internal::Forward<T>(state)) {}

    virtual RetType Invoke(ArgTypes &&...args) override {
      return m_state(Internal::Forward<ArgTypes>(args)...);
    }

    virtual Internal::UniquePtr<WrapperBase> Clone() const override {
      return Internal::UniquePtr<WrapperBase>(new FunctionWrapper(*this));
    }
  };

public:
  using Signature = RetType(ArgTypes...);
  using CallablePtr = Signature *;

  /**
   * Initializes unassigned function.
   */
  Function() = default;

  /**
   * Copy
   */
  Function(const Function &other)
      : m_wrappedFunction(other ? other.m_wrappedFunction->Clone() : nullptr) {}
  Function &operator=(const Function &other) {
    if (other) {
      m_wrappedFunction = other.m_wrappedFunction->Clone();
    } else {
      m_wrappedFunction = nullptr;
    }
    return *this;
  }

  /**
   * Move
   */
  Function(Function &&) = default;
  Function &operator=(Function &&) = default;

  /**
   * @brief Initializes Function from a callable.
   *
   * @param callable Callable to wrap - function pointer or lambda.
   */
  template <class T>
  Function(T &&callable)
      : m_wrappedFunction(
            new FunctionWrapper<T>(Internal::Forward<T>(callable))) {}

  /**
   * @brief Returns true if the function is bound to a callable
   * @returns true if bound
   */
  operator bool() const { return m_wrappedFunction; }

  /**
   * @brief Invokes the underlying function.
   * @returns Result of function call
   */
  RetType operator()(ArgTypes &&...args) {
    return m_wrappedFunction->Invoke(Internal::Forward<ArgTypes>(args)...);
  }

private:
  Internal::UniquePtr<WrapperBase> m_wrappedFunction = nullptr;
};

} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#endif // GAMELIFT_USE_STD