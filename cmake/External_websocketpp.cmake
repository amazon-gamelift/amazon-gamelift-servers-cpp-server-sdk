# Add asio project (required by websocketpp). Note: This is a header-only library so no build is required.
EXTERNALPROJECT_ADD(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-20-0
    PREFIX "asio"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    # custom patch command to install the include files into the install path
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_BINARY_DIR}/asio/src/asio/asio/include
        ${GameLiftServerSdk_INSTALL_PREFIX}/include/asio
    CMAKE_CACHE_ARGS
        ${GameLiftServerSdk_DEFAULT_ARGS}
)

# Add websocketpp project. Note: This is a header-only library so no build is required.
EXTERNALPROJECT_ADD(websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.8.2
    PREFIX "websocketpp"
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    CMAKE_CACHE_ARGS
        ${GameLiftServerSdk_DEFAULT_ARGS}
    DEPENDS "asio"
)
