#include "ServiceRequestQueryService.h"

ServiceRequestQueryService::ServiceRequestQueryService(ServiceRequestRepository &serviceRequestRepository)
    : serviceRequestRepository_(serviceRequestRepository)
{
}

std::vector<ServiceRequest> ServiceRequestQueryService::listServiceRequests()
{
    return serviceRequestRepository_.findAll();
}

std::optional<ServiceRequest> ServiceRequestQueryService::viewServiceRequest(int serviceRequestId)
{
    // Reuses the T09 lookup; not found is std::nullopt, never an error.
    return serviceRequestRepository_.findById(serviceRequestId);
}

ServiceRequestQueryResult ServiceRequestQueryService::searchServiceRequests(const std::string &searchTerm,
                                                                           const std::string &statusFilter)
{
    ServiceRequestQueryResult result;

    // A blank filter means "all statuses". Anything else must be exactly one
    // of the four supported statuses; it is never silently ignored.
    std::optional<ServiceRequestStatus> status = std::nullopt;
    std::string trimmedStatus = trim(statusFilter);

    if (!trimmedStatus.empty())
    {
        status = serviceRequestStatusFromString(trimmedStatus);

        if (!status.has_value())
        {
            result.unsupportedStatus = true;
            return result;
        }
    }

    // A blank term adds no text condition, so it behaves like listing (with
    // the optional status filter still applied).
    result.serviceRequests = serviceRequestRepository_.search(trim(searchTerm), status);
    result.success = true;
    return result;
}

std::string ServiceRequestQueryService::trim(const std::string &value) const
{
    std::size_t start = value.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
    {
        return ""; // entirely whitespace
    }

    std::size_t end = value.find_last_not_of(" \t\n\r\f\v");
    return value.substr(start, end - start + 1);
}
