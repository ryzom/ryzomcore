#include <nelns/naming_service/do_remove.h>

#include <nel/misc/string_common.h>
#include <nelns/naming_service/can_access.h>
#include <nelns/naming_service/effectively_remove.h>
#include <nelns/naming_service/service_instance_manager.h>
#include <nelns/naming_service/variables.h>

using std::list;
using std::string;
using std::vector;

using NLMISC::CTime;
using NLMISC::toString;
using NLNET::CInetAddress;
using NLNET::CMessage;
using NLNET::TServiceId;

/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
list<CServiceEntry>::iterator doRemove(list<CServiceEntry>::iterator it)
{
	nldebug("Unregister the service %s-%hu '%s'", (*it).Name.c_str(), it->SId.get(), (*it).Addr[0].asString().c_str());

	// tell to everybody that this service is unregistered

	CMessage msgout("UNB");
	msgout.serial((*it).Name);
	msgout.serial((*it).SId);

	vector<CInetAddress> accessibleAddress;
	nlinfo("Broadcast the Unregistration of %s-%hu to all registered services", (*it).Name.c_str(), it->SId.get());
	for (list<CServiceEntry>::iterator it3 = RegisteredServices.begin(); it3 != RegisteredServices.end(); it3++)
	{
		if (canAccess((*it).Addr, (*it3), accessibleAddress))
		{
			CallbackServer->send(msgout, (*it3).SockId);
			// CNetManager::send ("NS", msgout, (*it3).SockId);
			nldebug("Broadcast to %s-%hu", (*it3).Name.c_str(), it3->SId.get());
		}
	}

	// new system, after the unregistation broadcast, we wait ACK from all services before really remove
	// the service, before, we tag the service as 'wait before unregister'
	// if everybody didn't answer before the time out, we remove it

	(*it).SockId = NULL;

	(*it).WaitingUnregistration = true;
	(*it).WaitingUnregistrationTime = CTime::getLocalTime();

	// we remove all services awaiting his ACK because this service is down so it'll never ACK
	for (list<CServiceEntry>::iterator itr = RegisteredServices.begin(); itr != RegisteredServices.end(); itr++)
	{
		for (list<TServiceId>::iterator itw = (*itr).WaitingUnregistrationServices.begin(); itw != (*itr).WaitingUnregistrationServices.end();)
		{
			if ((*itw) == (*it).SId)
			{
				itw = (*itr).WaitingUnregistrationServices.erase(itw);
			}
			else
			{
				itw++;
			}
		}
	}

	string res;
	for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end(); it2++)
	{
		if (!(*it2).WaitingUnregistration)
		{
			(*it).WaitingUnregistrationServices.push_back((*it2).SId);
			res += toString((*it2).SId.get()) + " ";
		}
	}

	nlinfo("Before removing the service %s-%hu, we wait the ACK of '%s'", (*it).Name.c_str(), (*it).SId.get(), res.c_str());

	if ((*it).WaitingUnregistrationServices.empty())
	{
		return effectivelyRemove(it);
	}
	else
	{
		return ++it;
	}

	// Release from the service instance manager
	CServiceInstanceManager::getInstance()->releaseService((*it).SId);
}
