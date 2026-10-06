#pragma once

#include <optional>

#include "../models/ServiceRequest.h"

// Outcome of one status-change attempt. Four distinguishable outcomes:
//   success:            serviceRequest has a value (the updated, persisted request)
//   not found:          notFound true (no request has this id; nothing created)
//   unsupported status: unsupportedStatus true (target is not one of the four statuses)
//   invalid transition: invalidTransition true (both statuses known, move not allowed,
//                       including same-status requests)
// On every failure serviceRequest is empty and persistence is unchanged.
struct ServiceRequestStatusResult
{
    bool success{false};
    bool notFound{false};
    bool unsupportedStatus{false};
    bool invalidTransition{false};
    std::optional<ServiceRequest> serviceRequest{std::nullopt};
};
