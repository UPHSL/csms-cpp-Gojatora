#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../database/Database.h"
#include "../models/ServiceRequest.h"

// Stores and retrieves Service Requests using SQLite. It only translates
// between ServiceRequest objects and rows in "service_requests". It does
// NOT validate, and it does NOT check the Resident â€” those belong to
// ServiceRequestValidator and ServiceRequestSubmissionService.
class ServiceRequestRepository
{
public:
    explicit ServiceRequestRepository(Database &database);

    // Inserts the Service Request and returns a copy that includes the
    // database-generated id.
    ServiceRequest save(const ServiceRequest &request);

    // Returns std::nullopt if no Service Request has this id.
    std::optional<ServiceRequest> findById(int serviceRequestId);

    // Returns every persisted Service Request (all statuses), ordered by
    // date_requested DESC, then id DESC. An empty table gives an empty vector.
    std::vector<ServiceRequest> findAll();

    // Database-level search/filter. searchTerm matches service_type OR
    // description as a partial, case-insensitive substring; an empty
    // searchTerm adds no text condition. statusFilter, when it has a value,
    // must also match exactly (text AND status); std::nullopt adds no status
    // condition. Same ordering as findAll(); each row appears at most once.
    // The caller is expected to have trimmed searchTerm already. Read-only.
    std::vector<ServiceRequest> search(const std::string &searchTerm,
                                       std::optional<ServiceRequestStatus> statusFilter);

    // Updates ONLY the status column of the row with this id. Returns true
    // if exactly that one row was updated, false if no row has this id (in
    // which case nothing is inserted or changed). It applies no transition
    // rules — ServiceRequestStatusService decides whether a change is allowed.
    bool updateStatus(int serviceRequestId, ServiceRequestStatus newStatus);

private:
    Database &database_;

    ServiceRequestStatus statusFromString(const std::string &value) const;

    // Shared row->ServiceRequest conversion for findById, findAll and search.
    // Expects the column order: id, resident_id, service_type, description,
    // date_requested, status.
    ServiceRequest mapRowToServiceRequest(sqlite3_stmt *statement) const;
};
