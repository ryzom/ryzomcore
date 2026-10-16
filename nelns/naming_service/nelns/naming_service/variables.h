#ifndef NELNS_NAMING_SERVICE_VARIABLES_H
#define NELNS_NAMING_SERVICE_VARIABLES_H

#include <string>

#include <nel/misc/time_nl.h>
#include <nel/misc/types_nl.h>

#include <nel/net/callback_net_base.h>
#include <nel/net/unified_network.h>

#include <nelns/naming_service/service_entry.h>

//
// Variables
//

extern std::list<CServiceEntry> RegisteredServices; /// List of all registred services

extern uint16 MinBasePort; /// Ports begin at 51000
extern uint16 MaxBasePort; /// (note: in this implementation there can be no more than 1000 services)

extern const NLNET::TServiceId BaseSId; /// Allocated SIds begin at 128 (except for Agent Service)

extern const NLMISC::TTime UnregisterTimeout; /// After 10s we remove an unregister service if every server didn't ACK the message

extern NLNET::CCallbackServer *CallbackServer;

#endif // NELNS_NAMING_SERVICE_VARIABLES_H
