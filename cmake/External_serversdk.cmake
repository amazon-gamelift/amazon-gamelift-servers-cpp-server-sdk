set(serversdk_source "${CMAKE_CURRENT_SOURCE_DIR}/gamelift-server-sdk")
set(serversdk_build "${CMAKE_CURRENT_BINARY_DIR}/gamelift-server-sdk")

set(_deps "rapidjson" "websocketpp")
if (RUN_CLANG_FORMAT)
  list(APPEND _deps "clang-format")
endif()

ExternalProject_Add(aws-cpp-sdk-gamelift-server
  SOURCE_DIR ${serversdk_source}
  BINARY_DIR ${serversdk_build}
  BUILD_ALWAYS 1
  CMAKE_CACHE_ARGS
    ${GameLiftServerSdk_DEFAULT_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
    -DWEBSOCKET_LIB:STRING=${WEBSOCKET_LIB}
    -DPREFIX_INCLUDE_DIR:PATH=${GameLiftServerSdk_INSTALL_PREFIX}/include
    -DCLANG_FORMAT_EXECUTABLE_PATH:PATH=${CLANG_FORMAT_EXECUTABLE_PATH}
  DEPENDS
    ${_deps}
)

if(FORCE_STEP)
  ExternalProject_Add_Step(aws-cpp-sdk-gamelift-server forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of aws-cpp-sdk-gamelift-server"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
