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

#include "MockDerivedMetric.h"

#include <aws/gamelift/metrics/DerivedMetric.h>

using namespace ::testing;

TEST(DerivedMetricCollectionTests, WhenEmpty_ThenIsEmptyWhenVisited) {
  MockVisitor Visitor;
  EXPECT_CALL(Visitor, VisitDerivedMetric(_)).Times(Exactly(0));

  auto Collection = Aws::GameLift::Metrics::CollectDerivedMetrics();
  Collection.Visit(Visitor);
}

TEST(DerivedMetricCollectionTests, WhenOneMetric_ThenVisitsMetric) {
  MockVisitor Visitor;
  EXPECT_CALL(Visitor, VisitDerivedMetric(_)).Times(Exactly(1));

  auto Collection =
      Aws::GameLift::Metrics::CollectDerivedMetrics(MockDerivedMetric());
  Collection.Visit(Visitor);
}

TEST(DerivedMetricCollectionTests, WhenFourMetrics_ThenVisitsFourMetrics) {
  MockVisitor Visitor;
  EXPECT_CALL(Visitor, VisitDerivedMetric(_)).Times(Exactly(4));

  auto Collection = Aws::GameLift::Metrics::CollectDerivedMetrics(
      MockDerivedMetric(), MockDerivedMetric(), MockDerivedMetric(),
      MockDerivedMetric());
  Collection.Visit(Visitor);
}

TEST(DerivedMetricCollectionTests, WhenFourMetrics_ThenMetricsVisitedInOrder) {

  auto Collection = Aws::GameLift::Metrics::CollectDerivedMetrics(
      MockDerivedMetric("foo"), MockDerivedMetric("bar"),
      MockDerivedMetric("baz"), MockDerivedMetric("boz"));

  MockNameVisitor Visitor;
  Collection.Visit(Visitor);

  EXPECT_THAT(Visitor.Names, ElementsAre("foo", "bar", "baz", "boz"));
}
