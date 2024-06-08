#include <nelns/naming_service/do_unregister_service.h>

#include <list>
#include <nelns/naming_service/do_remove.h>
#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/variables.h>

using std::list;

using NLNET::CInetAddress;
using NLNET::TServiceId;
using NLNET::TSockId;

void doUnregisterService(const TServiceId &sid)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
	{
		if ((*it).SId == sid)
		{
			// found it, remove it
			doRemove(it);
			return;
		}
	}
	nlwarning("Service %hu not found", sid.get());
}

void doUnregisterService(const NLNET::TSockId &from)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end();)
	{
		if ((*it).SockId == from)
		{
			// it's possible that one "from" have more than one registred service, so we have to find in all the list
			// found it, remove it
			it = doRemove(it);
		}
		else
		{
			it++;
		}
	}
}
