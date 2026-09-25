#pragma once

#include <optional>
#include <vector>

#include "../database/Database.h"
#include "../models/Resident.h"

// Responsible for storing and retrieving Resident records using SQLite.
// This class knows how to translate between Resident objects (C++) and
// rows in the "residents" table (SQL). It does NOT validate Residents —
// that responsibility belongs to ResidentValidator (T02).
class ResidentRepository
{
public:
    // The repository needs a reference to an already-open Database
    // connection. It does not own or manage the connection itself.
    explicit ResidentRepository(Database &database);

    // Stores a valid Resident and returns a new Resident object that
    // includes the database-generated identifier.
    Resident save(const Resident &resident);

    // Looks up a Resident by its database-assigned id.
    // Returns std::nullopt if no matching Resident exists.
    std::optional<Resident> findById(int residentId);

    std::vector<Resident> findAll();
    std::vector<Resident> searchByName(const std::string &searchTerm);

    // Returns true if a Resident with this id existed and was updated.
    // Returns false if no Resident with this id exists (nothing changed).
    // Does NOT modify id or status — only the 5 editable fields.
    bool update(const Resident &resident);

private:
    Database &database_;

    // Converts a ResidentStatus string ("Active"/"Inactive") back into
    // the ResidentStatus enum used by our Resident model.
    ResidentStatus statusFromString(const std::string &value) const;

    Resident mapRowToResident(sqlite3_stmt *statement) const;
};


