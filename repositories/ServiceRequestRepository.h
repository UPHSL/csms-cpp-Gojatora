#pragma once

#include <optional>
#include <string>

#include "../database/Database.h"
#include "../models/ServiceRequest.h"

// Stores and retrieves Service Requests using SQLite. It only translates
// between ServiceRequest objects and rows in "service_requests". It does
// NOT validate, and it does NOT check the Resident — those belong to
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

private:
    Database &database_;

    ServiceRequestStatus statusFromString(const std::string &value) const;
};
