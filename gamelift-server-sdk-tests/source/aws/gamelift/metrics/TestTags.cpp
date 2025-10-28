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
#include <aws/gamelift/metrics/DefinitionMacros.h>
#include <aws/gamelift/metrics/Samplers.h>

#include <aws/gamelift/metrics/Tags.h>

using namespace ::testing;

namespace {
GAMELIFT_METRICS_DECLARE_GAUGE(MetricTest, "metric_test", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricTest);

GAMELIFT_METRICS_DECLARE_GAUGE(MetricTest2, "metric_test_2", MockEnabled,
                               Aws::GameLift::Metrics::SampleAll());
GAMELIFT_METRICS_DEFINE_GAUGE(MetricTest2);
} // namespace

TEST(TagsTests, InitializesOk) { Tags A; }

TEST(TagsTests, WhenTagSetMessageHandled_ThenSavesTagForThatMetric) {
  Tags A;

  auto SetMessage = MetricMessage::TagSet(MetricTest::Instance(), "foo", "bar");
  A.Handle(SetMessage);
  EXPECT_THAT(A.GetTags(&MetricTest::Instance()),
              UnorderedElementsAre(std::make_pair("foo", "bar")));
}

TEST(TagsTests, WhenTagSetMessageHandled_ThenDeletesDynamicData) {
  Tags A;

  MetricMessage Message =
      MetricMessage::TagSet(MetricTest::Instance(), "foo", "bar");
  EXPECT_NE(Message.SetTag.Ptr, nullptr);
  A.Handle(Message);
  EXPECT_EQ(Message.SetTag.Ptr, nullptr);
}

TEST(TagsTests, WhenTagSetMessageHandledTwice_ThenKeepsOnlyLatestValue) {
  Tags A;

  auto TestTagMessage =
      MetricMessage::TagSet(MetricTest::Instance(), "test_tag", "unchanged");
  A.Handle(TestTagMessage);

  auto FooBarMessage =
      MetricMessage::TagSet(MetricTest2::Instance(), "foo", "bar");
  A.Handle(FooBarMessage);
  auto FooChangedMessage =
      MetricMessage::TagSet(MetricTest2::Instance(), "foo", "changed_it");
  A.Handle(FooChangedMessage);
  auto AnotherTagMessage = MetricMessage::TagSet(MetricTest2::Instance(),
                                                 "another_tag", "some_value");
  A.Handle(AnotherTagMessage);

  EXPECT_THAT(A.GetTags(&MetricTest::Instance()),
              UnorderedElementsAre(std::make_pair("test_tag", "unchanged")));

  EXPECT_THAT(
      A.GetTags(&MetricTest2::Instance()),
      UnorderedElementsAre(std::make_pair("foo", "changed_it"),
                           std::make_pair("another_tag", "some_value")));
}

TEST(TagsTests, WhenTagRemoveHandled_ThenRemovesTag) {
  Tags A;

  auto SetMessage = MetricMessage::TagSet(MetricTest::Instance(), "foo", "bar");
  A.Handle(SetMessage);
  ASSERT_THAT(A.GetTags(&MetricTest::Instance()),
              UnorderedElementsAre(std::make_pair("foo", "bar")));

  auto RemoveMessage = MetricMessage::TagRemove(MetricTest::Instance(), "foo");
  A.Handle(RemoveMessage);
  EXPECT_THAT(A.GetTags(&MetricTest::Instance()), IsEmpty());
}

TEST(TagsTests, WhenTagRemoveHandled_ThenDeletesDynamicData) {
  Tags A;

  auto SetMessage = MetricMessage::TagSet(MetricTest::Instance(), "foo", "bar");
  A.Handle(SetMessage);
  ASSERT_THAT(A.GetTags(&MetricTest::Instance()),
              UnorderedElementsAre(std::make_pair("foo", "bar")));

  MetricMessage RemoveMessage =
      MetricMessage::TagRemove(MetricTest::Instance(), "foo");
  EXPECT_NE(RemoveMessage.SetTag.Ptr, nullptr);
  A.Handle(RemoveMessage);
  EXPECT_EQ(RemoveMessage.SetTag.Ptr, nullptr);
}

TEST(TagsTests, GivenNoExistingTag_WhenTagRemoveHandled_ThenNothingHappens) {
  Tags A;

  ASSERT_THAT(A.GetTags(&MetricTest::Instance()), IsEmpty());
  auto Message = MetricMessage::TagRemove(MetricTest::Instance(), "foo");
  A.Handle(Message);
  EXPECT_THAT(A.GetTags(&MetricTest::Instance()), IsEmpty());
}
