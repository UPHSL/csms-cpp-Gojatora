#pragma once

#include <vector>

#include "../models/ServiceRequest.h"

// Read-only snapshot of the CSMS state shown on the dashboard (T14). Every
// number is computed from persisted data when the dashboard is requested;
// nothing here is stored or kept as a running counter.

struct ResidentSummary
{
    int total{0};    // every persisted Resident (Inactive ones included)
    int active{0};
    int inactive{0}; // total == active + inactive
};

struct ServiceRequestSummary
{
    int total{0}; // every persisted Service Request, whatever its status
    int pending{0};
    int inProgress{0};
    int completed{0};
    int cancelled{0}; // total == pending + inProgress + completed + cancelled

    // Derived, never stored: Pending + In Progress. Completed and Cancelled
    // are terminal, so they are not open.
    int open{0};
};

struct DashboardSummary
{
    ResidentSummary residents{};
    ServiceRequestSummary serviceRequests{};

    // The most recent Service Requests (any status), newest first.
    std::vector<ServiceRequest> recentServiceRequests{};
};
