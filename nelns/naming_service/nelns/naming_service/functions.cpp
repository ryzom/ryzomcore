#include <nelns/naming_service/functions.h>

#include <list>

#include <nel/misc/debug.h>
#include <nel/misc/string_common.h>

#include <nelns/naming_service/service_instance_manager.h>
#include <nelns/naming_service/variables.h>

using std::list;
using std::string;
using std::vector;

using NLMISC::CLog;
using NLMISC::CTime;
using NLMISC::InfoLog;
using NLMISC::toString;
using NLNET::CCallbackNetBase;
using NLNET::CInetAddress;
using NLNET::CMessage;
using NLNET::TServiceId;
using NLNET::TSockId;

//
// Functions
//

bool canAccess(const vector<CInetAddress> &addr, const CServiceEntry &entry, vector<CInetAddress> &accessibleAddr)
{
	accessibleAddr.clear();

	if (entry.WaitingUnregistration)
		return false;

	for (uint i = 0; i < addr.size(); i++)
	{
		uint32 net = addr[i].internalNetAddress();
		for (uint j = 0; j < entry.Addr.size(); j++)
		{
			if (net == entry.Addr[j].internalNetAddress())
			{
				accessibleAddr.push_back(entry.Addr[j]);
			}
		}
	}

	if (accessibleAddr.empty())
	{
		nldebug("service %s-%hu is not accessible by '%s'", entry.Name.c_str(), entry.SId.get(), vectorCInetAddressToString(addr).c_str());
	}
	else
	{
		nldebug("service %s-%hu is accessible by '%s'", entry.Name.c_str(), entry.SId.get(), vectorCInetAddressToString(accessibleAddr).c_str());
	}

	return !accessibleAddr.empty();
}

void displayRegisteredServices(CLog *log)
{
	log->displayNL("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
	{
		TSockId id = (*it).SockId;
		if (id == NULL)
		{
			log->displayNL("> %s-%hu %s '%s' %s %d addr", (*it).Name.c_str(), it->SId.get(), "<NULL>", "<NULL>", (*it).WaitingUnregistration ? "WaitUnreg" : "", (*it).Addr.size());
			for (uint i = 0; i < (*it).Addr.size(); i++)
				log->displayNL("              '%s'", (*it).Addr[i].asString().c_str());
		}
		else
		{
			log->displayNL("> %s-%hu %s '%s' %s %d addr", (*it).Name.c_str(), it->SId.get(), (*it).SockId->asString().c_str(), CallbackServer->hostAddress((*it).SockId).asString().c_str(), (*it).WaitingUnregistration ? "WaitUnreg" : "", (*it).Addr.size());
			for (uint i = 0; i < (*it).Addr.size(); i++)
				log->displayNL("              '%s'", (*it).Addr[i].asString().c_str());
		}
	}
	log->displayNL("End of the list");
}

list<CServiceEntry>::iterator effectivelyRemove(list<CServiceEntry>::iterator &it)
{
	// remove the service from the registered service list
	nlinfo("Effectively remove the service %s-%hu", (*it).Name.c_str(), it->SId.get());
	return RegisteredServices.erase(it);
}

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

/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister(const string &name, const vector<CInetAddress> &addr, TServiceId sid, TSockId from, CCallbackNetBase &netbase, bool reconnection)
{
	// Find if the service is not already registered
	string reason;
	uint8 ok = true;

	if (sid.get() == 0)
	{
		// we have to find a sid
		sid = BaseSId;
		bool found = false;
		while (!found)
		{
			list<CServiceEntry>::iterator it;
			for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
			{
				if ((*it).SId == sid)
				{
					break;
				}
			}
			if (it == RegisteredServices.end())
			{
				// ok, we have an empty sid
				found = true;
			}
			else
			{
				sid.set(sid.get() + 1);
				if (sid.get() == 0) // round the clock
				{
					nlwarning("Service identifier allocation overflow");
					ok = false;
					break;
				}
			}
		}
	}
	else
	{
		// we have to check that the user provided sid is available
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
		{
			if ((*it).SId == sid)
			{
				nlwarning("Sid %d already used by another service", sid.get());
				ok = false;
				break;
			}
		}
		if (it != RegisteredServices.end())
		{
			ok = true;
		}
	}

	// if ok, register the service and send a broadcast to other people
	if (ok)
	{
		// Check if the instance is allowed to start, according to the restriction in the config file
		if (CServiceInstanceManager::getInstance()->queryStartService(name, sid, addr, reason))
		{
			// add him in the registered list
			RegisteredServices.push_back(CServiceEntry(from, addr, name, sid));

			// tell to everybody but not him that this service is registered
			if (!reconnection)
			{
				CMessage msgout("RGB");
				TServiceId::size_type s = 1;
				msgout.serial(s);
				msgout.serial(const_cast<string &>(name));
				msgout.serial(sid);
				// we need to send all addr to all services even if the service can't access because we use the address index
				// to know which connection comes.
				msgout.serialCont(const_cast<vector<CInetAddress> &>(addr));
				nlinfo("The service is %s-%d, broadcast the Registration to everybody", name.c_str(), sid.get());

				vector<CInetAddress> accessibleAddress;
				for (list<CServiceEntry>::iterator it3 = RegisteredServices.begin(); it3 != RegisteredServices.end(); it3++)
				{
					// send only services that can be accessed and not itself
					if ((*it3).SId != sid && canAccess(addr, (*it3), accessibleAddress))
					{
						CallbackServer->send(msgout, (*it3).SockId);
						// CNetManager::send ("NS", msgout, (*it3).SockId);
						nldebug("Broadcast to %s-%hu", (*it3).Name.c_str(), it3->SId.get());
					}
				}
			}

			// set the sid only if it s ok
			from->setAppId(sid.get());
		}
		else
		{
			// Reply "startup denied", and do not send registration to other services
			ok = false;
		}
	}

	// send the message to the service to say if it s ok or not
	if (!reconnection)
	{
		// send the answer to the client
		CMessage msgout("RG");
		msgout.serial(ok);
		if (ok)
		{
			msgout.serial(sid);

			// send him all services available (also itself)
			TServiceId::size_type nb = 0;

			vector<CInetAddress> accessibleAddress;

			for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end(); it2++)
			{
				// send only services that are available
				if (canAccess(addr, (*it2), accessibleAddress))
					nb++;
			}
			msgout.serial(nb);

			for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
			{
				// send only services that are available
				if (canAccess(addr, (*it), accessibleAddress))
				{
					msgout.serial((*it).Name);
					msgout.serial((*it).SId);
					msgout.serialCont((*it).Addr);
				}
			}
		}
		else
		{
			msgout.serial(reason);
		}

		netbase.send(msgout, from);
		netbase.flush(from);
	}

	// displayRegisteredServices ();

	return ok != 0;
}

void checkWaitingUnregistrationServices()
{
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end();)
	{
		if ((*it).WaitingUnregistration && ((*it).WaitingUnregistrationServices.empty() || CTime::getLocalTime() > (*it).WaitingUnregistrationTime + UnregisterTimeout))
		{
			if ((*it).WaitingUnregistrationServices.empty())
			{
				nlinfo("Removing the service %s-%hu because all services ACKd the removal", (*it).Name.c_str(), (*it).SId.get());
			}
			else
			{
				string res;
				for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end(); it2++)
				{
					res += toString(it2->get()) + " ";
				}
				nlwarning("Removing the service %s-%hu because time out occurs (service numbers %s didn't ACK)", (*it).Name.c_str(), (*it).SId.get(), res.c_str());
			}
			it = effectivelyRemove(it);
		}
		else
		{
			it++;
		}
	}
}

/**
 * Callback for service unregistration ACK. Mean that a service was ACK the unregistration broadcast
 */
void cbACKUnregistration(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial(sid);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
	{
		if ((*it).SId == sid && (*it).WaitingUnregistration)
		{
			for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end(); it2++)
			{
				if (*it2 == TServiceId(uint16(from->appId())))
				{
					// remove the acked service
					(*it).WaitingUnregistrationServices.erase(it2);
					checkWaitingUnregistrationServices();
					return;
				}
			}
		}
	}
}

/**
 * Callback for service unregistration.
 *
 * Message expected : UNI
 * - Service identifier (TServiceId)
 */
void cbUnregisterSId(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial(sid);

	doUnregisterService(sid);
	// displayRegisteredServices ();
}

/*
 * Helper function for cbQueryPort
 *
 * \warning QueryPort + Registration is not atomic so more than one service could ask a port before register
 */
uint16 doAllocatePort(const CInetAddress &addr)
{
	static uint16 nextAvailablePort = MinBasePort;

	// check if nextavailableport is free

	if (nextAvailablePort >= MaxBasePort) nextAvailablePort = MinBasePort;

	bool ok;
	do
	{
		ok = true;
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
		{
			if ((*it).Addr[0].port() == nextAvailablePort)
			{
				nextAvailablePort++;
				ok = false;
				break;
			}
		}
	} while (!ok);

	return nextAvailablePort++;
}

/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
void cbDisconnect /*(const string &serviceName, TSockId from, void *arg)*/ (TSockId from, void *arg)
{
	doUnregisterService(from);
	// displayRegisteredServices ();
}

/*
 * a service is connected, send him all services infos
 */
void cbConnect /*(const string &serviceName, TSockId from, void *arg)*/ (TSockId from, void *arg)
{
	// we have to wait the registred services message to send all services because it this points, we can't know which sub net
	// the service can use

	// displayRegisteredServices ();

	// set the appid with a bad id (-1)
	from->setAppId(~0);
}

/*
 * Helper that emulate layer5's getServiceName()
 */
string getServiceName(const TServiceId &sid)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
	{
		if ((*it).SId == sid)
		{
			return (*it).Name;
		}
	}
	return ""; // not found
}

/*
 * Helper that returns the first address of a service
 */
CInetAddress getHostAddress(const TServiceId &sid)
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end(); it++)
	{
		if ((*it).SId == sid)
		{
			return (*it).Addr[0];
		}
	}
	return {};
}
