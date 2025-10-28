# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Download only concurrentqueue.h and install it to the moodycamel include path:
# ${GameLiftServerSdk_INSTALL_PREFIX}/include/moodycamel/concurrentqueue.h
EXTERNALPROJECT_ADD(concurrentqueue
    PREFIX "concurrentqueue"
    URL https://raw.githubusercontent.com/cameron314/concurrentqueue/v1.0.4/concurrentqueue.h
    DOWNLOAD_NO_EXTRACT 1
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -E make_directory ${GameLiftServerSdk_INSTALL_PREFIX}/include/
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/concurrentqueue/src/concurrentqueue.h
        ${GameLiftServerSdk_INSTALL_PREFIX}/include/concurrentqueue.h
    CMAKE_CACHE_ARGS
    ${GameLiftServerSdk_DEFAULT_ARGS}
)