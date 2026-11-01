#include "DashboardController.h"

#include <memory>
#include <string>

#include "../services/DashboardSummary.h"
#include "../services/ResidentServices.h"
#include "ControllerSupport.h"

void DashboardController::show(const HttpRequestPtr &, Callback &&callback)
{
    // DashboardService does the aggregate querying and derives Open. This
    // only passes the finished summary to the view.
    const DashboardSummary summary = ResidentServices::instance().dashboard().getSummary();

    HttpViewData data;
    data.insert("pageTitle", std::string("CSMS Dashboard"));
    data.insert("summary", std::make_shared<DashboardSummary>(summary));
    callback(controller_support::render("Dashboard", data));
}
