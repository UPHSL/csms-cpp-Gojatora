#pragma once

#include <optional>
#include <string>

// The lifecycle states of a Service Request. These are only names so a
// status can be represented and stored; the rules for moving between them
// (transitions) belong to a later ticket. A NEW request must be Pending.
enum class ServiceRequestStatus
{
    Pending,
    InProgress,
    Completed,
    Cancelled
};

// Converts a ServiceRequestStatus into the text stored in the database.
std::string serviceRequestStatusToString(ServiceRequestStatus status);

// Represents Service Request information only. It does not validate,
// persist, submit, or check that the Resident exists (later tickets).
class ServiceRequest
{
public:
    // Creates a new, unsaved Service Request. The id is unassigned
    // (std::nullopt) until persistence gives it one. The status defaults
    // to Pending, so callers don't have to pass it.
    // dateRequested is stored as supplied text (e.g. "2026-09-25").
    ServiceRequest(int residentId,
                   const std::string &serviceType,
                   const std::string &description,
                   const std::string &dateRequested,
                   ServiceRequestStatus status = ServiceRequestStatus::Pending);

    // Used when the id is already known (e.g. rebuilding a Service Request
    // that was read from the database).
    ServiceRequest(int id,
                   int residentId,
                   const std::string &serviceType,
                   const std::string &description,
                   const std::string &dateRequested,
                   ServiceRequestStatus status);

    std::optional<int> getId() const;
    int getResidentId() const;
    std::string getServiceType() const;
    std::string getDescription() const;
    std::string getDateRequested() const;
    ServiceRequestStatus getStatus() const;

private:
    std::optional<int> id_;
    int residentId_;
    std::string serviceType_;
    std::string description_;
    std::string dateRequested_;
    ServiceRequestStatus status_;
};
