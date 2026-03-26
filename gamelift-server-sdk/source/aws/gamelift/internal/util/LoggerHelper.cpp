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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

using namespace Aws::GameLift::Internal;

namespace {

constexpr const char *LOG_PATTERN_CONSOLE = "%^[%Y-%m-%d %H:%M:%S] [%l] [%t] %v%$";
constexpr const char *LOG_PATTERN_FILE = "[%Y-%m-%d %H:%M:%S] [%l] [%t] %v";
constexpr size_t MAX_LOG_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
constexpr size_t MAX_LOG_FILES = 5;

void ConfigureLogger(const std::string &processId) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::string serverSdkLog = "logs/gamelift-server-sdk-";
    serverSdkLog.append(processId).append(".log");
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(serverSdkLog, MAX_LOG_FILE_SIZE, MAX_LOG_FILES);

    console_sink->set_pattern(LOG_PATTERN_CONSOLE);
    file_sink->set_pattern(LOG_PATTERN_FILE);

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(logger);
}

} // anonymous namespace

#ifdef GAMELIFT_USE_STD
void LoggerHelper::InitializeLogger(const std::string &process_Id) {
    ConfigureLogger(process_Id);
}
#else
void LoggerHelper::InitializeLogger(const char *process_Id) {
    ConfigureLogger(process_Id ? process_Id : "unknown");
}
#endif

