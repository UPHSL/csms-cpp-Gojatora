#include "Database.h"

#include <stdexcept>

Database::Database(const std::string &dbPath)
{
    // sqlite3_open creates the file if it doesn't exist yet.
    // It returns SQLITE_OK (0) on success.
    int result = sqlite3_open(dbPath.c_str(), &db_);

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to open SQLite database: " + std::string(sqlite3_errmsg(db_)));
    }

    createSchema();
}

Database::~Database()
{
    if (db_ != nullptr)
    {
        sqlite3_close(db_);
    }
}

sqlite3 *Database::handle() const
{
    return db_;
}

void Database::createSchema()
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS residents ("
        "    id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    first_name     TEXT    NOT NULL,"
        "    last_name      TEXT    NOT NULL,"
        "    address        TEXT    NOT NULL,"
        "    contact_number TEXT    NOT NULL,"
        "    email          TEXT    NOT NULL,"
        "    status         TEXT    NOT NULL"
        ");";

    char *errorMessage = nullptr;

    // sqlite3_exec runs a plain SQL statement directly (no parameters
    // needed here, so we don't need a prepared statement for this one).
    int result = sqlite3_exec(db_, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        std::string message = errorMessage != nullptr ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to create residents table: " + message);
    }

    // Stores only the Resident's id (resident_id), never a copy of the
    // Resident's personal information.
    const char *serviceRequestsSql =
        "CREATE TABLE IF NOT EXISTS service_requests ("
        "    id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    resident_id    INTEGER NOT NULL,"
        "    service_type   TEXT    NOT NULL,"
        "    description    TEXT    NOT NULL,"
        "    date_requested TEXT    NOT NULL,"
        "    status         TEXT    NOT NULL"
        ");";

    errorMessage = nullptr;
    result = sqlite3_exec(db_, serviceRequestsSql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        std::string message = errorMessage != nullptr ? errorMessage : "unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to create service_requests table: " + message);
    }
}