#include "ServiceRequestValidator.h"

#include <algorithm>
#include <cctype>

std::vector<std::string> ServiceRequestValidator::validate(const ServiceRequest &request) const
{
    std::vector<std::string> errors;

    // A new submission must not already carry a persisted id.
    if (request.getId().has_value())
    {
        errors.push_back("id");
    }

    // Structural check only: a positive number. Whether that Resident
    // actually exists is checked later by the submission service.
    if (request.getResidentId() <= 0)
    {
        errors.push_back("residentId");
    }

    if (isBlank(request.getServiceType()))
    {
        errors.push_back("serviceType");
    }

    if (isBlank(request.getDescription()))
    {
        errors.push_back("description");
    }

    if (!isValidDate(request.getDateRequested()))
    {
        errors.push_back("dateRequested");
    }

    // A new request may only begin as Pending.
    if (request.getStatus() != ServiceRequestStatus::Pending)
    {
        errors.push_back("status");
    }

    return errors;
}

bool ServiceRequestValidator::isValid(const ServiceRequest &request) const
{
    return validate(request).empty();
}

bool ServiceRequestValidator::isBlank(const std::string &value) const
{
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
}

// Accepts only a real calendar date written exactly as YYYY-MM-DD,
// e.g. "2026-09-15". "2026-02-30" and "2026-13-01" are rejected.
// No "today"/"future" rules — only that the date exists.
bool ServiceRequestValidator::isValidDate(const std::string &value) const
{
    if (value.size() != 10 || value[4] != '-' || value[7] != '-')
    {
        return false;
    }

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        if (i == 4 || i == 7)
        {
            continue;
        }

        if (!std::isdigit(static_cast<unsigned char>(value[i])))
        {
            return false;
        }
    }

    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));

    if (year < 1 || month < 1 || month > 12 || day < 1)
    {
        return false;
    }

    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[month - 1];

    const bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (month == 2 && isLeapYear)
    {
        maxDay = 29;
    }

    return day <= maxDay;
}
