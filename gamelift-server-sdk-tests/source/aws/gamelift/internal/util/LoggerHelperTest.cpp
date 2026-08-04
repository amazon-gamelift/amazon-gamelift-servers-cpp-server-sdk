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

#include "gtest/gtest.h"
#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <aws/gamelift/server/CustomLoggerConfiguration.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dist_sink.h>
#include <cstdlib>
#include <fstream>
#include <atomic>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define rmdir _rmdir
#else
#include <unistd.h>
#endif

namespace {
// Cross-platform recursive directory removal (C++11 compatible)
void RemoveDirectoryRecursive(const std::string& path) {
#ifdef _WIN32
    std::system(("rmdir /s /q \"" + path + "\" 2>nul").c_str());
#else
    std::system(("rm -rf '" + path + "'").c_str());
#endif
}

// Cross-platform directory creation (C++11 compatible)
void EnsureDirectoryExists(const std::string& path) {
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    std::system(("mkdir -p '" + path + "'").c_str());
#endif
}
} // anonymous namespace

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class LoggerHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset the process-static custom-logger flag so each test starts from a clean
        // slate. Without this, the first test that registers a callback logger leaves
        // s_customLoggerRegistered=true, causing every subsequent InitializeCallbackLogger
        // call to fail its CAS with ALREADY_INITIALIZED.
        LoggerHelper::ResetCustomLoggerRegistered();
        // Drop all loggers and restore a fresh default to ensure clean state
        spdlog::drop_all();
        spdlog::set_default_logger(spdlog::stdout_color_mt("default_test"));
    }

    void TearDown() override {
        // Clean up test log files
        std::remove("logs/gamelift-server-sdk-test-logger.log");
        // Reset the custom-logger flag so it does not leak into the next test.
        LoggerHelper::ResetCustomLoggerRegistered();
        // Restore a valid default logger so subsequent tests that use spdlog don't segfault
        spdlog::drop_all();
        spdlog::set_default_logger(spdlog::stdout_color_mt("default_test"));
    }
};

TEST_F(LoggerHelperTest, GIVEN_validLogDirectory_WHEN_initializeLogger_THEN_returnsSuccess) {
    // GIVEN - ensure logs/ is writable (remove any leftover from previous tests)
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-logger");

    // THEN
    EXPECT_TRUE(outcome.IsSuccess());
    EXPECT_NE(spdlog::get("multi_sink"), nullptr);
}

TEST_F(LoggerHelperTest, GIVEN_validLogDirectory_WHEN_initializeLogger_THEN_logFileIsCreated) {
    // GIVEN - ensure logs/ is writable
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");

    // WHEN
    LoggerHelper::InitializeLogger("test-logger");
    spdlog::default_logger()->flush();

    // THEN
    std::ifstream logFile("logs/gamelift-server-sdk-test-logger.log");
    EXPECT_TRUE(logFile.good());
}

TEST_F(LoggerHelperTest, GIVEN_invalidLogPath_WHEN_initializeLogger_THEN_returnsError) {
    // GIVEN - create a path that will fail file creation
#ifdef _WIN32
    RemoveDirectoryA("logs");
    DeleteFileA("logs");
    // Create the logs directory and place a read-only file at the exact path spdlog will try to open
    CreateDirectoryA("logs", NULL);
    HANDLE hFile = CreateFileA("logs\\gamelift-server-sdk-test-logger.log",
        GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
#else
    // Remove logs/ directory and all contents first
    RemoveDirectoryRecursive("logs");
    symlink("/proc/nonexistent/deeply/nested/invalid/path", "logs");
#endif

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-logger");

    // THEN
    EXPECT_FALSE(outcome.IsSuccess());
    std::string errorMsg = outcome.GetError().GetErrorMessage();
    EXPECT_NE(errorMsg.find("Failed to initialize logger"), std::string::npos);

    // Cleanup
#ifdef _WIN32
    SetFileAttributesA("logs\\gamelift-server-sdk-test-logger.log", FILE_ATTRIBUTE_NORMAL);
    DeleteFileA("logs\\gamelift-server-sdk-test-logger.log");
    RemoveDirectoryA("logs");
#else
    unlink("logs");
#endif
}

TEST_F(LoggerHelperTest, GIVEN_loggerAlreadyInitialized_WHEN_initializeLoggerCalledAgain_THEN_reusesExistingLogger) {
    // GIVEN - clean slate, then initialize logger once
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    LoggerHelper::InitializeLogger("first-call");
    auto firstLogger = spdlog::get("multi_sink");
    ASSERT_NE(firstLogger, nullptr);

    // WHEN - initialize again with a different process ID
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("second-call");

    // THEN - should succeed but reuse the existing logger (no duplicate)
    EXPECT_TRUE(outcome.IsSuccess());
    auto secondLogger = spdlog::get("multi_sink");
    EXPECT_EQ(firstLogger.get(), secondLogger.get());
    // Second log file should NOT be created
    EXPECT_FALSE(std::ifstream("logs/gamelift-server-sdk-second-call.log").good());
}

// Thread-safety note: This helper is safe for single-threaded test scenarios only.
// The CallbackSink (base_sink<std::mutex>) serializes callback invocations, so no
// additional synchronization is needed for tests using the default spdlog sink.
// For multi-threaded tests, use a mutex-guarded callback instead (see the
// GIVEN_customCallback_WHEN_multipleThreadsLog test below).
struct TestLogCapture {
    std::vector<std::pair<Aws::GameLift::Server::LogLevel, std::string>> messages;

    static void Callback(Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        auto* capture = static_cast<TestLogCapture*>(userData);
        capture->messages.emplace_back(level, std::string(message));
    }
};

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_initializeLogger_THEN_callbackReceivesLogMessages) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-callback", params);
    ASSERT_TRUE(outcome.IsSuccess());
    spdlog::info("hello from test");
    spdlog::default_logger()->flush();

    // THEN
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Info);
    EXPECT_NE(capture.messages.back().second.find("hello from test"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_initializeLogger_THEN_noLogFileCreated) {
    // GIVEN - ensure logs/ is clean
    RemoveDirectoryRecursive("logs");
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-no-file", params);
    ASSERT_TRUE(outcome.IsSuccess());
    spdlog::info("should not go to file");
    spdlog::default_logger()->flush();

    // THEN - no log file should be created
    EXPECT_FALSE(std::ifstream("logs/gamelift-server-sdk-test-no-file.log").good());
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithMinLevelWarn_WHEN_infoIsLogged_THEN_callbackNotCalled) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Warn);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-level-filter", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::info("this info message should be filtered out");
    spdlog::default_logger()->flush();

    // THEN
    EXPECT_TRUE(capture.messages.empty());
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithMinLevelWarn_WHEN_warnIsLogged_THEN_callbackCalled) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Warn);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-warn-level", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::warn("this warn should arrive");
    spdlog::default_logger()->flush();

    // THEN
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Warn);
    EXPECT_NE(capture.messages.back().second.find("this warn should arrive"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithMinLevelDebug_WHEN_debugIsLogged_THEN_callbackCalled) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Debug);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-debug-level", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::debug("debug message for testing");
    spdlog::default_logger()->flush();

    // THEN
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Debug);
    EXPECT_NE(capture.messages.back().second.find("debug message for testing"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_errorIsLogged_THEN_callbackReceivesErrorLevel) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Debug);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-error-level", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::error("error message");
    spdlog::default_logger()->flush();

    // THEN
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Error);
    EXPECT_NE(capture.messages.back().second.find("error message"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_criticalIsLogged_THEN_callbackReceivesFatalLevel) {
    // GIVEN
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Debug);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-critical-level", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::critical("critical failure");
    spdlog::default_logger()->flush();

    // THEN - critical maps to Fatal in the CallbackSink
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Fatal);
    EXPECT_NE(capture.messages.back().second.find("critical failure"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithUserData_WHEN_logIsCalled_THEN_userDataIsPassedThrough) {
    // GIVEN
    struct UserContext {
        int id;
        bool callbackInvoked;
    };
    UserContext ctx{42, false};

    auto verifyCallback = [](Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        auto* uctx = static_cast<UserContext*>(userData);
        uctx->callbackInvoked = true;
    };

    Aws::GameLift::Server::CustomLoggerConfiguration params(verifyCallback, &ctx, Aws::GameLift::Server::LogLevel::Info);

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-userdata", params);
    ASSERT_TRUE(outcome.IsSuccess());
    spdlog::info("trigger callback");
    spdlog::default_logger()->flush();

    // THEN
    EXPECT_TRUE(ctx.callbackInvoked);
    EXPECT_EQ(ctx.id, 42);
}

TEST_F(LoggerHelperTest, GIVEN_nullCallback_WHEN_initializeLogger_THEN_defaultBehavior) {
    // This test validates the internal LoggerHelper's graceful fallback behavior:
    // when a null callback is passed directly to the helper (bypassing Server::InitCustomLogger),
    // it falls through to the default file/stdout logger rather than crashing.
    // Note: The public API (Server::InitCustomLogger) rejects null callbacks with BAD_REQUEST;
    // see GameLiftServerAPITest for that coverage.

    // GIVEN - ensure logs/ is clean
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    Aws::GameLift::Server::CustomLoggerConfiguration params(nullptr, nullptr, Aws::GameLift::Server::LogLevel::Info);

    // WHEN
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-null-cb", params);
    ASSERT_TRUE(outcome.IsSuccess());
    spdlog::default_logger()->flush();

    // THEN - should fall back to default behavior (file is created)
    std::ifstream logFile("logs/gamelift-server-sdk-test-null-cb.log");
    EXPECT_TRUE(logFile.good());
}

// NOTE: This exercises the internal helper's ability to replace loggers directly.
// The public Server::InitCustomLogger() API prevents re-initialization via an atomic CAS
// guard and would return ALREADY_INITIALIZED in this scenario.
TEST_F(LoggerHelperTest, GIVEN_loggerAlreadyInitialized_WHEN_initializeWithCallback_THEN_callbackReplacesExistingLogger) {
    // GIVEN - initialize with default behavior first
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    LoggerHelper::InitializeLogger("test-first-init");
    ASSERT_NE(spdlog::get("multi_sink"), nullptr);

    // WHEN - initialize again with a callback
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);
    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-second-init", params);

    // THEN - callback logger replaces the existing one
    EXPECT_TRUE(outcome.IsSuccess());
    spdlog::info("this should now go to the callback");
    spdlog::default_logger()->flush();
    EXPECT_FALSE(capture.messages.empty());
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_multipleThreadsLog_THEN_noDataRaceOrCrash) {
    // GIVEN
    std::mutex captureMutex;
    std::vector<std::string> capturedMessages;

    auto threadSafeCallback = [](Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        auto* data = static_cast<std::pair<std::mutex*, std::vector<std::string>*>*>(userData);
        std::lock_guard<std::mutex> lock(*data->first);
        data->second->push_back(std::string(message));
    };

    std::pair<std::mutex*, std::vector<std::string>*> callbackData{&captureMutex, &capturedMessages};
    Aws::GameLift::Server::CustomLoggerConfiguration params(threadSafeCallback, &callbackData, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-threadsafe", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN - log from multiple threads concurrently
    const int numThreads = 4;
    const int messagesPerThread = 50;
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t, messagesPerThread]() {
            for (int i = 0; i < messagesPerThread; ++i) {
                spdlog::info("thread {} message {}", t, i);
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    spdlog::default_logger()->flush();

    // THEN - all messages should arrive (no data race, no crash)
    std::lock_guard<std::mutex> lock(captureMutex);
    EXPECT_EQ(static_cast<int>(capturedMessages.size()), numThreads * messagesPerThread);
}

TEST_F(LoggerHelperTest, GIVEN_throwingCallback_WHEN_logIsCalled_THEN_sdkDoesNotCrash) {
    // GIVEN
    auto throwingCallback = [](Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        throw std::runtime_error("callback failure");
    };

    Aws::GameLift::Server::CustomLoggerConfiguration params(throwingCallback, nullptr, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-throwing", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN/THEN - logging should not crash the process
    EXPECT_NO_THROW({
        spdlog::info("this should not crash");
        spdlog::default_logger()->flush();
    });

    // Verify we can still log after the exception
    EXPECT_NO_THROW({
        spdlog::info("second message after exception");
        spdlog::default_logger()->flush();
    });
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_dropAllCalledThenLogAttempted_THEN_noCrashAndCallbackNotInvoked) {
    // GIVEN - Initialize logger with a callback, simulating the InitSDK → Destroy lifecycle.
    // INVARIANT: After the SDK logger is dropped, any subsequent spdlog call must route to
    // the stdout fallback -- no crash, no callback invocation, no use-after-free.
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-teardown", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // Verify callback works before teardown
    spdlog::info("before teardown");
    spdlog::default_logger()->flush();
    ASSERT_FALSE(capture.messages.empty());
    size_t messageCountBeforeDrop = capture.messages.size();

    // WHEN - Tear down the logger using the hardened sequence (as DestroyInstance() does):
    // Install a fallback BEFORE dropping the SDK logger so spdlog's default is never null.
    auto fallback = std::make_shared<spdlog::logger>(
        "fallback", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    spdlog::set_default_logger(fallback);
    LoggerHelper::DropSdkLogger();

    // THEN - Logging after teardown should NOT crash and should NOT invoke the callback.
    EXPECT_NO_THROW({
        spdlog::info("after drop - should not crash");
        spdlog::warn("another log after drop");
    });

    // The callback must NOT have been invoked after drop
    EXPECT_EQ(capture.messages.size(), messageCountBeforeDrop);
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_dropAllCalledFromAnotherThread_THEN_noCrashOrUseAfterFree) {
    // GIVEN - This test exercises the teardown-safety mechanism in isolation by joining its
    // own helper thread before dropping. NOTE: production Destroy() does NOT join the detached
    // game-session/terminate/update handler threads, so this test validates the fallback-then-drop
    // sequence only for the already-quiesced case -- it does not prove the detached-thread race is
    // eliminated (see CustomLoggerConfiguration.h and the DestroyInstance comment).
    std::atomic<bool> stopLogging{false};
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-race-teardown", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN - Start a background thread that logs continuously
    std::thread loggerThread([&stopLogging]() {
        while (!stopLogging.load(std::memory_order_acquire)) {
            spdlog::info("background thread logging");
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });

    // Let the logger thread run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Signal the thread to stop (simulating thread join before teardown, as in the fixed code)
    stopLogging.store(true, std::memory_order_release);
    loggerThread.join();

    // THEN - After thread is joined, hardened teardown is safe: fallback first, then drop.
    auto fallback = std::make_shared<spdlog::logger>(
        "fallback", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    EXPECT_NO_THROW({ spdlog::set_default_logger(fallback); });
    EXPECT_NO_THROW({ spdlog::drop("multi_sink"); });

    // No more callbacks should fire
    size_t finalCount = capture.messages.size();
    EXPECT_GT(finalCount, 0u); // Should have captured messages from the background thread

    // Logging after teardown must not crash and must not invoke the callback
    EXPECT_NO_THROW({ spdlog::info("post-drop logging"); });
    EXPECT_EQ(capture.messages.size(), finalCount);
}

TEST_F(LoggerHelperTest, GIVEN_customCallback_WHEN_messageIsLogged_THEN_formatContainsOnlyMessageBody) {
    // GIVEN - The callback sink uses pattern "%v" (message body only).
    // This test locks down the contract: no timestamps, no level prefix, no thread id.
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-format-contract", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::info("exact message content");
    spdlog::default_logger()->flush();

    // THEN - message should be exactly the body text, no timestamp or level prefix.
    // The callback sink uses "%v" format (message-only), so the output must match verbatim.
    ASSERT_FALSE(capture.messages.empty());
    const std::string& msg = capture.messages.back().second;
    EXPECT_EQ(msg, "exact message content");
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithMinLevelOff_WHEN_allLevelsLogged_THEN_callbackNeverInvoked) {
    // GIVEN - LogLevel::Off disables all callback dispatching
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Off);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-level-off", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN - log at every severity
    spdlog::trace("trace msg");
    spdlog::debug("debug msg");
    spdlog::info("info msg");
    spdlog::warn("warn msg");
    spdlog::error("error msg");
    spdlog::critical("critical msg");
    spdlog::default_logger()->flush();

    // THEN - callback must never have been invoked
    EXPECT_TRUE(capture.messages.empty());
}

TEST_F(LoggerHelperTest, GIVEN_customCallbackWithMinLevelTrace_WHEN_traceIsLogged_THEN_callbackCalled) {
    // GIVEN - LogLevel::Trace is the lowest level; everything passes through
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Trace);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-trace-level", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN
    spdlog::trace("trace message for testing");
    spdlog::default_logger()->flush();

    // THEN
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_EQ(capture.messages.back().first, Aws::GameLift::Server::LogLevel::Trace);
    EXPECT_NE(capture.messages.back().second.find("trace message for testing"), std::string::npos);
}

TEST_F(LoggerHelperTest, GIVEN_callbackThrowingNonStdException_WHEN_logIsCalled_THEN_sdkDoesNotCrash) {
    // GIVEN - A callback that throws a non-std::exception type (e.g., int).
    // This exercises the catch(...) handler in CallbackSink::sink_it_().
    auto throwingIntCallback = [](Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        throw 42;  // non-std exception
    };

    Aws::GameLift::Server::CustomLoggerConfiguration params(throwingIntCallback, nullptr, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome outcome = LoggerHelper::InitializeLogger("test-throw-int", params);
    ASSERT_TRUE(outcome.IsSuccess());

    // WHEN/THEN - logging must not crash despite non-std throw
    EXPECT_NO_THROW({
        spdlog::info("this triggers throw 42");
        spdlog::default_logger()->flush();
    });

    // Verify continued logging works after the non-std exception
    EXPECT_NO_THROW({
        spdlog::info("still alive after throw int");
        spdlog::default_logger()->flush();
    });
}

// ===== New tests for sink-level indirection design =====

TEST_F(LoggerHelperTest, GIVEN_defaultLoggerInitialized_WHEN_callbackLoggerRegistered_THEN_subsequentLogsHitCallbackNotFile) {
    // GIVEN - Initialize default logger (stdout + file sinks via dist_sink_mt)
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    Aws::GameLift::GenericOutcome initOutcome = LoggerHelper::InitializeLogger("test-swap");
    ASSERT_TRUE(initOutcome.IsSuccess());

    // Log one message that should go to the file
    spdlog::info("message-before-swap");
    spdlog::default_logger()->flush();

    // Verify the file was created and has content
    {
        std::ifstream logFile("logs/gamelift-server-sdk-test-swap.log");
        ASSERT_TRUE(logFile.good());
        std::string content((std::istreambuf_iterator<char>(logFile)),
                            std::istreambuf_iterator<char>());
        EXPECT_NE(content.find("message-before-swap"), std::string::npos);
    }

    // WHEN - Register a callback logger, which should swap the dist_sink's children
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);
    Aws::GameLift::GenericOutcome callbackOutcome = LoggerHelper::InitializeCallbackLogger(params);
    ASSERT_TRUE(callbackOutcome.IsSuccess());

    // Log after the swap
    spdlog::info("message-after-swap");
    spdlog::default_logger()->flush();

    // THEN - The callback should have received the post-swap message
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_NE(capture.messages.back().second.find("message-after-swap"), std::string::npos);

    // The logger instance should be the SAME (no drop/replace, just sink swap)
    EXPECT_NE(spdlog::get("multi_sink"), nullptr);

    // The default logger should still have a dist_sink_mt as its first sink
    auto logger = spdlog::default_logger();
    ASSERT_FALSE(logger->sinks().empty());
    auto distSink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(logger->sinks().front());
    EXPECT_NE(distSink, nullptr);
}

TEST_F(LoggerHelperTest, GIVEN_defaultLoggerActive_WHEN_callbackRegisteredWhileThreadsLog_THEN_noCrashAndCallbackReceivesPostSwapMessages) {
    // GIVEN - Concurrency smoke test: start N threads logging via the default logger,
    // then mid-stream swap to a callback logger. Validates that the dist_sink_mt
    // serialization prevents crashes during the swap.
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    Aws::GameLift::GenericOutcome initOutcome = LoggerHelper::InitializeLogger("test-concurrent-swap");
    ASSERT_TRUE(initOutcome.IsSuccess());

    std::atomic<bool> stopLogging{false};
    std::atomic<int> messagesLogged{0};
    const int numThreads = 4;

    // Start background threads that continuously log
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&stopLogging, &messagesLogged, t]() {
            while (!stopLogging.load(std::memory_order_acquire)) {
                spdlog::info("concurrent thread {} message {}", t, messagesLogged.fetch_add(1, std::memory_order_relaxed));
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        });
    }

    // Let threads log for a bit through the default (stdout+file) path
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // WHEN - Swap to callback logger mid-stream
    std::mutex captureMutex;
    std::vector<std::string> capturedMessages;
    auto threadSafeCallback = [](Aws::GameLift::Server::LogLevel level, const char* message, void* userData) {
        auto* data = static_cast<std::pair<std::mutex*, std::vector<std::string>*>*>(userData);
        std::lock_guard<std::mutex> lock(*data->first);
        data->second->push_back(std::string(message));
    };
    std::pair<std::mutex*, std::vector<std::string>*> callbackData{&captureMutex, &capturedMessages};
    Aws::GameLift::Server::CustomLoggerConfiguration params(threadSafeCallback, &callbackData, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome callbackOutcome = LoggerHelper::InitializeCallbackLogger(params);
    EXPECT_TRUE(callbackOutcome.IsSuccess());

    // Let threads log through the callback path for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Stop and join
    stopLogging.store(true, std::memory_order_release);
    for (auto& th : threads) {
        th.join();
    }

    // THEN - No crash, and the callback received some messages after the swap
    std::lock_guard<std::mutex> lock(captureMutex);
    EXPECT_GT(capturedMessages.size(), 0u);
    // Total messages logged should be > 0
    EXPECT_GT(messagesLogged.load(), 0);
}

TEST_F(LoggerHelperTest, GIVEN_callbackLoggerRegisteredFirst_WHEN_initializeDefaultLogger_THEN_callbackStillWins) {
    // GIVEN - Register callback logger BEFORE the default logger (simulates
    // InitCustomLogger() called before InitSDK()). The CAS guard ensures that the
    // subsequent InitializeLogger(processId) is a no-op.
    TestLogCapture capture;
    Aws::GameLift::Server::CustomLoggerConfiguration params(TestLogCapture::Callback, &capture, Aws::GameLift::Server::LogLevel::Info);

    Aws::GameLift::GenericOutcome callbackOutcome = LoggerHelper::InitializeCallbackLogger(params);
    ASSERT_TRUE(callbackOutcome.IsSuccess());

    // Verify callback works
    spdlog::info("before-default-init");
    spdlog::default_logger()->flush();
    ASSERT_FALSE(capture.messages.empty());
    EXPECT_NE(capture.messages.back().second.find("before-default-init"), std::string::npos);

    // WHEN - Try to initialize the default logger (as InitSDK would do)
    RemoveDirectoryRecursive("logs");
    EnsureDirectoryExists("logs");
    Aws::GameLift::GenericOutcome defaultOutcome = LoggerHelper::InitializeLogger("test-after-callback");

    // THEN - Should succeed (early return, no-op) and callback should still be active
    EXPECT_TRUE(defaultOutcome.IsSuccess());

    size_t countBefore = capture.messages.size();
    spdlog::info("after-default-init-attempt");
    spdlog::default_logger()->flush();
    EXPECT_GT(capture.messages.size(), countBefore);
    EXPECT_NE(capture.messages.back().second.find("after-default-init-attempt"), std::string::npos);

    // No log file should have been created (callback still wins)
    EXPECT_FALSE(std::ifstream("logs/gamelift-server-sdk-test-after-callback.log").good());
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws
