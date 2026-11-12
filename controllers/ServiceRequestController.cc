#include "ServiceRequestController.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../models/ServiceRequest.h"
#include "../services/ResidentServices.h"
#include "ControllerSupport.h"

namespace
{
using FieldMap = std::map<std::string, std::string>;

using controller_support::parseId;
using controller_support::redirectTo;
using controller_support::render;

// The browser only ever sends a short code after a redirect; the wording
// lives here so arbitrary text can never be injected into the page.
std::string flashMessage(const std::string &code)
{
    if (code == "submitted") return "Service Request submitted successfully.";
    if (code == "started") return "Service Request is now In Progress.";
    if (code == "completed") return "Service Request completed successfully.";
    if (code == "cancelled") return "Service Request cancelled successfully.";
    return "";
}

// Turns the field names reported by the T09 validator into readable text.
std::string errorMessageFor(const std::string &field)
{
    if (field == "residentId") return "Resident ID must be a positive number.";
    if (field == "serviceType") return "Service type is required.";
    if (field == "description") return "Description is required.";
    if (field == "dateRequested") return "Date requested is required and must be a valid date (YYYY-MM-DD).";
    return "Service Request information is invalid (" + field + ").";
}

FieldMap toErrorMap(const std::vector<std::string> &fields)
{
    FieldMap errors;
    for (const auto &field : fields)
    {
        errors[field] = errorMessageFor(field);
    }
    return errors;
}

FieldMap readFormValues(const HttpRequestPtr &req)
{
    return FieldMap{{"residentId", req->getParameter("residentId")},
                    {"serviceType", req->getParameter("serviceType")},
                    {"description", req->getParameter("description")},
                    {"dateRequested", req->getParameter("dateRequested")}};
}

// Converts the Resident ID text into the int a ServiceRequest carries. This
// is type conversion only: anything that is not a plain number becomes 0,
// which the T09 validator then rejects as an invalid residentId.
int residentIdFromText(const std::string &text)
{
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return 0;
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    int id = 0;
    return parseId(text.substr(start, end - start + 1), id) ? id : 0;
}

HttpResponsePtr notFoundPage()
{
    HttpViewData data;
    data.insert("pageTitle", std::string("Service Request Not Found"));
    return render("ServiceRequestNotFound", data, k404NotFound);
}

HttpResponsePtr formPage(const FieldMap &values, const FieldMap &errors, HttpStatusCode code)
{
    HttpViewData data;
    data.insert("pageTitle", std::string("Submit Service Request"));
    data.insert("values", values);
    data.insert("errors", errors);
    return render("ServiceRequestForm", data, code);
}

HttpResponsePtr detailsPage(const ServiceRequest &request,
                            const std::string &message,
                            const std::string &errorMessage,
                            HttpStatusCode code = k200OK)
{
    HttpViewData data;
    data.insert("pageTitle", std::string("Service Request Details"));
    data.insert("serviceRequest", std::make_shared<ServiceRequest>(request));
    data.insert("message", message);
    data.insert("errorMessage", errorMessage);
    return render("ServiceRequestDetails", data, code);
}

// Shared by the three workflow actions. T10 owns the transition rules; this
// only names the target status and presents whatever outcome T10 reports.
HttpResponsePtr applyWorkflowAction(const std::string &id,
                                    const std::string &targetStatus,
                                    const std::string &successCode)
{
    int serviceRequestId = 0;
    if (!parseId(id, serviceRequestId))
    {
        return notFoundPage();
    }

    auto &services = ResidentServices::instance();
    const auto result = services.status().changeStatus(serviceRequestId, targetStatus);

    if (result.notFound)
    {
        return notFoundPage();
    }

    if (result.invalidTransition || result.unsupportedStatus)
    {
        // Nothing was changed. Show the request as it really is, with the reason.
        const auto current = services.query().viewServiceRequest(serviceRequestId);
        if (!current.has_value())
        {
            return notFoundPage();
        }
        return detailsPage(*current,
                           "",
                           "That action is not allowed for a " +
                               serviceRequestStatusToString(current->getStatus()) +
                               " Service Request.",
                           k409Conflict);
    }

    return redirectTo("/service-requests/" + id + "?msg=" + successCode);
}
} // namespace

void ServiceRequestController::list(const HttpRequestPtr &req, Callback &&callback)
{
    const std::string searchTerm = req->getParameter("q");
    const std::string statusFilter = req->getParameter("status");

    // T11 trims the term, matches serviceType OR description (partial,
    // case-insensitive), applies the optional status with AND, and returns
    // the established dateRequested DESC, id DESC order. No filtering or
    // sorting happens here.
    const auto result =
        ResidentServices::instance().query().searchServiceRequests(searchTerm, statusFilter);

    HttpViewData data;
    data.insert("pageTitle", std::string("Service Request Management"));
    data.insert("serviceRequests", result.serviceRequests);
    data.insert("searchTerm", searchTerm);
    data.insert("statusFilter", statusFilter);
    data.insert("message", flashMessage(req->getParameter("msg")));
    data.insert("errorMessage",
                result.unsupportedStatus ? std::string("Unsupported status filter.") : std::string());
    callback(render("ServiceRequestList", data, result.unsupportedStatus ? k400BadRequest : k200OK));
}

void ServiceRequestController::showSubmitForm(const HttpRequestPtr &, Callback &&callback)
{
    callback(formPage(FieldMap{}, FieldMap{}, k200OK));
}

void ServiceRequestController::submit(const HttpRequestPtr &req, Callback &&callback)
{
    const FieldMap values = readFormValues(req);

    // New requests carry no id (the database assigns it) and always start
    // Pending; any "id" or "status" field in the request is never read.
    const ServiceRequest request(residentIdFromText(values.at("residentId")),
                                 values.at("serviceType"),
                                 values.at("description"),
                                 values.at("dateRequested"));

    // T09 validates, verifies the Resident exists and is Active, then persists.
    const auto result = ResidentServices::instance().submission().submitServiceRequest(request);

    if (result.success)
    {
        callback(redirectTo("/service-requests?msg=submitted"));
        return;
    }

    FieldMap errors = toErrorMap(result.errors);
    if (result.residentNotFound)
    {
        errors["residentId"] = "Resident not found.";
    }
    if (result.residentInactive)
    {
        errors["residentId"] = "Inactive Residents cannot submit new Service Requests.";
    }

    callback(formPage(values, errors, k422UnprocessableEntity));
}

void ServiceRequestController::show(const HttpRequestPtr &req, Callback &&callback, std::string id)
{
    int serviceRequestId = 0;
    std::optional<ServiceRequest> serviceRequest;
    if (parseId(id, serviceRequestId))
    {
        serviceRequest = ResidentServices::instance().query().viewServiceRequest(serviceRequestId);
    }

    if (!serviceRequest.has_value())
    {
        callback(notFoundPage());
        return;
    }

    callback(detailsPage(*serviceRequest, flashMessage(req->getParameter("msg")), ""));
}

void ServiceRequestController::startProcessing(const HttpRequestPtr &, Callback &&callback, std::string id)
{
    callback(applyWorkflowAction(id, "In Progress", "started"));
}

void ServiceRequestController::complete(const HttpRequestPtr &, Callback &&callback, std::string id)
{
    callback(applyWorkflowAction(id, "Completed", "completed"));
}

void ServiceRequestController::cancel(const HttpRequestPtr &, Callback &&callback, std::string id)
{
    callback(applyWorkflowAction(id, "Cancelled", "cancelled"));
}
