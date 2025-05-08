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
#include <aws/gamelift/internal/GameLiftCommonState.h>

namespace Aws {
namespace GameLift {
namespace Common {
namespace Test {

class GameLiftErrorsTest : public ::testing::Test {
protected:
#ifdef GAMELIFT_USE_STD
    // Constants Strings used for Tests
    const std::string ALREADY_INIT_ERROR_NAME = "Already Initialized";
    const std::string ALREADY_INIT_ERROR_MESSAGE = "GameLift has already been initialized. You must call Destroy() before reinitializing the "
                                                   "client or server.";
    const std::string PROCESS_NOT_READY_ERROR_NAME = "Process not ready.";
    const std::string PROCESS_NOT_READY_ERROR_MESSAGE = "The process has not yet been activated by calling ProcessReady(). Processes in standby "
                                                        "cannot receive StartGameSession callbacks.";
    const std::string BAD_REQUEST_ERROR_NAME = "Bad request exception.";
    const std::string INTERNAL_SERVICE_ERROR_NAME = "Internal service exception.";
    const std::string TEST_ERROR_NAME_1 = "test error name 1";
    const std::string TEST_ERROR_MESSAGE_1 = "test error message 1";
    const std::string TEST_ERROR_NAME_2 = "test error name 1";
    const std::string TEST_ERROR_MESSAGE_2 = "test error message 2";

    // Helper Method
    void VerifyError(const GameLiftError &error, const GAMELIFT_ERROR_TYPE &expectedErrorType, const std::string &expectedErrorName,
                     const std::string &expectedErrorMessage) {
        ASSERT_EQ(error.GetErrorType(), expectedErrorType);
        ASSERT_EQ(error.GetErrorName(), expectedErrorName);
        ASSERT_EQ(error.GetErrorMessage(), expectedErrorMessage);
    }
#else
    // Constant Strings used for Tests
    const char *ALREADY_INIT_ERROR_NAME = "Already Initialized";
    const char *ALREADY_INIT_ERROR_MESSAGE = "GameLift has already been initialized. You must call Destroy() before reinitializing the "
                                             "client or server.";
    const char *PROCESS_NOT_READY_ERROR_NAME = "Process not ready.";
    const char *PROCESS_NOT_READY_ERROR_MESSAGE = "The process has not yet been activated by calling ProcessReady(). Processes in standby "
                                                  "cannot receive StartGameSession callbacks.";
    const char *BAD_REQUEST_ERROR_NAME = "Bad request exception.";
    const char *INTERNAL_SERVICE_ERROR_NAME = "Internal service exception.";
    const char *TEST_ERROR_NAME_1 = "test error name 1";
    const char *TEST_ERROR_MESSAGE_1 = "test error message 1";
    const char *TEST_ERROR_NAME_2 = "test error name 1";
    const char *TEST_ERROR_MESSAGE_2 = "test error message 2";

    // Helper Method
    void VerifyError(const GameLiftError &error, const GAMELIFT_ERROR_TYPE &expectedErrorType, const char *expectedErrorName,
                     const char *expectedErrorMessage) {
        ASSERT_EQ(error.GetErrorType(), expectedErrorType);
        ASSERT_STREQ(error.GetErrorName(), expectedErrorName);
        ASSERT_STREQ(error.GetErrorMessage(), expectedErrorMessage);
    }
#endif
};

TEST_F(GameLiftErrorsTest, GIVEN_errorType_WHEN_constructError_THEN_success) {
    // GIVEN / WHEN
    GameLiftError error1(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED);
    GameLiftError error2(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY);
    // THEN
    VerifyError(error1, GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, ALREADY_INIT_ERROR_NAME, ALREADY_INIT_ERROR_MESSAGE);
    VerifyError(error2, GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY, PROCESS_NOT_READY_ERROR_NAME, PROCESS_NOT_READY_ERROR_MESSAGE);
}

TEST_F(GameLiftErrorsTest, GIVEN_errorTypeAndMessage_WHEN_constructError_THEN_success) {
    // GIVEN /WHEN
    GameLiftError error1(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_MESSAGE_1);
    GameLiftError error2(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY, TEST_ERROR_MESSAGE_2);
    // THEN
    VerifyError(error1, GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, ALREADY_INIT_ERROR_NAME, TEST_ERROR_MESSAGE_1);
    VerifyError(error2, GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY, PROCESS_NOT_READY_ERROR_NAME, TEST_ERROR_MESSAGE_2);
}

TEST_F(GameLiftErrorsTest, GIVEN_errorTypeAndNameAndMessage_WHEN_constructError_THEN_success) {
    // GIVEN / WHEN
    GameLiftError error1(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_NAME_1, TEST_ERROR_MESSAGE_1);
    GameLiftError error2(GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY, TEST_ERROR_NAME_2, TEST_ERROR_MESSAGE_2);
    // THEN
    VerifyError(error1, GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_NAME_1, TEST_ERROR_MESSAGE_1);
    VerifyError(error2, GAMELIFT_ERROR_TYPE::PROCESS_NOT_READY, TEST_ERROR_NAME_2, TEST_ERROR_MESSAGE_2);
}

TEST_F(GameLiftErrorsTest, GIVEN_statusCodeAndMessage_WHEN_constructError_THEN_success) {
    // GIVEN/WHEN
    GameLiftError error1(403, TEST_ERROR_MESSAGE_1);
    GameLiftError error2(500, TEST_ERROR_MESSAGE_2);
    // THEN
    VerifyError(error1, GAMELIFT_ERROR_TYPE::BAD_REQUEST_EXCEPTION, BAD_REQUEST_ERROR_NAME, TEST_ERROR_MESSAGE_1);
    VerifyError(error2, GAMELIFT_ERROR_TYPE::INTERNAL_SERVICE_EXCEPTION, INTERNAL_SERVICE_ERROR_NAME, TEST_ERROR_MESSAGE_2);
}

TEST_F(GameLiftErrorsTest, GIVEN_validInput_WHEN_getAndSetErrorFields_THEN_success) {
    // GIVEN
    GameLiftError error(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED);
    // WHEN
    error.SetErrorName(TEST_ERROR_NAME_1);
    error.SetErrorMessage(TEST_ERROR_MESSAGE_1);
    // THEN
    VerifyError(error, GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_NAME_1, TEST_ERROR_MESSAGE_1);
}

TEST_F(GameLiftErrorsTest, GIVEN_validError_WHEN_copyConstructor_THEN_success) {
    // GIVEN
    GameLiftError originalError(GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_NAME_1, TEST_ERROR_MESSAGE_1);
    // WHEN
    GameLiftError copiedError(originalError);
    // THEN
    VerifyError(copiedError, GAMELIFT_ERROR_TYPE::ALREADY_INITIALIZED, TEST_ERROR_NAME_1, TEST_ERROR_MESSAGE_1);
}

} // namespace Test
} // namespace Common
} // namespace GameLift
} // namespace Aws