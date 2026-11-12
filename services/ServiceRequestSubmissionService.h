#pragma once

#include "../models/ServiceRequest.h"
#include "../models/ServiceRequestValidator.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "ServiceRequestSubmissionResult.h"

// Coordinates submitting a NEW Service Request: validate it, verify the
// Resident exists and is Active, and only then persist it. It contains no
// SQL, no validation rules, and never generates an id itself.
class ServiceRequestSubmissionService
{
public:
    ServiceRequestSubmissionService(const ServiceRequestValidator &validator,
                                     ResidentRepository &residentRepository,
                                     ServiceRequestRepository &serviceRequestRepository);

    ServiceRequestSubmissionResult submitServiceRequest(const ServiceRequest &request);

private:
    const ServiceRequestValidator &validator_;
    ResidentRepository &residentRepository_;
    ServiceRequestRepository &serviceRequestRepository_;
};
