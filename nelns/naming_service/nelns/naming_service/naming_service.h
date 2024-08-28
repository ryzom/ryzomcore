#ifndef NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H
#define NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H

#include <nel/net/buf_net_base.h>
#include <nel/net/callback_net_base.h>
#include <nel/net/message.h>
#include <nel/net/service.h>

#include <nelns/naming_service/service_instance_manager.h>

//
// Service
//

class CNamingService : public NLNET::IService
{
public:
	/**
	 * Init
	 */
	void init();

	/**
	 * Update
	 */
	bool update();

	void release();

private:
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
	void cbRegister(NLNET::CMessage &msgin, NLNET::TSockId from);

	/**
	 * Callback for service registration when the naming service goes down and up (don't need to broadcast)
	 */
	void cbResendRegistration(NLNET::CMessage &msgin, NLNET::TSockId from);

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
	void cbQueryPort(NLNET::CMessage &msgin, NLNET::TSockId from);

	/// Service instance manager singleton
	CServiceInstanceManager _ServiceInstances;
};

#endif // NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H
