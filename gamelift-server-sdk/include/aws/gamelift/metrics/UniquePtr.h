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
#include <memory>

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class T> using UniquePtr = std::unique_ptr<T>;

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#else

namespace Aws {
namespace GameLift {
namespace Metrics {
namespace Internal {

template <class T> class GAMELIFT_METRICS_API UniquePtr {
public:
  UniquePtr() : Ptr(nullptr) {}
  UniquePtr(T *Ptr) noexcept : Ptr(Ptr) {}

  ~UniquePtr() { delete Ptr; }

  // no copy
  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  UniquePtr(UniquePtr &&Other) : Ptr(nullptr) { swap(*this, Other); }
  UniquePtr &operator=(UniquePtr &&Other) {
    swap(*this, Other);
    return *this;
  }

  T *get() { return Ptr; }
  const T *get() const { return Ptr; }

  void reset(T *NewPtr = nullptr) {
    delete Ptr;
    Ptr = NewPtr;
  }

  T &operator*() { return *Ptr; }
  const T &operator*() const { return *Ptr; }

  T *operator->() { return Ptr; }
  const T *operator->() const { return Ptr; }

  operator bool() const { return Ptr ? true : false; }

  friend void swap(UniquePtr &A, UniquePtr &B) {
    auto Temp = A.Ptr;
    A.Ptr = B.Ptr;
    B.Ptr = Temp;
  }

private:
  T *Ptr = nullptr;
};

} // namespace Internal
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#endif // GAMELIFT_USE_STD
