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

#if __cplusplus >= 201703L
#define GAMELIFT_METRICS_CPP_17 1
#else
#define GAMELIFT_METRICS_CPP_17 0
#endif

// Prefer `if constexpr` when available to hint at static branch
#if GAMELIFT_METRICS_CPP_17
#define IF_CONSTEXPR if constexpr
#else
#define IF_CONSTEXPR if
#endif

/*
 * When building from source as Unreal module, we want to export some of the API
 * to other modules.
 */
#ifdef _WIN32
#if defined(USE_IMPORT_EXPORT) || defined(GAMELIFT_METRICS_UNREAL)
#ifdef AWS_GAMELIFT_EXPORTS
#define GAMELIFT_METRICS_API __declspec(dllexport)
#else
#define GAMELIFT_METRICS_API __declspec(dllimport)
#endif
#else
#define GAMELIFT_METRICS_API
#endif // USE_IMPORT_EXPORT
#else
#define GAMELIFT_METRICS_API
#endif // _WIN32
