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

#include <nelns/login_service/connection_client.h>

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

#include <nelns/login_service/login_service.h>
#include <nelns/login_service/functions.h>
#include <nelns/login_service/variables.h>
#include <nelns/login_service/mysql_helper.h>

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Functions
//

void ConnectionClient::sendToClient(CMessage &msgout, TSockId sockId)
{
	ClientsServer.send(msgout, sockId);
}

void string_escape(string &str)
{
	string::size_type pos = 0;
	while ((pos = str.find('\'')) != string::npos)
	{
		str.replace(pos, 1, " ");
	}
}

void ConnectionClient::cbClientVerifyLoginPassword(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S03: check the validity of the client login/password and send "VLP" message to client
	//

	// reason is empty if everything goes right or contains the reason of the failure
	string reason;
	sint32 uid = -1;
	ucstring login;
	string cpassword, application;
	msgin.serial(login);
	msgin.serial(cpassword);
	msgin.serial(application);

	breakable
	{
		// const CInetAddress &ia = netbase.hostAddress (from);
		auto maybeUser = persistence.findUserByLogin(login.toUtf8());
		reason = maybeUser.second;
		if (!reason.empty()) break;

		if (!maybeUser.first.has_value())
		{
			if (IService::getInstance()->ConfigFile.getVar("AcceptUnknownUsers").asInt() == 1)
			{
				// we accept new users, add it
				maybeUser = persistence.createUser(login.toUtf8(), cpassword);
				reason = maybeUser.second;
				if (!reason.empty()) break;
				nlinfo("The user %s was inserted in the database for the application '%s'!", login.toUtf8().c_str(), application.c_str());
			}
			else
			{
				reason = toString("Login '%s' doesn't exist", login.toUtf8().c_str());
				break;
			}
		}

		// now the user is on the database
		auto user = maybeUser.first.value();
		uid = user.uid;

		if (cpassword != user.password)
		{
			reason = toString("Bad password");
			break;
		}

		if (user.state != string("Offline"))
		{
			// 2 players are trying to play with the same id, disconnect all
			// reason = sqlQuery(string("update user set state='Offline', ShardId=-1 where UId=")+uid);
			// if(!reason.empty()) break;

			// send a message to the already connected player to disconnect
			CMessage msgout("DC");
			msgout.serial(uid);
			CUnifiedNetwork::getInstance()->send("WS", msgout);

			reason = "You are already connected.";
			break;
		}

		CLoginCookie c;
		c.set((uint32)(uintptr_t)from, rand(), uid);

		reason = persistence.authorizeUser(uid, c);
		if (!reason.empty()) break;

		auto maybeShards = persistence.findOnlineShardsByApplication(application);
		reason = maybeShards.second;
		if (!reason.empty()) break;

		// Send success message
		CMessage msgout("VLP");
		sint32 shardCount = maybeShards.first.size();
		msgout.serial(reason);
		msgout.serial(shardCount);

		// send address and name of all online shards
		for (auto shard : maybeShards.first)
		{
			// serial the name of the shard
			msgout.serial(shard.name, shard.nbplayers, shard.sid);
		}
		netbase.send(msgout, from);
		netbase.authorizeOnly("CS", from);

		return;
	}

	// Manage error
	CMessage msgout("VLP");
	if (reason.empty()) reason = "Unknown error";
	msgout.serial(reason);
	netbase.send(msgout, from);
	// FIX: On GNU/Linux, when we disconnect now, sometime the other side doesn't receive the message sent just before.
	//      So it is the other side to disconnect
	//		netbase.disconnect (from);
}

void ConnectionClient::cbClientChooseShard(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S06: receive "CS" message from client
	//

	string reason;

	breakable
	{
		auto users = persistence.findAuthorizedUsers();
		reason = users.second;
		if (!reason.empty()) break;

		if (users.first.empty())
		{
			reason = "You are not authorized to select a shard";
			break;
		}

		bool ok = false;
		CLoginCookie cookie;
		string uid;
		string priv;
		string expriv;
		for (auto user : users.first)
		{
			if (user.cookie.getUserAddr() == (uint32)(uintptr_t)from)
			{
				ok = true;
				uid = user.uid;
				cookie = user.cookie;
				priv = user.privilege;
				expriv = user.extendedPrivilege;
				break;
			}
		}

		if (!ok)
		{
			reason = "You are not authorized to select a shard";
			break;
		}

		// it is ok, so we find the wanted shard
		sint32 shardid;
		msgin.serial(shardid);

		auto shardExists = persistence.existsShardById(shardid);
		reason = shardExists.second;
		if (!reason.empty()) break;

		if (!shardExists.first)
		{
			reason = "This shard is not available";
			break;
		}

		sint32 s = findShard(shardid);
		if (s == -1)
		{
			reason = "Cannot find the shard internal id";
			break;
		}
		TServiceId sid = Shards[s].SId;

		reason = persistence.updateUserWaitingOnShard(uid, shardid);
		if (!reason.empty()) break;

		auto maybeUserLogin = persistence.findUserLoginById(uid);
		reason = maybeUserLogin.second;
		if (!reason.empty()) break;

		if (!maybeUserLogin.first.has_value())
		{
			reason = "Cannot retrieve the username";
			break;
		}

		string name = maybeUserLogin.first.value();

		CMessage msgout("CS");
		msgout.serial(cookie, name, priv, expriv);
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

void ConnectionClient::cbClientConnection(TSockId from)
{
	const CInetAddress &ia = ClientsServer.hostAddress(from);
	nldebug("new client connection: %s", ia.asString().c_str());
	Output->displayNL("CCC: Connection from %s", ia.asString().c_str());
	ClientsServer.authorizeOnly("VLP", from);
}

void ConnectionClient::cbClientDisconnection(TSockId from)
{
	const CInetAddress &ia = ClientsServer.hostAddress(from);

	nldebug("new client disconnection: %s", ia.asString().c_str());

	auto users = persistence.findNotOfflineUsers();
	string reason = users.second;
	if (!reason.empty()) {
		nlerror("cannot get user to disconnect: %s", reason.c_str());
	}

	if (users.first.empty())
	{
		return;
	}

	for (const auto& user : users.first)
	{
		if (user.cookie.isValid())
		{
			if (user.cookie.getUserAddr() == (uint32)(uintptr_t)from)
			{
				// got it, if he is not in waiting state, it s not normal, remove all
				if (user.state == string("Authorized"))
				{
					persistence.logoutUserById(user.uid);
				}
				return;
			}
		}
	}
}

void ConnectionClient::cbWSShardChooseShard(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	//
	// S10: receive "SCS" message from WS
	//
	CMessage msgout("SCS");

	CLoginCookie cookie;
	string reason;

	breakable
	{
		msgin.serial(reason);
		msgin.serial(cookie);

		if (!reason.empty())
		{
			nldebug("SCS from WS failed: %s", reason.c_str());
			persistence.logoutUserByCookie(cookie);
			break;
		}

		CMysqlResult result;
		MYSQL_ROW row;
		sint32 nbrow;
		reason = sqlQuery("select UId, Cookie, Privilege, ExtendedPrivilege from user where Cookie='" + cookie.setToString() + "'", nbrow, row, result);
		if (!reason.empty()) break;
		if (nbrow != 1)
		{
			reason = "More than one row was found";
			nldebug("SCS from WS failed with duplicate cookies, sending disconnect messages.");
			// disconnect them all
			while (row != 0)
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
		string str = cookie.setToString();
		msgout.serial(str);
		string addr;
		msgin.serial(addr);
		msgout.serial(addr);
		ClientsServer.send(msgout, (TSockId)cookie.getUserAddr()); // FIXME: 64-bit
		return;
	}
	msgout.serial(reason);
	ClientsServer.send(msgout, (TSockId)cookie.getUserAddr()); // FIXME: 64-bit
}

//
//
//

ConnectionClient::ConnectionClient(IPersistence& persistence)
    : persistence(persistence)
{
	uint16 port = (uint16)IService::getInstance()->ConfigFile.getVar("ClientsPort").asInt();
	ClientsServer.init(port);

	const TCallbackItem ClientCallbackArray[] = {
		{ "VLP", [=](auto &msgin, auto from, auto &netbase) { cbClientVerifyLoginPassword(msgin, from, netbase); } },
		{ "CS", [=](auto &msgin, auto from, auto &netbase) { cbClientChooseShard(msgin, from, netbase); } },
	};
	ClientsServer.addCallbackArray(ClientCallbackArray, std::size(ClientCallbackArray));
	ClientsServer.setConnectionCallback([=](auto from, auto arg) { cbClientConnection(from); }, nullptr);
	ClientsServer.setDisconnectionCallback([=](auto from, auto arg) { cbClientDisconnection(from); }, nullptr);

	// catch the messages from Welcome Service to know if the user can connect or not
	const TUnifiedCallbackItem WSCallbackArray[] = {
		{ "SCS", [=](auto &msgin, auto &serviceName, auto sid) { cbWSShardChooseShard(msgin, serviceName, sid); } },
	};
	CUnifiedNetwork::getInstance()->addCallbackArray(WSCallbackArray, std::size(WSCallbackArray));
}

void ConnectionClient::update()
{
	try
	{
		ClientsServer.update();
	}
	catch (Exception &e)
	{
		nlwarning("Error during update: '%s'", e.what());
	}
}
