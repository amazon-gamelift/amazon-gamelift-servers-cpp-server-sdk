if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
ExternalProject_add(clang-format
        URL "https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/llvm-project-13.0.1.src.tar.xz"
        SOURCE_SUBDIR llvm
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm-project
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm-project-bin
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        USES_TERMINAL_DOWNLOAD 1
        USES_TERMINAL_CONFIGURE 1
        USES_TERMINAL_BUILD 1
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
            -DBUILD_SHARED_LIBS=OFF
            -DLLVM_ENABLE_ZLIB=OFF
            -DLLVM_ENABLE_ZSTD=OFF
            -DLLVM_ENABLE_PROJECTS=clang
        BUILD_COMMAND ${CMAKE_COMMAND} --build . --target clang-format --config Release
)

if (MSVC)
    set(CLANG_FORMAT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm-project-bin/Release/bin)
else()
    set(CLANG_FORMAT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm-project-bin/bin)
endif()
set(CLANG_FORMAT_EXECUTABLE_PATH ${CLANG_FORMAT_BIN_DIR}/clang-format${CMAKE_EXECUTABLE_SUFFIX} PARENT_SCOPE)
