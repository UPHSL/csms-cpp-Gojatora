#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/ServiceRequest.h"
#include "../repositories/ServiceRequestRepository.h"
#include "ServiceRequestQueryResult.h"

// The read side of Service Requests: list, view one, and search/filter. It
// normalizes the search input, validates the status filter, and delegates the
// actual querying to ServiceRequestRepository. It contains no SQL, never
// writes, and has no knowledge of status transitions (that is
// ServiceRequestStatusService).
class ServiceRequestQueryService
{
public:
    explicit ServiceRequestQueryService(ServiceRequestRepository &serviceRequestRepository);

    // All Service Requests, any status, ordered date_requested DESC then
    // id DESC. Empty vector when there are none.
    std::vector<ServiceRequest> listServiceRequests();

    // The persisted Service Request with this id, or std::nullopt when no
    // such request exists (nothing is created).
    std::optional<ServiceRequest> viewServiceRequest(int serviceRequestId);

    // searchTerm is matched (partial, case-insensitive) against serviceType
    // OR description after leading/trailing whitespace is removed; a blank
    // term means no text condition. statusFilter is text so that unsupported
    // values (e.g. "Approved") can be received and rejected safely; a blank
    // filter means every status. Both given means text AND status.
    ServiceRequestQueryResult searchServiceRequests(const std::string &searchTerm = "",
                                                    const std::string &statusFilter = "");

private:
    ServiceRequestRepository &serviceRequestRepository_;

    std::string trim(const std::string &value) const;
};
