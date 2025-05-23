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

#include <aws/gamelift/internal/security/ContainerCredentialsFetcher.h>
#include <stdexcept>
#include <cstdlib>
#include "rapidjson/document.h"

using namespace Aws::GameLift::Internal;

ContainerCredentialsFetcher::ContainerCredentialsFetcher(HttpClient &httpClient)
        : httpClient(httpClient) {}

Aws::GameLift::Outcome<AwsCredentials, std::string> ContainerCredentialsFetcher::FetchContainerCredentials() {
    const char *credentialsRelativeUri = std::getenv(EnvironmentVariableContainerCredentialsRelativeUri.c_str());
    if (!credentialsRelativeUri) {
        return "The environment variable " + EnvironmentVariableContainerCredentialsRelativeUri + " is not set.";
    }

    std::string relativeUri = credentialsRelativeUri;
    HttpResponse response = httpClient.SendGetRequest(ContainerCredentialProviderUrl + relativeUri);

    if (!response.IsSuccessfulStatusCode()) {
        return std::string(
            "Failed to get Container Credentials from Container Credential Provider. HTTP Response Status Code is " +
            std::to_string(response.statusCode));
    }

    rapidjson::Document document;
    if (document.Parse(response.body.c_str()).HasParseError()) {
        return std::string("Error parsing Container Credential Provider JSON response");
    }

    if (!document.HasMember("AccessKeyId") || !document["AccessKeyId"].IsString()) {
        return std::string("AccessKeyId is not found in Container Credential Provider response");
    }

    if (!document.HasMember("SecretAccessKey") || !document["SecretAccessKey"].IsString()) {
        return std::string("SecretAccessKey is not found in Container Credential Provider response");
    }

    if (!document.HasMember("Token") || !document["Token"].IsString()) {
        return std::string("Token is not found in Container Credential Provider response");
    }

    AwsCredentials awsCredentials(
            document["AccessKeyId"].GetString(),
            document["SecretAccessKey"].GetString(),
            document["Token"].GetString());
    return awsCredentials;
}

