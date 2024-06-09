#include <nelns/naming_service/service_instance_manager.h>

#include <nel/misc/string_common.h>

#include <nelns/naming_service/do_unregister_service.h>
#include <nelns/naming_service/helper.h>
#include <nelns/naming_service/service_entry.h>

using std::map;
using std::set;
using std::string;
using std::vector;

using NLMISC::toString;
using NLNET::CInetAddress;
using NLNET::CUnifiedNetwork;
using NLNET::TServiceId;

CServiceInstanceManager *CServiceInstanceManager::_Instance = nullptr;

CServiceInstanceManager *CServiceInstanceManager::getInstance()
{
	nlassertex(_Instance, ("No Singleton Instance existing"));
	return _Instance;
}

/*
 * Constructor
 */
CServiceInstanceManager::CServiceInstanceManager()
{
	nlassertex(!_Instance, ("Singleton Instance already existing"));
	_Instance = this;

	// Note: addCallbackArray() done in CRangeMirrorManager::init()
}

CServiceInstanceManager::~CServiceInstanceManager()
{
	_Instance = nullptr;
}

/*
 * Check if a service is allowed to start. Answer with a GSTS (Grant Start Service) message
 */
bool CServiceInstanceManager::queryStartService(const string &serviceName, TServiceId serviceId, const vector<CInetAddress> &addr, string &reason)
{
	bool grantStarting = true;
	map<string, bool>::iterator ius = _UniqueServices.find(serviceName);
	if (ius != _UniqueServices.end())
	{
		// Service is restricted
		set<TServiceId>::iterator ios;
		bool uniqueOnShard = (*ius).second;
		for (ios = _OnlineServices.begin(); ios != _OnlineServices.end(); ++ios)
		{
			string name = getServiceName(*ios);
			if (name == serviceName)
			{
				if (uniqueOnShard)
				{
					// Only one service by shard is allowed => deny
					grantStarting = false;
					reason = toString("Service %s already found as %hu, must be unique on shard", serviceName.c_str(), ios->get());
					nlinfo(reason.c_str());
					break;
				}
				else
				{
					// Only one service by physical machine is allowed

					// Implementation for layer5
					// TSockId hostid1, hostid2;
					/*CCallbackNetBase *cnb1 = CUnifiedNetwork::getInstance()->getNetBase( serviceId, hostid1 );
					CCallbackNetBase *cnb2 = CUnifiedNetwork::getInstance()->getNetBase( *ios, hostid2 );
					if ( cnb1->hostAddress( hostid1 ).internalIPAddress() == cnb2->hostAddress( hostid2 ).internalIPAddress() )*/

					// Implementation for NS
					if (addr[0].getAddress() == getHostAddress(*ios).getAddress())
					{
						grantStarting = false;
						reason = toString("Service %s already found as %hu on same machine", serviceName.c_str(), ios->get());
						nlinfo(reason.c_str());
						break;
					}
				}
			}
		}
	}

	if (grantStarting)
	{
		_OnlineServices.insert(serviceId);
	}
	return grantStarting;
}

/*
 * Release a service instance
 */
void CServiceInstanceManager::releaseService(NLNET::TServiceId serviceId)
{
	_OnlineServices.erase(serviceId); // not a problem if not found
}

/*
 * Display information
 */
void CServiceInstanceManager::displayInfo(NLMISC::CLog *log) const
{
	log->displayNL("Restricted services:");
	map<string, bool>::const_iterator ius;
	for (ius = _UniqueServices.begin(); ius != _UniqueServices.end(); ++ius)
	{
		log->displayNL("%s -> only one per %s", (*ius).first.c_str(), (*ius).second ? "shard" : "machine");
	}
	log->displayNL("Online registered services:");
	set<TServiceId>::const_iterator ios;
	for (ios = _OnlineServices.begin(); ios != _OnlineServices.end(); ++ios)
	{
		log->displayNL("%s", CUnifiedNetwork::getInstance()->getServiceUnifiedName(*ios).c_str());
	}
}

/*
 * Make all controlled services quit
 */
void CServiceInstanceManager::killAllServices()
{
	// Send to all known online services
	std::set<TServiceId>::const_iterator ios;
	for (ios = _OnlineServices.begin(); ios != _OnlineServices.end(); ++ios)
	{
		doUnregisterService((TServiceId)(*ios));
	}
}
