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
#include <aws/gamelift/server/model/GameSession.h>
#include <aws/gamelift/utility/TestHelper.h>

namespace Aws {
namespace GameLift {
namespace Server {
namespace Model {
namespace Test {

class GamePropertyTest : public ::testing::Test {
protected:
    const std::string testKey = "testKey";
    const std::string testValue = "testValue";
    GameProperty testGameProperty;

    void SetUp() override {
        testGameProperty.SetKey(testKey.c_str());
        testGameProperty.SetValue(testValue.c_str());
    }

    // Helper Methods
    void AssertPropertyEqualsTestProperty(const GameProperty &property) {
        Utility::TestHelper::AssertStringsEqual(property.GetKey(), testKey);
        Utility::TestHelper::AssertStringsEqual(property.GetValue(), testValue);
    }
};

TEST_F(GamePropertyTest, GIVEN_noArgs_WHEN_defaultConstructor_THEN_success) {
    // WHEN
    GameProperty gameProperty;
    // THEN
    Utility::TestHelper::AssertStringsEqual(gameProperty.GetKey(), "");
    Utility::TestHelper::AssertStringsEqual(gameProperty.GetValue(), "");
}

TEST_F(GamePropertyTest, GIVEN_property_WHEN_copyConstruct_THEN_success) {
    // GIVEN / WHEN
    GameProperty gameProperty(testGameProperty);
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_NE(&gameProperty, &testGameProperty);
}

TEST_F(GamePropertyTest, GIVEN_property_WHEN_moveConstruct_THEN_success) {
    // GIVEN / WHEN
    GameProperty gameProperty(std::move(testGameProperty));
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_NE(&gameProperty, &testGameProperty);
}

TEST_F(GamePropertyTest, GIVEN_property_WHEN_copyAssignmentConstructor_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    gameProperty = testGameProperty;
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_NE(&gameProperty, &testGameProperty);
}

TEST_F(GamePropertyTest, GIVEN_property_WHEN_moveAssignmentConstructor_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    gameProperty = std::move(testGameProperty);
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_NE(&gameProperty, &testGameProperty);
}

TEST_F(GamePropertyTest, GIVEN_validCStrings_WHEN_withGamePropertyValues_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    GameProperty &propertyRef = gameProperty.WithKey(testKey.c_str()).WithValue(testValue.c_str());
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_EQ(&propertyRef, &gameProperty);
}

#ifdef GAMELIFT_USE_STD
/* -------------------------------------------------------------------------- */
/*                                STD Specific Tests                           */
/* -------------------------------------------------------------------------- */

TEST_F(GamePropertyTest, GIVEN_validInput_WHEN_setGamePropertyValues_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    gameProperty.SetKey(testKey);
    gameProperty.SetValue(testValue);
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
}

TEST_F(GamePropertyTest, GIVEN_rValueInput_WHEN_setGamePropertyValues_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    gameProperty.SetKey(std::string(testKey));
    gameProperty.SetValue(std::string(testValue));
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
}

TEST_F(GamePropertyTest, GIVEN_validInput_WHEN_withGamePropertyValues_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    GameProperty &propertyRef = gameProperty.WithKey(testKey).WithValue(testValue);
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_EQ(&propertyRef, &gameProperty);
}

TEST_F(GamePropertyTest, GIVEN_rValueInput_WHEN_withGamePropertyValues_THEN_success) {
    // GIVEN
    GameProperty gameProperty;
    // WHEN
    GameProperty &propertyRef = gameProperty.WithKey(std::string(testKey)).WithValue(std::string(testValue));
    // THEN
    AssertPropertyEqualsTestProperty(gameProperty);
    ASSERT_EQ(&propertyRef, &gameProperty);
}

#else
/* -------------------------------------------------------------------------- */
/*                                NoSTD Specific Tests                         */
/* -------------------------------------------------------------------------- */

TEST_F(GamePropertyTest, GIVEN_tooLongStrings_WHEN_SetValues_THEN_cutStringOff) {
    // GIVEN
    std::string tooLongKey = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_KEY_LENGTH);
    std::string tooLongValue = Utility::TestHelper::GenerateRandomAlphaNumericString(MAX_VALUE_LENGTH);
    ASSERT_EQ(tooLongKey.size(), MAX_KEY_LENGTH);
    ASSERT_EQ(tooLongValue.size(), MAX_VALUE_LENGTH);
    GameProperty gameProperty;
    // WHEN
    gameProperty.SetKey(tooLongKey.c_str());
    gameProperty.SetValue(tooLongValue.c_str());
    // THEN
    ASSERT_STRNE(gameProperty.GetKey(), tooLongKey.c_str());
    ASSERT_EQ(strlen(gameProperty.GetKey()), MAX_KEY_LENGTH - 1);
    ASSERT_STRNE(gameProperty.GetValue(), tooLongValue.c_str());
    ASSERT_EQ(strlen(gameProperty.GetValue()), MAX_VALUE_LENGTH - 1);
}

#endif

} // namespace Test
} // namespace Model
} // namespace Server
} // namespace GameLift
} // namespace Aws