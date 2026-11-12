#pragma once

#include <optional>

#include "../models/Resident.h"

// Outcome of one deactivation attempt. Three distinguishable outcomes:
//   newly deactivated: success, stateChanged true,  resident has a value
//   already Inactive:  success, stateChanged false, resident has a value
//   not found:         !success, residentNotFound true, resident empty
struct ResidentDeactivationResult
{
    bool success{false};
    bool residentNotFound{false};
    bool stateChanged{false};
    std::optional<Resident> resident{std::nullopt};
};
