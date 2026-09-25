#pragma once

#include <optional>
#include <string>

// The lifecycle states of a Service Request. T08 only needs the starting
// state; later tickets add the rest of the workflow.
enum class ServiceRequestStatus
{
    Pending
};

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
