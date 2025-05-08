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
#include <aws/gamelift/server/model/AttributeValue.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class AttributeValueTest : public ::testing::Test {
protected:
    const std::string testString = "testString";
    const double testDouble = 3.14;

    AttributeValue testDoubleAttribute;
    AttributeValue testStringAttribute;
    AttributeValue testStringListAttribute;
    AttributeValue testStringDoubleMapAttribute;

    void SetUp() override {
        testStringListAttribute = AttributeValue::ConstructStringList();
        testStringDoubleMapAttribute = AttributeValue::ConstructStringDoubleMap();
        testDoubleAttribute = AttributeValue(testDouble);
#ifdef GAMELIFT_USE_STD
        testStringAttribute = AttributeValue(testString);
        testStringListAttribute.AddString(testString);
        testStringDoubleMapAttribute.AddStringAndDouble(testString, testDouble);
#else
        testStringAttribute = AttributeValue(testString.c_str());
        testStringListAttribute.AddString(testString.c_str());
        testStringDoubleMapAttribute.AddStringAndDouble(testString.c_str(), testDouble);
#endif
    }

    // Helper Methods
#ifdef GAMELIFT_USE_STD
    void AssertStringListEmpty(const AttributeValue &attributeValue) {
        const std::vector<std::string> stringList = attributeValue.GetSL();
        ASSERT_TRUE(stringList.empty());
    }

    void AssertStringDoubleMapEmpty(const AttributeValue &attributeValue) {
        const std::map<std::string, double> stringDoubleMap = attributeValue.GetSDM();
        ASSERT_TRUE(stringDoubleMap.empty());
    }

    void AssertAttributeEqualsTestStringListAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING_LIST);
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING_LIST);
    }

    void AssertAttributeEqualsTestStringDoubleMapAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING_DOUBLE_MAP);
        const std::map<std::string, double> sdm = attributeValue.GetSDM();
        ASSERT_EQ(sdm.at(testString), testDouble);
    }
#else
    void AssertStringListEmpty(const AttributeValue &attributeValue) {
        int count;
        attributeValue.GetSL(count);
        ASSERT_EQ(count, 0);
    }

    void AssertStringDoubleMapEmpty(const AttributeValue &attributeValue) {
        int count;
        attributeValue.GetSDM(count);
        ASSERT_EQ(count, 0);
    }

    void AssertAttributeEqualsTestStringListAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING_LIST);
        int count;
        ASSERT_STREQ((attributeValue.GetSL(count))[0], testString.c_str());
    }

    void AssertAttributeEqualsTestStringDoubleMapAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING_DOUBLE_MAP);
        int count;
        ASSERT_STREQ((attributeValue.GetSDM(count))[0].GetKey(), testString.c_str());
        ASSERT_EQ((attributeValue.GetSDM(count))[0].GetValue(), testDouble);
    }
#endif

    void AssertAttributeValueEmpty(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetN(), 0);
        Utility::TestHelper::AssertStringsEqual(attributeValue.GetS(), "");
        AssertStringListEmpty(attributeValue);
        AssertStringDoubleMapEmpty(attributeValue);
    }

    void AssertAttributeEqualsTestStringAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::STRING);
        Utility::TestHelper::AssertStringsEqual(attributeValue.GetS(), testString);
    }

    void AssertAttributeEqualsTestDoubleAttribute(const AttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetType(), AttributeValue::AttrType::DOUBLE);
        ASSERT_EQ(attributeValue.GetN(), testDouble);
    }
};

TEST_F(AttributeValueTest, GIVEN_emptyInput_WHEN_constructStringList_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr = AttributeValue::ConstructStringList();
    // THEN
    ASSERT_EQ(attr.GetType(), AttributeValue::AttrType::STRING_LIST);
    AssertAttributeValueEmpty(attr);
}

TEST_F(AttributeValueTest, GIVEN_validInput_WHEN_constructStringDoubleMap_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr = AttributeValue::ConstructStringDoubleMap();
    // THEN
    ASSERT_EQ(attr.GetType(), AttributeValue::AttrType::STRING_DOUBLE_MAP);
    AssertAttributeValueEmpty(attr);
}

TEST_F(AttributeValueTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr;
    // THEN
    ASSERT_EQ(attr.GetType(), AttributeValue::AttrType::NONE);
    AssertAttributeValueEmpty(attr);
}

TEST_F(AttributeValueTest, GIVEN_double_WHEN_construct_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr(testDouble);
    // THEN
    AssertAttributeEqualsTestDoubleAttribute(attr);
}

TEST_F(AttributeValueTest, GIVEN_attribute_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    AttributeValue doubleAttribute(testDoubleAttribute);
    AttributeValue stringAttribute(testStringAttribute);
    AttributeValue slAttribute(testStringListAttribute);
    AttributeValue sdmAttribute(testStringDoubleMapAttribute);
    // THEN
    AssertAttributeEqualsTestDoubleAttribute(doubleAttribute);
    ASSERT_NE(&doubleAttribute, &testDoubleAttribute);
    AssertAttributeEqualsTestStringAttribute(stringAttribute);
    ASSERT_NE(&stringAttribute, &testStringAttribute);
    AssertAttributeEqualsTestStringListAttribute(slAttribute);
    ASSERT_NE(&slAttribute, &testStringListAttribute);
    AssertAttributeEqualsTestStringDoubleMapAttribute(sdmAttribute);
    ASSERT_NE(&sdmAttribute, &testStringDoubleMapAttribute);
}

TEST_F(AttributeValueTest, GIVEN_attributes_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    AttributeValue doubleAttribute(std::move(testDoubleAttribute));
    AttributeValue stringAttribute(std::move(testStringAttribute));
    AttributeValue slAttribute(std::move(testStringListAttribute));
    AttributeValue sdmAttribute(std::move(testStringDoubleMapAttribute));
    // THEN
    AssertAttributeEqualsTestDoubleAttribute(doubleAttribute);
    AssertAttributeEqualsTestStringAttribute(stringAttribute);
    AssertAttributeEqualsTestStringListAttribute(slAttribute);
    AssertAttributeEqualsTestStringDoubleMapAttribute(sdmAttribute);
}

TEST_F(AttributeValueTest, GIVEN_attributes_WHEN_copyAssignment_THEN_success) {
    // GIVEN / WHEN
    AttributeValue doubleAttribute;
    AttributeValue stringAttribute;
    AttributeValue slAttribute;
    AttributeValue sdmAttribute;
    // WHEN
    doubleAttribute = testDoubleAttribute;
    stringAttribute = testStringAttribute;
    slAttribute = testStringListAttribute;
    sdmAttribute = testStringDoubleMapAttribute;
    // THEN
    AssertAttributeEqualsTestDoubleAttribute(doubleAttribute);
    ASSERT_NE(&doubleAttribute, &testDoubleAttribute);
    AssertAttributeEqualsTestStringAttribute(stringAttribute);
    ASSERT_NE(&stringAttribute, &testStringAttribute);
    AssertAttributeEqualsTestStringListAttribute(slAttribute);
    ASSERT_NE(&slAttribute, &testStringListAttribute);
    AssertAttributeEqualsTestStringDoubleMapAttribute(sdmAttribute);
    ASSERT_NE(&sdmAttribute, &testStringDoubleMapAttribute);
}

TEST_F(AttributeValueTest, GIVEN_attributes_WHEN_moveAssignment_THEN_success) {
    // GIVEN / WHEN
    AttributeValue doubleAttribute;
    AttributeValue stringAttribute;
    AttributeValue slAttribute;
    AttributeValue sdmAttribute;
    // WHEN
    doubleAttribute = std::move(testDoubleAttribute);
    stringAttribute = std::move(testStringAttribute);
    slAttribute = std::move(testStringListAttribute);
    sdmAttribute = std::move(testStringDoubleMapAttribute);
    // THEN
    AssertAttributeEqualsTestDoubleAttribute(doubleAttribute);
    AssertAttributeEqualsTestStringAttribute(stringAttribute);
    AssertAttributeEqualsTestStringListAttribute(slAttribute);
    AssertAttributeEqualsTestStringDoubleMapAttribute(sdmAttribute);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(AttributeValueTest, GIVEN_string_WHEN_construct_std_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr(testString);
    // THEN
    AssertAttributeEqualsTestStringAttribute(attr);
}

TEST_F(AttributeValueTest, GIVEN_stringList_WHEN_slAttributeValueOperations_std_THEN_success) {
    // GIVEN
    std::string testString1 = "test string 1";
    std::string testString2 = "test string 2";
    // WHEN
    AttributeValue attr = AttributeValue::ConstructStringList().WithString(testString1);
    attr.AddString(testString2);
    // THEN
    ASSERT_EQ((attr.GetSL())[0], testString1);
    ASSERT_EQ((attr.GetSL())[1], testString2);
}

TEST_F(AttributeValueTest, GIVEN_stringDoubleMap_WHEN_sdmAttributeValueOperations_std_THEN_success) {
    // GIVEN
    std::string testString1 = "test string 1";
    double testDouble1 = 3.14;
    std::string testString2 = "test string 2";
    double testDouble2 = 2.72;
    // WHEN
    AttributeValue attr = AttributeValue::ConstructStringDoubleMap().WithStringAndDouble(testString1, testDouble1);
    attr.AddStringAndDouble(testString2, testDouble2);
    // THEN
    auto sdm = attr.GetSDM();
    ASSERT_EQ(sdm[testString1], testDouble1);
    ASSERT_EQ(sdm[testString2], testDouble2);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(AttributeValueTest, GIVEN_string_WHEN_construct_no_std_THEN_success) {
    // GIVEN / WHEN
    AttributeValue attr(testString.c_str());
    // THEN
    AssertAttributeEqualsTestStringAttribute(attr);
}

TEST_F(AttributeValueTest, GIVEN_stringList_WHEN_slAttributeValueOperations_no_std_THEN_success) {
    // GIVEN
    const char *testString1 = "test string 1";
    const char *testString2 = "test string 2";
    int count;
    // WHEN
    AttributeValue attr = AttributeValue::ConstructStringList().WithString(testString1);
    attr.AddString(testString2);
    // THEN
    const AttributeValue::AttributeStringType *sl = attr.GetSL(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(sl[0], testString1);
    ASSERT_STREQ(sl[1], testString2);
}

TEST_F(AttributeValueTest, GIVEN_stringDoubleMap_WHEN_sdmAttributeValueOperations_no_std_THEN_success) {
    // GIVEN
    const char *testString1 = "test string 1";
    double testDouble1 = 3.14;
    const char *testString2 = "test string 2";
    double testDouble2 = 2.72;
    int count;
    // WHEN
    AttributeValue attr = AttributeValue::ConstructStringDoubleMap().WithStringAndDouble(testString1, testDouble1);
    attr.AddStringAndDouble(testString2, testDouble2);
    // THEN
    const AttributeValue::KeyAndValue *sdm = attr.GetSDM(count);
    ASSERT_EQ(count, 2);
    ASSERT_STREQ(sdm[0].GetKey(), testString1);
    ASSERT_EQ(sdm[0].GetValue(), testDouble1);
    ASSERT_STREQ(sdm[1].GetKey(), testString2);
    ASSERT_EQ(sdm[1].GetValue(), testDouble2);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws