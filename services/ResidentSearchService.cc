#include "ResidentSearchService.h"

ResidentSearchService::ResidentSearchService(ResidentRepository &repository)
    : repository_(repository)
{
}

std::vector<Resident> ResidentSearchService::listResidents()
{
    return repository_.findAll();
}

std::vector<Resident> ResidentSearchService::searchResidents(const std::string &searchTerm)
{
    std::string trimmed = trim(searchTerm);

    if (trimmed.empty())
    {
        // Blank (or whitespace-only) search behaves as "list everything".
        return repository_.findAll();
    }

    return repository_.searchByName(trimmed);
}

std::string ResidentSearchService::trim(const std::string &value) const
{
    std::size_t start = value.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
    {
        return ""; // entirely whitespace
    }

    std::size_t end = value.find_last_not_of(" \t\n\r\f\v");
    return value.substr(start, end - start + 1);
}