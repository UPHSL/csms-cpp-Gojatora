#pragma once

#include <vector>

#include "../models/ServiceRequest.h"

// Outcome of one search/filter query. Two distinguishable outcomes:
//   success:            serviceRequests holds the matches (possibly empty,
//                       which is a normal "nothing matched" result)
//   unsupported status: unsupportedStatus true (the status filter was not
//                       blank and not one of the four supported statuses);
//                       serviceRequests is empty and no query was run
struct ServiceRequestQueryResult
{
    bool success{false};
    bool unsupportedStatus{false};
    std::vector<ServiceRequest> serviceRequests{};
};
