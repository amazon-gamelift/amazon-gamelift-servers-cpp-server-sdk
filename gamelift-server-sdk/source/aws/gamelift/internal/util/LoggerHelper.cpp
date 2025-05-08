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

#ifdef GAMELIFT_USE_STD
void LoggerHelper::InitializeLogger(const std::string& process_Id) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::string serverSdkLog = "logs/gamelift-server-sdk-";
    serverSdkLog.append(process_Id).append(".log");
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(serverSdkLog, 10485760, 5);

    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] %v%$");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    spdlog::logger logger("multi_sink", { console_sink, file_sink });
    logger.set_level(spdlog::level::info);
    logger.flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
}
#else
void LoggerHelper::InitializeLogger(const char* process_Id) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::string serverSdkLog = "logs/gamelift-server-sdk-";
    serverSdkLog.append(process_Id).append(".log");
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(serverSdkLog, 10485760, 5);

    console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] %v%$");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    spdlog::logger logger("multi_sink", { console_sink, file_sink });
    logger.set_level(spdlog::level::info);
    logger.flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
}
#endif

