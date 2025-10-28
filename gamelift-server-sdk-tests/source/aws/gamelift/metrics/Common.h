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

#include <mutex>
#include <string>
#include <unordered_map>

#include <aws/gamelift/metrics/IMetricsProcessor.h>
#include <aws/gamelift/metrics/MetricsSettings.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <aws/gamelift/metrics/DynamicTag.h>

class GameLiftMetricsTest : public ::testing::Test {};

struct PacketSendTest : public ::testing::Test {
  using ResultT = std::tuple<std::string, int>;

  static ResultT MakeResult(const char *Packet, int Size) {
    return ResultT(std::string(Packet), Size);
  }

  std::vector<ResultT> OutputPackets;
  Aws::GameLift::Metrics::MetricsSettings::SendPacketFunc MockSend;

  PacketSendTest()
      : MockSend([this](const char *Packet, int Size) {
          OutputPackets.emplace_back(MakeResult(Packet, Size));
        }) {}

  void ClearOutputPackets() { OutputPackets.clear(); }
};

inline void PrintTo(const ::Aws::GameLift::Metrics::MetricMessage &Message,
                    std::ostream *Stream) {
  *Stream << "([" << (int)Message.GetType() << "] ";
  if (Message.IsTag()) {
    *Stream << Message.GetMetric()->GetKey() << " : tag " << Message.SetTag.GetPtr()->Key
            << ":" << Message.SetTag.GetPtr()->Value << ")";
  } else {
    *Stream << Message.GetMetric()->GetKey() << " : " << Message.SubmitDouble.GetValue()
            << ")";
  }
}

class MockObject {
public:
  MOCK_METHOD(int, Func, (int), ());
};
