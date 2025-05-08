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
#include <aws/gamelift/internal/model/WebSocketAttributeValue.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>

namespace Aws {
namespace GameLift {
namespace Internal {
namespace Test {

class WebSocketAttributeValueTest : public ::testing::Test {
protected:
    const std::string testS = "testString";
    const double testN = 100.0;
    const std::vector<std::string> testStringList{testS};
    const std::map<std::string, double> testStringDoubleMap = {{testS, testN}};

    WebSocketAttributeValue testSAttribute;
    WebSocketAttributeValue testNAttribute;
    WebSocketAttributeValue testSLAttribute;
    WebSocketAttributeValue testSDMAttribute;

    std::string serializedS;
    std::string serializedN;
    std::string serializedSL;
    std::string serializedSDM;

    void SetUp() override {
        testSAttribute.SetAttributeType("STRING");
        testSAttribute.SetS(testS);

        testNAttribute.SetAttributeType("DOUBLE");
        testNAttribute.SetN(testN);

        testSLAttribute.SetAttributeType("STRING_LIST");
        testSLAttribute.SetSL(testStringList);

        testSDMAttribute.SetAttributeType("STRING_DOUBLE_MAP");
        testSDMAttribute.SetSDM(testStringDoubleMap);

        std::stringstream sS;
        sS << "{\"AttrType\":\""
           << "STRING"
           << "\",\"S\":\"" << testS << "\"}";
        serializedS = sS.str();

        std::stringstream sN;
        sN << "{\"AttrType\":\""
           << "DOUBLE"
           << "\",\"N\":100.0"
           << "}";
        serializedN = sN.str();

        std::stringstream sSL;
        sSL << "{\"AttrType\":\""
            << "STRING_LIST"
            << "\",\"SL\":[\"" << testS << "\"]}";
        serializedSL = sSL.str();

        std::stringstream sSDM;
        sSDM << "{\"AttrType\":\""
             << "STRING_DOUBLE_MAP"
             << "\",\"SDM\":{\"" << testS << "\":100.0"
             << "}}";
        serializedSDM = sSDM.str();
    }

    void AssertS(const WebSocketAttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetAttributeTypeAsString(), "STRING");
        ASSERT_EQ(attributeValue.GetS(), testS);
    }

    void AssertN(const WebSocketAttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetAttributeTypeAsString(), "DOUBLE");
        ASSERT_EQ(attributeValue.GetN(), testN);
    }

    void AssertSL(const WebSocketAttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetAttributeTypeAsString(), "STRING_LIST");
        ASSERT_EQ(attributeValue.GetSL()[0], testS);
    }

    void AssertSDM(const WebSocketAttributeValue &attributeValue) {
        ASSERT_EQ(attributeValue.GetAttributeTypeAsString(), "STRING_DOUBLE_MAP");
        ASSERT_EQ(attributeValue.GetSDM().at(testS), testN);
    }
};

TEST_F(WebSocketAttributeValueTest, GIVEN_noArgs_WHEN_defaultConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketAttributeValue attributeValue;
    // THEN
    ASSERT_EQ(attributeValue.GetAttributeTypeAsString(), "NONE");
}

TEST_F(WebSocketAttributeValueTest, GIVEN_message_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketAttributeValue attributeValue(testSAttribute);
    // THEN
    AssertS(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_message_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    WebSocketAttributeValue attributeValue(std::move(testNAttribute));
    // THEN
    AssertN(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_message_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue = testSLAttribute;
    // THEN
    AssertSL(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_message_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue = std::move(testSDMAttribute);
    // THEN
    AssertSDM(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSAttr_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketAttributeValue *message = &testSAttribute;
    // WHEN
    std::string serializedSAttribute = message->Serialize();
    // THEN
    ASSERT_EQ(serializedSAttribute, serializedS);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSAttr_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue.Deserialize(serializedS);
    // THEN
    AssertS(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputNAttr_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketAttributeValue *message = &testNAttribute;
    // WHEN
    std::string serializedNAttribute = message->Serialize();
    // THEN
    ASSERT_EQ(serializedNAttribute, serializedN);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputNAttr_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue.Deserialize(serializedN);
    // THEN
    AssertN(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSLAttr_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketAttributeValue *message = &testSLAttribute;
    // WHEN
    std::string serializedSLAttribute = message->Serialize();
    // THEN
    ASSERT_EQ(serializedSLAttribute, serializedSL);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSLAttr_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue.Deserialize(serializedSL);
    // THEN
    AssertSL(attributeValue);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSDMAttr_WHEN_serialize_THEN_success) {
    // GIVEN
    const WebSocketAttributeValue *message = &testSDMAttribute;
    // WHEN
    std::string serializedSDMAttribute = message->Serialize();
    // THEN
    ASSERT_EQ(serializedSDMAttribute, serializedSDM);
}

TEST_F(WebSocketAttributeValueTest, GIVEN_validInputSDMAttr_WHEN_deserialize_THEN_success) {
    // GIVEN
    WebSocketAttributeValue attributeValue;
    // WHEN
    attributeValue.Deserialize(serializedSDM);
    // THEN
    AssertSDM(attributeValue);
}

} // namespace Test
} // namespace Internal
} // namespace GameLift
} // namespace Aws