#pragma once

#include <string>

#include "../models/ServiceRequest.h"
#include "../repositories/ServiceRequestRepository.h"
#include "ServiceRequestStatusResult.h"

// Manages the lifecycle of an EXISTING Service Request. It loads the
// persisted request, decides whether the requested move is allowed, and only
// then asks the repository to update the status. It contains no SQL and
// never creates a Service Request.
//
// Allowed transitions (everything else, including same-status, is invalid):
//   Pending     -> In Progress, Cancelled
//   In Progress -> Completed, Cancelled
//   Completed   -> (none, terminal)
//   Cancelled   -> (none, terminal)
class ServiceRequestStatusService
{
public:
    explicit ServiceRequestStatusService(ServiceRequestRepository &serviceRequestRepository);

    // requestedStatus is text so that unsupported values (e.g. "Approved")
    // can be received and rejected safely.
    ServiceRequestStatusResult changeStatus(int serviceRequestId,
                                            const std::string &requestedStatus);

private:
    ServiceRequestRepository &serviceRequestRepository_;

    bool isTransitionAllowed(ServiceRequestStatus current, ServiceRequestStatus target) const;
};
