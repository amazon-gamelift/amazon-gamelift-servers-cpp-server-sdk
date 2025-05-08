cmake_minimum_required(VERSION 3.1)

project(GameLiftServerSdk CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  ${CMAKE_MODULE_PATH})

# Set up some standard defaults, these will be passed down into external the
# projects.
include(BuildType)
include(BuildLocation)
include(ExternalProject)
include(download_dir)

# Set up a few default arguments for all projects, such as the install prefix,
# CMake prefix path and the runtime/library output directories.
option(BUILD_SHARED_LIBS "Build libraries as SHARED" OFF)
option(GAMELIFT_USE_STD "Use C++ std when building" ON)
option(BUILD_FOR_UNREAL "Flag to easily configure the sdk for Unreal." OFF)
option(RUN_CLANG_FORMAT "Flag to auto-format the sdk's source code, will increase build time" OFF)
option(RUN_UNIT_TESTS "Flag to run unit tests" ON)

if(BUILD_FOR_UNREAL)
# For Unreal, we always build our dependencies as static libraries and 'hide' them under the shared objects.
   set(BUILD_SHARED_LIBS OFF)
   set(GAMELIFT_USE_STD OFF)
endif()

set(GameLiftServerSdk_INSTALL_PREFIX "${GameLiftServerSdk_BINARY_DIR}/prefix")
set(GameLiftServerSdk_DEFAULT_ARGS
  "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
  "-DCMAKE_PREFIX_PATH:PATH=${GameLiftServerSdk_INSTALL_PREFIX};${CMAKE_PREFIX_PATH}"
  "-DCMAKE_INSTALL_PREFIX:PATH=${GameLiftServerSdk_INSTALL_PREFIX}"
  "-DGAMELIFT_USE_STD:BOOL=${GAMELIFT_USE_STD}"
  "-DBUILD_FOR_UNREAL:BOOL=${BUILD_FOR_UNREAL}"
  "-DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}")

# If there is a CMAKE_BUILD_TYPE it is important to ensure it is passed down.
if(CMAKE_BUILD_TYPE)
  list(APPEND GameLiftServerSdk_DEFAULT_ARGS
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
endif()

set(FORCE_STEP_DEFAULT "build")
if(CMAKE_CONFIGURATION_TYPES)
  set(FORCE_STEP_DEFAULT "configure")
endif()

set(FORCE_STEP ${FORCE_STEP_DEFAULT} CACHE STRING
  "Should the top level projects force configure/build/install each make")
set_property(CACHE FORCE_STEP PROPERTY STRINGS OFF configure build install)

if(FORCE_STEP STREQUAL configure)
  set(dependee "update")
elseif(FORCE_STEP STREQUAL build)
  set(dependee "configure")
elseif(FORCE_STEP STREQUAL install)
  set(dependee "build")
endif()

set(FORCE_STEP_ARGS
  DEPENDEES ${dependee}
  DEPENDERS ${FORCE_STEP}
)
# Add the third party dependencies first
add_subdirectory(thirdparty)

# Now for the actual projects!
# Always build the server SDK
include(External_serversdk)
# Can't unit test a dynamically linked library, since it doesn't export all data, functions, classes, and member functions

if(NOT RUN_UNIT_TESTS)
  message(STATUS "Unit tests are disabled by RUN_UNIT_TESTS=0. Skipping unit tests")
elseif(WIN32 AND BUILD_SHARED_LIBS)
  message(STATUS "Unit tests are not supported for windows build with dynamic linked library. Skipping unit tests")
elseif(BUILD_FOR_UNREAL)
  message(STATUS "Unit tests are not supported for unreal build. Skipping unit tests")
else()
  message(STATUS "Including unit tests in the build")
  include(External_serversdk-tests)
endif()

