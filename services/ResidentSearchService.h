#pragma once

#include <string>
#include <vector>

#include "../models/Resident.h"
#include "../repositories/ResidentRepository.h"

// Decides whether a search term is "blank" (list everything) or a real
// search, then delegates the actual query work to ResidentRepository.
// This class does not touch SQL directly.
class ResidentSearchService
{
public:
    explicit ResidentSearchService(ResidentRepository &repository);

    std::vector<Resident> listResidents();
    std::vector<Resident> searchResidents(const std::string &searchTerm);

private:
    ResidentRepository &repository_;

    std::string trim(const std::string &value) const;
};