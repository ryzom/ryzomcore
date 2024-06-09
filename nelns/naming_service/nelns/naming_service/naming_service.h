#ifndef NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H
#define NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H

#include <nel/net/service.h>
#include <nel/net/buf_net_base.h>
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
	/// Service instance manager singleton
	CServiceInstanceManager _ServiceInstances;
};

#endif // NELNS_NAMING_SERVICE_SERVICE_NAMING_SERVICE_H
