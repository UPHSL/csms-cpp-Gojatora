#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/HttpViewData.h>

#include <drogon/HttpRequest.h>
#include <trantor/utils/Logger.h>

#include <cctype>
#include <exception>
#include <functional>
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

// Generic 500 page for failures nobody planned for (for example the database
// file cannot be opened). It deliberately says nothing about the cause: no
// exception text, SQL, or path ever reaches the browser. The details go to
// the server log only (see handleUnexpectedException).
inline drogon::HttpResponsePtr internalErrorResponse()
{
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setContentTypeCode(drogon::CT_TEXT_HTML);
    response->setBody(
        "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
        "<title>Something Went Wrong - CSMS</title></head><body>"
        "<main><h1>Something Went Wrong</h1>"
        "<p>The request could not be completed. Please try again, or go back "
        "to one of the modules below.</p>"
        "<p><a href=\"/dashboard\">Dashboard</a> | <a href=\"/residents\">Residents</a> | "
        "<a href=\"/service-requests\">Service Requests</a></p></main></body></html>");
    return response;
}

// Registered with drogon::app().setExceptionHandler() in main.cc.
inline void handleUnexpectedException(const std::exception &error,
                                      const drogon::HttpRequestPtr &req,
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    LOG_ERROR << "Unhandled exception in " << req->path() << ": " << error.what();
    callback(internalErrorResponse());
}
} // namespace controller_support
