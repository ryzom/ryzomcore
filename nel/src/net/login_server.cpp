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

#include "nel/net/callback_client.h"
#include "nel/net/service.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_server.h"

#include "nel/net/udp_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

struct CPendingUser
{
	CPendingUser (const CLoginCookie &cookie, const string &un, const string &up, const string &ux, uint32 instanceId, uint32 charSlot)
		: Cookie (cookie), UserName(un), UserPriv(up), UserExtended(ux), InstanceId(instanceId), CharSlot(charSlot)
	{
		Time = CTime::getSecondsSince1970();
	}

	CLoginCookie Cookie;
	string UserName;
	string UserPriv;		// privilege for executing commands from the clients
	string UserExtended;	// extended data (for free use)
	uint32 InstanceId;		// the world instance in witch the user is awaited
	uint32 CharSlot;		// the expected character slot, any other will be denied
	uint32 Time;			// when the cookie is inserted in pending list
};

static list<CPendingUser> PendingUsers;

static CCallbackServer *Server = NULL;
static string ListenAddr;

static bool AcceptInvalidCookie = false;

static string DefaultUserPriv;

static TDisconnectClientCallback DisconnectClientCallback = NULL;

// true=tcp   false=udp
static bool ModeTcp = 0;

// default value is 15 minutes
static uint TimeBeforeEraseCookie = 15*60;

/// contains the correspondance between userid and the sockid
map<uint32, TSockId> UserIdSockAssociations;

TNewClientCallback NewClientCallback = NULL;

TNewCookieCallback NewCookieCallback = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void	notifyWSRemovedPendingCookie(CLoginCookie& cookie)
{
	CMessage msgout ("RPC");	// remove pending cookie
	msgout.serial(cookie);
	CUnifiedNetwork::getInstance()->send ("WS", msgout);
}

void CLoginServer::refreshPendingList ()
{
	// delete too old cookie

	list<CPendingUser>::iterator it = PendingUsers.begin();
	uint32 Time = CTime::getSecondsSince1970();
	while (it != PendingUsers.end ())
	{
		if ((*it).Time < Time - TimeBeforeEraseCookie)
		{
			nlinfo("LS: Removing cookie '%s' because too old", (*it).Cookie.toString().c_str());
			notifyWSRemovedPendingCookie((*it).Cookie);
			it = PendingUsers.erase (it);
		}
		else
		{
			it++;
		}
	}
}


void cbWSChooseShard (CMessage &msgin, const std::string &/* serviceName */, TServiceId /* sid */)
{
	// the WS call me that a new client want to come in my shard
	string reason, userName, userPriv, userExtended;
	uint32 instanceId, charSlot;
	CLoginCookie cookie;

//	refreshPendingList ();

	//
	// S08: receive "CS" message from WS and send "SCS" message to WS
	//

	msgin.serial (cookie);
	msgin.serial (userName, userPriv, userExtended);
	msgin.serial (instanceId);
	msgin.serial (charSlot);

	vector<list<CPendingUser>::iterator> pendingToRemove;
	list<CPendingUser>::iterator it;
	for (it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		const CPendingUser &pu = *it;
		if (pu.Cookie == cookie)
		{
			// the cookie already exists, erase it and return false
			nlwarning ("LS: Cookie %s is already in the pending user list", cookie.toString().c_str());
//			notifyWSRemovedPendingCookie((*it).Cookie);
			PendingUsers.erase (it);
			reason = "cookie already exists";
			// iterator is invalid, set it to end
			it = PendingUsers.end();
			break;
		}
		else if (pu.Cookie.getUserId() == cookie.getUserId())
		{
			// we already have a cookie for this user, remove the old one
			pendingToRemove.push_back(it);
		}
	}
	// remove any useless cookie
	while (!pendingToRemove.empty())
	{
		PendingUsers.erase(pendingToRemove.back());
		pendingToRemove.pop_back();
	}

	if (it == PendingUsers.end ())
	{
		// add it to the awaiting client
		nlinfo ("LS: New cookie %s (name '%s' priv '%s' extended '%s' instance %u slot %u) inserted in the pending user list (awaiting new client)", cookie.toString().c_str(), userName.c_str(), userPriv.c_str(), userExtended.c_str(), instanceId, charSlot);
		PendingUsers.push_back (CPendingUser (cookie, userName, userPriv, userExtended, instanceId, charSlot));
		reason.clear();

		// callback if needed
		if (NewCookieCallback != NULL)
		{
			NewCookieCallback(cookie);
		}
	}

	CMessage msgout ("SCS");
	msgout.serial (reason);
	msgout.serial (cookie);
	msgout.serial (ListenAddr);
	uint32 nbPending = (uint32)PendingUsers.size();
	msgout.serial (nbPending);
	CUnifiedNetwork::getInstance()->send ("WS", msgout);
}

void cbWSDisconnectClient (CMessage &msgin, const std::string &serviceName, TServiceId /* sid */)
{
	// the WS tells me that i have to disconnect a client

	uint32 userid;
	msgin.serial (userid);

	if (ModeTcp)
	{
		map<uint32, TSockId>::iterator it = UserIdSockAssociations.find (userid);
		if (it == UserIdSockAssociations.end ())
		{
			nlwarning ("LS: Can't disconnect the user %d, he is not found", userid);
		}
		else
		{
			nlinfo ("LS: Disconnect the user %d", userid);
			Server->disconnect ((*it).second);
		}
	}

	if (DisconnectClientCallback != NULL)
	{
		DisconnectClientCallback (userid, serviceName);
	}
}

static TUnifiedCallbackItem WSCallbackArray[] =
{
	{ "CS", cbWSChooseShard },
	{ "DC", cbWSDisconnectClient },
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE CLIENT ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void cbShardValidation (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S13: receive "SV" message from the client
	//

	// the client send me a cookie
	CLoginCookie cookie;
	string reason;
	msgin.serial (cookie);

	string userName, userPriv, userExtended;
	uint32 instanceId, charSlot;
	// verify that the user was pending
	reason = CLoginServer::isValidCookie (cookie, userName, userPriv, userExtended, instanceId, charSlot);

	// if the cookie is not valid and we accept them, clear the error
	if(AcceptInvalidCookie && !reason.empty())
	{
		reason.clear();
		cookie.set (rand(), rand(), rand());
	}

	CMessage msgout2 ("SV");
	msgout2.serial (reason);
	netbase.send (msgout2, from);

	if (!reason.empty())
	{
		nlwarning ("LS: User (%s) is not in the pending user list (cookie:%s)", netbase.hostAddress(from).asString().c_str(), cookie.toString().c_str());
		// disconnect him
		netbase.disconnect (from);
	}
	else
	{
		// add the user association
		uint32 userid = cookie.getUserId();

		if (ModeTcp)
			UserIdSockAssociations.insert (make_pair(userid, from));

		// identification OK, let's call the user callback
		if (NewClientCallback != NULL)
			NewClientCallback (from, cookie);

		// ok, now, he can call all callback
		Server->authorizeOnly (NULL, from);
	}
}

void ClientConnection (TSockId from, void * /* arg */)
{
	nldebug("LS: new client connection: %s", from->asString ().c_str ());

	// the client could only call "SV" message
	Server->authorizeOnly ("SV", from);
}


static const TCallbackItem ClientCallbackArray[] =
{
	{ "SV", cbShardValidation },
};

void CLoginServer::setListenAddress(const string &la)
{
	// if the var is empty or not found, take it from the listenAddress()
	if (la.empty() && ModeTcp && Server != NULL)
	{
		ListenAddr = Server->listenAddress ().asIPString();
	}
	else
	{
		ListenAddr = la;
	}

	// check that listen address is valid
	if (ListenAddr.empty())
	{
		nlerror("FATAL : listen address in invalid, it should be either set via ListenAddress variable or with -D argument");
		nlstop;
	}

	nlinfo("LS: Listen Address that will be sent to the client is now '%s'", ListenAddr.c_str());
}

uint32 CLoginServer::getNbPendingUsers()
{
	return (uint32)PendingUsers.size();
}

void cfcbListenAddress (CConfigFile::CVar &var)
{
	CLoginServer::setListenAddress (var.asString());
}

void cfcbDefaultUserPriv(CConfigFile::CVar &var)
{
	// set the new ListenAddr
	DefaultUserPriv = var.asString();

	nlinfo("LS: The default user priv is '%s'", DefaultUserPriv.c_str());
}

void cfcbAcceptInvalidCookie(CConfigFile::CVar &var)
{
	// set the new ListenAddr
	AcceptInvalidCookie = var.asInt() == 1;

	nlinfo("LS: This service %saccept invalid cookie", AcceptInvalidCookie?"":"doesn't ");
}

void cfcbTimeBeforeEraseCookie(CConfigFile::CVar &var)
{
	// set the new ListenAddr
	TimeBeforeEraseCookie = var.asInt();

	nlinfo("LS: This service will remove cookie after %d seconds", TimeBeforeEraseCookie);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CONNECTION TO THE WELCOME SERVICE //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

// common init
void CLoginServer::init (const string &listenAddress)
{
	// connect to the welcome service
	connectToWS ();

	try {
		cfcbDefaultUserPriv(IService::getInstance()->ConfigFile.getVar("DefaultUserPriv"));
		IService::getInstance()->ConfigFile.setCallback("DefaultUserPriv", cfcbDefaultUserPriv);
	} catch(const Exception &) { }

	try {
		cfcbAcceptInvalidCookie (IService::getInstance()->ConfigFile.getVar("AcceptInvalidCookie"));
		IService::getInstance()->ConfigFile.setCallback("AcceptInvalidCookie", cfcbAcceptInvalidCookie);
	} catch(const Exception &) { }

	try {
		cfcbTimeBeforeEraseCookie (IService::getInstance()->ConfigFile.getVar("TimeBeforeEraseCookie"));
		IService::getInstance()->ConfigFile.setCallback("TimeBeforeEraseCookie", cfcbTimeBeforeEraseCookie);
	} catch(const Exception &) { }

	// setup the listen address

	string la;

	if (IService::getInstance()->haveArg('D'))
	{
		// use the command line param if set
		la = IService::getInstance()->getArg('D');
	}
	else if (IService::getInstance()->ConfigFile.exists ("ListenAddress"))
	{
		// use the config file param if set
		la = IService::getInstance()->ConfigFile.getVar ("ListenAddress").asString();
	}
	else
	{
		la = listenAddress;
	}
	setListenAddress (la);
	IService::getInstance()->ConfigFile.setCallback("ListenAddress", cfcbListenAddress);
}

// listen socket is TCP
void CLoginServer::init (CCallbackServer &server, TNewClientCallback ncl)
{
	init (server.listenAddress ().asIPString());

	// add callback to the server
	server.addCallbackArray (ClientCallbackArray, sizeof (ClientCallbackArray) / sizeof (ClientCallbackArray[0]));
	server.setConnectionCallback (ClientConnection, NULL);

	NewClientCallback = ncl;
	Server = &server;

	ModeTcp = true;
}

// listen socket is UDP
void CLoginServer::init (CUdpSock &server, TDisconnectClientCallback dc)
{
	init (server.localAddr ().asIPString());

	DisconnectClientCallback = dc;

	ModeTcp = false;
}

void CLoginServer::init (const std::string &listenAddr, TDisconnectClientCallback dc)
{
	init (listenAddr);

	DisconnectClientCallback = dc;

	ModeTcp = false;
}

void CLoginServer::addNewCookieCallback(TNewCookieCallback newCookieCb)
{
	NewCookieCallback = newCookieCb;
}


string CLoginServer::isValidCookie (const CLoginCookie &lc, string &userName, string &userPriv, string &userExtended, uint32 &instanceId, uint32 &charSlot)
{
	userName.clear();
	userPriv.clear();

	if (!AcceptInvalidCookie && !lc.isValid())
		return "The cookie is invalid";

	// verify that the user was pending
	list<CPendingUser>::iterator it;
	for (it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		CPendingUser &pu = *it;
		if (pu.Cookie == lc)
		{
			nlinfo ("LS: Cookie '%s' is valid and pending (user %s), send the client connection to the WS", lc.toString ().c_str (), pu.UserName.c_str());

			// warn the WS that the client effectively connected
			uint8 con = 1;
			CMessage msgout ("CC");
			uint32 userid = lc.getUserId();
			msgout.serial (userid);
			msgout.serial (con);

			CUnifiedNetwork::getInstance()->send("WS", msgout);

			userName = pu.UserName;
			userPriv = pu.UserPriv;
			userExtended = pu.UserExtended;
			instanceId = pu.InstanceId;
			charSlot = pu.CharSlot;

			// ok, it was validate, remove it
			PendingUsers.erase (it);

			return "";
		}
	}

	// we accept invalid cookie and it is one, fake
	if (AcceptInvalidCookie)
	{
		userName = "InvalidUserName";
		userPriv = DefaultUserPriv;
		instanceId = 0xffffffff;
		return "";
	}

	// problem
	return "I didn't receive the cookie from WS";
}

void CLoginServer::connectToWS ()
{
	CUnifiedNetwork::getInstance()->addCallbackArray(WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));
}

void CLoginServer::clientDisconnected (uint32 userId)
{
	uint8 con = 0;
	CMessage msgout ("CC");
	msgout.serial (userId);
	msgout.serial (con);

	CUnifiedNetwork::getInstance()->send("WS", msgout);

	// remove the user association
	if (ModeTcp)
		UserIdSockAssociations.erase (userId);
}

/// Call this method to retrieve the listen address
const std::string &CLoginServer::getListenAddress()
{
	return ListenAddr;
}

bool CLoginServer::acceptsInvalidCookie()
{
	return AcceptInvalidCookie;
}


//
// Commands
//

NLMISC_CATEGORISED_COMMAND(nel, lsUsers, "displays the list of all connected users", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	if (ModeTcp)
	{
		log.displayNL ("Display the %d connected users :", UserIdSockAssociations.size());
		for (map<uint32, TSockId>::iterator it = UserIdSockAssociations.begin(); it != UserIdSockAssociations.end (); it++)
		{
			log.displayNL ("> %u %s", (*it).first, (*it).second->asString().c_str());
		}
		log.displayNL ("End of the list");
	}
	else
	{
		log.displayNL ("No user list in udp mode");
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, lsPending, "displays the list of all pending users", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	log.displayNL ("Display the %d pending users :", PendingUsers.size());
	for (list<CPendingUser>::iterator it = PendingUsers.begin(); it != PendingUsers.end (); it++)
	{
		log.displayNL ("> %s %s", (*it).Cookie.toString().c_str(), (*it).UserName.c_str());
	}
	log.displayNL ("End of the list");

	return true;
}


NLMISC_CATEGORISED_DYNVARIABLE(nel, string, LSListenAddress, "the listen address sended to the client to connect on this front_end")
{
	nlunreferenced(human);

	if (get)
	{
		*pointer = ListenAddr;
	}
	else
	{
		if ((*pointer).find (":") == string::npos)
		{
			nlwarning ("LS: You must set the address + port (ie: \"itsalive.nevrax.org:38000\")");
			return;
		}
		else if ((*pointer).empty())
		{
			ListenAddr = Server->listenAddress ().asIPString();
		}
		else
		{
			ListenAddr = *pointer;
		}
		nlinfo ("LS: Listen Address that will be send to client is '%s'", ListenAddr.c_str());
	}
}

NLMISC_CATEGORISED_VARIABLE(nel, string, DefaultUserPriv, "Default User priv for people who don't use the login system");

} // NLNET
