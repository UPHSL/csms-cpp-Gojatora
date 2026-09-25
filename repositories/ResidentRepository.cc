#include "ResidentRepository.h"

#include <stdexcept>

ResidentRepository::ResidentRepository(Database &database)
    : database_(database)
{
}

Resident ResidentRepository::save(const Resident &resident)
{
    const char *sql =
        "INSERT INTO residents "
        "(first_name, last_name, address, contact_number, email, status) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *statement = nullptr;

    // sqlite3_prepare_v2 compiles the SQL text into a reusable "prepared
    // statement" object. The "?" placeholders will be filled in safely
    // using sqlite3_bind_*, instead of pasting Resident values directly
    // into the SQL string (which would risk SQL injection).
    int prepareResult = sqlite3_prepare_v2(
        database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to prepare INSERT statement: " +
            std::string(sqlite3_errmsg(database_.handle())));
    }

    // Bind each Resident field to its corresponding "?" placeholder.
    // Placeholder positions are 1-indexed, in the order they appear in the SQL.
    // SQLITE_TRANSIENT tells SQLite to make its own internal copy of the
    // string, since our std::string may be destroyed before SQLite is done.
    sqlite3_bind_text(statement, 1, resident.getFirstName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, resident.getLastName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, resident.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, resident.getContactNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, resident.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 6, residentStatusToString(resident.getStatus()).c_str(), -1, SQLITE_TRANSIENT);

    // sqlite3_step actually executes the statement. For an INSERT,
    // a successful execution returns SQLITE_DONE (not SQLITE_OK).
    int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_DONE)
    {
        std::string errorMessage = sqlite3_errmsg(database_.handle());
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to insert Resident: " + errorMessage);
    }

    // We're done with this prepared statement now — release its resources.
    sqlite3_finalize(statement);

    // sqlite3_last_insert_rowid returns the id SQLite just generated
    // for the row we inserted, from this same database connection.
    sqlite3_int64 generatedId = sqlite3_last_insert_rowid(database_.handle());

    // Build and return a new Resident that now includes the real,
    // database-assigned id — using the constructor that accepts an id.
    return Resident(static_cast<int>(generatedId),
                     resident.getFirstName(),
                     resident.getLastName(),
                     resident.getAddress(),
                     resident.getContactNumber(),
                     resident.getEmail(),
                     resident.getStatus());
}

std::optional<Resident> ResidentRepository::findById(int residentId)
{
    const char *sql =
        "SELECT id, first_name, last_name, address, contact_number, email, status "
        "FROM residents WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;

    int prepareResult = sqlite3_prepare_v2(
        database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to prepare SELECT statement: " +
            std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_int(statement, 1, residentId);

    // For a SELECT, sqlite3_step returns SQLITE_ROW if a matching row
    // was found, or SQLITE_DONE if there were no results at all.
    int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_ROW)
    {
        // No matching Resident — this is a normal, expected outcome,
        // not an error. Safely handle "not found" with std::nullopt.
        sqlite3_finalize(statement);
        return std::nullopt;
    }

    // Read each column out of the row, in the same order as the SELECT list.
    // Column indices are 0-indexed here (unlike bind, which is 1-indexed).
    int id = sqlite3_column_int(statement, 0);
    std::string firstName = reinterpret_cast<const char *>(sqlite3_column_text(statement, 1));
    std::string lastName = reinterpret_cast<const char *>(sqlite3_column_text(statement, 2));
    std::string address = reinterpret_cast<const char *>(sqlite3_column_text(statement, 3));
    std::string contactNumber = reinterpret_cast<const char *>(sqlite3_column_text(statement, 4));
    std::string email = reinterpret_cast<const char *>(sqlite3_column_text(statement, 5));
    std::string statusText = reinterpret_cast<const char *>(sqlite3_column_text(statement, 6));

    sqlite3_finalize(statement);

    Resident resident(id,
                       firstName,
                       lastName,
                       address,
                       contactNumber,
                       email,
                       statusFromString(statusText));

    return resident;
}

ResidentStatus ResidentRepository::statusFromString(const std::string &value) const
{
    if (value == "Active")
    {
        return ResidentStatus::Active;
    }

    if (value == "Inactive")
    {
        return ResidentStatus::Inactive;
    }

    // Fallback for unexpected data — shouldn't normally happen since
    // we control what gets written via residentStatusToString().
    return ResidentStatus::Active;
}

// Shared row->Resident conversion, used by findById, findAll, and searchByName.
Resident ResidentRepository::mapRowToResident(sqlite3_stmt *statement) const
{
    int id = sqlite3_column_int(statement, 0);
    std::string firstName = reinterpret_cast<const char *>(sqlite3_column_text(statement, 1));
    std::string lastName = reinterpret_cast<const char *>(sqlite3_column_text(statement, 2));
    std::string address = reinterpret_cast<const char *>(sqlite3_column_text(statement, 3));
    std::string contactNumber = reinterpret_cast<const char *>(sqlite3_column_text(statement, 4));
    std::string email = reinterpret_cast<const char *>(sqlite3_column_text(statement, 5));
    std::string statusText = reinterpret_cast<const char *>(sqlite3_column_text(statement, 6));

    return Resident(id, firstName, lastName, address, contactNumber, email, statusFromString(statusText));
}

// Returns every Resident, ordered lastName -> firstName -> id (all ascending).
// COLLATE NOCASE makes the ordering itself case-insensitive too.
std::vector<Resident> ResidentRepository::findAll()
{
    const char *sql =
        "SELECT id, first_name, last_name, address, contact_number, email, status "
        "FROM residents "
        "ORDER BY last_name COLLATE NOCASE ASC, first_name COLLATE NOCASE ASC, id ASC;";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare findAll statement: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    std::vector<Resident> residents;
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        residents.push_back(mapRowToResident(statement));
    }

    sqlite3_finalize(statement);
    return residents;
}

// Case-insensitive partial match against firstName OR lastName.
// SQLite's LIKE is case-insensitive by default for ASCII text, and
// (a OR b) on a single row can only ever produce that row once, so a
// Resident matching on both first and last name still appears only once.
std::vector<Resident> ResidentRepository::searchByName(const std::string &searchTerm)
{
    const char *sql =
        "SELECT id, first_name, last_name, address, contact_number, email, status "
        "FROM residents "
        "WHERE first_name LIKE ? OR last_name LIKE ? "
        "ORDER BY last_name COLLATE NOCASE ASC, first_name COLLATE NOCASE ASC, id ASC;";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare search statement: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    // Wrap the term in % wildcards for a partial/substring match, and bind
    // it (never concatenate it into the SQL string directly).
    std::string pattern = "%" + searchTerm + "%";
    sqlite3_bind_text(statement, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<Resident> residents;
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        residents.push_back(mapRowToResident(statement));
    }

    sqlite3_finalize(statement);
    return residents;
}

bool ResidentRepository::update(const Resident &resident)
{
    const char *sql =
        "UPDATE residents "
        "SET first_name = ?, last_name = ?, address = ?, contact_number = ?, email = ? "
        "WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare UPDATE statement: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, resident.getFirstName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, resident.getLastName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, resident.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, resident.getContactNumber().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, resident.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 6, resident.getId().value());

    int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_DONE)
    {
        std::string errorMessage = sqlite3_errmsg(database_.handle());
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to update Resident: " + errorMessage);
    }

    sqlite3_finalize(statement);

    // sqlite3_changes() reports how many rows the most recent statement
    // actually modified. If the id didn't match any row, this is 0 —
    // that's how we detect "Resident not found" without a separate query.
    return sqlite3_changes(database_.handle()) > 0;
}