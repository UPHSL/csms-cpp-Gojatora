#include "ResidentController.h"

#include <cctype>
#include <map>
#include <string>
#include <vector>

#include "../services/ResidentServices.h"

namespace
{
using FieldMap = std::map<std::string, std::string>;

// The browser only ever sends a short code after a redirect; the wording
// lives here so arbitrary text can never be injected into the page.
std::string flashMessage(const std::string &code)
{
    if (code == "registered") return "Resident registered successfully.";
    if (code == "updated") return "Resident information updated successfully.";
    if (code == "deactivated") return "Resident deactivated successfully.";
    if (code == "alreadyinactive") return "Resident is already Inactive.";
    return "";
}

// Turns the field names reported by the T02 validator into readable text.
std::string errorMessageFor(const std::string &field)
{
    if (field == "firstName") return "First name is required.";
    if (field == "lastName") return "Last name is required.";
    if (field == "address") return "Address is required.";
    if (field == "contactNumber") return "Contact number must follow 09XXXXXXXXX.";
    if (field == "email") return "Email is invalid.";
    return "Resident information is invalid (" + field + ").";
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
    return FieldMap{{"firstName", req->getParameter("firstName")},
                    {"lastName", req->getParameter("lastName")},
                    {"address", req->getParameter("address")},
                    {"contactNumber", req->getParameter("contactNumber")},
                    {"email", req->getParameter("email")}};
}

FieldMap valuesOf(const Resident &resident)
{
    return FieldMap{{"firstName", resident.getFirstName()},
                    {"lastName", resident.getLastName()},
                    {"address", resident.getAddress()},
                    {"contactNumber", resident.getContactNumber()},
                    {"email", resident.getEmail()}};
}

// Parses a path segment as a positive id. Anything else (letters, "-1",
// overflow) cannot name a Resident, so it is treated as not found.
bool parseId(const std::string &text, int &id)
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

HttpResponsePtr render(const char *view, HttpViewData &data, HttpStatusCode status = k200OK)
{
    auto response = HttpResponse::newHttpViewResponse(view, data);
    response->setStatusCode(status);
    return response;
}

// 303 See Other: the browser follows the redirect with a GET, so a refresh
// of the resulting page cannot re-submit the form.
HttpResponsePtr redirectTo(const std::string &url)
{
    auto response = HttpResponse::newRedirectionResponse(url);
    response->setStatusCode(k303SeeOther);
    return response;
}

HttpResponsePtr notFoundPage()
{
    HttpViewData data;
    data.insert("pageTitle", std::string("Resident Not Found"));
    return render("ResidentNotFound", data, k404NotFound);
}

HttpResponsePtr formPage(bool editing,
                         const std::string &action,
                         const FieldMap &values,
                         const FieldMap &errors,
                         const std::string &residentId,
                         const std::string &status,
                         HttpStatusCode code)
{
    HttpViewData data;
    data.insert("pageTitle", std::string(editing ? "Edit Resident" : "Register Resident"));
    data.insert("editing", editing);
    data.insert("action", action);
    data.insert("values", values);
    data.insert("errors", errors);
    data.insert("residentId", residentId);
    data.insert("status", status);
    return render("ResidentForm", data, code);
}
} // namespace

void ResidentController::list(const HttpRequestPtr &req, Callback &&callback)
{
    const std::string searchTerm = req->getParameter("q");

    // T05: blank terms list everything, otherwise a name search, in the
    // established lastName, firstName, id order. No filtering happens here.
    const std::vector<Resident> residents =
        ResidentServices::instance().search().searchResidents(searchTerm);

    HttpViewData data;
    data.insert("pageTitle", std::string("Resident Management"));
    data.insert("residents", residents);
    data.insert("searchTerm", searchTerm);
    data.insert("message", flashMessage(req->getParameter("msg")));
    callback(render("ResidentList", data));
}

void ResidentController::showRegisterForm(const HttpRequestPtr &, Callback &&callback)
{
    callback(formPage(false, "/residents/register", FieldMap{}, FieldMap{}, "", "", k200OK));
}

void ResidentController::registerResident(const HttpRequestPtr &req, Callback &&callback)
{
    const FieldMap values = readFormValues(req);

    // New Residents carry no id (the database assigns it) and start Active.
    const Resident resident(values.at("firstName"),
                            values.at("lastName"),
                            values.at("address"),
                            values.at("contactNumber"),
                            values.at("email"),
                            ResidentStatus::Active);

    const auto result = ResidentServices::instance().registration().registerResident(resident);

    if (!result.success)
    {
        callback(formPage(false,
                          "/residents/register",
                          values,
                          toErrorMap(result.errors),
                          "",
                          "",
                          k422UnprocessableEntity));
        return;
    }

    callback(redirectTo("/residents?msg=registered"));
}

void ResidentController::showEditForm(const HttpRequestPtr &, Callback &&callback, std::string id)
{
    int residentId = 0;
    std::optional<Resident> resident;
    if (parseId(id, residentId))
    {
        resident = ResidentServices::instance().repository().findById(residentId);
    }

    if (!resident.has_value())
    {
        callback(notFoundPage());
        return;
    }

    callback(formPage(true,
                      "/residents/" + id + "/edit",
                      valuesOf(*resident),
                      FieldMap{},
                      id,
                      residentStatusToString(resident->getStatus()),
                      k200OK));
}

void ResidentController::updateResident(const HttpRequestPtr &req, Callback &&callback, std::string id)
{
    int residentId = 0;
    if (!parseId(id, residentId))
    {
        callback(notFoundPage());
        return;
    }

    const FieldMap values = readFormValues(req);

    // T06 looks the Resident up, keeps its id and status, validates, and
    // persists. Any "status" or "id" field in the request is never read.
    const auto result = ResidentServices::instance().update().updateResident(residentId,
                                                                             values.at("firstName"),
                                                                             values.at("lastName"),
                                                                             values.at("address"),
                                                                             values.at("contactNumber"),
                                                                             values.at("email"));

    if (result.residentNotFound)
    {
        callback(notFoundPage());
        return;
    }

    if (!result.success)
    {
        const auto current = ResidentServices::instance().repository().findById(residentId);
        callback(formPage(true,
                          "/residents/" + id + "/edit",
                          values,
                          toErrorMap(result.errors),
                          id,
                          current ? residentStatusToString(current->getStatus()) : "",
                          k422UnprocessableEntity));
        return;
    }

    callback(redirectTo("/residents?msg=updated"));
}

void ResidentController::deactivateResident(const HttpRequestPtr &, Callback &&callback, std::string id)
{
    int residentId = 0;
    if (!parseId(id, residentId))
    {
        callback(notFoundPage());
        return;
    }

    // T07 is idempotent and never deletes; it reports not-found and
    // already-Inactive outcomes for us to present.
    const auto result = ResidentServices::instance().deactivation().deactivateResident(residentId);

    if (result.residentNotFound)
    {
        callback(notFoundPage());
        return;
    }

    callback(redirectTo(result.stateChanged ? "/residents?msg=deactivated"
                                            : "/residents?msg=alreadyinactive"));
}
