// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <nel/misc/debug.h>
#include <nel/misc/config_file.h>
#include <nel/misc/displayer.h>
#include <nel/misc/log.h>
#include <nel/misc/wang_hash.h>

#include <nel/net/buf_server.h>
#include <nel/net/login_cookie.h>

#include <nelns/login_service/connection_ws.h>
#include <nelns/login_service/functions.h>
#include <nelns/login_service/login_service.h>
#include <nelns/login_service/variables.h>
#include <nelns/login_service/mysql_helper.h>

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

// uint32 is the connection id of the web connection (see allocateWebConnectionId)
map<uint32, CLoginCookie> TempCookies;

// The cookie's address field carries a per-connection id, and the reply
// routing looks the socket up again through these maps. Storing the TSockId
// pointer itself in the 32 bit field truncated it on 64 bit builds, and a
// stale value could match another connection.
static uint32 WebConnectionIdCounter = 0;
static map<uint32, TSockId> WebIdToSock;
static map<TSockId, uint32> WebSockToId;

static uint32 webConnectionId(TSockId from)
{
	map<TSockId, uint32>::iterator it = WebSockToId.find(from);
	return it != WebSockToId.end() ? it->second : 0;
}

static TSockId webConnectionById(uint32 id)
{
	map<uint32, TSockId>::iterator it = WebIdToSock.find(id);
	return it != WebIdToSock.end() ? it->second : InvalidSockId;
}

static uint32 allocateWebConnectionId(TSockId from)
{
	// one id per connection, reuse it if this socket already has one
	uint32 id = webConnectionId(from);
	if (id != 0)
		return id;
	// hash the counter so consecutive connections don't hand out consecutive
	// ids: keys spread over the whole 32 bit range (kinder to map
	// implementations and nothing to guess from), and lowbias32 is a
	// bijection so a colliding live id means a full 2^32 wrap
	do
	{
		id = NLMISC::lowbias32(++WebConnectionIdCounter);
	} while (id == 0 || WebIdToSock.find(id) != WebIdToSock.end());
	WebIdToSock[id] = from;
	WebSockToId[from] = id;
	return id;
}

static void cbWebDisconnection(TSockId from, void *arg)
{
	map<TSockId, uint32>::iterator it = WebSockToId.find(from);
	if (it == WebSockToId.end())
		return;
	// forget any cookie still waiting for a WS answer on this connection
	TempCookies.erase(it->second);
	WebIdToSock.erase(it->second);
	WebSockToId.erase(it);
}


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

	// this cookie is answered, it is no longer waiting
	TempCookies.erase (it);

	if (!reason.empty())
	{
		// the WS refused the user: the row was set to 'Waiting' when the
		// request went out, put it back offline
		sqlQuery("update user set State='Offline', ShardId=-1, Cookie='' where Cookie='" + sqlEscape(cookie.setToString()) + "'");
	}
	else
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

	TSockId to = webConnectionById (cookie.getUserAddr ());
	if (to == InvalidSockId)
	{
		// the web request is gone (php timed out or closed): nobody will
		// ever present this cookie to the shard, put the row back offline
		nlinfo ("SCS from WS for cookie %s but the web connection is gone, logging the user out", cookie.toString ().c_str ());
		sqlQuery("update user set State='Offline', ShardId=-1, Cookie='' where Cookie='" + sqlEscape(cookie.setToString()) + "'");
		return;
	}
	WebServer->send (msgout, to);
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

	// The web path has to drive the same state machine as the direct client
	// path, or the WS 'CC' confirmation finds an 'Offline' row and refuses
	// to mark the player online (no presence tracking, no double login
	// protection). Same rules as the direct path: 'Online' refuses, a stale
	// mid-login state is reclaimed, and the row goes to 'Waiting' with the
	// cookie before the request reaches the welcome service.
	{
		CMysqlResult result;
		MYSQL_ROW row;
		sint32 nbrow;
		string reason = sqlQuery("select State from user where UId="+toString(userId), nbrow, row, result);
		if (!reason.empty() || nbrow == 0)
		{
			CMemStream msgout;
			uint32 fake = 0;
			msgout.serial(fake);
			string answer = reason.empty() ? string("Unknown user") : string("Database error");
			nlwarning("Web asked to connect userid %d but: %s", userId, answer.c_str());
			msgout.serial (answer);
			WebServer->send (msgout, host);
			return;
		}
		string state = row[0] ? row[0] : "";
		if (state == "Online")
		{
			CMemStream msgout;
			uint32 fake = 0;
			msgout.serial(fake);
			string answer = "You are already connected.";
			msgout.serial (answer);
			WebServer->send (msgout, host);
			return;
		}
		else if (state != "Offline")
		{
			// a login that never finished; a user actually playing is
			// 'Online', so reclaim the row and let this login proceed
			nlinfo("user %d was stuck in state '%s', reclaiming the row for a new login", userId, state.c_str());
			sqlQuery("update user set State='Offline', ShardId=-1, Cookie='' where UId="+toString(userId));
		}
	}

	uint32 i;
	for (i = 0; i < Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
		{
			// generate a cookie carrying this connection's id
			CLoginCookie Cookie (allocateWebConnectionId(host), userId);

			// enter the login state machine so the WS confirmation can move
			// the row to 'Online' when the client reaches the shard
			string reason = sqlQuery("update user set State='Waiting', ShardId="+toString(shardId)+", Cookie='"+sqlEscape(Cookie.setToString())+"' where UId="+toString(userId));
			if (!reason.empty())
			{
				CMemStream msgout;
				uint32 fake = 0;
				msgout.serial(fake);
				string answer = "Database error";
				msgout.serial (answer);
				WebServer->send (msgout, host);
				return;
			}

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
	// clean up the connection id and any cookie still waiting for a WS
	// answer when a web request drops its connection
	WebServer->setDisconnectionCallback (cbWebDisconnection, NULL);

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
