#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../models/Resident.h"

// Represents the outcome of one update attempt. Three distinguishable
// outcomes, unlike registration's two:
//   success:            resident has a value, errors empty, residentNotFound false
//   validation failure: resident empty, errors non-empty, residentNotFound false
//   not found:          resident empty, errors empty, residentNotFound true
struct ResidentUpdateResult
{
    bool success{false};
    bool residentNotFound{false};
    std::optional<Resident> resident{std::nullopt};
    std::vector<std::string> errors;
};