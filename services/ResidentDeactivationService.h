#pragma once

#include "../repositories/ResidentRepository.h"
#include "ResidentDeactivationResult.h"

// Coordinates soft deactivation of an existing Resident (Active -> Inactive).
// Looks the Resident up, handles not-found and already-Inactive, and asks the
// repository to change only the status. It takes no validator because it
// never touches personal information, and it never deletes or creates rows.
class ResidentDeactivationService
{
public:
    explicit ResidentDeactivationService(ResidentRepository &repository);

    ResidentDeactivationResult deactivateResident(int residentId);

private:
    ResidentRepository &repository_;
};
