# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Rapidjson is a header-only library, so no need to look for a system version
include(External_rapidjson)

# websocketpp is a header-only library, so no need to look for system version
include(External_websocketpp)

# spdlog is a header-only library, so no need to look for system version
include(External_spdlog)

# concurrentqueue is a header-only library, so no need to look for system version
include(External_concurrentqueue)

# clang-format used to format our source code
if (RUN_CLANG_FORMAT)
    include(External_clang-format)
endif()