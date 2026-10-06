#pragma once

#include <string>

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"
#include "ResidentUpdateResult.h"

// Coordinates updating an existing Resident's editable information:
// looks it up, preserves id/status, validates the proposed change via
// the existing T02 validator, then persists via the existing T03
// repository. Does not reimplement validation or SQL itself.
class ResidentUpdateService
{
public:
    ResidentUpdateService(const ResidentValidator &validator,
                           ResidentRepository &repository);

    ResidentUpdateResult updateResident(int residentId,
                                         const std::string &firstName,
                                         const std::string &lastName,
                                         const std::string &address,
                                         const std::string &contactNumber,
                                         const std::string &email);

private:
    const ResidentValidator &validator_;
    ResidentRepository &repository_;
};