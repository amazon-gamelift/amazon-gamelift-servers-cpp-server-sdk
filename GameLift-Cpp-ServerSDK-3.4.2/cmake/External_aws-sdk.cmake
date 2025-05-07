# Build a minimal AWS SDK for cloudformation, sqs, and sts, required for exampleApp
ExternalProject_Add(aws-sdk
    GIT_REPOSITORY    https://github.com/aws/aws-sdk-cpp.git
    GIT_TAG           1.8.92
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/aws"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/aws-bin"
    CMAKE_CACHE_ARGS
        ${GameLiftServerSdk_DEFAULT_ARGS}
    LIST_SEPARATOR    "|"
    CMAKE_ARGS
        -DBUILD_ONLY=cloudformation|sqs|sts
        -DENABLE_TESTING=OFF
    BUILD_ALWAYS      TRUE
    TEST_COMMAND      ""
)