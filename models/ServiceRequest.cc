#include "ServiceRequest.h"

std::string serviceRequestStatusToString(ServiceRequestStatus status)
{
    switch (status)
    {
        case ServiceRequestStatus::Pending:
            return "Pending";
        case ServiceRequestStatus::InProgress:
            return "In Progress";
        case ServiceRequestStatus::Completed:
            return "Completed";
        case ServiceRequestStatus::Cancelled:
            return "Cancelled";
        default:
            return "Unknown";
    }
}

ServiceRequest::ServiceRequest(int id,
                               int residentId,
                               const std::string &serviceType,
                               const std::string &description,
                               const std::string &dateRequested,
                               ServiceRequestStatus status)
    : id_(id),
      residentId_(residentId),
      serviceType_(serviceType),
      description_(description),
      dateRequested_(dateRequested),
      status_(status)
{
}

ServiceRequest::ServiceRequest(int residentId,
                               const std::string &serviceType,
                               const std::string &description,
                               const std::string &dateRequested,
                               ServiceRequestStatus status)
    : id_(std::nullopt),
      residentId_(residentId),
      serviceType_(serviceType),
      description_(description),
      dateRequested_(dateRequested),
      status_(status)
{
}

std::optional<int> ServiceRequest::getId() const { return id_; }
int ServiceRequest::getResidentId() const { return residentId_; }
std::string ServiceRequest::getServiceType() const { return serviceType_; }
std::string ServiceRequest::getDescription() const { return description_; }
std::string ServiceRequest::getDateRequested() const { return dateRequested_; }
ServiceRequestStatus ServiceRequest::getStatus() const { return status_; }
