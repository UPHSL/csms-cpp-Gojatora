#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/Resident.h"

// Represents the outcome of one registration attempt.
// Successful: success=true, resident has a value, errors is empty.
// Failed (validation): success=false, resident is std::nullopt,
// errors contains the names of the fields that failed validation.
struct ResidentRegistrationResult
{
    bool success{false};
    std::optional<Resident> resident{std::nullopt};
    std::vector<std::string> errors;
};