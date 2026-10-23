#include "ResidentUpdateService.h"

ResidentUpdateService::ResidentUpdateService(const ResidentValidator &validator,
                                              ResidentRepository &repository)
    : validator_(validator),
      repository_(repository)
{
}

ResidentUpdateResult ResidentUpdateService::updateResident(int residentId,
                                                             const std::string &firstName,
                                                             const std::string &lastName,
                                                             const std::string &address,
                                                             const std::string &contactNumber,
                                                             const std::string &email)
{
    std::optional<Resident> existing = repository_.findById(residentId);

    if (!existing.has_value())
    {
        ResidentUpdateResult result;
        result.residentNotFound = true;
        return result;
    }

    // Build the proposed updated state: new editable fields, but the
    // EXISTING id and EXISTING status — these are never taken from the
    // caller's input, so this operation cannot change them even if it
    // wanted to.
    Resident candidate(existing->getId().value(),
                        firstName,
                        lastName,
                        address,
                        contactNumber,
                        email,
                        existing->getStatus());

    std::vector<std::string> errors = validator_.validate(candidate);

    if (!errors.empty())
    {
        ResidentUpdateResult result;
        result.success = false;
        result.errors = errors;
        return result;
    }

    bool updated = repository_.update(candidate);

    // This should always be true here, since we just confirmed the
    // Resident exists moments ago — but we check defensively anyway.
    if (!updated)
    {
        ResidentUpdateResult result;
        result.residentNotFound = true;
        return result;
    }

    std::optional<Resident> refreshed = repository_.findById(residentId);

    ResidentUpdateResult result;
    result.success = true;
    result.resident = refreshed;
    return result;
}