#ifndef NELNS_NAMING_SERVICE_FUNCTIONS_H
#define NELNS_NAMING_SERVICE_FUNCTIONS_H

#include <list>
#include <string>
#include <vector>

#include <nel/net/buf_net_base.h>
#include <nel/net/callback_net_base.h>
#include <nel/net/inet_address.h>
#include <nel/net/message.h>
#include <nel/net/unified_network.h>

#include <nelns/naming_service/service_entry.h>

bool canAccess(const std::vector<NLNET::CInetAddress> &addr, const CServiceEntry &entry, std::vector<NLNET::CInetAddress> &accessibleAddr);

void displayRegisteredServices(NLMISC::CLog *log = NLMISC::InfoLog);

std::list<CServiceEntry>::iterator effectivelyRemove (std::list<CServiceEntry>::iterator &it);

/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
std::list<CServiceEntry>::iterator doRemove(std::list<CServiceEntry>::iterator it);

// Asks a service to stop and tell every one
void doUnregisterService(const NLNET::TServiceId &sid);

void doUnregisterService(const NLNET::TSockId &from);

/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister(const std::string &name, const std::vector<NLNET::CInetAddress> &addr, NLNET::TServiceId sid, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase, bool reconnection = false);

void checkWaitingUnregistrationServices();

/**
 * Callback for service unregistration ACK. Mean that a service was ACK the unregistration broadcast
 */
void cbACKUnregistration(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

/**
 * Callback for service registration when the naming service goes down and up (don't need to broadcast)
 */
void cbResendRegisteration(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

/**
 * Callback for service unregistration.
 *
 * Message expected : UNI
 * - Service identifier (TServiceId)
 */
void cbUnregisterSId(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

/*
 * Helper function for cbQueryPort
 *
 * \warning QueryPort + Registration is not atomic so more than one service could ask a port before register
 */
uint16 doAllocatePort(const NLNET::CInetAddress &addr);

/**
 * Callback for port allocation
 * Note: if a service queries a port but does not register itself to the naming service, the
 * port will remain allocated and unused.
 *
 * Message expected : QP
 * - Name of service to register (string)
 * - Address of service (CInetAddress) (its port can be 0)
 *
 * Message emitted : QP
 * - Allocated port number (uint16)
 */
void cbQueryPort(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
void cbDisconnect(NLNET::TSockId from, void *arg);

/*
 * a service is connected, send him all services infos
 */
void cbConnect(NLNET::TSockId from, void *arg);

// Helper that emulate layer5's getServiceName()
std::string getServiceName(const NLNET::TServiceId& sid);

// Helper that returns the first address of a service
NLNET::CInetAddress getHostAddress( const NLNET::TServiceId&  sid );

#endif // NELNS_NAMING_SERVICE_FUNCTIONS_H
