CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(aws-cpp-sdk-gamelift-server-tests)
SET(CMAKE_CXX_STANDARD 11)
SET(TARGET_NAME aws-cpp-sdk-gamelift-server-tests)

if(GAMELIFT_USE_STD)
    message("GameLift SDK will use STD in its interface")
    add_definitions(-DGAMELIFT_USE_STD)
endif(GAMELIFT_USE_STD)

# -----------------------------
# Setup Google Test (from github)
# -----------------------------
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)
enable_testing()

# -----------------------------
# Create Test Executable
# -----------------------------
set(GAMELIFT_TEST_ROOT ${CMAKE_SOURCE_DIR})
file(GLOB AWS_GAMELIFT_COMMON_TEST  "" "${GAMELIFT_TEST_ROOT}/source/aws/gamelift/common/*.cpp")
file(GLOB AWS_GAMELIFT_SERVER_TEST  "" "${GAMELIFT_TEST_ROOT}/source/aws/gamelift/server/*.cpp")
file(GLOB_RECURSE AWS_GAMELIFT_INTERNAL_TEST  "" "${GAMELIFT_TEST_ROOT}/source/aws/gamelift/internal/*.cpp")
file(GLOB AWS_GAMELIFT_MODEL_TEST  "" "${GAMELIFT_TEST_ROOT}/source/aws/gamelift/server/model/*.cpp")
file(GLOB AWS_GAMELIFT_UTILITY  "" "${GAMELIFT_TEST_ROOT}/include/aws/gamelift/utility/*.h")
set(GAMELIFT_TEST_SRC
        ${AWS_GAMELIFT_COMMON_TEST}
        ${AWS_GAMELIFT_SERVER_TEST}
        ${AWS_GAMELIFT_INTERNAL_TEST}
        ${AWS_GAMELIFT_MODEL_TEST}
        ${AWS_GAMELIFT_UTILITY}
)
add_executable(
        ${TARGET_NAME}
        ${GAMELIFT_TEST_SRC}
)

# -----------------------------
# Set up include directories
# -----------------------------

# serversdk
find_package(aws-cpp-sdk-gamelift-server REQUIRED)

target_include_directories(${TARGET_NAME}
    PUBLIC
        # Build interface will point to the SDK's include dir, eg: <root>/gamelift-server-sdk-tests/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${SERVERSDK_INCLUDE_DIR}>
)

# OpenSSL
find_package(OpenSSL REQUIRED)

# -----------------------------
# Set up link targets
# -----------------------------
target_link_libraries(${TARGET_NAME}
    PUBLIC
        gtest_main
        gmock_main
        ${SERVERSDK_LIBRARIES}
)

# Run clang-format if it exists
if (NOT CLANG_FORMAT_EXECUTABLE_PATH STREQUAL "")
    add_custom_command(
            TARGET ${TARGET_NAME}
            PRE_BUILD
            COMMENT "Running clang-format over the SDK test source..."
            COMMAND ${CLANG_FORMAT_EXECUTABLE_PATH} --Werror --style=file -i ${GAMELIFT_TEST_SRC}
    )
endif()

# -----------------------------
# Initialize GoogleTests
# -----------------------------
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})

# Run all other tests
add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMENT "Run Unit Tests"
        COMMAND ${TARGET_NAME}
)
