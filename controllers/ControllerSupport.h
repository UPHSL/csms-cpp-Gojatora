#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/HttpViewData.h>

#include <cctype>
#include <string>

// Small HTTP helpers shared by the Resident and Service Request controllers.
// Presentation plumbing only: no business rules live here.
namespace controller_support
{
// Parses a path segment as a positive id. Anything else (letters, "-1",
// overflow) cannot name a record, so callers treat it as not found.
inline bool parseId(const std::string &text, int &id)
{
    if (text.empty() || text.size() > 9)
    {
        return false;
    }
    for (unsigned char c : text)
    {
        if (!std::isdigit(c))
        {
            return false;
        }
    }
    id = std::stoi(text);
    return id > 0;
}

inline drogon::HttpResponsePtr render(const char *view,
                                      drogon::HttpViewData &data,
                                      drogon::HttpStatusCode status = drogon::k200OK)
{
    auto response = drogon::HttpResponse::newHttpViewResponse(view, data);
    response->setStatusCode(status);
    return response;
}

// 303 See Other: the browser follows the redirect with a GET, so a refresh
// of the resulting page cannot re-submit the form.
inline drogon::HttpResponsePtr redirectTo(const std::string &url)
{
    auto response = drogon::HttpResponse::newRedirectionResponse(url);
    response->setStatusCode(drogon::k303SeeOther);
    return response;
}
} // namespace controller_support
