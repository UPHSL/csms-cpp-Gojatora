#pragma once

#include <optional>
#include <string>

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

    // Updates ONLY the status column of the row with this id. Returns true
    // if exactly that one row was updated, false if no row has this id (in
    // which case nothing is inserted or changed). It applies no transition
    // rules — ServiceRequestStatusService decides whether a change is allowed.
    bool updateStatus(int serviceRequestId, ServiceRequestStatus newStatus);

private:
    Database &database_;

    ServiceRequestStatus statusFromString(const std::string &value) const;
};
