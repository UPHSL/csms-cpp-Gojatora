#pragma once

#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "DashboardSummary.h"

// The read/query side of the dashboard. It asks the Resident and Service
// Request repositories for aggregate counts and the recent requests, derives
// the Open count from the Pending and In Progress counts, and returns one
// DashboardSummary. It contains no SQL, renders no HTML, and never writes:
// registration, deactivation, submission, and status changes stay with their
// own services.
class DashboardService
{
public:
    // How many recent Service Requests the dashboard shows.
    static constexpr int kRecentServiceRequestLimit = 5;

    DashboardService(ResidentRepository &residentRepository,
                     ServiceRequestRepository &serviceRequestRepository);

    // Builds the summary from the current persisted data. Safe to call any
    // number of times: it does not change anything.
    DashboardSummary getSummary();

private:
    ResidentRepository &residentRepository_;
    ServiceRequestRepository &serviceRequestRepository_;
};
