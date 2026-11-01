#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Resident Management interface (T12). Receives browser requests, calls the
// existing T04-T07 application services through ResidentServices, and
// renders the result with the CSP views in views/. It holds no validation,
// SQL, or search logic of its own.
class ResidentController : public HttpController<ResidentController>
{
public:
    using Callback = std::function<void(const HttpResponsePtr &)>;

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ResidentController::list, "/residents", Get);
    ADD_METHOD_TO(ResidentController::showRegisterForm, "/residents/register", Get);
    ADD_METHOD_TO(ResidentController::registerResident, "/residents/register", Post);
    ADD_METHOD_TO(ResidentController::showEditForm, "/residents/{1}/edit", Get);
    ADD_METHOD_TO(ResidentController::updateResident, "/residents/{1}/edit", Post);
    ADD_METHOD_TO(ResidentController::deactivateResident, "/residents/{1}/deactivate", Post);
    METHOD_LIST_END

    // GET: list all Residents, or search when ?q= is supplied (read-only).
    void list(const HttpRequestPtr &req, Callback &&callback);

    // GET: empty registration form (read-only).
    void showRegisterForm(const HttpRequestPtr &req, Callback &&callback);

    // POST: register a Resident via T04, then redirect (Post/Redirect/Get).
    void registerResident(const HttpRequestPtr &req, Callback &&callback);

    // GET: edit form prefilled from the persisted Resident (read-only).
    void showEditForm(const HttpRequestPtr &req, Callback &&callback, std::string id);

    // POST: update a Resident via T06, then redirect.
    void updateResident(const HttpRequestPtr &req, Callback &&callback, std::string id);

    // POST: soft-deactivate a Resident via T07, then redirect.
    void deactivateResident(const HttpRequestPtr &req, Callback &&callback, std::string id);
};
