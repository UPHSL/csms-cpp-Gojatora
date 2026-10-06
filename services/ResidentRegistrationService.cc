#include "ResidentRegistrationService.h"

ResidentRegistrationService::ResidentRegistrationService(
    const ResidentValidator &validator,
    ResidentRepository &repository)
    : validator_(validator),
      repository_(repository)
{
}

ResidentRegistrationResult ResidentRegistrationService::registerResident(
    const Resident &resident)
{
    const std::vector<std::string> errors = validator_.validate(resident);

    if (!errors.empty())
    {
        // Validation failed — stop here. repository_.save() is never
        // called, so invalid Resident information is never persisted.
        return ResidentRegistrationResult{false, std::nullopt, errors};
    }

    // Valid — hand off to the existing T03 repository. It generates the
    // id via SQLite; the service never creates the id itself.
    Resident persistedResident = repository_.save(resident);

    return ResidentRegistrationResult{true, persistedResident, {}};
}