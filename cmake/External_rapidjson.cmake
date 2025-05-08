# Download rapidjson header files and install them to the default
# include path: ${GameLiftServerSdk_INSTALL_PREFIX}/include
ExternalProject_Add( rapidjson
    PREFIX "rapidjson"
    GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
    GIT_TAG v1.1.0
    UPDATE_COMMAND ""
    BUILD_COMMAND ""
    CMAKE_ARGS
        -DRAPIDJSON_BUILD_TESTS=OFF
        -DRAPIDJSON_BUILD_DOC=OFF
        -DRAPIDJSON_BUILD_EXAMPLES=OFF
        -DDOC_INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/rapidjson/doc
        -DLIB_INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/rapidjson/lib
    CMAKE_CACHE_ARGS
        ${GameLiftServerSdk_DEFAULT_ARGS}
)