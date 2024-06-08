#ifndef NELNS_NAMING_SERVICE_DO_UNREGISTER_SERVICE_H
#define NELNS_NAMING_SERVICE_DO_UNREGISTER_SERVICE_H

#include <nel/net/buf_net_base.h>
#include <nel/net/unified_network.h>

// Asks a service to stop and tell every one
void doUnregisterService(const NLNET::TServiceId &sid);

void doUnregisterService(const NLNET::TSockId &from);

#endif // NELNS_NAMING_SERVICE_DO_UNREGISTER_SERVICE_H
