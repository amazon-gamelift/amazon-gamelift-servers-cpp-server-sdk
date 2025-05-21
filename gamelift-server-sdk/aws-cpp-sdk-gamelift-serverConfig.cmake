# - Config file for the serversdk package
# It defines the following variables
#  SERVERSDK_FOUND        - system has serversdk
#  SERVERSDK_INCLUDE_DIR  - include directory for serversdk
#  SERVERSDK_LIBRARY      - the serversdk library
#  SERVERSDK_LIBRARIES    - the serversdk & dependencies libraries

find_path(SERVERSDK_INCLUDE_DIR
          NAMES aws/gamelift/server/GameLiftServerAPI.h)
find_library(SERVERSDK_LIBRARY
             NAMES aws-cpp-sdk-gamelift-server)

# Dependency libraries
find_package(OpenSSL REQUIRED)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SERVERSDK DEFAULT_MSG
  SERVERSDK_INCLUDE_DIR SERVERSDK_LIBRARY)

if(SERVERSDK_FOUND)
  set(SERVERSDK_LIBRARIES ${SERVERSDK_LIBRARY} ${OPENSSL_LIBRARIES})
endif()

mark_as_advanced(SERVERSDK_INCLUDE_DIR SERVERSDK_LIBRARY SERVERSDK_LIBRARIES)
