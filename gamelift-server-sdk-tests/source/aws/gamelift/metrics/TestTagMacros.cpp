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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <type_traits>

#include "MetricMacrosTests.h"

#include <aws/gamelift/metrics/Tags.h>
#include <aws/gamelift/metrics/GameLiftMetrics.h>

using namespace ::testing;

class TagMacrosTests : public MetricMacrosTests {};

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricFoo, "foo", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFoo);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricFooDisabled, "foo", MockDisabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricFooDisabled);
} // namespace

TEST_F(TagMacrosTests,
       GivenEnabledPlatform_WhenGlobalTagSetCalled_ThenSetCalled) {
  EXPECT_CALL(MockProcessor, SetGlobalTag(StrEq("hello"), StrEq("world")))
      .Times(Exactly(1));
  EXPECT_CALL(MockProcessor, SetGlobalTag(StrEq("foo"), StrEq("bar")))
      .Times(Exactly(1));

  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockEnabled, "hello", "world");
  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockEnabled, "foo", "bar");
}

TEST_F(TagMacrosTests,
       GivenDisabledPlatform_WhenGlobalTagSetCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, SetGlobalTag(_, _)).Times(Exactly(0));

  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockDisabled, "hello", "world");
  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockDisabled, "foo", "bar");
}

TEST_F(TagMacrosTests,
       GivenEnabledPlatform_WhenGlobalTagRemoveCalled_ThenRemoveCalled) {
  EXPECT_CALL(MockProcessor, RemoveGlobalTag(StrEq("hello"))).Times(Exactly(1));
  EXPECT_CALL(MockProcessor, RemoveGlobalTag(StrEq("foo"))).Times(Exactly(1));

  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockEnabled, "hello");
  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockEnabled, "foo");
}

TEST_F(TagMacrosTests,
       GivenDisabledPlatform_WhenGlobalTagRemoveCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, RemoveGlobalTag(_)).Times(Exactly(0));

  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockDisabled, "hello");
  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockDisabled, "foo");
}

TEST_F(TagMacrosTests, WhenTagSetCalled_ThenTagSetMessageEnqueued) {
  auto ExpectedA =
      MetricMessage::TagSet(MetricFoo::Instance(), "hello", "world");
  auto ExpectedB = MetricMessage::TagSet(MetricFoo::Instance(), "foo", "bar");

  EXPECT_CALL(MockProcessor, Enqueue(ExpectedA)).Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(ExpectedB)).Times(Exactly(1));

  GAMELIFT_METRICS_TAG_SET(MetricFoo, "hello", "world");
  GAMELIFT_METRICS_TAG_SET(MetricFoo, "foo", "bar");

  // Delete expected tags
  Tags TagHandler;
  TagHandler.Handle(ExpectedA);
  TagHandler.Handle(ExpectedB);
}

TEST_F(
    TagMacrosTests,
    WhenTagSetCalledWithOutOfScopeValues_ThenTagSetMessageEnqueuedWithCopies) {
  auto Expected1 = MetricMessage::TagSet(MetricFoo::Instance(), "foo", "bar");

  EXPECT_CALL(MockProcessor, Enqueue(Expected1)).Times(Exactly(1));

  auto SetTag = [&]() {
    char Foo[4] = "foo";
    char Bar[4] = "bar";
    GAMELIFT_METRICS_TAG_SET(MetricFoo, Foo, Bar);
  };
  SetTag();

  // Delete expected tags
  Tags TagHandler;
  TagHandler.Handle(Expected1);
}

TEST_F(TagMacrosTests,
       GivenDisabledMetric_WhenTagSetCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_TAG_SET(MetricFooDisabled, "hello", "world");
  GAMELIFT_METRICS_TAG_SET(MetricFooDisabled, "foo", "bar");
}

TEST_F(TagMacrosTests, WhenTagRemoveCalled_ThenRemoveCalled) {
  auto ExpectedA = MetricMessage::TagRemove(MetricFoo::Instance(), "hello");
  auto ExpectedB = MetricMessage::TagRemove(MetricFoo::Instance(), "foo");

  EXPECT_CALL(MockProcessor, Enqueue(ExpectedA)).Times(Exactly(1));
  EXPECT_CALL(MockProcessor, Enqueue(ExpectedB)).Times(Exactly(1));

  GAMELIFT_METRICS_TAG_REMOVE(MetricFoo, "hello");
  GAMELIFT_METRICS_TAG_REMOVE(MetricFoo, "foo");

  // Delete expected tags
  Tags TagHandler;
  TagHandler.Handle(ExpectedA);
  TagHandler.Handle(ExpectedB);
}

TEST_F(TagMacrosTests,
       GivenDisabledMetric_WhenTagRemoveCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  GAMELIFT_METRICS_TAG_REMOVE(MetricFooDisabled, "hello");
  GAMELIFT_METRICS_TAG_REMOVE(MetricFooDisabled, "foo");
}

#ifdef GAMELIFT_USE_STD
class TagMacrosTestsWithStd : public MetricMacrosTests {};

TEST_F(TagMacrosTestsWithStd,
       GivenEnabledPlatform_WhenGlobalTagSetCalled_ThenSetCalled) {
  EXPECT_CALL(MockProcessor,
              SetGlobalTag(StrEq("test_key"), StrEq("test_value")))
      .Times(Exactly(1));

  std::string TestValue = "test_value";
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockEnabled, TestKey, TestValue);
}

TEST_F(TagMacrosTestsWithStd,
       GivenDisabledPlatform_WhenGlobalTagSetCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, SetGlobalTag(_, _)).Times(Exactly(0));

  std::string TestValue = "test_value";
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_GLOBAL_TAG_SET(MockDisabled, TestKey, TestValue);
}

TEST_F(TagMacrosTestsWithStd,
       GivenEnabledPlatform_WhenGlobalTagRemoveCalled_ThenRemoveCalled) {
  EXPECT_CALL(MockProcessor, RemoveGlobalTag(StrEq("test_key")))
      .Times(Exactly(1));

  std::string TestKey = "test_key";
  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockEnabled, TestKey);
}

TEST_F(TagMacrosTestsWithStd,
       GivenDisabledPlatform_WhenGlobalTagRemoveCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, RemoveGlobalTag(_)).Times(Exactly(0));

  std::string TestValue = "test_value";
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_GLOBAL_TAG_REMOVE(MockDisabled, TestKey);
}

TEST_F(TagMacrosTestsWithStd, WhenTagSetCalled_ThenTagSetMessageEnqueued) {
  auto Expected =
      MetricMessage::TagSet(MetricFoo::Instance(), "test_key", "test_value");
  EXPECT_CALL(MockProcessor, Enqueue(Expected)).Times(Exactly(1));

  std::string TestValue = "test_value";
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_TAG_SET(MetricFoo, TestKey, TestValue);

  Tags TagHandler;
  TagHandler.Handle(Expected);
}

TEST_F(
    TagMacrosTestsWithStd,
    WhenTagSetCalledWithOutOfScopeValues_ThenTagSetMessageEnqueuedWithCopies) {
  auto Expected =
      MetricMessage::TagSet(MetricFoo::Instance(), "hello", "world");
  EXPECT_CALL(MockProcessor, Enqueue(Expected)).Times(Exactly(1));

  auto SetTagStd = [&]() {
    std::string Hello = "hello";
    std::string World = "world";
    GAMELIFT_METRICS_TAG_SET(MetricFoo, Hello, World);
  };
  SetTagStd();

  Tags TagHandler;
  TagHandler.Handle(Expected);
}

TEST_F(TagMacrosTestsWithStd,
       GivenDisabledMetric_WhenTagSetCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  std::string TestValue = "test_value";
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_TAG_SET(MetricFooDisabled, TestKey, TestValue);
}

TEST_F(TagMacrosTestsWithStd, WhenTagRemoveCalled_ThenRemoveCalled) {
  auto Expected = MetricMessage::TagRemove(MetricFoo::Instance(), "test_key");
  EXPECT_CALL(MockProcessor, Enqueue(Expected)).Times(Exactly(1));
  std::string TestKey = "test_key";
  GAMELIFT_METRICS_TAG_REMOVE(MetricFoo, TestKey);

  Tags TagHandler;
  TagHandler.Handle(Expected);
}

TEST_F(TagMacrosTestsWithStd,
       GivenDisabledMetric_WhenTagRemoveCalled_ThenNothingHappens) {
  EXPECT_CALL(MockProcessor, Enqueue(_)).Times(Exactly(0));

  std::string TestKey = "test_key";
  GAMELIFT_METRICS_TAG_REMOVE(MetricFooDisabled, TestKey);
}
#endif // GAMELIFT_USE_STD
