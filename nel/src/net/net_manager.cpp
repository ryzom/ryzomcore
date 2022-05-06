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

#include "stdnet.h"

/**************************************************************************
********************* THIS CLASS IS DEPRECATED ****************************
**************************************************************************/
#ifdef NL_OS_WINDOWS
#	pragma message(NL_LOC_WRN "You are using a deprecated feature of NeL, consider rewriting your code with replacement feature")
#else // NL_OS_UNIX
#	warning "You are using a deprecated feature of NeL, consider rewriting your code with replacement feature"
#endif

#include "nel/misc/time_nl.h"

#include "nel/net/naming_client.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"
#include "nel/net/naming_client.h"
#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {


CNetManager::TBaseMap	CNetManager::_BaseMap;

CCallbackNetBase::TRecordingState	CNetManager::_RecordingState;

TTime CNetManager::_NextUpdateTime = 0;

static void nmNewConnection (TSockId from, void *arg)
{
	nlassert (arg != NULL);
	CBaseStruct *basest = (CBaseStruct *)arg;

	nldebug("HNETL4: nmNewConnection() from service '%s'", basest->Name.c_str ());

	// call the client callback if necessary
	if (basest->ConnectionCallback != NULL)
		basest->ConnectionCallback (basest->Name, from, basest->ConnectionCbArg);
}

static void nmNewDisconnection (TSockId from, void *arg)
{
	nlassert (arg != NULL);
	CBaseStruct *basest = (CBaseStruct *)arg;

	nldebug("HNETL4: nmNewDisconnection() from service '%s'", basest->Name.c_str ());

	// call the client callback if necessary
	if (basest->DisconnectionCallback != NULL)
		basest->DisconnectionCallback (basest->Name, from, basest->DisconnectionCbArg);
}


// find a not connected callbackclient or create a new one and connect to the Addr
void CNetManager::createConnection(CBaseStruct &Base, const CInetAddress &Addr, const string& name)
{
	uint i;
	for (i = 0; i < Base.NetBase.size (); i++)
	{
		if (!Base.NetBase[i]->connected ())
		{
			break;
		}
	}
	if (i == Base.NetBase.size ())
	{
		CCallbackClient *cc = new CCallbackClient( _RecordingState, name+string(".nmr") );
		Base.NetBase.push_back (cc);
	}

	CCallbackClient *cc = dynamic_cast<CCallbackClient *>(Base.NetBase[i]);

	cc->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &Base);

	try
	{
		cc->connect (Addr);

		if (Base.ConnectionCallback != NULL)
			Base.ConnectionCallback (Base.Name, cc->getSockId(), Base.ConnectionCbArg);
	}
	catch (ESocketConnectionFailed &e)
	{
		nlinfo ("HNETL4: can't connect now (%s)", e.what ());
	}
}


void RegistrationBroadcast (const std::string &name, TServiceId sid, const vector<CInetAddress> &addr)
{
	nldebug("HNETL4: RegistrationBroadcast() of service %s-%hu", name.c_str (), (uint16)sid.get());

	// find if this new service is interesting
	for (CNetManager::ItBaseMap itbm = CNetManager::_BaseMap.begin (); itbm != CNetManager::_BaseMap.end (); itbm++)
	{
		if ((*itbm).second.Type == CBaseStruct::Client && !(*itbm).second.NetBase[0]->connected())
		{
			if (name == (*itbm).first)
			{
				// ok! it's cool, the service is here, go and connect to him!
// ace warning don't work if more than one connection
				CNetManager::createConnection ((*itbm).second, addr[0], name);
			}
		}
		else if ((*itbm).second.Type == CBaseStruct::Group)
		{
			// ok, it's a group, try to see if it wants this!
			for (uint i = 0; i < (*itbm).second.ServiceNames.size (); i++)
			{
				if ((*itbm).second.ServiceNames[i] == name)
				{
// ace warning don't work if more than one connection
					CNetManager::createConnection ((*itbm).second, addr[0], name);
					break;
				}
			}
		}
	}

}

static void UnregistrationBroadcast (const std::string &name, TServiceId sid, const vector<CInetAddress> &addr)
{
	nldebug("HNETL4: UnregistrationBroadcast() of service %s-%hu", name.c_str (), (uint16)sid.get());
}

void CNetManager::init (const CInetAddress *addr, CCallbackNetBase::TRecordingState rec )
{
	if (addr == NULL) return;

	_RecordingState = rec;

	// connect to the naming service (may generate a ESocketConnectionFailed exception)

	vector<CInetAddress> laddr = CInetAddress::localAddresses();
	CNamingClient::connect( *addr, _RecordingState, laddr );

	// connect the callback to know when a new service comes in or goes down
	CNamingClient::setRegistrationBroadcastCallback (RegistrationBroadcast);
	CNamingClient::setUnregistrationBroadcastCallback (UnregistrationBroadcast);
}

void CNetManager::release ()
{
	if (CNamingClient::connected ())
		CNamingClient::disconnect ();

	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			(*itbm).second.NetBase[i]->disconnect ();
			delete (*itbm).second.NetBase[i];
		}
	}
	_BaseMap.clear ();
}

void CNetManager::addServer (const std::string &serviceName, uint16 servicePort, bool external)
{
	TServiceId sid;
	addServer (serviceName, servicePort, sid, external);
}

void CNetManager::addServer (const std::string &serviceName, uint16 servicePort, TServiceId &sid, bool external)
{
	nldebug ("HNETL4: Adding server '%s' in CNetManager", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);

	// check if it's a new server
	nlassert ((*itbm).second.NetBase.empty());

	CCallbackServer *cs = new CCallbackServer( _RecordingState, serviceName+string(".nmr") );
	(*itbm).second.NetBase.push_back (cs);

	(*itbm).second.Type = CBaseStruct::Server;

	// install the server

	cs->setConnectionCallback (nmNewConnection, (void*) &((*itbm).second));
	cs->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &((*itbm).second));

	if (servicePort == 0)
	{
		nlassert (CNamingClient::connected ());
		servicePort = CNamingClient::queryServicePort ();
	}

	cs->init (servicePort);

	// register the server to the naming service if we are connected to Naming Service

	if (CNamingClient::connected () && !external)
	{
		//CInetAddress addr = CInetAddress::localHost ();
		//addr.setPort (servicePort);
		vector<CInetAddress> addr = CInetAddress::localAddresses();
		for (uint i = 0; i < addr.size(); i++)
			addr[i].setPort(servicePort);

		if (sid.get() == 0)
		{
			CNamingClient::registerService (serviceName, addr, sid);
		}
		else
		{
			CNamingClient::registerServiceWithSId (serviceName, addr, sid);
		}
	}
	nlinfo ("HNETL4: Server '%s' added, registered and listen to port %hu", serviceName.c_str (), servicePort);
}


void CNetManager::addClient (const std::string &serviceName, const std::string &addr, bool autoRetry)
{
	nldebug ("HNETL4: Adding client '%s' with addr '%s' in CNetManager", serviceName.c_str (), addr.c_str());
	ItBaseMap itbm = find (serviceName);

	// it's a new client, add the connection
	(*itbm).second.Type = CBaseStruct::ClientWithAddr;
	(*itbm).second.AutoRetry = autoRetry;

	if ((*itbm).second.ServiceNames.empty())
	{
		(*itbm).second.ServiceNames.push_back(addr);
	}
	else
	{
		(*itbm).second.ServiceNames[0] = addr;
	}

	nlassert ((*itbm).second.NetBase.size() < 2);

	createConnection ((*itbm).second, addr, serviceName);
}


void CNetManager::addClient (const std::string &serviceName)
{
	nlassert (CNamingClient::connected ());
	nldebug ("HNETL4: Adding client '%s' in CNetManager", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);

	// check if it's a new client
	nlassert ((*itbm).second.NetBase.empty());

	CCallbackClient *cc = new CCallbackClient( _RecordingState, serviceName+string(".nmr") ); // ? - would not work if several clients with the same name
	(*itbm).second.NetBase.push_back (cc);

	(*itbm).second.Type = CBaseStruct::Client;

	cc->CCallbackNetBase::setDisconnectionCallback (nmNewDisconnection, (void*) &((*itbm).second));

	// find the service in the naming_service and connect if exists
	if (CNamingClient::lookupAndConnect (serviceName, *cc))
	{
		// call the user that we are connected
		if ((*itbm).second.ConnectionCallback != NULL)
			(*itbm).second.ConnectionCallback (serviceName, cc->getSockId(), (*itbm).second.ConnectionCbArg);
	}
}



void CNetManager::addGroup (const std::string &groupName, const std::string &serviceName)
{
	nlassert (CNamingClient::connected ());
	nldebug ("HNETL4: Adding '%s' to group '%s' in CNetManager", serviceName.c_str (), groupName.c_str());
	ItBaseMap itbm = find (groupName);

	(*itbm).second.Type = CBaseStruct::Group;

	// check if you don't already add this service in this group
	vector<string>::iterator it = std::find ((*itbm).second.ServiceNames.begin(), (*itbm).second.ServiceNames.end(), serviceName);
	nlassert (it == (*itbm).second.ServiceNames.end());

	(*itbm).second.ServiceNames.push_back(serviceName);


	// find the service in the naming_service and connect if exists
	vector<CInetAddress> addrs;
	CNamingClient::lookupAll (serviceName, addrs);

	// connect to all these services
	for (uint i = 0; i < addrs.size (); i++)
	{
		createConnection ((*itbm).second, addrs[i], serviceName);
	}
}


void CNetManager::addCallbackArray (const std::string &serviceName, const TCallbackItem *callbackarray, CStringIdArray::TStringId arraysize)
{
	nldebug ("HNETL4: addingCallabckArray() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
	{
//		if ((*itbm).second.NetBase[i]->connected())
		(*itbm).second.NetBase[i]->addCallbackArray (callbackarray, arraysize);
	}
}

void CNetManager::update (TTime timeout)
{
//	nldebug ("HNETL4: update()");

//	sint64 p1 = CTime::getPerformanceTime ();

	TTime t0 = CTime::getLocalTime ();

	if (timeout > 0)
	{
		if (_NextUpdateTime == 0)
		{
			_NextUpdateTime = t0 + timeout;
		}
		else
		{
			TTime err = t0 - _NextUpdateTime;
			_NextUpdateTime += timeout;

			// if we are too late, resync to the next value
			while (err > timeout)
			{
				err -= timeout;
				_NextUpdateTime += timeout;
			}

			timeout -= err;
			if (timeout < 0) timeout = 0;
		}
	}

//	sint64 p2 = CTime::getPerformanceTime ();

	while (true)
	{
		for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
		{
			for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
			{
				// we get and treat all messages in this connection
				(*itbm).second.NetBase[i]->update (0);
				if ((*itbm).second.NetBase[i]->connected())
				{
					// if connected, update
//					(*itbm).second.NetBase[i]->update ();
				}
				else
				{
					static TTime lastTime = CTime::getLocalTime();
					if (CTime::getLocalTime() - lastTime > 5000)
					{
						lastTime = CTime::getLocalTime();

						// if not connected, try to connect ClientWithAddr
						if ((*itbm).second.Type == CBaseStruct::ClientWithAddr && (*itbm).second.AutoRetry)
						{
							CCallbackClient *cc = dynamic_cast<CCallbackClient *>((*itbm).second.NetBase[i]);
							try
							{
								nlassert ((*itbm).second.ServiceNames.size()==1);
								cc->connect (CInetAddress((*itbm).second.ServiceNames[0]));

								if ((*itbm).second.ConnectionCallback != NULL)
									(*itbm).second.ConnectionCallback ((*itbm).second.Name, cc->getSockId(), (*itbm).second.ConnectionCbArg);
							}
							catch (ESocketConnectionFailed &e)
							{
								// can't connect now, try later
								nlinfo("HNETL4: can't connect now to %s (reason: %s)", (*itbm).second.ServiceNames[0].c_str(), e.what());
							}
						}
					}
				}
			}
		}

		// If it's the end, don't nlSleep()
		if (CTime::getLocalTime() - t0 > timeout)
			break;

		// Enable windows multithreading before rescanning all connections
		// slow down the layer H_BEFORE (CNetManager_update_nlSleep);
		nlSleep (1);
		// slow down the layer H_AFTER (CNetManager_update_nlSleep);
	}

//	sint64 p3 = CTime::getPerformanceTime ();

	if (CNamingClient::connected ())
		CNamingClient::update ();

//	sint64 p4 = CTime::getPerformanceTime ();

//	nlinfo("time : %f %f %f %d", CTime::ticksToSecond(p2-p1), CTime::ticksToSecond(p3-p2), CTime::ticksToSecond(p4-p3), timeout);
}


void CNetManager::send (const std::string &serviceName, const CMessage &buffer, TSockId hostid)
{
	nldebug ("HNETL4: send for service '%s' message %s to %s", serviceName.c_str(), buffer.toString().c_str(), hostid->asString().c_str());
	ItBaseMap itbm = find (serviceName);
	for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
	{
		if ((*itbm).second.NetBase[i]->connected())
			(*itbm).second.NetBase[i]->send (buffer, hostid);
	}
}

CCallbackNetBase *CNetManager::getNetBase (const std::string &serviceName)
{
	ItBaseMap itbm = find (serviceName);
	return (*itbm).second.NetBase[0];
}

void CNetManager::setConnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg)
{
	nldebug ("HNETL4: setConnectionCallback() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	(*itbm).second.ConnectionCallback = cb;
	(*itbm).second.ConnectionCbArg = arg;
}

void CNetManager::setDisconnectionCallback (const std::string &serviceName, TNetManagerCallback cb, void *arg)
{
	nldebug ("HNETL4: setDisconnectionCallback() for service '%s'", serviceName.c_str ());
	ItBaseMap itbm = find (serviceName);
	(*itbm).second.DisconnectionCallback = cb;
	(*itbm).second.DisconnectionCbArg = arg;
}


CNetManager::ItBaseMap CNetManager::find (const std::string &serviceName)
{
	// find the service or add it if not found
	pair<ItBaseMap, bool> p;
	p = _BaseMap.insert (make_pair (serviceName, CBaseStruct (serviceName)));
	return p.first;
}

uint64 CNetManager::getBytesSent ()
{
	uint64 sent = 0;
	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			sent += (*itbm).second.NetBase[i]->getBytesSent ();
		}
	}
	return sent;
}

uint64 CNetManager::getBytesReceived ()
{
	uint64 received = 0;
	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			received += (*itbm).second.NetBase[i]->getBytesReceived ();
		}
	}
	return received;
}

uint64 CNetManager::getSendQueueSize ()
{
	uint64 val = 0;
	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			val += (*itbm).second.NetBase[i]->getSendQueueSize ();
		}
	}
	return val;
}

uint64 CNetManager::getReceiveQueueSize ()
{
	uint64 val = 0;
	for (ItBaseMap itbm = _BaseMap.begin (); itbm != _BaseMap.end (); itbm++)
	{
		for (uint32 i = 0; i < (*itbm).second.NetBase.size(); i++)
		{
			val += (*itbm).second.NetBase[i]->getReceiveQueueSize ();
		}
	}
	return val;
}

} // NLNET
