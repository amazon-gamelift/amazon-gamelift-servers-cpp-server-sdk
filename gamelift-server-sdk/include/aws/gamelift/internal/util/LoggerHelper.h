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

#include <aws/gamelift/common/Outcome.h>
#include <aws/gamelift/server/CustomLoggerConfiguration.h>
#include <string>

namespace Aws {
namespace GameLift {
namespace Internal {

/**
 * Internal helper that manages SDK logging configuration via sink-level indirection.
 *
 * Design: The default spdlog logger ("multi_sink") is created once and never reassigned
 * during the SDK's lifetime. Its sole sink is a dist_sink_mt, which wraps child sinks
 * (stdout+file for default, or a CallbackSink for custom logging). When InitCustomLogger() is
 * called after InitSDK(), the dist_sink's children are atomically swapped via set_sinks()
 * — no spdlog::drop() or set_default_logger() occurs in the hot path, eliminating the
 * data race on spdlog's unsynchronized default_logger_raw() pointer.
 *
 * Thread safety: dist_sink_mt::set_sinks() and dist_sink_mt::log() both acquire the same
 * base_sink<std::mutex>::mutex_ (vendored spdlog v1.14.0, dist_sink.h:41, base_sink-inl.h:27),
 * so they are mutually exclusive. Logger::set_level() is safe because level_ is
 * std::atomic<int> (common.h:228).
 *
 * Call ordering:
 * - InitCustomLogger before InitSDK: callback logger created with dist_sink wrapping CallbackSink.
 *   Subsequent InitializeLogger(processId) from InitSDK is a no-op (CAS guard check).
 * - InitSDK before InitCustomLogger: default logger created with dist_sink wrapping [stdout, file].
 *   InitializeCallbackLogger swaps children to [CallbackSink] via set_sinks().
 * - InitCustomLogger can be called at most once; subsequent calls return ALREADY_INITIALIZED.
 */
class LoggerHelper {
public:
    /// Registers a custom callback logger. Safe to call before or after InitSDK().
    /// After InitSDK(), background threads may be actively logging; the sink swap is
    /// serialized with in-flight log calls via the dist_sink_mt mutex.
    static GenericOutcome InitializeCallbackLogger(const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters);
    static bool IsCustomLoggerRegistered();
    static void ResetCustomLoggerRegistered();

    /// Drops the SDK logger from spdlog's registry. Encapsulates the logger name
    /// so callers don't need to hard-code the "multi_sink" literal.
    static void DropSdkLogger();

#ifdef GAMELIFT_USE_STD
    static GenericOutcome InitializeLogger(const std::string& process_Id);
    /// Initializes the logger using logParameters. If logParameters.callback is null, this
    /// overload treats it as "no custom logger configured" and falls back to the default file
    /// logger — unlike the public Server::InitCustomLogger() API, which rejects null with
    /// BAD_REQUEST_EXCEPTION because it represents explicit user intent.
    static GenericOutcome InitializeLogger(const std::string& process_Id, const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters);
#else
    static GenericOutcome InitializeLogger(const char* process_Id);
    /// Initializes the logger using logParameters. If logParameters.callback is null, this
    /// overload treats it as "no custom logger configured" and falls back to the default file
    /// logger — unlike the public Server::InitCustomLogger() API, which rejects null with
    /// BAD_REQUEST_EXCEPTION because it represents explicit user intent.
    static GenericOutcome InitializeLogger(const char* process_Id, const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters);
#endif
};

} // namespace Internal
} // namespace GameLift
} // namespace Aws
