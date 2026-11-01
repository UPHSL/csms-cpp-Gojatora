#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Service Request Management interface (T13). Receives browser requests,
// calls the existing T09 submission, T10 status, and T11 query services
// through ResidentServices, and renders the result with the CSP views in
// views/. It holds no validation, transition, SQL, or search logic of its own.
class ServiceRequestController : public HttpController<ServiceRequestController>
{
public:
    using Callback = std::function<void(const HttpResponsePtr &)>;

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ServiceRequestController::list, "/service-requests", Get);
    ADD_METHOD_TO(ServiceRequestController::showSubmitForm, "/service-requests/new", Get);
    ADD_METHOD_TO(ServiceRequestController::submit, "/service-requests/new", Post);
    ADD_METHOD_TO(ServiceRequestController::show, "/service-requests/{1}", Get);
    ADD_METHOD_TO(ServiceRequestController::startProcessing, "/service-requests/{1}/start", Post);
    ADD_METHOD_TO(ServiceRequestController::complete, "/service-requests/{1}/complete", Post);
    ADD_METHOD_TO(ServiceRequestController::cancel, "/service-requests/{1}/cancel", Post);
    METHOD_LIST_END

    // GET: list all Service Requests, or search/filter when ?q= and/or
    // ?status= are supplied (read-only).
    void list(const HttpRequestPtr &req, Callback &&callback);

    // GET: empty submission form (read-only).
    void showSubmitForm(const HttpRequestPtr &req, Callback &&callback);

    // POST: submit a Service Request via T09, then redirect (Post/Redirect/Get).
    void submit(const HttpRequestPtr &req, Callback &&callback);

    // GET: details of one persisted Service Request (read-only).
    void show(const HttpRequestPtr &req, Callback &&callback, std::string id);

    // POST: Pending -> In Progress, decided by T10.
    void startProcessing(const HttpRequestPtr &req, Callback &&callback, std::string id);

    // POST: In Progress -> Completed, decided by T10.
    void complete(const HttpRequestPtr &req, Callback &&callback, std::string id);

    // POST: Pending or In Progress -> Cancelled, decided by T10.
    void cancel(const HttpRequestPtr &req, Callback &&callback, std::string id);
};
