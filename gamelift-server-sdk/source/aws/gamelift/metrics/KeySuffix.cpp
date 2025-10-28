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
#include <aws/gamelift/metrics/KeySuffix.h>

#ifdef GAMELIFT_USE_STD
namespace Aws {
namespace GameLift {
namespace Metrics {
void KeySuffix::Apply(const IMetric &original, DynamicMetric &target) {
  target.SetKey(std::string(original.GetKey()) + m_suffix);
}
} // namespace Metrics
} // namespace GameLift
} // namespace Aws
#else
#include <algorithm>
#include <cstring>
#include <string>

namespace Aws {
namespace GameLift {
namespace Metrics {
KeySuffix::KeySuffix() {
  std::fill(std::begin(m_suffix), std::end(m_suffix), '\0');
}

KeySuffix::KeySuffix(const char *newSuffix) {
#ifdef _WIN32
  strncpy_s(m_suffix, KeySuffix::MAXIMUM_SUFFIX_LENGTH, newSuffix,
            KeySuffix::MAXIMUM_SUFFIX_LENGTH);
#else
  strncpy(m_suffix, newSuffix, KeySuffix::MAXIMUM_SUFFIX_LENGTH);
#endif // _WIN32
}

void KeySuffix::SetSuffix(const char *newSuffix) {
#ifdef _WIN32
  strncpy_s(m_suffix, KeySuffix::MAXIMUM_SUFFIX_LENGTH, newSuffix,
            KeySuffix::MAXIMUM_SUFFIX_LENGTH);
#else
  strncpy(m_suffix, newSuffix, KeySuffix::MAXIMUM_SUFFIX_LENGTH);
#endif // _WIN32
}

void KeySuffix::Apply(const IMetric &original, DynamicMetric &target) {
  std::string key = original.GetKey();
  key += m_suffix;
  target.SetKey(key.c_str());
}

} // namespace Metrics
} // namespace GameLift
} // namespace Aws

#endif // GAMELIFT_USE_STD