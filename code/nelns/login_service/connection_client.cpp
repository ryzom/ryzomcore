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


//
// Includes
//

#include <nel/misc/types_nl.h>

#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include <map>
#include <vector>

#include <nel/misc/log.h>
#include <nel/misc/debug.h>
#include <nel/misc/displayer.h>
#include <nel/misc/config_file.h>

#include <nel/net/service.h>
#include <nel/net/login_cookie.h>

#include "login_service.h"
#include "mysql_helper.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Variables
//

static CCallbackServer *ClientsServer = 0;


//
// Functions
//

void sendToClient(CMessage &msgout, TSockId sockId)
{
	nlassert(ClientsServer != 0);
	ClientsServer->send(msgout, sockId);
}

void string_escape(string &str)
{
	string::size_type pos = 0;
	while((pos=str.find('\'')) != string::npos)
	{
		str.replace(pos, 1, " ");
	}
}

static void cbClientVerifyLoginPassword(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S03: check the validity of the client login/password and send "VLP" message to client
	//

	// reason is empty if everything goes right or contains the reason of the failure
	string reason;
	sint32 uid = -1;
	ucstring login;
	string cpassword, application;
	msgin.serial (login);
	msgin.serial (cpassword);
	msgin.serial (application);

	breakable
	{
		CMysqlResult result;
		MYSQL_ROW row;
		sint32 nbrow;
		//const CInetAddress &ia = netbase.hostAddress (from);
retry:
		reason = sqlQuery("select * from user where Login='"+login.toUtf8()+"'", nbrow, row, result);
		if(!reason.empty()) break;

		if(nbrow == 0)
		{
			if(IService::getInstance ()->ConfigFile.getVar("AcceptUnknownUsers").asInt () == 1)
			{
				// we accept new users, add it
				string query = "insert into user (Login, Password) values ('"+login.toUtf8()+"', '"+cpassword+"')";
				reason = sqlQuery(query, nbrow, row, result);
				if (!reason.empty()) break;
				nlinfo("The user %s was inserted in the database for the application '%s'!", login.toUtf8().c_str(), application.c_str());
				goto retry;
			}
			else
			{
				reason = toString("Login '%s' doesn't exist", login.toUtf8().c_str());
				break;
			}
		}

		if(nbrow != 1)
		{
			reason = toString("Too much login '%s' exists", login.toUtf8().c_str());
			break;
		}

		// now the user is on the database

		uid = atoi(row[0]);

		if(cpassword != row[2])
		{
			reason = toString("Bad password");
			break;
		}

		if(row[4] != string("Offline"))
		{
			// 2 players are trying to play with the same id, disconnect all
			//reason = sqlQuery(string("update user set state='Offline', ShardId=-1 where UId=")+uid);
			//if(!reason.empty()) break;

			// send a message to the already connected player to disconnect
			CMessage msgout("DC");
			msgout.serial(uid);
			CUnifiedNetwork::getInstance()->send("WS", msgout);

			reason = "You are already connected.";
			break;
		}

		CLoginCookie c;
		c.set((uint32)(uintptr_t)from, rand(), uid);

		reason = sqlQuery("update user set state='Authorized', Cookie='"+c.setToString()+"' where UId=" + NLMISC::toString(uid));
		if(!reason.empty()) break;

		reason = sqlQuery("select * from shard where Online>0 and ClientApplication='"+application+"'", nbrow, row, result);
		if(!reason.empty()) break;

		// Send success message
		CMessage msgout ("VLP");
		msgout.serial(reason);
		msgout.serial(nbrow);

		// send address and name of all online shards
		while(row != 0)
		{
			// serial the name of the shard
			ucstring shardname;
			shardname.fromUtf8(row[3]);
			uint8 nbplayers = atoi(row[2]);
			uint32 sid = atoi(row[0]);
			msgout.serial (shardname, nbplayers, sid);
			row = mysql_fetch_row(result);
		}
		netbase.send (msgout, from);
		netbase.authorizeOnly ("CS", from);

		return;
	}

	// Manage error
	CMessage msgout("VLP");
	if(reason.empty()) reason = "Unknown error";
	msgout.serial(reason);
	netbase.send(msgout, from);
	// FIX: On GNU/Linux, when we disconnect now, sometime the other side doesn't receive the message sent just before.
	//      So it is the other side to disconnect
	//		netbase.disconnect (from);
}

static void cbClientChooseShard(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S06: receive "CS" message from client
	//

	string reason;

	breakable
	{
		CMysqlResult result;
		MYSQL_ROW row;
		sint32 nbrow;
		reason = sqlQuery("select UId, Cookie, Privilege, ExtendedPrivilege from user where State='Authorized'", nbrow, row, result);
		if(!reason.empty()) break;

		if(nbrow == 0)
		{
			reason = "You are not authorized to select a shard";
			break;
		}

		bool ok = false;
		while(row != 0)
		{
			CLoginCookie lc;
			lc.setFromString(row[1]);
			if(lc.getUserAddr() == (uint32)(uintptr_t)from)
			{
				ok = true;
				break;
			}
			row = mysql_fetch_row(result);
		}

		if(!ok)
		{
			reason = "You are not authorized to select a shard";
			break;
		}

		string uid = row[0];
		string cookie = row[1];
		string priv = row[2];
		string expriv = row[3];

		// it is ok, so we find the wanted shard
		sint32 shardid;
		msgin.serial(shardid);

		reason = sqlQuery("select * from shard where ShardId="+toString(shardid), nbrow, row, result);
		if(!reason.empty()) break;

		if(nbrow == 0)
		{
			reason = "This shard is not available";
			break;
		}

		sint32 s = findShard (shardid);
		if (s == -1)
		{
			reason = "Cannot find the shard internal id";
			break;
		}
		TServiceId sid = Shards[s].SId;

		reason = sqlQuery("update user set State='Waiting', ShardId="+toString(shardid)+" where UId="+uid);
		if(!reason.empty()) break;

		reason = sqlQuery("select Login from user where UId="+uid, nbrow, row, result);
		if(!reason.empty()) break;

		if(nbrow == 0)
		{
			reason = "Cannot retrieve the username";
			break;
		}

		ucstring name;
		name.fromUtf8(row[0]);

		CLoginCookie lc;
		lc.setFromString(cookie);
		CMessage msgout("CS");
		msgout.serial(lc, name, priv, expriv);
		CUnifiedNetwork::getInstance()->send(sid, msgout);

		return;
	}

	// Manage error
	CMessage msgout("SCS");
	msgout.serial(reason);
	netbase.send(msgout, from);
	// FIX: On GNU/Linux, when we disconnect now, sometime the other side doesn't receive the message sent just before.
	//      So it's the other side to disconnect
	//			netbase.disconnect (from);
}

static void cbClientConnection (TSockId from, void *arg)
{
	CCallbackNetBase *cnb = ClientsServer;
	const CInetAddress &ia = cnb->hostAddress (from);
	nldebug("new client connection: %s", ia.asString ().c_str ());
	Output->displayNL ("CCC: Connection from %s", ia.asString ().c_str ());
	cnb->authorizeOnly ("VLP", from);
}

static void cbClientDisconnection (TSockId from, void *arg)
{
	CCallbackNetBase *cnb = ClientsServer;
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("new client disconnection: %s", ia.asString ().c_str ());

	string reason;

	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	reason = sqlQuery("select UId, State, Cookie from user where State!='Offline'", nbrow, row, result);
	if(!reason.empty()) return;

	if(nbrow == 0)
	{
		return;
	}

	while(row != 0)
	{
		CLoginCookie lc;
		string str = row[2];
		if(!str.empty())
		{
			lc.setFromString(str);
			if(lc.getUserAddr() == (uint32)(uintptr_t)from)
			{
				// got it, if he is not in waiting state, it s not normal, remove all
				if(row[1] == string("Authorized"))
					sqlQuery("update user set state='Offline', ShardId=-1, Cookie='' where UId="+string(row[0]));
				return;
			}
		}
		row = mysql_fetch_row(result);
	}
}


const TCallbackItem ClientCallbackArray[] =
{
	{ "VLP", cbClientVerifyLoginPassword },
	{ "CS", cbClientChooseShard },
};


static void cbWSShardChooseShard (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	//
	// S10: receive "SCS" message from WS
	//
	CMessage msgout("SCS");

	CLoginCookie cookie;
	string reason;

	breakable
	{
		msgin.serial (reason);
		msgin.serial (cookie);

		if(!reason.empty())
		{
			nldebug("SCS from WS failed: %s", reason.c_str());
			sqlQuery("update user set state='Offline', ShardId=-1, Cookie='' where Cookie='" + cookie.setToString() + "'");
			break;
		}

		CMysqlResult result;
		MYSQL_ROW row;
		sint32 nbrow;
		reason = sqlQuery("select UId, Cookie, Privilege, ExtendedPrivilege from user where Cookie='"+cookie.setToString()+"'", nbrow, row, result);
		if(!reason.empty()) break;
		if(nbrow != 1)
		{
			reason = "More than one row was found";
			nldebug("SCS from WS failed with duplicate cookies, sending disconnect messages.");
			// disconnect them all
			while(row != 0)
			{
				CMessage msgout("DC");
				uint32 uid = atoui(row[0]);
				msgout.serial(uid);
				CUnifiedNetwork::getInstance()->send("WS", msgout);
				row = mysql_fetch_row(result);
			}
			break;
		}

		msgout.serial(reason);
		string str = cookie.setToString ();
		msgout.serial (str);
		string addr;
		msgin.serial (addr);
		msgout.serial (addr);
		ClientsServer->send (msgout, (TSockId)cookie.getUserAddr ()); // FIXME: 64-bit
		return;
	}
	msgout.serial(reason);
	ClientsServer->send (msgout, (TSockId)cookie.getUserAddr ()); // FIXME: 64-bit
}

static const TUnifiedCallbackItem WSCallbackArray[] =
{
	{ "SCS", cbWSShardChooseShard },
};


//
//
//

void connectionClientInit ()
{
	nlassert(ClientsServer == 0);

	ClientsServer = new CCallbackServer();
	nlassert(ClientsServer != 0);

	uint16 port = (uint16) IService::getInstance ()->ConfigFile.getVar ("ClientsPort").asInt();
	ClientsServer->init (port);

	ClientsServer->addCallbackArray(ClientCallbackArray, sizeof(ClientCallbackArray)/sizeof(ClientCallbackArray[0]));
	ClientsServer->setConnectionCallback(cbClientConnection, 0);
	ClientsServer->setDisconnectionCallback(cbClientDisconnection, 0);

	// catch the messages from Welcome Service to know if the user can connect or not
	CUnifiedNetwork::getInstance ()->addCallbackArray (WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));
}

void connectionClientUpdate ()
{
	nlassert(ClientsServer != 0);

	try
	{
		ClientsServer->update();
	}
	catch (Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}

void connectionClientRelease ()
{
	nlassert(ClientsServer != 0);

	delete ClientsServer;
	ClientsServer = 0;
}
