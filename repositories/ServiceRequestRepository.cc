#include "ServiceRequestRepository.h"

#include <stdexcept>

ServiceRequestRepository::ServiceRequestRepository(Database &database)
    : database_(database)
{
}

ServiceRequest ServiceRequestRepository::save(const ServiceRequest &request)
{
    // "?" placeholders are filled by sqlite3_bind_*, never by pasting values
    // into the SQL text. The id column is left out so SQLite generates it.
    const char *sql =
        "INSERT INTO service_requests "
        "(resident_id, service_type, description, date_requested, status) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request INSERT: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_int(statement, 1, request.getResidentId());
    sqlite3_bind_text(statement, 2, request.getServiceType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, request.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, request.getDateRequested().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, serviceRequestStatusToString(request.getStatus()).c_str(), -1, SQLITE_TRANSIENT);

    int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_DONE)
    {
        std::string errorMessage = sqlite3_errmsg(database_.handle());
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to insert Service Request: " + errorMessage);
    }

    sqlite3_finalize(statement);

    sqlite3_int64 generatedId = sqlite3_last_insert_rowid(database_.handle());

    return ServiceRequest(static_cast<int>(generatedId),
                          request.getResidentId(),
                          request.getServiceType(),
                          request.getDescription(),
                          request.getDateRequested(),
                          request.getStatus());
}

std::optional<ServiceRequest> ServiceRequestRepository::findById(int serviceRequestId)
{
    const char *sql =
        "SELECT id, resident_id, service_type, description, date_requested, status "
        "FROM service_requests WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request SELECT: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_int(statement, 1, serviceRequestId);

    if (sqlite3_step(statement) != SQLITE_ROW)
    {
        // Not found is a normal outcome, not an error.
        sqlite3_finalize(statement);
        return std::nullopt;
    }

    ServiceRequest request = mapRowToServiceRequest(statement);

    sqlite3_finalize(statement);

    return request;
}

std::vector<ServiceRequest> ServiceRequestRepository::findAll()
{
    return search("", std::nullopt);
}

std::vector<ServiceRequest> ServiceRequestRepository::search(const std::string &searchTerm,
                                                             std::optional<ServiceRequestStatus> statusFilter,
                                                             std::optional<int> limit)
{
    // The SQL text is assembled only from fixed fragments chosen by whether
    // a condition applies; the search term and status are never pasted into
    // it, they are bound to "?" placeholders below.
    std::string sql =
        "SELECT id, resident_id, service_type, description, date_requested, status "
        "FROM service_requests WHERE 1 = 1";

    bool hasText = !searchTerm.empty();

    if (hasText)
    {
        // One condition covering both columns, so a row that matches in both
        // is still one row. SQLite's LIKE is case-insensitive for ASCII.
        // ESCAPE makes %, _ and \ in the user's term match literally.
        sql += " AND (service_type LIKE ? ESCAPE '\\' OR description LIKE ? ESCAPE '\\')";
    }

    if (statusFilter.has_value())
    {
        sql += " AND status = ?";
    }

    sql += " ORDER BY date_requested DESC, id DESC";

    if (limit.has_value())
    {
        sql += " LIMIT ?";
    }

    sql += ";";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql.c_str(), -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request search: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    int parameterIndex = 1;

    if (hasText)
    {
        std::string escaped;
        for (char c : searchTerm)
        {
            if (c == '\\' || c == '%' || c == '_')
            {
                escaped += '\\';
            }
            escaped += c;
        }

        std::string pattern = "%" + escaped + "%";
        sqlite3_bind_text(statement, parameterIndex++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, parameterIndex++, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }

    if (statusFilter.has_value())
    {
        sqlite3_bind_text(statement, parameterIndex++,
                          serviceRequestStatusToString(*statusFilter).c_str(), -1, SQLITE_TRANSIENT);
    }

    if (limit.has_value())
    {
        sqlite3_bind_int(statement, parameterIndex++, *limit);
    }

    std::vector<ServiceRequest> requests;
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        requests.push_back(mapRowToServiceRequest(statement));
    }

    sqlite3_finalize(statement);
    return requests;
}

std::vector<ServiceRequest> ServiceRequestRepository::findRecent(int limit)
{
    // A negative LIMIT means "no limit" in SQLite, so guard it here.
    if (limit <= 0)
    {
        return {};
    }

    // Same query and ordering as listing (T11), just capped.
    return search("", std::nullopt, limit);
}

int ServiceRequestRepository::readCount(sqlite3_stmt *statement, const char *what) const
{
    // COUNT(*) always yields exactly one row, even over an empty table.
    if (sqlite3_step(statement) != SQLITE_ROW)
    {
        std::string errorMessage = sqlite3_errmsg(database_.handle());
        sqlite3_finalize(statement);
        throw std::runtime_error(std::string("Failed to count ") + what + ": " + errorMessage);
    }

    int count = sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    return count;
}

int ServiceRequestRepository::countAll()
{
    const char *sql = "SELECT COUNT(*) FROM service_requests;";

    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request count: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    return readCount(statement, "Service Requests");
}

int ServiceRequestRepository::countByStatus(ServiceRequestStatus status)
{
    const char *sql = "SELECT COUNT(*) FROM service_requests WHERE status = ?;";

    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request status count: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, serviceRequestStatusToString(status).c_str(), -1, SQLITE_TRANSIENT);

    return readCount(statement, "Service Requests by status");
}

bool ServiceRequestRepository::updateStatus(int serviceRequestId, ServiceRequestStatus newStatus)
{
    // Only the status column is touched, and only the row with this id.
    const char *sql = "UPDATE service_requests SET status = ? WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;
    int prepareResult = sqlite3_prepare_v2(database_.handle(), sql, -1, &statement, nullptr);

    if (prepareResult != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare service request status UPDATE: " +
                                  std::string(sqlite3_errmsg(database_.handle())));
    }

    sqlite3_bind_text(statement, 1, serviceRequestStatusToString(newStatus).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, serviceRequestId);

    int stepResult = sqlite3_step(statement);

    if (stepResult != SQLITE_DONE)
    {
        std::string errorMessage = sqlite3_errmsg(database_.handle());
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to update Service Request status: " + errorMessage);
    }

    sqlite3_finalize(statement);

    // sqlite3_changes() counts the rows the UPDATE matched; 0 means the id
    // does not exist.
    return sqlite3_changes(database_.handle()) == 1;
}

ServiceRequestStatus ServiceRequestRepository::statusFromString(const std::string &value) const
{
    if (value == "In Progress")
    {
        return ServiceRequestStatus::InProgress;
    }

    if (value == "Completed")
    {
        return ServiceRequestStatus::Completed;
    }

    if (value == "Cancelled")
    {
        return ServiceRequestStatus::Cancelled;
    }

    return ServiceRequestStatus::Pending;
}


ServiceRequest ServiceRequestRepository::mapRowToServiceRequest(sqlite3_stmt *statement) const
{
    int id = sqlite3_column_int(statement, 0);
    int residentId = sqlite3_column_int(statement, 1);
    std::string serviceType = reinterpret_cast<const char *>(sqlite3_column_text(statement, 2));
    std::string description = reinterpret_cast<const char *>(sqlite3_column_text(statement, 3));
    std::string dateRequested = reinterpret_cast<const char *>(sqlite3_column_text(statement, 4));
    std::string statusText = reinterpret_cast<const char *>(sqlite3_column_text(statement, 5));

    return ServiceRequest(id, residentId, serviceType, description, dateRequested,
                          statusFromString(statusText));
}
