#pragma once

#include <string>
#include <sqlite3.h>

// Responsible for opening a connection to a SQLite database file and
// ensuring the residents table exists. This class does NOT know anything
// about Resident objects — it only manages the raw database connection
// and schema. ResidentRepository (built next) will use this connection
// to actually store/retrieve Residents.
class Database
{
public:
    // Opens (or creates) the SQLite database file at the given path,
    // and ensures the residents table exists.
    explicit Database(const std::string &dbPath);

    // Closes the database connection automatically when this object
    // is destroyed (RAII — no need to manually remember to close it).
    ~Database();

    // Prevent copying: two Database objects should never share the
    // same raw sqlite3* connection pointer.
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    // Gives access to the raw SQLite connection handle, so
    // ResidentRepository can run queries against it.
    sqlite3 *handle() const;

private:
    sqlite3 *db_ = nullptr;

    void createSchema();
};