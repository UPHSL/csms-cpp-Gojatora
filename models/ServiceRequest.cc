#include "ServiceRequest.h"

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
