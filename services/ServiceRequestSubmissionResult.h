#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/ServiceRequest.h"

// Outcome of one submission attempt. Four distinguishable outcomes:
//   success:            serviceRequest has a value (with its generated id)
//   validation failure: errors non-empty (names of the failing fields)
//   Resident not found: residentNotFound true
//   Resident Inactive:  residentInactive true (Resident exists, not eligible)
struct ServiceRequestSubmissionResult
{
    bool success{false};
    bool residentNotFound{false};
    bool residentInactive{false};
    std::optional<ServiceRequest> serviceRequest{std::nullopt};
    std::vector<std::string> errors;
};
