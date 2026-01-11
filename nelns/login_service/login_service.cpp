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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#define NELNS_LOGS ""
#endif // NELNS_LOGS

#ifndef NELNS_STATE
#define NELNS_STATE ""
#endif // NELNS_STATE

#include "nel/misc/types_nl.h"

#include <cstdio>
#include <ctype.h>
#include <cmath>

#include <vector>
#include <map>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/log.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/path.h"

#include "nel/net/service.h"
#include "nel/net/login_cookie.h"

#include "login_service.h"
#include "connection_client.h"
#include "connection_ws.h"
#include "connection_web.h"
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


// store specific user information
NLMISC::CFileDisplayer *Fd = NULL;
NLMISC::CStdDisplayer Sd;
NLMISC::CLog *Output = NULL;

//uint32 CUser::NextUserId = 1;	// 0 is reserved

//vector<CUser>	Users;
//vector<CShard>	Shards;

vector<CShard> Shards;


//
// Functions
//

sint findShard (sint32 shardId)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}

sint findShardWithSId (TServiceId sid)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].SId == sid)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}


// transform "192.168.1.1:80" into "192.168.1.1"
string removePort (const string &addr)
{
	return addr.substr (0, addr.find (":"));
}

void checkClients ()
{
	nldebug ("checkClients ()");
	/*for (uint i = 0; i < Users.size (); i++)
	{
		switch (Users[i].State)
		{
		case CUser::Offline:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Authorized:
			nlassert (Users[i].SockId != NULL);
			nlassert (Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Awaiting:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Online:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId != NULL);
			break;
		default:
			nlstop;
			break;
		}
	}*/
}

/*void disconnectClient (CUser &user, bool disconnectClient, bool disconnectShard)
{
	nlinfo ("User %d '%s' need to be disconnect (%d %d %d)", user.Id, user.Login.c_str(), user.State, disconnectClient, disconnectShard);

	switch (user.State)
	{
	case CUser::Offline:
		break;

	case CUser::Authorized:
		if (disconnectClient)
		{
			CNetManager::getNetBase("LS")->disconnect(user.SockId);
		}
		user.Cookie.clear ();
		break;

	case CUser::Awaiting:
		break;

	case CUser::Online:
		if (disconnectShard)
		{
			// ask the WS to disconnect the player from the shard
			CMessage msgout (CNetManager::getNetBase("WSLS")->getSIDA (), "DC");
			msgout.serial (user.Id);
			CNetManager::send ("WSLS", msgout, user.ShardId);
		}
		user.ShardId = NULL;
		break;

	default:
		nlstop;
		break;
	}

	user.SockId = NULL;
	user.State = CUser::Offline;
}
*/
sint findUser (uint32 Id)
{
/*	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		if (Users[i].Id == Id)
		{
			return i;
		}
	}
	// user not found
*/	return -1;
}
/*
string CUser::Authorize (TSockId sender, CCallbackNetBase &netbase)
{
	string reason;

	switch (State)
	{
	case Offline:
		State = Authorized;
		SockId = sender;
		Cookie = CLoginCookie(netbase.hostAddress(sender).internalIPAddress(), Id);
		break;

	case Authorized:
		nlwarning ("user %d already authorized! disconnect him and the other one", Id);
		reason = "You are already authorized (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Awaiting:
		nlwarning ("user %d already awaiting! disconnect the new user and the other one", Id);
		reason = "You are already awaiting (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Online:
		nlwarning ("user %d already online! disconnect the new user and the other one", Id);
		reason = "You are already online (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	default:
		reason = "default case should never occurs, there's a bug in the login_service.cpp";
		nlstop;
		break;
	}
	return reason;
}
*/
void displayShards ()
{
	ICommand::execute ("shards", *InfoLog);
}

void displayUsers ()
{
	ICommand::execute ("users", *InfoLog);
}



////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// SERVICE IMPLEMENTATION /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void beep (uint freq, uint nb, uint beepDuration, uint pauseDuration)
{
#ifdef NL_OS_WINDOWS
	try
	{
		if (IService::getInstance()->ConfigFile.getVar ("Beep").asInt() == 1)
		{
			for (uint i = 0; i < nb; i++)
			{
				Beep (freq, beepDuration);
				nlSleep (pauseDuration);
			}
		}
	}
	catch (Exception &)
	{
	}
#endif // NL_OS_WINDOWS
}

class CLoginService : public IService
{
public:
	
	bool UseDirectClient;

	CLoginService () : UseDirectClient(false) { }

	/// Init the service, load the universal time.
	void init ()
	{
		beep ();

		Output = new CLog;

		if(ConfigFile.exists("UseDirectClient"))
			UseDirectClient = ConfigFile.getVar("UseDirectClient").asBool();

		string fn = IService::getInstance()->SaveFilesDirectory;
		fn += "login_service.stat";
		nlinfo("Login stat in directory '%s'", fn.c_str());
		Fd = new NLMISC::CFileDisplayer(fn);
		Output->addDisplayer (Fd);
		if (WindowDisplayer) Output->addDisplayer (WindowDisplayer);

		// Initialize the database access
		sqlInit();

		connectionWSInit ();

		if(UseDirectClient)
			connectionClientInit ();
		else
			connectionWebInit ();

		Output->displayNL ("Login Service initialized");
	}

	bool update ()
	{
		connectionWSUpdate ();
		if(UseDirectClient)
			connectionClientUpdate ();
		else
			connectionWebUpdate ();
		return true;
	}

	/// release the service, save the universal time
	void release ()
	{
		connectionWSRelease ();
		if(UseDirectClient)
			connectionClientRelease ();
		else
			connectionWebRelease ();
		
		Output->displayNL ("Login Service released");
	}
};

// Service instantiation
NLNET_SERVICE_MAIN (CLoginService, "LS", "login_service", 49999, EmptyCallbackArray, NELNS_CONFIG, NELNS_LOGS);

//
// Variables
//

NLMISC_DYNVARIABLE(uint, OnlineUsersNumber, "number of actually connected users")
{
	// we can only read the value
	if (get)
	{
		//uint32 nbusers = 0;
		//for (uint i = 0; i < Users.size(); i++)
		//{
		//	if (Users[i].State == CUser::Online)
		//		nbusers++;
		//}
		*pointer = NbPlayers;
	}
}

//
// Commands
//

NLMISC_COMMAND (shards, "displays the list of all registered shards", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered shards :", Shards.size());
	for (uint i = 0; i < Shards.size(); i++)
	{
		log.displayNL ("* ShardId: %d SId: %d NbPlayers: %d", Shards[i].ShardId, Shards[i].SId.get(), Shards[i].NbPlayers);
		CUnifiedNetwork::getInstance()->displayUnifiedConnection (Shards[i].SId, &log);
	}
	log.displayNL ("End of the list");

	return true;
}

NLMISC_COMMAND (shardDatabase, "displays the list of all shards in the database", "")
{
	if(args.size() != 0) return false;

	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;

	string reason = sqlQuery("select ShardId, WsAddr, NbPlayers, Name, Online, ClientApplication, Version, DynPatchURL from shard", nbrow, row, result);
	if(!reason.empty()) { log.displayNL("Display registered users failed; '%s'", reason.c_str()); return true; }

	log.displayNL("Display the %d shards in the database :", nbrow);
	log.displayNL(" > ShardId, WsAddr, NbPlayers, Name, Online, ClientApplication, Version, DynPatchURL");

	if (nbrow != 0) while(row != 0)
	{
		log.displayNL(" > '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
		row = mysql_fetch_row(result);
	}

	log.displayNL("End of the list");

	return true;
}

NLMISC_COMMAND (registeredUsers, "displays the list of all registered users", "")
{
	if(args.size() != 0) return false;

	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;

	string reason = sqlQuery("select UId, Login, Password, ShardId, State, Privilege, ExtendedPrivilege, Cookie from user", nbrow, row, result);
	if(!reason.empty()) { log.displayNL("Display registered users failed; '%s'", reason.c_str()); return true; }

	log.displayNL("Display the %d registered users :", nbrow);
	log.displayNL(" > UId, Login, Password, ShardId, State, Privilege, ExtendedPrivilege, Cookie");

	if (nbrow != 0) while(row != 0)
	{
		log.displayNL(" > '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
		row = mysql_fetch_row(result);
	}

	log.displayNL("End of the list");

	return true;
}

NLMISC_COMMAND (onlineUsers, "displays the list of online users", "")
{
	if(args.size() != 0) return false;

	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;

	uint32 nbusers = 0, nbwait = 0;
	log.displayNL ("Display the online users :");

	string reason = sqlQuery("select UId, Login, Password, ShardId, State, Privilege, ExtendedPrivilege, Cookie from user where State='Online'", nbrow, row, result);
	if(!reason.empty()) { log.displayNL("Display online users failed; '%s'", reason.c_str()); return true; }
	if (nbrow != 0) while(row != 0)
	{
		log.displayNL(" > '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
		row = mysql_fetch_row(result);
	}
	nbusers = nbrow;

	reason = sqlQuery("select UId, Login, Password, ShardId, State, Privilege, ExtendedPrivilege, Cookie from user where State='Waiting'", nbrow, row, result);
	if(!reason.empty()) { log.displayNL("Display waiting users failed; '%s'", reason.c_str()); return true; }
	if (nbrow != 0) while(row != 0)
	{
		log.displayNL(" > '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
		row = mysql_fetch_row(result);
	}
	nbwait = nbrow;

	reason = sqlQuery("select UId, Login, Password, ShardId, State, Privilege, ExtendedPrivilege, Cookie from user where State='Authorized'", nbrow, row, result);
	if(!reason.empty()) { log.displayNL("Display authorized users failed; '%s'", reason.c_str()); return true; }
	if (nbrow != 0) while(row != 0)
	{
		log.displayNL(" > '%s' '%s' '%s' '%s' '%s' '%s' '%s' '%s'", row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
		row = mysql_fetch_row(result);
	}

	log.displayNL ("End of the list (%d online users, %d waiting, %d authorized)", nbusers, nbwait, nbrow);

	return true;
}

