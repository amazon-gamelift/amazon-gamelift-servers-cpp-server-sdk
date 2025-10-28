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
#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <aws/gamelift/metrics/DerivedMetric.h>

#include <string>
#include <vector>

struct MockVisitor : public Aws::GameLift::Metrics::IDerivedMetricVisitor {
  MOCK_METHOD(void, VisitDerivedMetric,
              (Aws::GameLift::Metrics::IDerivedMetric &), (override));
};

struct MockDerivedMetric : public Aws::GameLift::Metrics::IDerivedMetric {
  std::string Name;
  std::vector<Aws::GameLift::Metrics::MetricMessage> HandledMessages;
  int CallsToEmit = 0;

  MockDerivedMetric() = default;
  explicit MockDerivedMetric(std::string Name) : Name(Name) {}

  // We don't MOCK_ here because we need this class to be copyable

  virtual void
  HandleMessage(Aws::GameLift::Metrics::MetricMessage &Message,
                Aws::GameLift::Metrics::IMetricsEnqueuer &) override {
    HandledMessages.emplace_back(Message);
  }

  virtual void
  EmitMetrics(const Aws::GameLift::Metrics::IMetric *,
              Aws::GameLift::Metrics::IMetricsEnqueuer &) override {
    CallsToEmit++;
  }
};

struct MockNameVisitor : public Aws::GameLift::Metrics::IDerivedMetricVisitor {
  std::vector<std::string> Names;

  virtual void
  VisitDerivedMetric(Aws::GameLift::Metrics::IDerivedMetric &Metric) override {
    MockDerivedMetric *MockMetric = dynamic_cast<MockDerivedMetric *>(&Metric);
    Names.emplace_back(MockMetric->Name);
  }
};

struct MockVectorEnqueuer : public Aws::GameLift::Metrics::IMetricsEnqueuer {
  std::vector<Aws::GameLift::Metrics::MetricMessage> Values;

  virtual void Enqueue(Aws::GameLift::Metrics::MetricMessage Message) override {
    Values.emplace_back(Message);
  }
};

inline Aws::GameLift::Metrics::IDerivedMetric *
GetNthDerivedMetric(Aws::GameLift::Metrics::IMetric *Metric, size_t N) {
  assert(Metric);

  struct GetNthDerivedMetricVisitor
      : public Aws::GameLift::Metrics::IDerivedMetricVisitor {
    size_t N;
    size_t I = 0;
    Aws::GameLift::Metrics::IDerivedMetric *Result = nullptr;

    explicit GetNthDerivedMetricVisitor(size_t N) noexcept : N(N) {}

    virtual void VisitDerivedMetric(
        Aws::GameLift::Metrics::IDerivedMetric &Metric) override {
      if (I == N) {
        Result = &Metric;
      }
    }
  };

  GetNthDerivedMetricVisitor Visitor(N);
  Metric->GetDerivedMetrics().Visit(Visitor);
  return Visitor.Result;
}
