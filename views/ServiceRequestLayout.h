#pragma once

#include <string>

#include "models/ServiceRequest.h"
#include "views/ResidentLayout.h"

// Helpers shared by the Service Request views. Presentation only.

// CSS class for a status badge. The status text is always printed beside it,
// so colour never carries the meaning on its own.
inline std::string statusBadgeClass(ServiceRequestStatus status)
{
    switch (status)
    {
        case ServiceRequestStatus::Pending: return "pending";
        case ServiceRequestStatus::InProgress: return "inprogress";
        case ServiceRequestStatus::Completed: return "completed";
        case ServiceRequestStatus::Cancelled: return "cancelled";
    }
    return "";
}

inline std::string statusBadge(ServiceRequestStatus status)
{
    return "<span class=\"badge " + statusBadgeClass(status) + "\">" +
           h(serviceRequestStatusToString(status)) + "</span>";
}

// Shortens long text for list display without splitting a UTF-8 character.
// The complete text stays available on the details page.
inline std::string shorten(const std::string &text, std::size_t maxBytes = 60)
{
    if (text.size() <= maxBytes)
    {
        return text;
    }
    std::size_t cut = maxBytes;
    while (cut > 0 && (static_cast<unsigned char>(text[cut]) & 0xC0) == 0x80)
    {
        --cut;
    }
    return text.substr(0, cut) + "...";
}

// The workflow buttons that make sense for a status. This only decides what
// to SHOW; ServiceRequestStatusService (T10) still decides what is allowed.
// Completed and Cancelled are terminal, so they get no buttons at all.
inline std::string workflowActions(int id, ServiceRequestStatus status)
{
    auto button = [&](const char *path, const char *label, bool secondary) {
        return "<form method=\"post\" action=\"/service-requests/" + std::to_string(id) + "/" +
               path + "\"><button type=\"submit\"" + (secondary ? " class=\"secondary\"" : "") +
               ">" + label + "</button></form>";
    };

    switch (status)
    {
        case ServiceRequestStatus::Pending:
            return button("start", "Start Processing", false) + button("cancel", "Cancel", true);
        case ServiceRequestStatus::InProgress:
            return button("complete", "Complete", false) + button("cancel", "Cancel", true);
        case ServiceRequestStatus::Completed:
        case ServiceRequestStatus::Cancelled:
            break;
    }
    return "";
}
