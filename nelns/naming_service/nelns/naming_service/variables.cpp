#include <nelns/naming_service/variables.h>

using std::list;

using NLMISC::TTime;
using NLNET::CCallbackServer;
using NLNET::TServiceId;

list<CServiceEntry> RegisteredServices; /// List of all registred services

uint16 MinBasePort = 51000; /// Ports begin at 51000
uint16 MaxBasePort = 52000; /// (note: in this implementation there can be no more than 1000 services)

const TServiceId BaseSId(128); /// Allocated SIds begin at 128 (except for Agent Service)

const TTime UnregisterTimeout = 10000; /// After 10s we remove an unregister service if every server didn't ACK the message

CCallbackServer *CallbackServer = nullptr;
