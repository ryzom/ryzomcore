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

#include <nelns/login_service/connection_ws.h>

#include <cstdio>
#include <ctype.h>
#include <cmath>

#include <vector>
#include <map>

#include <nel/misc/debug.h>
#include <nel/misc/config_file.h>
#include <nel/misc/displayer.h>
#include <nel/misc/log.h>

#include <nel/net/service.h>
#include <nel/net/login_cookie.h>

#include <nelns/login_service/functions.h>
#include <nelns/login_service/login_service.h>
#include <nelns/login_service/mysql_helper.h>
#include <nelns/login_service/variables.h>

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Functions
//

void refuseShard (TServiceId sid, const char *format, ...)
{
	string reason;
	NLMISC_CONVERT_VARGS (reason, format, NLMISC::MaxCStringSize);
	nlwarning(reason.c_str ());
	CMessage msgout("FAILED");
	msgout.serial (reason);
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
}

void ConnectionWS::cbWSConnection(const std::string &serviceName, TServiceId sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("new potential shard: %s", ia.asString ().c_str ());

	// if we accept external shard, don't need to check if address is valid
	if(IService::getInstance ()->ConfigFile.getVar("AcceptExternalShards").asInt () == 1)
		return;

	string reason;
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;

	string query = "select ShardId from shard where WsAddr='"+sqlEscape(ia.ipAddress())+"'";
	reason = sqlQuery(query, nbrow, row, result);
	if (!reason.empty())
	{
		refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	if (nbrow == 0)
	{
		// if we are here, it s that the shard have not a valid wsaddr in the database
		// we can't accept unknown shard
		refuseShard (sid, "Bad shard identification, the shard (WSAddr %s) is not in the database and can't be added", ia.ipAddress ().c_str ());
		return;
	}
}

void ConnectionWS::cbWSDisconnection (const std::string &serviceName, TServiceId sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("shard disconnection: %s", ia.asString ().c_str ());

	for (uint32 i = 0; i < Shards.size (); i++)
	{
		if (Shards[i].SId == sid)
		{
			// shard disconnected
			nlinfo("ShardId %d with IP '%s' is offline!", Shards[i].ShardId, ia.asString ().c_str());
			nlinfo("*** ShardId %3d NbPlayers %3d -> %3d", Shards[i].ShardId, Shards[i].NbPlayers, 0);
			
			string query = "update shard set Online=0, NbPlayers=NbPlayers-"+toString(Shards[i].NbPlayers)+" where ShardId="+toString(Shards[i].ShardId);
			sint ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			}

			NbPlayers -= Shards[i].NbPlayers;

			Shards[i].NbPlayers = 0;

			// put users connected on this shard offline

			query = "update user set State='Offline', ShardId=-1 where ShardId="+toString(Shards[i].ShardId);
			ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			}
			
			Shards.erase (Shards.begin () + i);

			return;
		}
	}
	nlwarning("Shard %s goes offline but wasn't online!", ia.asString ().c_str ());
}

/** Shard accepted the new user, so warn the user that he could connect to the shard now */
/*void cbClientShardAcceptedTheUser (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint8 res;
	string IP;
	uint32 Key, Id;
	msgin.serial (IP);
	msgin.serial (Key);
	msgin.serial (Id);
	msgin.serial (res);

	CMessage msgout (netbase.getSIDA (), "ACC");
	msgout.serial (res);

	if(res)
	{
		// the shard accept the user
		msgout.serial (IP);
		msgout.serial (Key);
	}
	else
	{
		// the shard don't want him!
		string reason;
		msgin.serial (reason);
		msgout.serial (reason);
	}

	// find the user
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).Authorized && (*it).Key == Key)
		{
			// send the answer to the user
			netbase.send (msgout, (*it).SockId);

			(*it).Authorized = false;
			return;
		}
	}
}
*/


/*
void cbShardComesIn (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	const CInetAddress &ia = netbase.hostAddress (from);

	// at this time, it could be a new shard or a ne client.

	nldebug("new potential shard: %s", ia.asString ().c_str ());

	// first, check if it an authorized shard
	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].Address.ipAddress () == ia.ipAddress ())
		{
			if (Shards[i].Online)
			{
				nlwarning("Shard with ip '%s' is already online! Disconnect the new one", ia.asString().c_str ());
				netbase.disconnect (from);
			}
			else
			{
				// new shard connected
				Shards[i].Address = ia;
				Shards[i].Online = true;
				Shards[i].SockId = from;
				nlinfo("Shard with ip '%s' is online!", Shards[i].Address.asString().c_str ());
			}
			return;
		}
	}
#if ACCEPT_EXTERNAL_SHARD
	// New externam shard connected, add it in the file
	Shards.push_back (CShard(ia));
	sint32 pos = Shards.size()-1;
	Shards[pos].Online = true;
	Shards[pos].SockId = from;
	nlinfo("External shard with ip '%s' is online!", Shards[pos].Address.asString().c_str ());
	writeConfigFile ();
#else
	nlwarning("It's not a authorized shard, disconnect it");
	netbase.close (from);
#endif
}
*/

//

void ConnectionWS::cbWSIdentification (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	sint32 shardId;
	msgin.serial(shardId);
	string application;
	try {
		msgin.serial(application);
	} catch (Exception &) { }
	nldebug("shard identification, It says to be ShardId %d, let's check that!", shardId);

	string reason;

	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	string query = "select WsAddr, domain_id from shard where ShardId="+toString(shardId);
	reason = sqlQuery(query, nbrow, row, result);
	if (!reason.empty())
	{
		refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	if (nbrow == 0)
	{
		if(IService::getInstance ()->ConfigFile.getVar("AcceptExternalShards").asInt () == 1)
		{
			// the shard table hangs off the domain table now: resolve the
			// domain from the application name the WS identified with
			if (application.empty())
			{
				refuseShard (sid, "Bad shard identification, ShardId %d is not in the database and sent no application name to resolve its domain", shardId);
				return;
			}
			string domainId;
			{
				CMysqlResult domainResult;
				MYSQL_ROW domainRow;
				sint32 nbDomainRow;
				query = "select domain_id from domain where domain_name='"+sqlEscape(application)+"'";
				reason = sqlQuery(query, nbDomainRow, domainRow, domainResult);
				if (!reason.empty())
				{
					refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
					return;
				}
				if (nbDomainRow == 0)
				{
					// dev convenience, same spirit as accepting the shard:
					// create a minimal domain row for this application
					query = "insert into domain (domain_name) values ('"+sqlEscape(application)+"')";
					reason = sqlQuery(query);
					if (!reason.empty())
					{
						refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
						return;
					}
					nlinfo("The domain '%s' was inserted in the database for ShardId %d!", application.c_str (), shardId);
					CMysqlResult createdResult;
					query = "select domain_id from domain where domain_name='"+sqlEscape(application)+"'";
					reason = sqlQuery(query, nbDomainRow, domainRow, createdResult);
					if (!reason.empty() || nbDomainRow == 0)
					{
						refuseShard (sid, "Failed to read back the domain '%s' just created", application.c_str ());
						return;
					}
					domainId = domainRow[0];
				}
				else
				{
					domainId = domainRow[0];
				}
			}

			// we accept new shard, add it (MOTD has no default value)
			query = "insert into shard (ShardId, domain_id, WsAddr, Online, Name, MOTD) values ("+toString(shardId)+", "+domainId+", '"+sqlEscape(ia.ipAddress())+"', 1, '"+sqlEscape(ia.ipAddress())+"', '')";
			reason = sqlQuery(query, nbrow, row, result);
			if (!reason.empty())
			{
				refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			}
			else
			{
				nlinfo("The ShardId %d with ip '%s' was inserted in the database and is online!", shardId, ia.ipAddress ().c_str ());
				Shards.push_back (CShard (shardId, sid));
			}
		}
		else
		{
			// can't accept new shard
			refuseShard (sid, "Bad shard identification, The shard %d is not in the database and can't be added", shardId);
		}
		return;
	}
	else if (nbrow == 1)
	{
		// check that the ip is ok
		CInetAddress iadb;
		iadb.setNameAndPort (row[0] ? row[0] : "");
		nlinfo ("check %s with %s (%s)", ia.ipAddress ().c_str(), iadb.ipAddress().c_str(), row[0] ? row[0] : "");
		if (ia.ipAddress () != iadb.ipAddress())
		{
			// good shard id but from a bad computer address
			refuseShard (sid, "Bad shard identification, ShardId %d should come from '%s' and come from '%s'", shardId, row[0] ? row[0] : "", ia.ipAddress ().c_str ());
			return;
		}

		// when the WS says which application it serves, make sure the shard
		// row belongs to that domain: in a shared setup a wrong ShardId in a
		// config would otherwise attach a shard to another domain's row
		if (!application.empty())
		{
			CMysqlResult domainResult;
			MYSQL_ROW domainRow;
			sint32 nbDomainRow;
			query = "select domain_id from domain where domain_name='"+sqlEscape(application)+"'";
			string domainReason = sqlQuery(query, nbDomainRow, domainRow, domainResult);
			if (domainReason.empty() && nbDomainRow == 1 && string(row[1] ? row[1] : "") != string(domainRow[0]))
			{
				refuseShard (sid, "Bad shard identification, ShardId %d belongs to domain %s, not to '%s'", shardId, row[1] ? row[1] : "?", application.c_str ());
				return;
			}
		}

		sint32 s = findShard (shardId);
		if (s != -1)
		{
			// the shard is already online, disconnect the old one and set the new one
			refuseShard (Shards[s].SId, "A new shard connects with the same IP/ShardId, you was replaced");

			Shards[s].SId = sid;
			Shards[s].ShardId = shardId;
		}
		else
		{
			string query = "update shard set Online=1 where ShardId="+toString(shardId);
			sint ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				refuseShard (sid, "mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
				return;
			}

			Shards.push_back (CShard (shardId, sid));
		}

		// ok, the shard is identified correctly
		nlinfo("ShardId %d with ip '%s' is online!", shardId, ia.ipAddress ().c_str ());
		return;
	}
	else
	{
		refuseShard (sid, "mysql problem, There's more than 1 shard with the shardId %d in the database", shardId);
		return;
	}
	
	nlstop;
}

void ConnectionWS::cbWSClientConnected (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	//
	// S16: Receive "CC" message from WS
	//

	// a WS tells me that a player is connected or disconnected
	// find the user
	uint32 Id;
	uint8 con;
	msgin.serial (Id);
	msgin.serial (con);	// con=1 means a client is connected on the shard, 0 means a client disconnected

	if(con)
		nlinfo ("Received a validation that a client is connected on the frontend");
	else
		nlinfo ("Received a validation that a client is disconnected on the frontend");
	
	string reason;
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;

	// name the column instead of trusting the table layout: 'select *' with a
	// positional index broke silently every time the schema gained a column
	string query = "select State from user where UId="+toString(Id);
	reason = sqlQuery(query, nbrow, row, result);
	if(!reason.empty()) return;

	if(nbrow == 0)
	{
		nlwarning ("Id %d doesn't exist", Id);
		Output->displayNL ("###: %3d UId doesn't exist", Id);
		return;
	}
	else if (nbrow > 1)
	{
		nlerror ("Id %d have more than one entry!!!", Id);
		return;
	}

	if (con == 1 && string(row[0]) != string("Waiting"))
	{
		nlwarning("Id %d is not waiting", Id);
		Output->displayNL("###: %3d User isn't waiting, his state is '%s'", Id, row[0]);
		return;
	}
	else if (con == 0 && string(row[0]) != string ("Online"))
	{
		nlwarning ("Id %d wasn't connected on a shard", Id);
		Output->displayNL ("###: %3d User wasn't connected on a shard, his state is '%s'", Id, row[0]);
		return;
	}

	sint ShardPos = findShardWithSId (sid);

	if (con == 1)
	{
		// new client on the shard


		string query = "update user set State='Online', ShardId="+toString(Shards[ShardPos].ShardId)+" where UId="+toString(Id);
		string rea = sqlQuery(query);
		if(!rea.empty()) return;

		if (ShardPos != -1)
		{
			nlinfo("*** ShardId %3d NbPlayers %3d -> %3d", Shards[ShardPos].ShardId, Shards[ShardPos].NbPlayers, Shards[ShardPos].NbPlayers+1);
			Shards[ShardPos].NbPlayers++;

			string query = "update shard set NbPlayers=NbPlayers+1 where ShardId="+toString(Shards[ShardPos].ShardId);
			string rea = sqlQuery(query);
			if(!rea.empty()) return;
		}
		else
			nlwarning ("user connected shard isn't in the shard list");

		nldebug ("Id %d is connected on the shard", Id);
		Output->displayNL ("###: %3d User connected to the shard (%d)", Id, Shards[ShardPos].ShardId);

		NbPlayers++;
		if (NbPlayers > RecordNbPlayers)
		{
			RecordNbPlayers = NbPlayers;
			beep (2000, 1, 100, 0);
			nlwarning("New player number record!!! %d players online on all shards", RecordNbPlayers);
		}
	}
	else
	{
		// client removed from the shard (true is for potential other client with the same id that wait for a connection)
	//		disconnectClient (Users[pos], true, false);

		string query = "update user set State='Offline', ShardId=-1 where UId="+toString(Id);
		string rea = sqlQuery(query);
		if(!rea.empty()) return;

		if (ShardPos != -1)
		{
			nlinfo("*** ShardId %3d NbPlayers %3d -> %3d", Shards[ShardPos].ShardId, Shards[ShardPos].NbPlayers, Shards[ShardPos].NbPlayers-1);
			Shards[ShardPos].NbPlayers--;

			string query = "update shard set NbPlayers=NbPlayers-1 where ShardId="+toString(Shards[ShardPos].ShardId);
			string rea = sqlQuery(query);
			if(!rea.empty()) return;
		}
		else
			nlwarning ("user disconnected shard isn't in the shard list");
			
		nldebug ("Id %d is disconnected from the shard", Id);
		Output->displayNL ("###: %3d User disconnected from the shard (%d)", Id, Shards[ShardPos].ShardId);

		NbPlayers--;
	}
}


void ConnectionWS::cbWSReportFSState(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	sint	shardPos = findShardWithSId (sid);

	if (shardPos == -1)
	{
		nlwarning ("unknown WS %d reported state of a fs", sid.get());
		return;
	}

	CShard&	shard = Shards[shardPos];

	TServiceId	FSSId;
	bool		alive;
	bool		patching;
	std::string	patchURI;

	msgin.serial(FSSId);
	msgin.serial(alive);
	msgin.serial(patching);
	msgin.serial(patchURI);

	if (!alive)
	{
		nlinfo("Shard %d frontend %d reported as offline", shard.ShardId, FSSId.get());
		std::vector<CFrontEnd>::iterator	itfs;
		for (itfs=shard.FrontEnds.begin(); itfs!=shard.FrontEnds.end(); ++itfs)
		{
			if ((*itfs).SId == FSSId)
			{
				shard.FrontEnds.erase(itfs);
				break;
			}
		}
	}
	else
	{
		CFrontEnd*	updateFS = NULL;
		std::vector<CFrontEnd>::iterator	itfs;
		for (itfs=shard.FrontEnds.begin(); itfs!=shard.FrontEnds.end(); ++itfs)
		{
			if ((*itfs).SId == FSSId)
			{
				// update FS state
				CFrontEnd&	fs = (*itfs);
				fs.Patching = patching;
				fs.PatchURI = patchURI;

				updateFS = &fs;

				break;
			}
		}

		if (itfs == shard.FrontEnds.end())
		{
			nlinfo("Shard %d frontend %d reported as online", shard.ShardId, FSSId.get());
			// unknown fs, create new entry
			shard.FrontEnds.push_back(CFrontEnd(FSSId, patching, patchURI));

			updateFS = &(shard.FrontEnds.back());
		}

		if (updateFS != NULL)
		{
			nlinfo("Shard %d frontend %d status updated: patching=%s patchURI=%s", shard.ShardId, FSSId.get(), (updateFS->Patching ? "yes" : "no"), updateFS->PatchURI.c_str());
		}
	}

	// update DynPatchURLS in database
	std::string	dynPatchURL;
	uint	i;
	for (i=0; i<shard.FrontEnds.size(); ++i)
	{
		if (shard.FrontEnds[i].Patching)
		{
			if (!dynPatchURL.empty())
				dynPatchURL += ' ';
			dynPatchURL += shard.FrontEnds[i].PatchURI;
		}
	}

	// the shard table lost its DynPatchURL column (patch urls live on the
	// domain row now); the per-frontend patch state is only kept in memory
	if (!dynPatchURL.empty())
		nlinfo("Shard %d dynamic patch URL: %s", shard.ShardId, dynPatchURL.c_str());
}

void ConnectionWS::cbWSReportNoPatch(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	sint	shardPos = findShardWithSId (sid);

	if (shardPos == -1)
	{
		nlwarning ("unknown WS %d reported state of a fs", sid.get());
		return;
	}

	CShard&	shard = Shards[shardPos];

	uint	i;
	for (i=0; i<shard.FrontEnds.size(); ++i)
	{
		shard.FrontEnds[i].Patching = false;
	}
	// DynPatchURL is gone from the shard table; nothing to clear in database
}

void ConnectionWS::cbWSSetShardOpen(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	sint	shardPos = findShardWithSId (sid);

	if (shardPos == -1)
	{
		nlwarning ("unknown WS %d reported shard open state", sid.get());
		return;
	}

	uint8	shardOpenState;
	msgin.serial(shardOpenState);

	CShard&	shard = Shards[shardPos];

	string query = toString("UPDATE shard SET Online='%d' WHERE ShardId='%d'", shardOpenState+1, shard.ShardId);
	sint ret = mysql_query (DatabaseConnection, query.c_str ());
	if (ret != 0)
	{
		nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}
}


//
// Functions
//

ConnectionWS::ConnectionWS()
    : NbPlayers(0)
    , RecordNbPlayers(0)
{
	const TUnifiedCallbackItem WSCallbackArray[] =
	{
		{ "CC",					[=](auto &msgin, auto& serviceName, const auto& sid) { cbWSClientConnected(msgin, serviceName, sid); } },
		{ "WS_IDENT",			[=](auto &msgin, auto& serviceName, const auto& sid) { cbWSIdentification(msgin, serviceName, sid); } },

		{ "REPORT_FS_STATE",	[=](auto &msgin, auto& serviceName, const auto& sid) { cbWSReportFSState(msgin, serviceName, sid); } },
		{ "REPORT_NO_PATCH",	[=](auto &msgin, auto& serviceName, const auto& sid) { cbWSReportNoPatch(msgin, serviceName, sid); } },
		{ "SET_SHARD_OPEN",		[=](auto &msgin, auto& serviceName, const auto& sid) { cbWSSetShardOpen(msgin, serviceName, sid); } },
	};
	CUnifiedNetwork::getInstance ()->addCallbackArray (WSCallbackArray, std::size(WSCallbackArray));

	CUnifiedNetwork::getInstance ()->setServiceUpCallback ("WS", [=](auto &serviceName, const auto& sid, auto *arg) { cbWSConnection(serviceName, sid, arg); });
	CUnifiedNetwork::getInstance ()->setServiceDownCallback ("WS", [=](auto &serviceName, const auto& sid, auto *arg) { cbWSDisconnection(serviceName, sid, arg); });
}

void ConnectionWS::update()
{
}

ConnectionWS::~ConnectionWS()
{
	nlinfo ("I'm going down, clean the database");


	while (!Shards.empty())
	{
		cbWSDisconnection ("", Shards[0].SId, NULL);
	}
	
/*	
	// we remove all shards online from my list
	for (uint32 i = 0; i < Shards.size (); i++)
	{
		TSockId from;
		CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (Shards[i].SId, from);
		const CInetAddress &ia = cnb->hostAddress (from);

		// shard disconnected
		nlinfo("Set ShardId %d with IP '%s' offline in the database and set %d players to offline", Shards[i].ShardId, ia.asString ().c_str());
		
		string query = "update shard set Online=Online-1, NbPlayers=NbPlayers-"+toString(Shards[i].NbPlayers)+" where ShardId="+toString(Shards[i].ShardId);
		sint ret = mysql_query (DatabaseConnection, query.c_str ());
		if (ret != 0)
		{
			nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		}

		// put users connected on this shard offline
	}
	Shards.clear ();
*/

}
