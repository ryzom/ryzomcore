#ifndef NELNS_NAMING_SERVICE_CAN_ACCESS_H
#define NELNS_NAMING_SERVICE_CAN_ACCESS_H

#include <vector>

#include <nel/net/inet_address.h>
#include <nel/net/unified_network.h>

#include <nelns/naming_service/service_entry.h>

bool canAccess(const std::vector<NLNET::CInetAddress> &addr, const CServiceEntry &entry, std::vector<NLNET::CInetAddress> &accessibleAddr);

#endif // NELNS_NAMING_SERVICE_CAN_ACCESS_H
