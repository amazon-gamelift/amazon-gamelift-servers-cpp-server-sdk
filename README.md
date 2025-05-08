# GameLiftServerSdk C++

## Documentation

You can find the official Amazon GameLift documentation [here](https://aws.amazon.com/documentation/gamelift/).

## Minimum requirements:

* Either of the following:
    * Microsoft Visual Studio 2012 or later
    * GNU Compiler Collection (GCC) 4.9 or later
* CMake version 3.1 or later
* A Git client available on the PATH.
* OpenSSL installation

## Prerequisites

### OpenSSL

1. Install the full version of OpenSSL from https://wiki.openssl.org/index.php/Binaries for the appropriate platform.
   *  Note: For Unreal Engine, the version of OpenSSL should match the version used by Unreal to package your game server.
      The version can be identified under the Unreal installation directory `Engine\Source\ThirdParty\OpenSSL`.
2. Follow the installation instructions provided by OpenSSL.
3. Once installed, navigate to the OpenSSL folder (for example: C:\Program Files\OpenSSL-Win64). **COPY** the following
   two DLLs and save them to a safe location for later use (such as your desktop).  
   **These two DLLs must be included in your game server build when you upload it to Amazon GameLift.**
    * libssl-3-x64.dll
    * libcrypto-3-x64.dll
4. Note the directory where OpenSSL is installed and follow the instructions below for each platform.

#### Mac

1. Add the following environment variables to your **.zshrc** file by running `vim ~/.zshrc`. Replace the bracketed
   values (<>) with the path to your OpenSSL directory.
    ```zsh
    export OPENSSL_ROOT_DIR=<PATH_TO_OPENSSL_DIR>
    export OPENSSL_LIBRARIES=<PATH_TO_OPENSSL_DIR>/lib
    ```
2. After updating **.zshrc**, run `source ~/.zshrc` to apply the changes in any existing terminal sessions.

#### Windows

1. Open **System Properties > Environmental Varaibles**.
2. Add your OpenSSL install directory to your PATH = `<PATH_TO_OPENSSL_DIR>\bin`.
3. Create the following environment variables:
    ```
    OPENSSL_INCLUDE_DIR = <PATH_TO_OPENSSL_DIR>\include
    OPENSSL_LIBRARIES = <PATH_TO_OPENSSL_DIR>\lib
    OPENSSL_ROOT_DIR = <PATH_TO_OPENSSL_DIR>\OpenSSL
    ```

### CMake 3.1+

1. Download CMake 3.1+ from https://cmake.org/download/ for the appropriate platform.
2. Follow the installation instructions provided by CMake.

### Python 3.6+

1. Download Python 3.6+ from https://www.python.org/downloads/ for the appropriate platform.
2. Follow the installation instructions provided by the above site.

## Building the SDK

To build the server SDK, follow these instructions for your operating system:

### Linux

1. Create the build directory.
   ```
   mkdir cmake-build
   ```
2. Make the solution and project files.
   ```
   cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B ./cmake-build
   ```
3. Compile the solution.
   ```
   cmake --build cmake-build --target all
   ```
4. Find the compiled binaries in **prefix/lib**

### Windows

1. **IMPORTANT** Move the downloaded SDK source code to **C:\GameLiftServerSdkCpp** to prevent build failure due to
   path exceeding 260 path length limit.
2. Create the build directory.
   ```
   mkdir cmake-build
   ```
3. Make the solution and project files.
   *Note: you might see warnings such as `Detected a Windows Shared STD/NOSTD or Unreal build. Skipping unit tests`.
   These are expected and indicate that certain features are not included for the specific options passed.*
   ```
   cmake.exe -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -S . -B ./cmake-build
   ```
4. Compile the solution for release.
   ```
   cmake.exe --build ./cmake-build --target ALL_BUILD
   ```
5. Find the compiled binaries in **prefix\bin**

## CMake options

### Generator and architecture

This SDK is known to work with these CMake generators and architectures:

| Generator (-G flag)     | Valid Target Architecture (-A flag) | Supported Operating System |
|-------------------------|-------------------------------------|----------------------------|
| `Visual Studio 17 2022` | `win32`, `x64`, `arm`, `arm64`      | Windows                    |
| `Visual Studio 16 2019` | `win32`, `x64`, `arm`, `arm64`      | Windows                    |
| `Visual Studio 15 2017` | `win32`, `x64`, `arm`, `arm64`      | Windows                    |
| `Visual Studio 14 2015` | `win32`, `x64`, `arm`               | Windows                    |
| `Visual Studio 12 2013` | `win32`, `x64`, `arm`               | Windows                    |
| `Visual Studio 11 2012` | `win32`, `x64`, `arm`               | Windows                    |
| `Unix MakeFiles`        | Unsupported, host architecture only | Linux                      |

### BUILD_FOR_UNREAL

Optional variable to build the SDK for use with Unreal Engine. When enabled, all dependencies are built statically and 
then rolled into a single shared object library. 

#### Available options
* `0` **(Default)**: Disable build for unreal
* `1`: Enable build for unreal.

#### Example
```
-DBUILD_FOR_UNREAL=1
```

### BUILD_SHARED_LIBS

Optional variable to select static linking or dynamic linking.

#### Available options
* `0` **(Default)**: Use static linking.
* `1`: use dynamic linking.

#### Example
```
-DBUILD_SHARED_LIBS=1
```

### GAMELIFT_USE_STD

Optional variable to select whether using the C++ standard library in the build. 

Whether the C++ standard library should be used is generally a matter of preference, but there are some
considerations.

* Use of `-DBUILD_FOR_UNREAL=1` will override use of `-DGAMELIFT_USE_STD=1` and the std library will not be used in an
  unreal build.
* Use of the `-DGAMELIFT_USE_STD=0` flag modifies certain SDK API function prototypes, because `std::string` will not be
  accepted as a parameter in this case. When using libraries built with `-DGAMELIFT_USE_STD=1` in your application, it is
  important that you continue to define the `GAMELIFT_USE_STD=1` preprocessor definition prior to including the headers:
  ```
  #define GAMELIFT_USE_STD=1  
  #include "aws\gamelift\server\GameLiftServerAPI.h"
  ```
  Otherwise, the prototype in the built library will not match the prototype in the header that you are including, and you
  will get compiler errors such as these:
  ```
  error C2039: 'InitSDKOutcome' : is not a member of 'Aws::GameLift::Server'
  error C2065: 'InitSDKOutcome' : undeclared identifier
  ```

  Likewise, if the libraries are built with `-DGAMELIFT_USE_STD=0`, then the following is acceptable though not required:
  ```
  #define GAMELIFT_USE_STD=0
  #include "aws\gamelift\server\GameLiftServerAPI.h" GAMELIFT_USE_STD 
  ```

#### Available options
* `1` **(Default)**: Use C++ standard library, for example `#include <cstdio>`.
* `0`: Use C library functionality, for example `#include <string.h>`.

#### Example
```
-DGAMELIFT_USE_STD=0
```

### CMAKE_BUILD_TYPE

Option to specify the build type.

#### Available options

* `Release` **(Default)**: Set build type to release. This option should be used in most cases. 
* `Debug`: Set build type to debug. This option will generate PDB file to help debugging.

#### Example
```
-DCMAKE_BUILD_TYPE=Debug
```

### RUN_CLANG_FORMAT

Option to automatically run clang-format as part of the build process over all SDK and test source code, and 
automatically correct any styling inconsistencies. Note that this will add significant amount of build time.

#### Available options
* `0` **(Default)**: Disable clang-format
* `1`: Enable clang-format

#### Example
```
-DRUN_CLANG_FORMAT=1
```

### RUN_UNIT_TESTS

Option to run unit tests as part of the build process.

#### Available options
* `1` **(Default)**: Enable unit tests
* `0`: Disable unit tests

#### Example
```
-DRUN_UNIT_TESTS=0
```

## Common Issues

### File path too long errors when running msbuild

* **Cause**: There is a known issue with Visual Studio where regardless of settings for long file/folder paths in
  Windows Registry, some file IO functions will still only allow for a short file/folder path.
* **Mitigation**: While not ideal, make sure you are building the Amazon GameLift and AWS SDKs with as short a folder
  structure as possible. If you encounter the issue, then try moving your project to a folder closer to the root.

### Runtime error for missing libssl and/or libcrypto DLLs

```
LogWindows: Failed to load ‘../GameLiftServerSDK/ThirdParty/GameLiftServerSDK/Win64/aws-cpp-sdk-gamelift-server.dll’ (GetLastError=0)
LogWindows: Missing import: libssl-3-x64.dll
LogWindows: Missing import: libcrypto-3-x64.dll
```

* **Cause**: When uploading your application to Amazon GameLift, the related OpenSSL DLLs were not included in the
  application's path.
* **Mitigation**: Using the same version of OpenSSL that you built the Amazon GameLift SDK with, go to the OpenSSL
  folder and find the libssl-3-x64.dll and libcrypto-3-x64.dll DLLs in the root folder. Re-upload your application with
  these DLLs included.
  * For Unreal Engine, the version of OpenSSL should match the version used by Unreal to package your game server.
    The version can be identified under the Unreal installation directory `Engine\Source\ThirdParty\OpenSSL`.