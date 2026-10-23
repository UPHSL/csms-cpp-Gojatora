#pragma once

#include <memory>
#include <string>

#include "../database/Database.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"
#include "ResidentDeactivationService.h"
#include "ResidentRegistrationService.h"
#include "ResidentSearchService.h"
#include "ResidentUpdateService.h"

// Wires the existing Resident validator, repository, and application
// services together once, so the presentation layer (which Drogon
// constructs itself and therefore cannot receive constructor arguments)
// can reach them. It adds no behavior of its own.
class ResidentServices
{
public:
    // The shared instance. On first use it opens the database named by the
    // CSMS_DB_PATH environment variable, or "csms.sqlite3" if unset.
    static ResidentServices &instance();

    // Replaces the shared instance with one backed by the given database
    // file. Used by tests so the web layer talks to a throwaway database.
    static void useDatabase(const std::string &dbPath);

    ResidentRepository &repository() { return repository_; }
    ResidentRegistrationService &registration() { return registration_; }
    ResidentSearchService &search() { return search_; }
    ResidentUpdateService &update() { return update_; }
    ResidentDeactivationService &deactivation() { return deactivation_; }

private:
    explicit ResidentServices(const std::string &dbPath);

    Database database_;
    ResidentValidator validator_;
    ResidentRepository repository_;
    ResidentRegistrationService registration_;
    ResidentSearchService search_;
    ResidentUpdateService update_;
    ResidentDeactivationService deactivation_;
};
