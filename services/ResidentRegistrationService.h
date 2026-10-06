#pragma once

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"
#include "ResidentRegistrationResult.h"

// Coordinates Resident validation (T02) and persistence (T03) into a
// single registration operation. Does not reimplement either
// responsibility — it only calls into the existing validator and
// repository.
class ResidentRegistrationService
{
public:
    ResidentRegistrationService(const ResidentValidator &validator,
                                 ResidentRepository &repository);

    ResidentRegistrationResult registerResident(const Resident &resident);

private:
    const ResidentValidator &validator_;
    ResidentRepository &repository_;
};