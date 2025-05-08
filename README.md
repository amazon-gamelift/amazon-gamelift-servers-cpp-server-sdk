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

### Out of source build

To build the server SDK, follow these instructions for your operating system:

#### Linux

```sh
mkdir out
cd out
cmake ..
make
```

#### Windows

1. Create the build directory.
    ```sh
    mkdir out
    cd out
    ```
1. Make the solution and project files.
   *Note: you might see warnings such as `Detected a Windows Shared STD/NOSTD or Unreal build. Skipping unit tests`.
   These are expected and indicate that certain features are not included for the specific options passed.*
    ```
    cmake -G "Visual Studio 15 2017 Win64" ..
    ```
   or for VS2022+
    ```
    cmake -G "Visual Studio 17 2022" ..
    ```
1. Compile the solution for release.
    ```
    msbuild ALL_BUILD.vcxproj /p:Configuration=Release
    ```

This SDK is known to work with these CMake generators:

* Visual Studio 17 2022 Win64
* Visual Studio 16 2019 Win64
* Visual Studio 15 2017 Win64
* Visual Studio 14 2015 Win64
* Visual Studio 12 2013 Win64
* Visual Studio 11 2012 Win64
* Unix MakeFiles

After running 'msbuild', you can edit or test your server SDK with the solutions file '
out/gamelift-server-sdk/aws-cpp-sdk-gamelift-server.sln'.

### CMake options

#### BUILD_FOR_UNREAL

Optional variable to build the SDK for use with Unreal Engine. Set variable to 1 to create an Unreal build. Default
setting is false.
With this option enabled, all dependencies are built statically and then rolled into a single shared object library.

```sh
cmake -DBUILD_FOR_UNREAL=1 ..
```

#### BUILD_SHARED_LIBS

Optional variable to select either a static or dynamic build. Default setting is **static**. Set variable to 1 for
dynamic.

```sh
cmake -DBUILD_SHARED_LIBS=1 ..
```

#### GAMELIFT_USE_STD

Optional variable to use the C++ standard library when building. Default setting is **true**.
You can build the Amazon GameLift server SDK to use the C++ standard library (#include <cstdio> and using namespace
std;) or use the C library functionality (#include <string.h> for example).

To build the server SDK libraries using the C++ standard library, use the usual form for building the SDK. For example,
for Windows:

```sh
mkdir out
cd out
cmake -G "Visual Studio 14 2015 Win64" ..
msbuild ALL_BUILD.vcxproj /p:Configuration=Release
```

This implies the use of the default -DGAMELIFT_USE_STD=1 flag.

To turn off the dependency with the C++ std:: library, substitute the following cmake command.

    cmake -G "Visual Studio 14 2015 Win64" -DGAMELIFT_USE_STD=0 ..

Whether or not the C++ standard library should be used is generally a matter of preference, but there are some
considerations.

* Use of -DBUILD_FOR_UNREAL=1 will override use of -DGAMELIFT_USE_STD=1 and the std:: library will not be used in an
  unreal build.
* Use of the -DGAMELIFT_USE_STD=0 flag modifies certain SDK API function prototypes, because std::string will not be
  accepted as a parameter in this case. When using libraries built with -DGAMELIFT_USE_STD=1 in your application, it is
  important that you continue to define the GAMELIFT_USE_STD=1 preprocessor definition prior to including the headers:
  ```sh
    #define GAMELIFT_USE_STD=1  
    #include "aws\gamelift\server\GameLiftServerAPI.h"
  ```
  Otherwise the prototype in the built library will not match the prototype in the header that you are including and you
  will get compiler errors such as these:
    ```sh
      error C2039: 'InitSDKOutcome' : is not a member of 'Aws::GameLift::Server'
      error C2065: 'InitSDKOutcome' : undeclared identifier
    ```

  Likewise, if the libraries are built with -DGAMELIFT_USE_STD=0, then the following is usual (though not required):
  ```sh
    #define GAMELIFT_USE_STD=0
    #include "aws\gamelift\server\GameLiftServerAPI.h" GAMELIFT_USE_STD 
  ```

#### CMAKE_BUILD_TYPE

Option to build in either debug or release. Options are Debug or Release (case sensitive). Default is Release.

```sh
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

#### RUN_CLANG_FORMAT

Option to automatically run clang-format as part of the build process over all SDK and test source code. This will take
some time, but correct any styling inconsistencies.

## Common Issues

### File path too long errors when running msbuild

* **Cause**: There is a long running issue with Visual Studio where regardless of settings for long file/folder paths in
  Windows Registry, some of the file IO functions will still only allow for a short file/folder path.
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