#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// CSMS Dashboard (T14). A read-only page: it asks DashboardService for the
// summary and renders it with views/Dashboard.csp. It holds no SQL, counting,
// or business rules, and it handles GET only, so viewing or refreshing the
// dashboard cannot change any Resident or Service Request.
class DashboardController : public HttpController<DashboardController>
{
public:
    using Callback = std::function<void(const HttpResponsePtr &)>;

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DashboardController::show, "/dashboard", Get);
    METHOD_LIST_END

    // GET: operational summary of Residents and Service Requests (read-only).
    void show(const HttpRequestPtr &req, Callback &&callback);
};
