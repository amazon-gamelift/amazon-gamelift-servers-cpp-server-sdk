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

#include <aws/gamelift/internal/model/adapter/StartMatchBackfillAdapter.h>

namespace Aws {
namespace GameLift {
namespace Internal {

Server::Model::StartMatchBackfillResult StartMatchBackfillAdapter::convert(const WebSocketStartMatchBackfillResponse *webSocketResponse) {
    Server::Model::StartMatchBackfillResult result;
    if (webSocketResponse) {
        result.SetTicketId(webSocketResponse->GetTicketId().c_str());
    }
    return result;
}

WebSocketStartMatchBackfillRequest StartMatchBackfillAdapter::convert(const Server::Model::StartMatchBackfillRequest &request) {
    std::vector<Server::Model::Player> requestPlayersAsVector;
#ifdef GAMELIFT_USE_STD
    requestPlayersAsVector = request.GetPlayers();
#else
    int countOfPlayers;
    const Server::Model::Player *requestPlayers = request.GetPlayers(countOfPlayers);
    for (int i = 0; i < countOfPlayers; i++) {
        Server::Model::Player playerToStore = *(requestPlayers + i);
        requestPlayersAsVector.push_back(playerToStore);
    }
#endif
    std::vector<WebSocketPlayer> players;

    for (auto it = requestPlayersAsVector.begin(); it != requestPlayersAsVector.end(); ++it) {
        Server::Model::Player playerToConvert = *it;
        WebSocketPlayer player = WebSocketPlayer()
                                     .WithPlayerId(playerToConvert.GetPlayerId())
                                     .WithTeam(playerToConvert.GetTeam())
                                     .WithPlayerAttributes(fetchAndConvertAttributes(playerToConvert))
                                     .WithLatencyInMs(fetchAndConvertLatency(playerToConvert));
        players.push_back(player);
    }

    return WebSocketStartMatchBackfillRequest()
        .WithTicketId(request.GetTicketId())
        .WithGameSessionArn(request.GetGameSessionArn())
        .WithMatchmakingConfigurationArn(request.GetMatchmakingConfigurationArn())
        .WithPlayers(players);
}

std::map<std::string, int> StartMatchBackfillAdapter::fetchAndConvertLatency(const Server::Model::Player &player) {
    std::map<std::string, int> result;
#ifdef GAMELIFT_USE_STD
    result = player.GetLatencyInMs();
#else
    int countOfLatencies;
    const Server::Model::Player::RegionAndLatency *regionAndLatency = player.GetLatencyMs(countOfLatencies);
    for (int i = 0; i < countOfLatencies; i++) {
        Server::Model::Player::RegionAndLatency latencyToConvert = *(regionAndLatency + i);
        result[latencyToConvert.GetRegion()] = latencyToConvert.GetLatencyMs();
    }
#endif

    return result;
}

std::map<std::string, WebSocketAttributeValue> StartMatchBackfillAdapter::fetchAndConvertAttributes(const Server::Model::Player &player) {
    std::map<std::string, Server::Model::AttributeValue> attrs;
#ifdef GAMELIFT_USE_STD
    attrs = player.GetPlayerAttributes();
#else
    int countOfAttributes;
    const Server::Model::Player::NamedAttribute *attributes = player.GetPlayerAttributes(countOfAttributes);
    for (int i = 0; i < countOfAttributes; i++) {
        Server::Model::Player::NamedAttribute attributeToConvert = *(attributes + i);
        attrs[attributeToConvert.GetName()] = attributeToConvert.GetValue();
    }
#endif

    std::map<std::string, WebSocketAttributeValue> result;
    for (auto const &attr : attrs) {
        Server::Model::AttributeValue attributeValue = attr.second;
        WebSocketAttributeValue value;
        switch (attributeValue.GetType()) {
        case Server::Model::AttributeValue::AttrType::NONE:
            value.SetAttributeType(WebSocketAttrType::NONE);
            break;
        case Server::Model::AttributeValue::AttrType::STRING:
            value.SetAttributeType(WebSocketAttrType::STRING);
            value.SetS(attributeValue.GetS());
            break;
        case Server::Model::AttributeValue::AttrType::DOUBLE:
            value.SetAttributeType(WebSocketAttrType::DOUBLE);
            value.SetN(attributeValue.GetN());
            break;
        case Server::Model::AttributeValue::AttrType::STRING_LIST:
            value.SetAttributeType(WebSocketAttrType::STRING_LIST);
            value.SetSL(convertStringList(attributeValue));
            break;
        case Server::Model::AttributeValue::AttrType::STRING_DOUBLE_MAP:
            value.SetAttributeType(WebSocketAttrType::STRING_DOUBLE_MAP);
            value.SetSDM(convertStringDoubleMap(attributeValue));
            break;
        default:
            value.SetAttributeType(WebSocketAttrType::NONE);
            break;
        }
        result[attr.first] = value;
    }
    return result;
}

std::vector<std::string> StartMatchBackfillAdapter::convertStringList(const Server::Model::AttributeValue &attributeValue) {
    std::vector<std::string> result;

#ifdef GAMELIFT_USE_STD
    result = attributeValue.GetSL();
#else
    int countOfStringList;
    const Server::Model::AttributeValue::AttributeStringType *stringList = attributeValue.GetSL(countOfStringList);
    for (int i = 0; i < countOfStringList; i++) {
        const Server::Model::AttributeValue::AttributeStringType *stringListItemPointer = stringList + i;
        std::string itemAsString = std::string(*stringListItemPointer);
        result.push_back(itemAsString);
    }
#endif

    return result;
}

std::map<std::string, double> StartMatchBackfillAdapter::convertStringDoubleMap(const Server::Model::AttributeValue &attributeValue) {
    std::map<std::string, double> result;

#ifdef GAMELIFT_USE_STD
    result = attributeValue.GetSDM();
#else
    int countOfStringMap;
    const Server::Model::AttributeValue::KeyAndValue *stringAndDoubleMap = attributeValue.GetSDM(countOfStringMap);
    for (int i = 0; i < countOfStringMap; i++) {
        Server::Model::AttributeValue::KeyAndValue keyAndValue = *(stringAndDoubleMap + i);
        std::string key = std::string(keyAndValue.GetKey());
        result[key] = keyAndValue.GetValue();
    }
#endif

    return result;
}
} // namespace Internal
} // namespace GameLift
} // namespace Aws