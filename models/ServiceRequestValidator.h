#pragma once

#include <string>
#include <vector>

#include "ServiceRequest.h"

// Checks whether a NEW Service Request's own information is acceptable.
// It never touches the database, so it cannot tell whether the Resident
// exists or is Active — that is the submission service's job.
class ServiceRequestValidator
{
public:
    // Returns the names of the fields that failed validation
    // ("id", "residentId", "serviceType", "description", "dateRequested",
    // "status"). Empty list = valid.
    std::vector<std::string> validate(const ServiceRequest &request) const;

    bool isValid(const ServiceRequest &request) const;

private:
    bool isBlank(const std::string &value) const;
    bool isValidDate(const std::string &value) const;   // real YYYY-MM-DD date?
};
