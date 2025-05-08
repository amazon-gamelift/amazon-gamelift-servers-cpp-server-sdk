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
#include <aws/gamelift/internal/util/JsonHelper.h>

using namespace Aws::GameLift::Internal;

std::string JsonHelper::SafelyDeserializeString(const rapidjson::Value &value, const char *key) {
    return value.HasMember(key) && value[key].IsString() ? value[key].GetString() : "";
}

void JsonHelper::WriteNonEmptyString(rapidjson::Writer<rapidjson::StringBuffer> *writer, const char *key, const std::string &value) {
    if (!value.empty()) {
        writer->String(key);
        writer->String(value.c_str());
    }
}

int JsonHelper::SafelyDeserializeInt(const rapidjson::Value &value, const char *key) {
    return value.HasMember(key) && value[key].IsInt() ? value[key].GetInt() : -1;
}

void JsonHelper::WritePositiveInt(rapidjson::Writer<rapidjson::StringBuffer> *writer, const char *key, int value) {
    if (value > 0) {
        writer->String(key);
        writer->Int(value);
    }
}

long JsonHelper::SafelyDeserializeLong(const rapidjson::Value &value, const char *key) {
    return value.HasMember(key) && value[key].IsInt64() ? static_cast<long>(value[key].GetInt64()) : -1;
}

void JsonHelper::WritePositiveLong(rapidjson::Writer<rapidjson::StringBuffer> *writer, const char *key, long value) {
    if (value > 0) {
        writer->String(key);
        writer->Int64(value);
    }
}

Aws::GameLift::Server::LogParameters JsonHelper::SafelyDeserializeLogParameters(const rapidjson::Value &value, const char *key) {
    if (!value.HasMember(key) || !value[key].IsArray() || value[key].Size() == 0) {
        return Aws::GameLift::Server::LogParameters();
    }

    const rapidjson::SizeType numLogPaths = value[key].Size();
#ifdef GAMELIFT_USE_STD
    std::vector<std::string> logPaths = std::vector<std::string>();
#else
    char **logPaths = new char *[numLogPaths];
#endif

    for (rapidjson::SizeType i = 0; i < numLogPaths; i++) {
        if (value[key][i].IsString()) {
#ifdef GAMELIFT_USE_STD
            logPaths.push_back(value[key][i].GetString());
#else
            logPaths[i] = new char[MAX_PATH_LENGTH];
#ifdef WIN32
            strcpy_s(logPaths[i], MAX_PATH_LENGTH, value[key][i].GetString());
#else
            strncpy(logPaths[i], value[key][i].GetString(), MAX_PATH_LENGTH);
#endif
#endif
        }
    }

#ifdef GAMELIFT_USE_STD
    return Aws::GameLift::Server::LogParameters(logPaths);
#else
    Aws::GameLift::Server::LogParameters toReturn(logPaths, numLogPaths);

    for (rapidjson::SizeType i = 0; i < numLogPaths; i++) {
        delete[] logPaths[i];
    }
    delete[] logPaths;

    return toReturn;
#endif
}

void JsonHelper::WriteLogParameters(rapidjson::Writer<rapidjson::StringBuffer> *writer, const char *key, const Aws::GameLift::Server::LogParameters &value) {
    if (value.getLogPathCount() == 0) {
        return;
    }

    writer->String(key);
    writer->StartArray();

    for (size_t i = 0; i < value.getLogPathCount(); i++) {
        const char *logPath = value.getLogPath(i);
        // If logPath is not empty, write it.
        if (logPath && logPath[0]) {
            writer->String(value.getLogPath(i));
        }
    }

    writer->EndArray();
}
