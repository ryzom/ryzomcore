// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/misc/types_nl.h"

#include <cstdio>
#include <ctype.h>
#include <cmath>

#include <vector>
#include <map>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/buf_server.h"
#include "nel/net/login_cookie.h"

#include "login_service.h"
#include "connection_ws.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Variables
//

CBufServer *WebServer = NULL;

// uint32 is the hostid to the web connection
map<uint32, CLoginCookie> TempCookies;


//
// Callbacks
//

static void cbWSShardChooseShard/* (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)*/ (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	nlassert(WebServer != NULL);

	//
	// S10: receive "SCS" message from WS
	//

	CMemStream msgout;
	uint32 fake = 0;
	msgout.serial(fake);

	string reason;
	msgin.serial (reason);
	msgout.serial (reason);
	
	CLoginCookie cookie;
	msgin.serial (cookie);

	// search the cookie
	map<uint32, CLoginCookie>::iterator it = TempCookies.find (cookie.getUserAddr ());

	if (it == TempCookies.end ())
	{
		// not found in TempCookies, can't do anything
		nlwarning ("Receive an answer from welcome service but no connection waiting");
		return;
	}

	if (reason.empty())
	{
		string str = cookie.setToString ();
		msgout.serial (str);

		string addr;
		msgin.serial (addr);
		msgout.serial (addr);

		// MTR: No longer sent by WS?
		//uint32 nbPendingUser;
		//msgin.serial(nbPendingUser);

		// read patch addresses sent by WS
		/*
		// OBSOLETE: web doesn't read incoming patching URLs any longer, but them directly from database
		std::string	patchURLS;
		try
		{
			msgin.serial(patchURLS);
		}
		catch (Exception&)
		{
			patchURLS.clear();
		}

		msgout.serial(patchURLS);
		*/
	}

	WebServer->send (msgout, (TSockId)cookie.getUserAddr ());  // FIXME: 64-bit
}

static const TUnifiedCallbackItem WSCallbackArray[] =
{
	{ "SCS", cbWSShardChooseShard },
};

void cbAskClientConnection (CMemStream &msgin, TSockId host)
{
	sint32 shardId;
	uint32 userId;
	string userName, userPriv, userExtended;
	msgin.serial (shardId);
	msgin.serial (userId);
	msgin.serial (userName);

	try
	{
		msgin.serial (userPriv);
	}
	catch (Exception &)
	{
		nlwarning ("Web didn't give me the user privilege for user '%s', set to empty", userName.c_str());
	}

	try
	{
		msgin.serial (userExtended);
	}
	catch (Exception &)
	{
		nlwarning ("Web didn't give me the extended data for user '%s', set to empty", userName.c_str());
	}

	nlinfo ("Web wants to add userid %d (name '%s' priv '%s' extended '%s') to the shardid %d, send request to the shard", userId, userName.c_str(), userPriv.c_str(), userExtended.c_str(), shardId);

	uint32 i;
	for (i = 0; i < Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
		{
			// generate a cookie
			CLoginCookie Cookie ((uint32)(uintptr_t)host, userId);

			// send message to the welcome service to see if it s ok and know the front end ip
			CMessage msgout ("CS");
			msgout.serial (Cookie);
			msgout.serial (userName, userPriv, userExtended);
			//WSServer->send (msgout, Shards[i].SockId);
			CUnifiedNetwork::getInstance ()->send (Shards[i].SId, msgout);
			beep (1000, 1, 100, 100);

			// add the connection in temp cookie
			TempCookies.insert(make_pair(Cookie.getUserAddr(), Cookie));
			return;
		}
	}

	// the shard is not available, denied the user
	nlwarning("ShardId %d is not available, can't add the userid %d", shardId, userId);

	CMemStream msgout;
	uint32 fake = 0;
	msgout.serial(fake);
	string reason = "Selected shard is not available";
	msgout.serial (reason);
	WebServer->send (msgout, host);
}

void cbDisconnectClient (CMemStream &msgin, TSockId host)
{
	sint32 shardId;
	sint32 userId;
	msgin.serial (shardId);
	msgin.serial (userId);

	nlinfo ("Web wants to disconnect userid %d, send request to the shard %d", userId, shardId);

	for (uint i = 0; i < Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
		{
			// ask the WS to disconnect the player from the shard
			CMessage msgout ("DC");
			msgout.serial (userId);
			//WSServer->send (msgout, Shards[i].SockId);
			CUnifiedNetwork::getInstance ()->send (Shards[i].SId, msgout);

			// send answer to the web
			CMemStream msgout2;
			uint32 fake = 0;
			msgout2.serial(fake);
			string reason = "";
			msgout2.serial (reason);
			WebServer->send (msgout2, host);
			return;
		}
	}

	nlwarning("ShardId %d is not available, can't disconnect the userid %d", shardId, userId);

	CMemStream msgout;
	uint32 fake = 0;
	msgout.serial(fake);
	string reason = "ShardId "+toString(shardId)+"is not available, can't disconnect the userid"+toString(userId);
	msgout.serial (reason);
	WebServer->send (msgout, host);
}

typedef void (*WebCallback)(CMemStream &msgin, TSockId host);

WebCallback WebCallbackArray[] = {
	cbAskClientConnection,
	cbDisconnectClient
};

//
// Functions
//

void connectionWebInit ()
{
	nlassert(WebServer == NULL);

	WebServer = new CBufServer ();
	nlassert(WebServer != NULL);

	uint16 port = (uint16) IService::getInstance ()->ConfigFile.getVar ("WebPort").asInt();
	WebServer->init (port);

	// catch the messages from Welcome Service to know if the user can connect or not
	CUnifiedNetwork::getInstance ()->addCallbackArray (WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));

	nlinfo ("Set the server connection for web to port %hu", port);
}

void connectionWebUpdate ()
{
	nlassert(WebServer != NULL);

	try
	{
		WebServer->update ();

		while (WebServer->dataAvailable ())
		{
			// create a string mem stream to easily communicate with web server
			NLMISC::CMemStream msgin (true);
			TSockId host;
			uint8 messageType = 0;

			try
			{
				WebServer->receive (msgin, &host);
				uint32 fake = 0;
				msgin.serial(fake);

				msgin.serial (messageType);
			}
			catch (Exception &e)
			{
				nlwarning ("Error during receiving: '%s'", e.what ());
			}

			if(messageType<sizeof(WebCallbackArray)/sizeof(WebCallbackArray[0]))
			{
				WebCallbackArray[messageType](msgin, host);
			}
			else
			{
				nlwarning ("Received an unknown message type %d from web server", messageType);
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}

void connectionWebRelease ()
{
	nlassert(WebServer != NULL);

	delete WebServer;
	WebServer = 0;
}
