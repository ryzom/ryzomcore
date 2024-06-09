#ifndef NELNS_NAMING_SERVICE_FUNCTIONS_H
#define NELNS_NAMING_SERVICE_FUNCTIONS_H

#include <string>
#include <vector>

#include <nel/net/buf_net_base.h>
#include <nel/net/callback_net_base.h>
#include <nel/net/inet_address.h>
#include <nel/net/message.h>
#include <nel/net/unified_network.h>

void displayRegisteredServices(NLMISC::CLog *log = NLMISC::InfoLog);

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
 * Callback for service registration.
 *
 * Message expected : RG
 * - Name of service to register (string)
 * - Address of service (CInetAddress)
 *
 * Message emitted : RG
 * - Allocated service identifier (TServiceId) or 0 if failed
 */
void cbRegister(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase);

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

#endif // NELNS_NAMING_SERVICE_FUNCTIONS_H
