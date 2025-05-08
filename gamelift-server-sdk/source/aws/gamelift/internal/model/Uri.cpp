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

#include <aws/gamelift/internal/model/Uri.h>
#include <sstream>

namespace Aws {
namespace GameLift {
namespace Internal {

Uri::UriBuilder &Uri::UriBuilder::AddQueryParam(const std::string &key, const std::string &value) {
    m_queryParams[key] = value;
    return *this;
}

Uri::UriBuilder &Uri::UriBuilder::WithBaseUri(const std::string &baseUriString) {
    m_baseUriString = baseUriString;
    return *this;
}

Uri Uri::UriBuilder::Build() const { return Uri(m_baseUriString, m_queryParams); }

Uri::Uri(const std::string &baseUriString, const std::map<std::string, std::string> &queryMap) : m_baseUriString(baseUriString), m_queryMap(queryMap) {
    SetQueryString(queryMap);
    m_uriString = m_baseUriString + m_queryString;
}

void Uri::SetQueryString(const std::map<std::string, std::string> &queryMap) {
    std::ostringstream queryStringStream;
    queryStringStream << "?";
    for (auto const &queryPair : queryMap) {
        // All but first param requires "&" delimiter
        if (queryStringStream.str().size() != 1) {
            queryStringStream << "&";
        }
        queryStringStream << queryPair.first << "=" << queryPair.second;
    }
    m_queryString = queryStringStream.str();
}

std::ostream &operator<<(std::ostream &os, const Uri &uri) {
    os << uri.GetUriString();
    return os;
}

} // namespace Internal
} // namespace GameLift
} // namespace Aws