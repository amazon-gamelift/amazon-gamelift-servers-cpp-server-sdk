/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include "gtest/gtest.h"
#include <aws/gamelift/internal/model/Uri.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class UriTest : public ::testing::Test {
protected:
    const std::string testBaseUriString = "wss://test-ws-url.com/domain";
    const std::string testQueryParamKey = "testQueryParamKey";
    const std::string testQueryParamValue = "testQueryParamValue";

    Uri testUri;
    std::string testUriString;
    void SetUp() override {
        testUri = Uri::UriBuilder().WithBaseUri(testBaseUriString).AddQueryParam(testQueryParamKey, testQueryParamValue).Build();

        std::stringstream ss;
        ss << testBaseUriString << "?" << testQueryParamKey << "=" << testQueryParamValue;
        testUriString = ss.str();
    }

    void AssertUriEqualsTestUri(const Uri &uri) {
        ASSERT_EQ(uri.GetUriString(), testUriString);
        ASSERT_EQ(uri.GetBaseUriString(), testBaseUriString);
        ASSERT_EQ(uri.GetQueryMap().size(), 1);
        ASSERT_EQ(uri.GetQueryMap().find(testQueryParamKey)->second, testQueryParamValue);
    }
};

TEST_F(UriTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    Uri uri;
    // THEN
    ASSERT_EQ(uri.GetUriString(), "");
    ASSERT_EQ(uri.GetBaseUriString(), "");
    ASSERT_EQ(uri.GetQueryMap().size(), 0);
}

TEST_F(UriTest, GIVEN_uri_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    Uri uri(testUri);
    // THEN
    AssertUriEqualsTestUri(uri);
}

TEST_F(UriTest, GIVEN_uri_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    Uri uri(std::move(testUri));
    // THEN
    AssertUriEqualsTestUri(uri);
}

TEST_F(UriTest, GIVEN_uri_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    Uri uri;
    // WHEN
    uri = testUri;
    // THEN
    AssertUriEqualsTestUri(uri);
}

TEST_F(UriTest, GIVEN_uri_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    Uri uri;
    // WHEN
    uri = std::move(testUri);
    // THEN
    AssertUriEqualsTestUri(uri);
}

TEST_F(UriTest, GIVEN_uri_WHEN_operatorEquals_THEN_success) {
    // GIVEN
    Uri uri = testUri;
    // WHEN
    bool urisEqual = (uri == testUri);
    // THEN
    ASSERT_TRUE(urisEqual);
}
} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws