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
#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <aws/gamelift/server/CustomLoggerConfiguration.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/pattern_formatter.h>
#include <iostream>
#include <mutex>
#include <atomic>

using namespace Aws::GameLift::Internal;

static constexpr const char* LOGGER_NAME = "multi_sink";
static constexpr size_t MAX_LOG_FILE_SIZE = 10 * 1024 * 1024; // 10MB
static constexpr size_t MAX_LOG_FILES = 5;

// Tracks whether a custom callback logger has been registered via InitCustomLogger.
// Uses std::atomic for thread-safety consistent with the surrounding SDK code.
static std::atomic<bool> s_customLoggerRegistered{false};

namespace {

class CallbackSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    CallbackSink(Aws::GameLift::Server::LogCallback callback, void* userData)
        : m_callback(callback), m_userData(userData) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
        // Null-terminate the buffer in-place to avoid a std::string heap allocation.
        formatted.push_back('\0');

        Aws::GameLift::Server::LogLevel level;
        switch (msg.level) {
            case spdlog::level::trace:
                level = Aws::GameLift::Server::LogLevel::Trace;
                break;
            case spdlog::level::debug:
                level = Aws::GameLift::Server::LogLevel::Debug;
                break;
            case spdlog::level::info:
                level = Aws::GameLift::Server::LogLevel::Info;
                break;
            case spdlog::level::warn:
                level = Aws::GameLift::Server::LogLevel::Warn;
                break;
            case spdlog::level::err:
                level = Aws::GameLift::Server::LogLevel::Error;
                break;
            case spdlog::level::critical:
                level = Aws::GameLift::Server::LogLevel::Fatal;
                break;
            default:
                level = Aws::GameLift::Server::LogLevel::Info;
                break;
        }

        try {
            m_callback(level, formatted.data(), m_userData);
        } catch (const std::exception& ex) {
            std::cerr << "[error] Custom log callback threw an exception: "
                      << ex.what() << ". Message was dropped." << std::endl;
        } catch (...) {
            std::cerr << "[error] Custom log callback threw a non-standard exception. "
                         "Message was dropped." << std::endl;
        }
    }

    void flush_() override {}

private:
    Aws::GameLift::Server::LogCallback m_callback;
    void* m_userData;
};

spdlog::level::level_enum MapLogLevel(Aws::GameLift::Server::LogLevel level) {
    switch (level) {
        case Aws::GameLift::Server::LogLevel::Trace:
            return spdlog::level::trace;
        case Aws::GameLift::Server::LogLevel::Debug:
            return spdlog::level::debug;
        case Aws::GameLift::Server::LogLevel::Warn:
            return spdlog::level::warn;
        case Aws::GameLift::Server::LogLevel::Error:
            return spdlog::level::err;
        case Aws::GameLift::Server::LogLevel::Fatal:
            return spdlog::level::critical;
        case Aws::GameLift::Server::LogLevel::Off:
            return spdlog::level::off;
        case Aws::GameLift::Server::LogLevel::Info:
        default:
            return spdlog::level::info;
    }
}

/// Retrieves the dist_sink_mt from the current default logger, if the logger exists
/// and its first (and only) sink is a dist_sink_mt. Returns nullptr otherwise.
std::shared_ptr<spdlog::sinks::dist_sink_mt> GetDistSink() {
    auto logger = spdlog::default_logger();
    if (!logger || logger->sinks().empty()) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(logger->sinks().front());
}

/// Creates the "multi_sink" logger with a dist_sink_mt wrapping the given children,
/// registers it in spdlog's registry, and sets it as the default logger.
/// The dist_sink_mt provides the indirection layer: log() and set_sinks() are
/// mutually exclusive under the same std::mutex (base_sink-inl.h:27, dist_sink.h:41
/// in vendored spdlog v1.14.0), so child sinks can be swapped atomically while
/// background threads are concurrently logging through the same logger instance.
std::shared_ptr<spdlog::sinks::dist_sink_mt> CreateDistSinkLogger(
    std::vector<std::shared_ptr<spdlog::sinks::sink>> children,
    spdlog::level::level_enum level,
    spdlog::level::level_enum flushLevel) {

    auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(std::move(children));
    auto logger = std::make_shared<spdlog::logger>(LOGGER_NAME, spdlog::sinks_init_list{distSink});
    logger->set_level(level);
    logger->flush_on(flushLevel);
    spdlog::set_default_logger(logger);
    return distSink;
}

} // anonymous namespace

bool LoggerHelper::IsCustomLoggerRegistered() {
    return s_customLoggerRegistered.load(std::memory_order_acquire);
}

void LoggerHelper::ResetCustomLoggerRegistered() {
    s_customLoggerRegistered.store(false, std::memory_order_release);
}

void LoggerHelper::DropSdkLogger() {
    spdlog::drop(LOGGER_NAME);
}

Aws::GameLift::GenericOutcome LoggerHelper::InitializeCallbackLogger(const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters) {
    // Atomic CAS ensures only one concurrent caller succeeds in registering the custom logger.
    bool expected = false;
    if (!s_customLoggerRegistered.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED,
            "Custom logger has already been initialized via InitCustomLogger()."));
    }

    try {
        auto callbackSink = std::make_shared<CallbackSink>(logParameters.callback, logParameters.userData);
        callbackSink->set_formatter(
            std::unique_ptr<spdlog::pattern_formatter>(new spdlog::pattern_formatter("%v", spdlog::pattern_time_type::local, "")));

        auto distSink = GetDistSink();
        if (distSink) {
            // The default logger already exists (InitSDK ran first and called InitializeLogger(processId)).
            // Swap the dist_sink's children from [stdout, file] to [callbackSink].
            // THREAD SAFETY: dist_sink_mt::set_sinks() acquires base_sink<std::mutex>::mutex_
            // (dist_sink.h:41, vendored spdlog v1.14.0). dist_sink_mt::log() (called by any
            // concurrent spdlog::info() etc.) also acquires the SAME mutex via base_sink::log()
            // (base_sink-inl.h:27). Therefore set_sinks() and log() are mutually exclusive —
            // no data race, no use-after-free on the old child sinks.
            distSink->set_sinks({callbackSink});

            // Update the logger level. spdlog::logger::level_ is std::atomic<int>
            // (common.h:228, vendored spdlog v1.14.0), so set_level() is safe to call
            // concurrently with should_log() checks on other threads.
            spdlog::default_logger()->set_level(MapLogLevel(logParameters.minimumLogLevel));
        } else {
            // No logger exists yet (InitCustomLogger called before InitSDK).
            // Create the dist_sink logger with only the callback sink as its child.
            // No race here — if InitSDK hasn't run, no background threads are logging yet,
            // and the CAS above prevents concurrent InitializeCallbackLogger calls.
            CreateDistSinkLogger({callbackSink}, MapLogLevel(logParameters.minimumLogLevel), spdlog::level::off);
        }
        // Note: flush_on is not set/changed for callback path because CallbackSink has no
        // internal buffer — messages are dispatched to the callback immediately in sink_it_().

        return Aws::GameLift::GenericOutcome(nullptr);
    } catch (const std::exception& ex) {
        // Rollback the flag so a retry can succeed after the transient failure is resolved.
        s_customLoggerRegistered.store(false, std::memory_order_release);
        std::string errorMessage = "Failed to initialize custom logger: " + std::string(ex.what());
        std::cerr << "[error] " << errorMessage << std::endl;
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED,
#ifdef GAMELIFT_USE_STD
            errorMessage));
#else
            errorMessage.c_str()));
#endif
    }
}

#ifdef GAMELIFT_USE_STD
Aws::GameLift::GenericOutcome LoggerHelper::InitializeLogger(const std::string& process_Id) {
    // If a custom logger was already registered via InitCustomLogger, skip default logger initialization.
    // The callback logger's dist_sink is already in place; InitSDK's default sinks must NOT clobber it.
    if (s_customLoggerRegistered.load(std::memory_order_acquire)) {
        return Aws::GameLift::GenericOutcome(nullptr);
    }
    if (spdlog::get(LOGGER_NAME)) {
        spdlog::warn("Logger already initialized, skipping duplicate initialization");
        return Aws::GameLift::GenericOutcome(nullptr);
    }
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        std::string serverSdkLog = "logs/gamelift-server-sdk-";
        serverSdkLog.append(process_Id).append(".log");
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(serverSdkLog, MAX_LOG_FILE_SIZE, MAX_LOG_FILES);

        console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] [tid %t] %v%$");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [tid %t] %v");

        // Create the logger with a dist_sink_mt wrapping the console and file sinks.
        // The dist_sink provides sink-level indirection: if InitCustomLogger() is called later
        // (after InitSDK), it can atomically swap children via set_sinks() without replacing
        // the logger instance — eliminating the data race on spdlog's default_logger_raw()
        // pointer that the old drop()+set_default_logger() approach caused.
        CreateDistSinkLogger({console_sink, file_sink}, spdlog::level::info, spdlog::level::info);

        return Aws::GameLift::GenericOutcome(nullptr);
    } catch (const std::exception& ex) {
        std::string errorMessage = "Failed to initialize logger: " + std::string(ex.what());
        std::cerr << "[error] " << errorMessage << std::endl;
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED,
            errorMessage));
    }
}
#else
Aws::GameLift::GenericOutcome LoggerHelper::InitializeLogger(const char* process_Id) {
    // If a custom logger was already registered via InitCustomLogger, skip default logger initialization.
    // The callback logger's dist_sink is already in place; InitSDK's default sinks must NOT clobber it.
    if (s_customLoggerRegistered.load(std::memory_order_acquire)) {
        return Aws::GameLift::GenericOutcome(nullptr);
    }
    if (spdlog::get(LOGGER_NAME)) {
        spdlog::warn("Logger already initialized, skipping duplicate initialization");
        return Aws::GameLift::GenericOutcome(nullptr);
    }
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        std::string serverSdkLog = "logs/gamelift-server-sdk-";
        serverSdkLog.append(process_Id).append(".log");
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(serverSdkLog, MAX_LOG_FILE_SIZE, MAX_LOG_FILES);

        console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] [tid %t] %v%$");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [tid %t] %v");

        // Create the logger with a dist_sink_mt wrapping the console and file sinks.
        // See the GAMELIFT_USE_STD overload above for the full concurrency rationale.
        CreateDistSinkLogger({console_sink, file_sink}, spdlog::level::info, spdlog::level::info);

        return Aws::GameLift::GenericOutcome(nullptr);
    } catch (const std::exception& ex) {
        std::string errorMessage = "Failed to initialize logger: " + std::string(ex.what());
        std::cerr << "[error] " << errorMessage << std::endl;
        return Aws::GameLift::GenericOutcome(Aws::GameLift::GameLiftError(Aws::GameLift::GAMELIFT_ERROR_TYPE::GAMELIFT_SERVER_NOT_INITIALIZED,
            errorMessage.c_str()));
    }
}
#endif

#ifdef GAMELIFT_USE_STD
Aws::GameLift::GenericOutcome LoggerHelper::InitializeLogger(const std::string& process_Id, const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters) {
    // If a callback is provided, delegate to the callback logger path.
    if (logParameters.callback != nullptr) {
        return InitializeCallbackLogger(logParameters);
    }
    // No callback — fall through to the default file/stdout logger.
    return InitializeLogger(process_Id);
}
#else
Aws::GameLift::GenericOutcome LoggerHelper::InitializeLogger(const char* process_Id, const Aws::GameLift::Server::CustomLoggerConfiguration& logParameters) {
    // If a callback is provided, delegate to the callback logger path.
    if (logParameters.callback != nullptr) {
        return InitializeCallbackLogger(logParameters);
    }
    // No callback — fall through to the default file/stdout logger.
    return InitializeLogger(process_Id);
}
#endif
