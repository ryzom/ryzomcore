#include <nelns/naming_service/functions.h>

#include <list>

#include <nel/misc/debug.h>
#include <nel/misc/string_common.h>

#include <nelns/naming_service/can_access.h>
#include <nelns/naming_service/do_unregister_service.h>
#include <nelns/naming_service/effectively_remove.h>
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
 * Callback for service registration when the naming service goes down and up (don't need to broadcast)
 */
void cbResendRegisteration(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial(name);
	msgin.serialCont(addr);
	msgin.serial(sid);

	doRegister(name, addr, sid, from, netbase, true);
}

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
void cbRegister(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial(name);
	msgin.serialCont(addr);
	msgin.serial(sid);

	doRegister(name, addr, sid, from, netbase);
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
void cbQueryPort(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Allocate port
	uint16 port = doAllocatePort(netbase.hostAddress(from));

	// Send port back
	CMessage msgout("QP");
	msgout.serial(port);
	netbase.send(msgout, from);

	nlinfo("The service got port %hu", port);
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
