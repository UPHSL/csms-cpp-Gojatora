#include "ServiceRequestStatusService.h"

ServiceRequestStatusService::ServiceRequestStatusService(
    ServiceRequestRepository &serviceRequestRepository)
    : serviceRequestRepository_(serviceRequestRepository)
{
}

ServiceRequestStatusResult ServiceRequestStatusService::changeStatus(
    int serviceRequestId,
    const std::string &requestedStatus)
{
    // 1. The request must already exist. An unknown id stops here: nothing
    //    is inserted and nothing is modified.
    std::optional<ServiceRequest> existing = serviceRequestRepository_.findById(serviceRequestId);

    if (!existing.has_value())
    {
        ServiceRequestStatusResult result;
        result.notFound = true;
        return result;
    }

    // 2. The requested status must be one of the four supported values.
    std::optional<ServiceRequestStatus> target = serviceRequestStatusFromString(requestedStatus);

    if (!target.has_value())
    {
        ServiceRequestStatusResult result;
        result.unsupportedStatus = true;
        return result;
    }

    // 3. Compare the current PERSISTED status with the target. This happens
    //    before any write, so a rejected change never touches the database.
    if (!isTransitionAllowed(existing->getStatus(), target.value()))
    {
        ServiceRequestStatusResult result;
        result.invalidTransition = true;
        return result;
    }

    // 4. Valid transition: update only the status, then re-read the row so
    //    the returned request reflects what is actually persisted.
    serviceRequestRepository_.updateStatus(serviceRequestId, target.value());

    ServiceRequestStatusResult result;
    result.success = true;
    result.serviceRequest = serviceRequestRepository_.findById(serviceRequestId);
    return result;
}

bool ServiceRequestStatusService::isTransitionAllowed(ServiceRequestStatus current,
                                                      ServiceRequestStatus target) const
{
    switch (current)
    {
        case ServiceRequestStatus::Pending:
            return target == ServiceRequestStatus::InProgress ||
                   target == ServiceRequestStatus::Cancelled;

        case ServiceRequestStatus::InProgress:
            return target == ServiceRequestStatus::Completed ||
                   target == ServiceRequestStatus::Cancelled;

        // Completed and Cancelled are terminal: nothing may follow them.
        case ServiceRequestStatus::Completed:
        case ServiceRequestStatus::Cancelled:
        default:
            return false;
    }
}
