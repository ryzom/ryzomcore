#ifndef NELNS_NAMING_SERVICE_HELPER_H
#define NELNS_NAMING_SERVICE_HELPER_H

#include <string>

#include <nel/net/unified_network.h>

// Helper that emulate layer5's getServiceName()
std::string getServiceName(NLNET::TServiceId sid);

#endif // NELNS_NAMING_SERVICE_HELPER_H
