set(serversdk_test_source "${CMAKE_CURRENT_SOURCE_DIR}/gamelift-server-sdk-tests")
set(serversdk_test_build "${CMAKE_CURRENT_BINARY_DIR}/gamelift-server-sdk-tests")

set(_deps "aws-cpp-sdk-gamelift-server")
if (RUN_CLANG_FORMAT)
    list(APPEND _deps "clang-format")
endif()

ExternalProject_Add(aws-cpp-sdk-gamelift-server-tests
        SOURCE_DIR ${serversdk_test_source}
        BINARY_DIR ${serversdk_test_build}
        DEPENDS ${_deps}
        CMAKE_CACHE_ARGS
            ${GameLiftServerSdk_DEFAULT_ARGS}
            -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
            -DCLANG_FORMAT_EXECUTABLE_PATH:PATH=${CLANG_FORMAT_EXECUTABLE_PATH}
        INSTALL_COMMAND ""
)