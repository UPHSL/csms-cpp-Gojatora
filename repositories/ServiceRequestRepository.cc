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
                                                             std::optional<ServiceRequestStatus> statusFilter)
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

    sql += " ORDER BY date_requested DESC, id DESC;";

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

    std::vector<ServiceRequest> requests;
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        requests.push_back(mapRowToServiceRequest(statement));
    }

    sqlite3_finalize(statement);
    return requests;
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
