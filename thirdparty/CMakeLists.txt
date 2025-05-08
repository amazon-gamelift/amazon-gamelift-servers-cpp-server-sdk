# Rapidjson is a header-only library, so no need to look for a system version
include(External_rapidjson)

# websocketpp is a header-only library, so no need to look for system version
include(External_websocketpp)

# spdlog is a header-only library, so no need to look for system version
include(External_spdlog)

# clang-format used to format our source code
if (RUN_CLANG_FORMAT)
    include(External_clang-format)
endif()