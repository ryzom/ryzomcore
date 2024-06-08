// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#define NELNS_LOGS ""
#endif // NELNS_LOGS


//
// Includes
//

#include <nel/misc/types_nl.h>

#include <list>
#include <string>

#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/variable.h>
#include <nel/misc/displayer.h>

#include <nel/net/callback_server.h>
#include <nel/net/service.h>
#include <nel/net/module_manager.h>

#include <nelns/naming_service/can_access.h>
#include <nelns/naming_service/do_unregister_service.h>
#include <nelns/naming_service/effectively_remove.h>
#include <nelns/naming_service/helper.h>
#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/service_instance_manager.h>
#include <nelns/naming_service/variables.h>

//
// Namespaces
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;


NLMISC_COMMAND(test, "none", "none")
{
	log.displayNL("Raw cmd line : '%s'", rawCommandString.c_str());
	log.displayNL("Dumping %u parameters :", args.size());
	for (uint i=0; i<args.size(); ++i)
	{
		log.displayNL("  %u : '%s'", i, args[i].c_str());
	}
	return true;
}


/*
 * Constructor
 */
CServiceInstanceManager::CServiceInstanceManager()
{
	nlassert( ! _Instance );
	_Instance = this;

	// Note: addCallbackArray() done in CRangeMirrorManager::init()
}


/*
 * Check if a service is allowed to start. Answer with a GSTS (Grant Start Service) message
 */
bool		CServiceInstanceManager::queryStartService( const std::string& serviceName, TServiceId  serviceId, const vector<CInetAddress> &addr, string& reason )
{
	bool grantStarting = true;
	std::map< std::string, bool >::iterator ius = _UniqueServices.find( serviceName );
	if ( ius != _UniqueServices.end() )
	{
		// Service is restricted
		set< TServiceId >::iterator ios;
		bool uniqueOnShard = (*ius).second;
		for ( ios=_OnlineServices.begin(); ios!=_OnlineServices.end(); ++ios )
		{
			string name = getServiceName( *ios );
			if ( name == serviceName )
			{
				if ( uniqueOnShard )
				{
					// Only one service by shard is allowed => deny
					grantStarting = false;
					reason = toString( "Service %s already found as %hu, must be unique on shard", serviceName.c_str(), ios->get() );
					nlinfo( reason.c_str() );
					break;
				}
				else
				{
					// Only one service by physical machine is allowed

					// Implementation for layer5
					//TSockId hostid1, hostid2;
					/*CCallbackNetBase *cnb1 = CUnifiedNetwork::getInstance()->getNetBase( serviceId, hostid1 );
					CCallbackNetBase *cnb2 = CUnifiedNetwork::getInstance()->getNetBase( *ios, hostid2 );
					if ( cnb1->hostAddress( hostid1 ).internalIPAddress() == cnb2->hostAddress( hostid2 ).internalIPAddress() )*/

					// Implementation for NS
					if ( addr[0].getAddress() == getHostAddress( *ios ).getAddress() )
					{
						grantStarting = false;
						reason = toString( "Service %s already found as %hu on same machine", serviceName.c_str(), ios->get() );
						nlinfo( reason.c_str() );
						break;
					}
				}
			}
		}
	}

	if ( grantStarting )
	{
		_OnlineServices.insert( serviceId );
	}
	return grantStarting;
}


/*
 * Release a service instance
 */
void		CServiceInstanceManager::releaseService( NLNET::TServiceId serviceId )
{
	_OnlineServices.erase( serviceId ); // not a problem if not found
}


/*
 * Display information
 */
void		CServiceInstanceManager::displayInfo( NLMISC::CLog *log ) const
{
	log->displayNL( "Restricted services:" );
	std::map< std::string, bool >::const_iterator ius;
	for ( ius=_UniqueServices.begin(); ius!=_UniqueServices.end(); ++ius )
	{
		log->displayNL( "%s -> only one per %s", (*ius).first.c_str(), (*ius).second?"shard":"machine" );
	}
	log->displayNL( "Online registered services:" );
	std::set< TServiceId >::const_iterator ios;
	for ( ios=_OnlineServices.begin(); ios!=_OnlineServices.end(); ++ios )
	{
		log->displayNL( "%s", CUnifiedNetwork::getInstance()->getServiceUnifiedName( *ios ).c_str() );
	}
}


/*
 * Make all controlled services quit
 */
void		CServiceInstanceManager::killAllServices()
{
	// Send to all known online services
	std::set< TServiceId >::const_iterator ios;
	for ( ios=_OnlineServices.begin(); ios!=_OnlineServices.end(); ++ios )
	{
		doUnregisterService( (TServiceId)(*ios) );
	}
}

CServiceInstanceManager * CServiceInstanceManager::_Instance = nullptr;

CServiceInstanceManager *CServiceInstanceManager::getInstance()
{
	nlassert( _Instance );
	return _Instance;
}

//
// Functions
//


void displayRegisteredServices (CLog *log = InfoLog)
{
	log->displayNL ("Display the %d registered services :", RegisteredServices.size());
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		TSockId id = (*it).SockId;
		if (id == NULL)
		{
			log->displayNL ("> %s-%hu %s '%s' %s %d addr", (*it).Name.c_str(), it->SId.get(), "<NULL>", "<NULL>", (*it).WaitingUnregistration?"WaitUnreg":"", (*it).Addr.size());
			for(uint i = 0; i < (*it).Addr.size(); i++)
				log->displayNL ("              '%s'", (*it).Addr[i].asString().c_str());
		}
		else
		{
			log->displayNL ("> %s-%hu %s '%s' %s %d addr", (*it).Name.c_str(), it->SId.get(), (*it).SockId->asString().c_str(), CallbackServer->hostAddress((*it).SockId).asString().c_str(), (*it).WaitingUnregistration?"WaitUnreg":"", (*it).Addr.size());
			for(uint i = 0; i < (*it).Addr.size(); i++)
				log->displayNL ("              '%s'", (*it).Addr[i].asString().c_str());
		}
	}
	log->displayNL ("End of the list");
}


/*
 * Helper function for cbRegister.
 * If alloc_sid is true, sid is ignored
 * Returns false in case of failure of sid allocation or bad sid provided
 * Note: the reply is included in this function, because it must be done before things such as syncUniTime()
 */
bool doRegister (const string &name, const vector<CInetAddress> &addr, TServiceId sid, TSockId from, CCallbackNetBase &netbase, bool reconnection = false)
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
			for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
			{
				if ((*it).SId == sid)
				{
					break;
				}
			}
			if (it == RegisteredServices.end ())
			{
				// ok, we have an empty sid
				found = true;
			}
			else
			{
				sid.set(sid.get()+1);
				if (sid.get() == 0) // round the clock
				{
					nlwarning ("Service identifier allocation overflow");
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
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).SId == sid)
			{
				nlwarning ("Sid %d already used by another service", sid.get());
				ok = false;
				break;
			}
		}
		if (it != RegisteredServices.end ())
		{
			ok = true;
		}
	}

	// if ok, register the service and send a broadcast to other people
	if (ok)
	{
		// Check if the instance is allowed to start, according to the restriction in the config file
		if ( CServiceInstanceManager::getInstance()->queryStartService( name, sid, addr, reason ) )
		{
			// add him in the registered list
			RegisteredServices.push_back (CServiceEntry(from, addr, name, sid));

			// tell to everybody but not him that this service is registered
			if (!reconnection)
			{
				CMessage msgout ("RGB");
				TServiceId::size_type s = 1;
				msgout.serial (s);
				msgout.serial (const_cast<string &>(name));
				msgout.serial (sid);
				// we need to send all addr to all services even if the service can't access because we use the address index
				// to know which connection comes.
				msgout.serialCont (const_cast<vector<CInetAddress> &>(addr));
				nlinfo ("The service is %s-%d, broadcast the Registration to everybody", name.c_str(), sid.get());

				vector<CInetAddress> accessibleAddress;
				for (list<CServiceEntry>::iterator it3 = RegisteredServices.begin(); it3 != RegisteredServices.end (); it3++)
				{
					// send only services that can be accessed and not itself
					if ((*it3).SId != sid && canAccess(addr, (*it3), accessibleAddress))
					{
						CallbackServer->send (msgout, (*it3).SockId);
						//CNetManager::send ("NS", msgout, (*it3).SockId);
						nldebug ("Broadcast to %s-%hu", (*it3).Name.c_str(), it3->SId.get());
					}
				}
			}

			// set the sid only if it s ok
			from->setAppId (sid.get());
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
		CMessage msgout ("RG");
		msgout.serial (ok);
		if (ok)
		{
			msgout.serial (sid);

			// send him all services available (also itself)
			TServiceId::size_type nb = 0;

			vector<CInetAddress> accessibleAddress;

			for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end (); it2++)
			{
				// send only services that are available
				if (canAccess(addr, (*it2), accessibleAddress))
					nb++;
			}
			msgout.serial (nb);

			for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
			{
				// send only services that are available
				if (canAccess(addr, (*it), accessibleAddress))
				{
					msgout.serial ((*it).Name);
					msgout.serial ((*it).SId);
					msgout.serialCont ((*it).Addr);
				}
			}
		}
		else
		{
			msgout.serial( reason );
		}

		netbase.send (msgout, from);
		netbase.flush (from);
	}

	//displayRegisteredServices ();

	return ok!=0;
}

void checkWaitingUnregistrationServices ()
{
	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end ();)
	{
		if ((*it).WaitingUnregistration && ((*it).WaitingUnregistrationServices.empty() || CTime::getLocalTime() > (*it).WaitingUnregistrationTime + UnregisterTimeout))
		{
			if ((*it).WaitingUnregistrationServices.empty())
			{
				nlinfo ("Removing the service %s-%hu because all services ACKd the removal", (*it).Name.c_str(), (*it).SId.get());
			}
			else
			{
				string res;
				for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end (); it2++)
				{
					res += toString(it2->get()) + " ";
				}
				nlwarning ("Removing the service %s-%hu because time out occurs (service numbers %s didn't ACK)", (*it).Name.c_str(), (*it).SId.get(), res.c_str());
			}
			it = effectivelyRemove (it);
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
static void cbACKUnregistration (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial (sid);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid && (*it).WaitingUnregistration)
		{
			for (list<TServiceId>::iterator it2 = (*it).WaitingUnregistrationServices.begin(); it2 != (*it).WaitingUnregistrationServices.end (); it2++)
			{
				if (*it2 == TServiceId(uint16(from->appId())))
				{
					// remove the acked service
					(*it).WaitingUnregistrationServices.erase (it2);
					checkWaitingUnregistrationServices ();
					return;
				}
			}
		}
	}
}


/**
 * Callback for service registration when the naming service goes down and up (don't need to broadcast)
 */
static void cbResendRegisteration (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial (name);
	msgin.serialCont (addr);
	msgin.serial (sid);

	doRegister (name, addr, sid, from, netbase, true);
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
static void cbRegister (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial (name);
	msgin.serialCont (addr);
	msgin.serial (sid);

	doRegister (name, addr, sid, from, netbase);
}


/**
 * Callback for service unregistration.
 *
 * Message expected : UNI
 * - Service identifier (TServiceId)
 */
static void cbUnregisterSId (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId sid;
	msgin.serial( sid );

	doUnregisterService (sid);
	//displayRegisteredServices ();
}


/*
 * Helper function for cbQueryPort
 *
 * \warning QueryPort + Registration is not atomic so more than one service could ask a port before register
 */
uint16 doAllocatePort (const CInetAddress &addr)
{
	static uint16 nextAvailablePort = MinBasePort;

	// check if nextavailableport is free

	if (nextAvailablePort >= MaxBasePort) nextAvailablePort = MinBasePort;

	bool ok;
	do
	{
		ok = true;
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Addr[0].port () == nextAvailablePort)
			{
				nextAvailablePort++;
				ok = false;
				break;
			}
		}
	}
	while (!ok);

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
static void cbQueryPort (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Allocate port
	uint16 port = doAllocatePort (netbase.hostAddress (from));

	// Send port back
	CMessage msgout ("QP");
	msgout.serial (port);
	netbase.send (msgout, from);

	nlinfo ("The service got port %hu", port);
}


/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
static void cbDisconnect /*(const string &serviceName, TSockId from, void *arg)*/ ( TSockId from, void *arg )
{
	doUnregisterService (from);
	//displayRegisteredServices ();
}

/*
 * a service is connected, send him all services infos
 */
static void cbConnect /*(const string &serviceName, TSockId from, void *arg)*/ ( TSockId from, void *arg )
{
	// we have to wait the registred services message to send all services because it this points, we can't know which sub net
	// the service can use

	//displayRegisteredServices ();

	// set the appid with a bad id (-1)
	from->setAppId (~0);
}

/*// returns the list of accessible services with a list of address
static void cbRegisteredServices(CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	vector<CInetAddress> addr;
	msgin.serialCont (addr);

	nlinfo ("New service ask me the available services, sending him all services available");
	// send to the new service the list of all services that this service can access (depending of his sub net)

	CMessage msgout ("RGB");

	uint8 nb = 0;

	vector<CInetAddress> accessibleAddress;

	for (list<CServiceEntry>::iterator it2 = RegisteredServices.begin(); it2 != RegisteredServices.end (); it2++)
	{
		// send only services that are available
		if (canAccess(addr, (*it2), accessibleAddress))
			nb++;
	}

	msgout.serial (nb);

	for (list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		// send only services that are available
		if (canAccess(addr, (*it), accessibleAddress))
		{
			msgout.serial ((*it).Name);
			msgout.serial ((*it).SId);
			msgout.serialCont (accessibleAddress);
		}
	}

	CNetManager::send ("NS", msgout, from);
}*/


/*
 * Helper that emulates layer5 send()
 */
/*void sendToService( uint16 sid, CMessage& msgout )
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid)
		{
			CallbackServer->send (msgout, (*it).SockId);
		}
	}
}*/


//
// Callback array
//

TCallbackItem CallbackArray[] =
{
	{ "RG", cbRegister },
	{ "RRG", cbResendRegisteration },
	{ "QP", cbQueryPort },
	{ "UNI", cbUnregisterSId },
	{ "ACK_UNI", cbACKUnregistration },
//	{ "RS", cbRegisteredServices },
};


//
// Service
//

class CNamingService : public NLNET::IService
{
public:

	/**
	 * Init
	 */
	void init()
	{
		// if a baseport is available in the config file, get it
		CConfigFile::CVar *var;
		if ((var = ConfigFile.getVarPtr ("BasePort")) != NULL)
		{
			uint16 newBasePort = var->asInt ();
			nlinfo ("Changing the MinBasePort number from %hu to %hu", MinBasePort, newBasePort);
			sint32 delta = MaxBasePort - MinBasePort;
			nlassert (delta > 0);
			MinBasePort = newBasePort;
			MaxBasePort = MinBasePort + uint16 (delta);
		}

		// Parameters for the service instance manager
		try
		{
			CConfigFile::CVar& uniqueServices = ConfigFile.getVar("UniqueOnShardServices");
			for ( uint i=0; i!=uniqueServices.size(); ++i )
			{
				_ServiceInstances.addUniqueService( uniqueServices.asString(i), true );
			}
		}
		catch(Exception &)
		{}
		try
		{
			CConfigFile::CVar& uniqueServicesM = ConfigFile.getVar("UniqueByMachineServices");
			for ( uint i=0; i!=uniqueServicesM.size(); ++i )
			{
				_ServiceInstances.addUniqueService( uniqueServicesM.asString(i), false );
			}
		}
		catch(Exception &)
		{}

/*
		// we don't try to associate message from client
		CNetManager::getNetBase ("NS")->ignoreAllUnknownId (true);

		// add the callback in case of disconnection
		CNetManager::setConnectionCallback ("NS", cbConnect, NULL);
		
		// add the callback in case of disconnection
		CNetManager::setDisconnectionCallback ("NS", cbDisconnect, NULL);
*/
		// DEBUG
		// DebugLog->addDisplayer( new CStdDisplayer() );

		vector<CInetAddress> v = CInetAddress::localAddresses();
		nlinfo ("%d detected local addresses:", v.size());
		for (uint i = 0; i < v.size(); i++)
		{
			nlinfo (" %d - '%s'",i, v[i].asString().c_str());
		}

		uint16 nsport = 50000;
		if ((var = ConfigFile.getVarPtr ("NSPort")) != NULL)
		{
			nsport = var->asInt ();
		}

		CallbackServer = new CCallbackServer;
		CallbackServer->init(nsport);
		CallbackServer->addCallbackArray(CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
		CallbackServer->setConnectionCallback(cbConnect, NULL);
		CallbackServer->setDisconnectionCallback(cbDisconnect, NULL);
	}

	/**
	 * Update
	 */
	bool update ()
	{
		checkWaitingUnregistrationServices ();

		CallbackServer->update ();

		return true;
	}

	void release()
	{
		if (CallbackServer != NULL)
			delete CallbackServer;
		CallbackServer = NULL;
	}

private:

	/// Service instance manager singleton
	CServiceInstanceManager		_ServiceInstances;
};


static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s;
	s= "naming_service";

	if (theService->haveLongArg("nsname"))
	{
		s+= "_"+theService->getLongArg("nsname");
	}

	if (theService->haveLongArg("fullnsname"))
	{
		s= theService->getLongArg("fullnsname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s;
	s= "NS";

	if (theService->haveLongArg("shortnsname"))
	{
		s= theService->getLongArg("shortnsname");
	}
	
	return s.c_str();
}
//
/// Naming Service
//
NLNET_SERVICE_MAIN( CNamingService, getShortServiceName(scn), getCompleteServiceName(scn), 0, EmptyCallbackArray, NELNS_CONFIG, NELNS_LOGS)


//
// Commands
//


NLMISC_COMMAND (nsServices, "displays the list of all registered services", "")
{
	if(args.size() != 0) return false;

	displayRegisteredServices (&log);

	return true;
}

NLMISC_COMMAND (kill, "kill a service and send an unregister broadcast to other service", "<ServiceShortName>|<ServiceId>")
{
	if(args.size() != 1) return false;

	// try with number

	TServiceId sid(atoi(args[0].c_str()));

	if(sid.get() == 0)
	{
		// not a number, try a name
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Name == args[0])
			{
				sid = (*it).SId;
				break;
			}
		}
		if (it == RegisteredServices.end())
		{
			log.displayNL ("Bad service name or id '%s'", args[0].c_str());
			return false;
		}
	}

	doUnregisterService (sid);
	return true;
}

NLMISC_DYNVARIABLE(uint32, NbRegisteredServices, "display the number of service that are registered in naming service")
{
	if (get) *pointer = (uint32)RegisteredServices.size();
}

NLMISC_COMMAND( displayServiceInstances, "SIM: Display info on service instances", "" )
{
	CServiceInstanceManager::getInstance()->displayInfo( &log );
	return true;
}

NLMISC_COMMAND( killAllServices, "SIM: Make all the controlled services quit", "" )
{
	CServiceInstanceManager::getInstance()->killAllServices();
	return true;
}
