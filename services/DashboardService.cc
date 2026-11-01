#include "DashboardService.h"

DashboardService::DashboardService(ResidentRepository &residentRepository,
                                   ServiceRequestRepository &serviceRequestRepository)
    : residentRepository_(residentRepository),
      serviceRequestRepository_(serviceRequestRepository)
{
}

DashboardSummary DashboardService::getSummary()
{
    DashboardSummary summary;

    // Residents: COUNT(*) in the database, not findAll() plus a loop.
    summary.residents.total = residentRepository_.countAll();
    summary.residents.active = residentRepository_.countByStatus(ResidentStatus::Active);
    summary.residents.inactive = residentRepository_.countByStatus(ResidentStatus::Inactive);

    // Service Requests: same approach, one filtered COUNT per status.
    ServiceRequestSummary &requests = summary.serviceRequests;
    requests.total = serviceRequestRepository_.countAll();
    requests.pending = serviceRequestRepository_.countByStatus(ServiceRequestStatus::Pending);
    requests.inProgress = serviceRequestRepository_.countByStatus(ServiceRequestStatus::InProgress);
    requests.completed = serviceRequestRepository_.countByStatus(ServiceRequestStatus::Completed);
    requests.cancelled = serviceRequestRepository_.countByStatus(ServiceRequestStatus::Cancelled);

    // Open is derived from existing statuses; it is not a status of its own.
    requests.open = requests.pending + requests.inProgress;

    // Recent requests reuse the T11 ordering (date_requested DESC, id DESC)
    // with a LIMIT, so only the rows to be shown are loaded.
    summary.recentServiceRequests =
        serviceRequestRepository_.findRecent(kRecentServiceRequestLimit);

    return summary;
}
