#include "ResidentDeactivationService.h"

ResidentDeactivationService::ResidentDeactivationService(ResidentRepository &repository)
    : repository_(repository)
{
}

ResidentDeactivationResult ResidentDeactivationService::deactivateResident(int residentId)
{
    std::optional<Resident> existing = repository_.findById(residentId);

    if (!existing.has_value())
    {
        ResidentDeactivationResult result;
        result.residentNotFound = true;
        return result;
    }

    // Already Inactive: nothing to change. This is a safe, normal outcome
    // (idempotent), so we do not touch persistence at all.
    if (existing->getStatus() == ResidentStatus::Inactive)
    {
        ResidentDeactivationResult result;
        result.success = true;
        result.stateChanged = false;
        result.resident = existing;
        return result;
    }

    bool deactivated = repository_.deactivateById(residentId);

    // Should always be true since we just found the Resident; checked defensively.
    if (!deactivated)
    {
        ResidentDeactivationResult result;
        result.residentNotFound = true;
        return result;
    }

    ResidentDeactivationResult result;
    result.success = true;
    result.stateChanged = true;
    result.resident = repository_.findById(residentId);
    return result;
}
