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

#include <aws/gamelift/internal/security/ContainerMetadataFetcher.h>
#include <stdexcept>
#include <cstdlib>
#include "rapidjson/document.h"

using namespace Aws::GameLift::Internal;

ContainerMetadataFetcher::ContainerMetadataFetcher(HttpClient &httpClient)
        : httpClient(httpClient) {}

Aws::GameLift::Outcome<ContainerTaskMetadata, std::string> ContainerMetadataFetcher::FetchContainerTaskMetadata() {
    const char *containerMetadataUri = std::getenv(EnvironmentVariableContainerMetadataUri.c_str());
    if (!containerMetadataUri) {
        return "The environment variable " + EnvironmentVariableContainerMetadataUri + " is not set.";
    }
    std::string baseUrl = containerMetadataUri;
    HttpResponse response = httpClient.SendGetRequest(baseUrl + "/" + TaskMetadataRelativePath);

    if(!response.IsSuccessfulStatusCode()) {
        return std::string(
                "Failed to get Container Task Metadata from Container Metadata Service. HTTP Response Status Code is " +
                std::to_string(response.statusCode));
    }
    rapidjson::Document document;
    if (document.Parse(response.body.c_str()).HasParseError()) {
        return std::string("Error parsing Container Metadata Service JSON response");
    }
    if (!document.HasMember("TaskARN") || !document["TaskARN"].IsString()) {
        return std::string("TaskArn is not found in Container Metadata Service response");
    }
    std::string taskArn = document["TaskARN"].GetString();
    if (taskArn.empty()) {
        return std::string("Invalid TaskARN, value is empty");
    }
    if (taskArn.find('/') == std::string::npos) {
        return std::string("Failed to extract Task ID from container TaskArn with value " + taskArn);
    }
    std::string taskId = taskArn.substr(taskArn.find_last_of('/') + 1);
    return ContainerTaskMetadata(taskId);
}
