CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(aws-cpp-sdk-gamelift-server)
SET(CMAKE_CXX_STANDARD 11)
SET(TARGET_NAME aws-cpp-sdk-gamelift-server)

if(MSVC)
    # Unlock object file size limit
    add_compile_options(/bigobj)
endif()

# We need to find the local openssl .dll or .so files BEFORE setting CMAKE_FIND_LIBRARY_SUFFIXES, since openssl will
# always be dynamically linked. All other libraries used by the SDK may be statically linked, depending on the flags
# BUILD_FOR_UNREAL and BUILD_SHARED_LIBS.
find_package(OpenSSL REQUIRED)

if(BUILD_FOR_UNREAL)
   set(BUILD_SHARED_LIBS ON)
   set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

if(GAMELIFT_USE_STD)
    message("GameLift SDK will use STD in its interface")
    add_definitions(-DGAMELIFT_USE_STD)
endif(GAMELIFT_USE_STD)

# When cross compiling using something other than gcc, we need to tell rapidjson what endianness to use
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "" AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL CMAKE_SYSTEM_PROCESSOR AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_BYTE_ORDER STREQUAL LITTLE_ENDIAN)
        message(STATUS "Detected cmake is cross-compiling to a different architecture, setting rapidjson to little-endian to match host")
        add_definitions(-DRAPIDJSON_ENDIAN=0)
    else()
        message(STATUS "Detected cmake is cross-compiling to a different architecture, setting rapidjson to big-endian to match host")
        add_definitions(-DRAPIDJSON_ENDIAN=1)
    endif()
endif()

# -----------------------------
# Set up library
# -----------------------------

set(GAMELIFT_SOURCE_ROOT ${CMAKE_SOURCE_DIR})
file(GLOB AWS_GAMELIFT_COMMON_HEADERS "" "${GAMELIFT_SOURCE_ROOT}/include/aws/gamelift/common/*.h*")
file(GLOB AWS_GAMELIFT_COMMON_SOURCE  "" "${GAMELIFT_SOURCE_ROOT}/source/aws/gamelift/common/*.cpp")
file(GLOB AWS_GAMELIFT_SERVER_HEADERS "" "${GAMELIFT_SOURCE_ROOT}/include/aws/gamelift/server/*.h*")
file(GLOB AWS_GAMELIFT_SERVER_SOURCE  "" "${GAMELIFT_SOURCE_ROOT}/source/aws/gamelift/server/*.cpp")
file(GLOB AWS_GAMELIFT_MODEL_HEADERS "" "${GAMELIFT_SOURCE_ROOT}/include/aws/gamelift/server/model/*.h*")
file(GLOB_RECURSE AWS_GAMELIFT_INTERNAL_HEADERS "" "${GAMELIFT_SOURCE_ROOT}/include/aws/gamelift/internal/*.h")
file(GLOB_RECURSE AWS_GAMELIFT_INTERNAL_SOURCE "" "${GAMELIFT_SOURCE_ROOT}/source/aws/gamelift/internal/*.cpp")
set(GAMELIFT_SERVER_SRC
    ${AWS_GAMELIFT_INTERNAL_HEADERS}
    ${AWS_GAMELIFT_INTERNAL_SOURCE}
    ${AWS_GAMELIFT_COMMON_HEADERS}
    ${AWS_GAMELIFT_MODEL_HEADERS}
    ${AWS_GAMELIFT_SERVER_HEADERS}
    ${AWS_GAMELIFT_COMMON_SOURCE}
    ${AWS_GAMELIFT_SERVER_SOURCE}
)

add_library(${TARGET_NAME} ${GAMELIFT_SERVER_SRC})

# -----------------------------
# Configure WebSocket Library
# -----------------------------
add_definitions(-DASIO_STANDALONE)
if(WIN32)
    add_definitions(-D_WEBSOCKETPP_CPP11_STRICT_)
endif()

# -----------------------------
# Set up include directories
# -----------------------------

if(WIN32)
    add_definitions(/wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305 /wd4065 /wd4506)
endif()

target_include_directories(${TARGET_NAME}
    PUBLIC
        # Build interface will point to the SDK's include dir, eg: <root>/gamelift-server-sdk/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        # Build interface to include all prefix include directory header files. This will contain asio, websocketpp, and rapidjson headers.
        $<BUILD_INTERFACE:${PREFIX_INCLUDE_DIR}>
        # Include asio subdirectory, since asio is structured in strange way that requires us to import both the root (live above) and the asio sub-directory (line below)
        $<BUILD_INTERFACE:${PREFIX_INCLUDE_DIR}/asio>
        # Include openssl, which is in a separate directory
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
)

# -----------------------------
# Set up link targets
# -----------------------------
if(BUILD_SHARED_LIBS)    
    #Exporting symbols on the shared object/dll.
    add_definitions(-DUSE_IMPORT_EXPORT)
    add_definitions(-DAWS_GAMELIFT_EXPORTS)
endif(BUILD_SHARED_LIBS)

if(BUILD_FOR_UNREAL AND UNIX)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Clang not supported; CMake will exit.")
    else ()
        # We roll all dependencies static libraries under a single shared object file.
        set(LD_WHOLE_ARCHIVE "-Wl,--whole-archive")
        set(LD_NO_WHOLE_ARCHIVE "-Wl,--no-whole-archive")

        set(LD_VERSION_SCRIPT "-Wl,--version-script=${GAMELIFT_SOURCE_ROOT}/exports.version")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LD_VERSION_SCRIPT}")

        target_link_libraries(${TARGET_NAME}
            PRIVATE
                ${LD_WHOLE_ARCHIVE} ${LD_NO_WHOLE_ARCHIVE} ${OPENSSL_LIBRARIES}
        )
    endif ()
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            ${OPENSSL_LIBRARIES}
    )
endif()

# If we are compiling for visual studio, create a sane directory tree.
if(MSVC)
    source_group("Header Files\\aws\\gamelift\\common" FILES ${AWS_GAMELIFT_COMMON_HEADERS})
    source_group("Header Files\\aws\\gamelift\\server" FILES ${AWS_GAMELIFT_SERVER_HEADERS})
    source_group("Header Files\\aws\\gamelift\\internal" FILES ${AWS_GAMELIFT_INTERNAL_HEADERS})
    source_group("Header Files\\aws\\gamelift\\internal\\network" FILES ${AWS_GAMELIFT_INTERNAL_NETWORK_HEADERS})
    source_group("Header Files\\aws\\gamelift\\server\\model" FILES ${AWS_GAMELIFT_MODEL_HEADERS})
    source_group("Source Files\\common" FILES ${AWS_GAMELIFT_COMMON_SOURCE})
    source_group("Source Files\\internal" FILES ${AWS_GAMELIFT_INTERNAL_SOURCE})
    source_group("Source Files\\server" FILES ${AWS_GAMELIFT_SERVER_SOURCE})
    add_definitions("/Zi")
    # Make the Release version create a PDB
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    # Enable Generate Debug Info for both debug and release configure
    set_property(TARGET aws-cpp-sdk-gamelift-server APPEND PROPERTY LINK_FLAGS /DEBUG)
    # Minimize the size of the resulting DLLs
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF")
endif()

# Run clang-format if it exists
if (NOT CLANG_FORMAT_EXECUTABLE_PATH STREQUAL "")
    add_custom_command(
            TARGET ${TARGET_NAME}
            PRE_BUILD
            COMMENT "Running clang-format over the SDK source..."
            COMMAND ${CLANG_FORMAT_EXECUTABLE_PATH} --Werror --style=file -i ${GAMELIFT_SERVER_SRC}
    )
endif()

# -----------------------------
# Set up install targets
# -----------------------------

install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)
install(FILES ${TARGET_NAME}Config.cmake
        DESTINATION cmake
)
install(DIRECTORY include/
        DESTINATION include
)
