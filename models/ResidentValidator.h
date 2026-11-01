#pragma once

#include <string>
#include <vector>

#include "Resident.h"

// Checks whether Resident information satisfies the T02 validation rules.
// Does not modify Resident data — only inspects and reports on it.
class ResidentValidator
{
public:
    // Returns the list of field names that failed validation.
    // Empty list = fully valid.
    std::vector<std::string> validate(const Resident &resident) const;

    // True if the Resident has no validation errors.
    bool isValid(const Resident &resident) const;

private:
    bool isBlank(const std::string &value) const;           // empty or whitespace-only?
    bool isValidContactNumber(const std::string &value) const; // matches 09XXXXXXXXX?
    bool isValidEmail(const std::string &value) const;       // has basic email shape?
    bool isSupportedStatus(ResidentStatus status) const;      // Active or Inactive?
};