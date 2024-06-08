#ifndef NELNS_NAMING_SERVICE_HELPER_H
#define NELNS_NAMING_SERVICE_HELPER_H

#include <string>

#include <nel/net/inet_address.h>
#include <nel/net/unified_network.h>

// Helper that emulate layer5's getServiceName()
std::string getServiceName(const NLNET::TServiceId& sid);

// Helper that returns the first address of a service
NLNET::CInetAddress getHostAddress( const NLNET::TServiceId&  sid );

#endif // NELNS_NAMING_SERVICE_HELPER_H
