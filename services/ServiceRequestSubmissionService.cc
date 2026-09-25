#include "ServiceRequestSubmissionService.h"

ServiceRequestSubmissionService::ServiceRequestSubmissionService(
    const ServiceRequestValidator &validator,
    ResidentRepository &residentRepository,
    ServiceRequestRepository &serviceRequestRepository)
    : validator_(validator),
      residentRepository_(residentRepository),
      serviceRequestRepository_(serviceRequestRepository)
{
}

ServiceRequestSubmissionResult ServiceRequestSubmissionService::submitServiceRequest(
    const ServiceRequest &request)
{
    // 1. Validate the request's own information. Stop before touching
    //    either repository if it is invalid.
    std::vector<std::string> errors = validator_.validate(request);

    if (!errors.empty())
    {
        ServiceRequestSubmissionResult result;
        result.errors = errors;
        return result;
    }

    // 2. The Resident must exist (existing T03 repository).
    std::optional<Resident> resident = residentRepository_.findById(request.getResidentId());

    if (!resident.has_value())
    {
        ServiceRequestSubmissionResult result;
        result.residentNotFound = true;
        return result;
    }

    // 3. The Resident must be Active. Inactive Residents stay stored and
    //    searchable, but cannot create new transactions.
    if (resident->getStatus() != ResidentStatus::Active)
    {
        ServiceRequestSubmissionResult result;
        result.residentInactive = true;
        return result;
    }

    // 4. Everything passed — persist. The repository generates the id.
    ServiceRequest persisted = serviceRequestRepository_.save(request);

    ServiceRequestSubmissionResult result;
    result.success = true;
    result.serviceRequest = persisted;
    return result;
}
