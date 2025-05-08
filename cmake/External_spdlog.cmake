# Download spdlog header files and install them to the default
# include path: ${GameLiftServerSdk_INSTALL_PREFIX}/include
ExternalProject_Add(spdlog
    PREFIX "spdlog"
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG v1.14.0
    UPDATE_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
    CMAKE_ARGS
        -DSPDLOG_BUILD_EXAMPLE=OFF
        -DSPDLOG_BUILD_TESTS=OFF
    CMAKE_CACHE_ARGS
        ${GameLiftServerSdk_DEFAULT_ARGS}
)