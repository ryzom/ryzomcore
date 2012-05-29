// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

//
// Includes
//

#include "stdnet.h"

#include "nel/net/naming_client.h"
#include "nel/net/callback_client.h"
#include "nel/net/service.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


namespace NLNET {

//
// Variables
//

CCallbackClient *CNamingClient::_Connection = NULL;
CNamingClient::TRegServices CNamingClient::_RegisteredServices;

static TBroadcastCallback _RegistrationBroadcastCallback = NULL;
static TBroadcastCallback _UnregistrationBroadcastCallback = NULL;

TServiceId CNamingClient::_MySId(0);

std::list<CNamingClient::CServiceEntry>	CNamingClient::RegisteredServices;
NLMISC::CMutex CNamingClient::RegisteredServicesMutex("CNamingClient::RegisteredServicesMutex");

//
//
//

void CNamingClient::setRegistrationBroadcastCallback (TBroadcastCallback cb)
{
	_RegistrationBroadcastCallback = cb;
}

void CNamingClient::setUnregistrationBroadcastCallback (TBroadcastCallback cb)
{
	_UnregistrationBroadcastCallback = cb;
}

//

//

static bool Registered;
static bool RegisteredSuccess;
static TServiceId *RegisteredSID = NULL;
static string Reason;
void cbRegisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);

static void cbRegister (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlassert(RegisteredSID != NULL);

	msgin.serial (RegisteredSuccess);
	if (RegisteredSuccess)
	{
		msgin.serial (*RegisteredSID);

		// decode the registered services at the register process
		cbRegisterBroadcast (msgin, from, netbase);
	}
	else
	{
		msgin.serial( Reason );
	}
	Registered = true;
}

//

static bool QueryPort;
static uint16 QueryPortPort;

static void cbQueryPort (CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */)
{
	msgin.serial (QueryPortPort);
	QueryPort = true;
}

//

//static bool FirstRegisteredBroadcast;

void cbRegisterBroadcast (CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */)
{
	TServiceId::size_type size;
	string name;
	TServiceId sid;
	vector<CInetAddress> addr;

	msgin.serial (size);

	for (TServiceId::size_type i = 0; i < size; i++)
	{
		msgin.serial (name);
		msgin.serial (sid);
		msgin.serialCont (addr);

		// add it in the list

		std::vector<CInetAddress> addrs;
		CNamingClient::find (sid, addrs);

		if (addrs.size() == 0)
		{
			CNamingClient::RegisteredServicesMutex.enter ();
			CNamingClient::RegisteredServices.push_back (CNamingClient::CServiceEntry (name, sid, addr));
			CNamingClient::RegisteredServicesMutex.leave ();

			nlinfo ("NC: Registration Broadcast of the service %s-%hu '%s'", name.c_str(), sid.get(), vectorCInetAddressToString(addr).c_str());

			if (_RegistrationBroadcastCallback != NULL)
				_RegistrationBroadcastCallback (name, sid, addr);
		}
		else if (addrs.size() == 1)
		{
			CNamingClient::RegisteredServicesMutex.enter ();
			for (std::list<CNamingClient::CServiceEntry>::iterator it = CNamingClient::RegisteredServices.begin(); it != CNamingClient::RegisteredServices.end (); it++)
			{
				if (sid == (*it).SId)
				{
					(*it).Name = name;
					(*it).Addr = addr;
					break;
				}
			}
			CNamingClient::RegisteredServicesMutex.leave ();
			nlinfo ("NC: Registration Broadcast (update) of the service %s-%hu '%s'", name.c_str(), sid.get(), addr[0].asString().c_str());
		}
		else
		{
			nlstop;
		}
	}

//	FirstRegisteredBroadcast = true;

	//CNamingClient::displayRegisteredServices ();
}

//

void cbUnregisterBroadcast (CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */)
{
	string name;
	TServiceId sid;
	vector<CInetAddress> addrs;

	msgin.serial (name);
	msgin.serial (sid);

	// remove it in the list, if the service is not found, ignore it

	CNamingClient::RegisteredServicesMutex.enter ();
	for (std::list<CNamingClient::CServiceEntry>::iterator it = CNamingClient::RegisteredServices.begin(); it != CNamingClient::RegisteredServices.end (); it++)
	{
		CNamingClient::CServiceEntry &serviceEntry = *it;
		if (serviceEntry.SId == sid)
		{
			// check the structure
			nlassertex (serviceEntry.Name == name, ("%s %s",serviceEntry.Name.c_str(), name.c_str()));

			addrs = serviceEntry.Addr;

			CNamingClient::RegisteredServices.erase (it);
			break;
		}
	}
	CNamingClient::RegisteredServicesMutex.leave ();

	nlinfo ("NC: Unregistration Broadcast of the service %s-%hu", name.c_str(), sid.get());

	// send the ACK to the NS

	CMessage msgout ("ACK_UNI");
	msgout.serial (sid);
	CNamingClient::_Connection->send (msgout);

	// oh my god, it s my sid! but i m alive, why this f*cking naming service want to kill me? ok, i leave it alone!
	if(CNamingClient::_MySId == sid)
	{
		nlwarning ("NC: Naming Service asked me to leave, I leave!");
		IService::getInstance()->exit();
		return;
	}

	if (_UnregistrationBroadcastCallback != NULL)
		_UnregistrationBroadcastCallback (name, sid, addrs);

	//CNamingClient::displayRegisteredServices ();
}


//

static TCallbackItem NamingClientCallbackArray[] =
{
	{ "RG", cbRegister },
	{ "QP", cbQueryPort },

	{ "RGB", cbRegisterBroadcast },
	{ "UNB", cbUnregisterBroadcast }
};

void CNamingClient::connect( const CInetAddress &addr, CCallbackNetBase::TRecordingState rec, const vector<CInetAddress> &/* addresses */ )
{
	nlassert (_Connection == NULL || (_Connection != NULL && !_Connection->connected ()));

	if (_Connection == NULL)
	{
		_Connection = new CCallbackClient( rec, "naming_client.nmr" );
		_Connection->addCallbackArray (NamingClientCallbackArray, sizeof (NamingClientCallbackArray) / sizeof (NamingClientCallbackArray[0]));
	}

	_Connection->connect (addr);

/*	// send the available addresses
	CMessage msgout ("RS");
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addresses));
	_Connection->send (msgout);

	// wait the message that contains all already connected services
	FirstRegisteredBroadcast = false;
	while (!FirstRegisteredBroadcast && _Connection->connected ())
	{
		_Connection->update (-1);
		nlSleep (1);
	}
*/}


void CNamingClient::disconnect ()
{
	if (_Connection != NULL)
	{
		if (_Connection->connected ())
		{
			_Connection->disconnect ();
		}
		delete _Connection;
		_Connection = NULL;
	}

	// we don't call unregisterAllServices because when the naming service will see the disconnection,
	// it'll automatically unregister all services registered by this client.
}

string CNamingClient::info ()
{
	string res;

	if (connected ())
	{
		res = "connected to ";
		res += _Connection->remoteAddress().asString();
	}
	else
	{
		res = "Not connected";
	}

	return res;
}

bool CNamingClient::registerService (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId &sid)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout ("RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	sid.set(0);
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RG"
	Registered = false;
	RegisteredSID = &sid;
	while (!Registered)
	{
		_Connection->update (-1);
		nlSleep (1);
	}
	if (RegisteredSuccess)
	{
		_MySId = sid;
		_RegisteredServices.insert (make_pair (*RegisteredSID, name));
		nldebug ("NC: Registered service %s-%hu at %s", name.c_str(), sid.get(), addr[0].asString().c_str());
	}
	else
	{
		nldebug ("NC: Naming service refused to register service %s at %s", name.c_str(), addr[0].asString().c_str());
		nlwarning ("NC: Startup denied: %s", Reason.c_str());
		Reason.clear();
	}

	RegisteredSID = NULL;

	return RegisteredSuccess;
}

bool CNamingClient::registerServiceWithSId (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId sid)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout ("RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RGI"
	Registered = false;
	RegisteredSID = &sid;
	while (!Registered)
	{
		_Connection->update (-1);
		nlSleep (1);
	}
	if (RegisteredSuccess)
	{
		_MySId = sid;
		_RegisteredServices.insert (make_pair (*RegisteredSID, name));
		nldebug ("NC: Registered service with sid %s-%hu at %s", name.c_str(), RegisteredSID->get(), addr[0].asString().c_str());
	}
	else
	{
		nlerror ("NC: Naming service refused to register service with sid %s at %s", name.c_str(), addr[0].asString().c_str());
	}

	return RegisteredSuccess == 1;
}

void CNamingClient::resendRegisteration (const std::string &name, const std::vector<CInetAddress> &addr, TServiceId sid)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout ("RRG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serialCont (const_cast<vector<CInetAddress>&>(addr));
	msgout.serial (sid);
	_Connection->send (msgout);
}

void CNamingClient::unregisterService (TServiceId sid)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout ("UNI");
	msgout.serial (sid);
	_Connection->send (msgout);

	nldebug ("NC: Unregistering service %s-%hu", _RegisteredServices[sid].c_str(), sid.get());
	_RegisteredServices.erase (sid);
}

void CNamingClient::unregisterAllServices ()
{
	nlassert (_Connection != NULL && _Connection->connected ());

	while (!_RegisteredServices.empty())
	{
		TRegServices::iterator irs = _RegisteredServices.begin();
		TServiceId sid = (*irs).first;
		unregisterService (sid);
	}
}

uint16 CNamingClient::queryServicePort ()
{
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout ("QP");
	_Connection->send (msgout);

	// wait the answer of the naming service "QP"
	QueryPort = false;
	while (!QueryPort)
	{
		_Connection->update (-1);
		nlSleep (1);
	}

	nlinfo ("NC: Received the answer of the query port (%hu)", QueryPortPort);

	return QueryPortPort;
}

bool CNamingClient::lookup (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

bool CNamingClient::lookup (TServiceId sid, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (sid, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

bool CNamingClient::lookupAlternate (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// remove it from his local list

	RegisteredServicesMutex.enter ();
	for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).Addr[0] == addr)
		{
			RegisteredServices.erase (it);
			break;
		}
	}
	RegisteredServicesMutex.leave ();

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

void CNamingClient::lookupAll (const std::string &name, std::vector<CInetAddress> &addrs)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	find (name, addrs);
}

bool CNamingClient::lookupAndConnect (const std::string &name, CCallbackClient &sock)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// look up for service
	CInetAddress servaddr;

	// if service not found, return false
	if (!CNamingClient::lookup (name, servaddr))
		return false;

	for(;;)
	{
		try
		{
			// try to connect to the server
			sock.connect (servaddr);

			// connection succeeded
			return true;
		}
		catch (const ESocketConnectionFailed &e)
		{
			nldebug( "NC: Connection to %s failed: %s, tring another service if available", servaddr.asString().c_str(), e.what() );

			// try another server and if service is not found, return false
			if (!CNamingClient::lookupAlternate (name, servaddr))
				return false;
		}
	}
}



void CNamingClient::update ()
{
	// get message for naming service (new registration for example)
	if (_Connection != NULL && _Connection->connected ())
		_Connection->update ();
}

//
// Commands
//

NLMISC_CATEGORISED_COMMAND(nel, services, "displays registered services", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	CNamingClient::displayRegisteredServices (&log);

	return true;
}

} // NLNET
