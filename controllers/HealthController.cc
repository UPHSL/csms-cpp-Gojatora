#include "HealthController.h"

#include "AppInfo.h"

// Reports identity, release and availability only. It deliberately exposes
// nothing else (no paths, configuration, database details or environment).
void HealthController::asyncHandleHttpRequest(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value result;

    result["status"] = "OK";
    result["application"] = app_info::kShortName;
    result["name"] = app_info::kFullName;
    result["version"] = app_info::kVersion;

    auto response = HttpResponse::newHttpJsonResponse(result);

    callback(response);
}
