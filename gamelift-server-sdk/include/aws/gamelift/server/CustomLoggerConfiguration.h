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

#include <aws/gamelift/common/GameLift_EXPORTS.h>

namespace Aws {
namespace GameLift {
namespace Server {

/**
 * Log levels for the custom logging callback.
 * Values are ordered by increasing severity. Trace is the most verbose;
 * Fatal is the most severe. Off disables all callback dispatching.
 */
enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    // Values 6–99 reserved for future severity levels.
    Off = 100  ///< Disables all log callback dispatching
};

/**
 * Callback function type for custom logging.
 * @param level The severity level of the log message.
 * @param message The pre-formatted log message string (null-terminated).
 *        The pointer is only valid for the duration of the callback invocation.
 *        Callers that need the message after the callback returns must copy it
 *        (e.g., into a std::string or a queue buffer).
 *        The message length is unbounded -- messages may embed full service
 *        payloads (e.g., game session data or matchmaker data) and can be
 *        arbitrarily large. Do not copy the message into fixed-size buffers
 *        without bounds checking.
 * @param userData The user-provided context pointer passed in CustomLoggerConfiguration.
 *
 * Thread safety: The SDK serializes callback invocations via an internal mutex
 * (the callback will not be invoked from multiple threads concurrently), but it
 * may be called from any thread. Implementations do not need their own
 * synchronization unless they access shared external state.
 *
 * This serialization guarantee holds only while the SDK is running normally.
 * Teardown is NOT fully synchronized: the game-session, process-terminate, and
 * update-game-session handlers run on detached threads that Destroy() does not
 * join, so a callback may still be invoked while Destroy() runs or shortly after
 * it returns. There is therefore no point before process exit at which the SDK
 * can guarantee no callback is in flight. Keep the callback, its module (DLL/
 * shared library), and userData valid until the process exits; do not free or
 * unload them after Destroy() returns. Likewise, avoid retaining your own
 * shared_ptr to the SDK logger across Destroy() -- logging through a retained
 * reference may still invoke the callback.
 *
 * @note The callback is invoked synchronously on the logging thread.
 *       Long-running operations (network I/O, file writes, etc.) will block
 *       all SDK logging until the callback returns. Consider queuing messages
 *       for asynchronous processing if your logging backend involves I/O.
 *
 * @note The callback pointer, its module, and userData must remain valid until
 *       process exit -- not merely until Destroy() returns. See the thread
 *       safety notes above.
 */
using LogCallback = void(*)(LogLevel level, const char* message, void* userData);

/**
 * Parameters for configuring a custom logger.
 * When a LogCallback is provided, it replaces the default stdout and file logging.
 * When no LogCallback is provided (nullptr), the SDK uses its default logging behavior.
 *
 * @note Log configuration is immutable after InitSDK() is called. The log level
 *       and callback cannot be changed at runtime. To change logging behavior,
 *       the process must be restarted.
 */
struct AWS_GAMELIFT_API CustomLoggerConfiguration {
    /** Custom log callback. Set to nullptr for default SDK logging behavior. */
    LogCallback callback = nullptr;

    /** User-provided context pointer passed to the callback.
     *  The SDK does not manage its lifetime.
     *
     *  Must remain valid from InitSDK() until the process terminates -- not merely
     *  until Destroy() returns. See the thread safety notes on LogCallback above.
     *
     *  @code
     *    Server::Destroy();
     *    // Do NOT delete myLogContext here: a detached callback thread may still be
     *    // running and dereference it. Let the OS reclaim it at process exit, or
     *    // otherwise guarantee no callback can run before freeing it.
     *  @endcode
     */
    void* userData = nullptr;

    /** Minimum log level. Messages below this level are not dispatched. */
    LogLevel minimumLogLevel = LogLevel::Info;

    CustomLoggerConfiguration() = default;

    explicit CustomLoggerConfiguration(LogCallback cb, void* ud = nullptr, LogLevel minLevel = LogLevel::Info)
        : callback(cb), userData(ud), minimumLogLevel(minLevel) {}
};

} // namespace Server
} // namespace GameLift
} // namespace Aws
