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

#include <nel/misc/types_nl.h>

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
		auto loginService = dynamic_cast<CLoginService*>(IService::getInstance());
		*pointer = loginService->nbPlayers();
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
