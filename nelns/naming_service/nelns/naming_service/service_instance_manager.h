#ifndef NELNS_NAMING_SERVICE_SERVICE_INSTANCE_MANAGER_H
#define NELNS_NAMING_SERVICE_SERVICE_INSTANCE_MANAGER_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/log.h>

#include <nel/net/inet_address.h>
#include <nel/net/unified_network.h>

/**
 * Manager for services instances
 * (Moved from the TICKS to the NS)
 * Implementable with layer 5, here implemented in NS (layer 3)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CServiceInstanceManager
{
private:
	static CServiceInstanceManager *_Instance;

public:
	static CServiceInstanceManager *getInstance();

	/// Constructor
	CServiceInstanceManager();

	virtual ~CServiceInstanceManager();

	/** Add the name of a service which must not be duplicated
	 * If uniqueOnShard is true, only one service is allowed.
	 * If uniqueOnShard is false, one service is allowed by physical machine.
	 */
	void addUniqueService(const std::string &serviceName, bool uniqueOnShard)
	{
		_UniqueServices.insert(std::make_pair(serviceName, uniqueOnShard));
	}

	/// Check if a service is allowed to start (if so, add it)
	bool queryStartService(const std::string &serviceName, NLNET::TServiceId serviceId, const std::vector<NLNET::CInetAddress> &addr, std::string &reason);

	/// Release a service instance
	void releaseService(NLNET::TServiceId serviceId);

	/// Display information
	void displayInfo(NLMISC::CLog *log = NLMISC::InfoLog) const;

	/// Make all controlled services quit
	void killAllServices();

private:
	/// List of restricted services
	std::map<std::string, bool> _UniqueServices;

	/// List of granted (online) services
	std::set<NLNET::TServiceId> _OnlineServices;
};

#endif // NELNS_NAMING_SERVICE_SERVICE_INSTANCE_MANAGER_H
