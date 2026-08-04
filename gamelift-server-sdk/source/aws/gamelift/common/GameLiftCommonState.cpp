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

#include <aws/gamelift/internal/GameLiftCommonState.h>
#include <aws/gamelift/internal/util/LoggerHelper.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace Aws::GameLift;

// This is a shared pointer because many contexts (DLLs) may point to the same state.
Aws::GameLift::Internal::GameLiftCommonState * Aws::GameLift::Internal::GameLiftCommonState::m_instance;

Aws::GameLift::Internal::GameLiftCommonState::GameLiftCommonState() {}

Aws::GameLift::Internal::GameLiftCommonState::~GameLiftCommonState() {}

GenericOutcome Aws::GameLift::Internal::GameLiftCommonState::SetInstance(Aws::GameLift::Internal::GameLiftCommonState *instance) {
    // If there already is an instance, fail.
    if (m_instance) {
        return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED));
    }

    // take ownership of the new instance
    m_instance = instance;
    return GenericOutcome(nullptr);
}

Aws::GameLift::Internal::GetInstanceOutcome Aws::GameLift::Internal::GameLiftCommonState::GetInstance() {
    if (!m_instance) {
        return Internal::GetInstanceOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::NOT_INITIALIZED));
    }

    return m_instance;
}

Aws::GameLift::Internal::GetInstanceOutcome Aws::GameLift::Internal::GameLiftCommonState::GetInstance(Aws::GameLift::Internal::GAMELIFT_INTERNAL_STATE_TYPE stateType) {
    if (!m_instance) {
        return Internal::GetInstanceOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::NOT_INITIALIZED));
    }

    if (m_instance->GetStateType() != stateType) {
        return Internal::GetInstanceOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::INITIALIZATION_MISMATCH));
    }
    return m_instance;
}

GenericOutcome Aws::GameLift::Internal::GameLiftCommonState::DestroyInstance() {
    // The m_instance null check guards against double-destroy and prevents duplicate
    // fallback logger registration if DestroyInstance() is called more than once.
    if (m_instance) {
        // Warn BEFORE teardown swaps to the fallback logger so the message reaches the
        // integrator's callback sink (not just stdout). Must precede set_default_logger().
        // Ordering rationale: this runs on the caller's thread while the callback sink is
        // still installed, so no teardown race is introduced.
        if (LoggerHelper::IsCustomLoggerRegistered()) {
            // This warning is filtered out if the user's minimumLogLevel is above Warn — intentional, since they opted out of warn-level messages.
            spdlog::warn("Destroy() called with a custom log callback registered. "
                "Detached SDK handler threads may still invoke the callback after Destroy() returns; "
                "keep the callback, its module, and userData valid until process exit.");
        }

        // Deleting m_instance joins the health-check thread and runs destructors that may
        // log, so it must precede dropping/replacing the logger. NOTE: this does NOT join
        // the game-session/terminate/update handler threads, which are detached elsewhere
        // in GameLiftServerState. Those threads (or SDK code they call) can therefore still
        // log during/after this teardown, so this sequence cannot fully guarantee the custom
        // callback is idle -- it only prevents a null default logger. See CustomLoggerConfiguration.h
        // for the userData lifetime contract (valid until process exit, not until Destroy()).

        auto* instance = m_instance;
        m_instance = nullptr;
        delete instance;

        // Install the fallback BEFORE dropping the SDK logger so that spdlog's default
        // logger is never null -- a concurrent spdlog::info() from an application thread
        // between the drop and set would otherwise dereference a null pointer.
        // We construct the logger directly (not via stdout_color_mt) to avoid a
        // duplicate-name exception if DestroyInstance() is ever called twice.
        // NOTE: Under the dist_sink_mt design, the "multi_sink" logger's dist_sink still
        // exists and holds shared_ptrs to its child sinks (callback or stdout/file). Dropping
        // "multi_sink" from the registry and replacing the default logger releases those
        // references, allowing the child sinks (and thus the CallbackSink) to be destroyed.
        auto fallback = std::make_shared<spdlog::logger>(
            "fallback", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        spdlog::set_default_logger(fallback);
        LoggerHelper::DropSdkLogger();

        // Reset the custom logger flag so a subsequent InitCustomLogger() call succeeds.
        LoggerHelper::ResetCustomLoggerRegistered();

        return GenericOutcome(nullptr);
    }
    return GenericOutcome(GameLiftError(GAMELIFT_ERROR_TYPE::NOT_INITIALIZED));
}
