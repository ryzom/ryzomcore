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

#include <nelns/login_service/mysql_persistence.h>

#include <nel/misc/string_common.h>
#include <nelns/login_service/mysql_helper.h>

using NLMISC::toString;
using NLNET::CLoginCookie;
using std::optional;
using std::pair;
using std::string;
using std::vector;

void CMysqlPersistence::init()
{
	sqlInit();
}

pair<optional<LoginUserProjection>, string> CMysqlPersistence::findUserByLogin(const string &login)
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	string reason = sqlQuery("select uid, password, state from user where Login='" + sqlEscape(login) + "'", nbrow, row, result);

	if (!reason.empty())
	{
		return std::make_pair(std::nullopt, reason);
	}

	if (nbrow <= 0)
	{
		return std::make_pair(std::nullopt, "");
	}

	if (nbrow > 1)
	{
		return std::make_pair(std::nullopt, toString("Too much login '%s' exists", login.c_str()));
	}

	LoginUserProjection user {
		.uid = -1,
		.password = row[1],
		.state = row[2]
	};
	NLMISC::fromString(row[0], user.uid);

	return std::make_pair(std::make_optional(user), "");
}

string CMysqlPersistence::authorizeUser(sint32 uid, const NLNET::CLoginCookie &cookie)
{
	return sqlQuery("update user set state='Authorized', Cookie='" + sqlEscape(cookie.setToString()) + "' where UId=" + toString(uid));
}

pair<vector<OnlineShardProjection>, string> CMysqlPersistence::findOnlineShardsByApplication(const string &application)
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	std::vector<OnlineShardProjection> shards;
	string reason = sqlQuery("select shardid, name, nbplayers from shard where Online>0 and ClientApplication='" + sqlEscape(application) + "'", nbrow, row, result);
	if (!reason.empty())
	{
		return std::make_pair(shards, reason);
	}

	// send address and name of all online shards
	while (row != nullptr)
	{
		// serial the name of the shard

		shards.push_back({
		    .sid = (static_cast<uint32>(atoi(row[0]))),
		    .name = (ucstring::makeFromUtf8(row[1])),
		    .nbplayers = (static_cast<uint8>(atoi(row[2]))),
		});
		row = mysql_fetch_row(result);
	}

	return std::make_pair(shards, "");
}

pair<optional<LoginUserProjection>, string> CMysqlPersistence::createUser(const std::string &login, const std::string &cpassword)
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	string reason = sqlQuery("insert into user (Login, Password) values ('" + sqlEscape(login) + "', '" + sqlEscape(cpassword) + "')", nbrow, row, result);

	if (!reason.empty())
	{
		return std::make_pair(std::nullopt, reason);
	}

	return findUserByLogin(login);
}

pair<vector<AuthorizedUserProjection>, string> CMysqlPersistence::findAuthorizedUsers()
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	vector<AuthorizedUserProjection> users;
	string reason = sqlQuery("select UId, Cookie, Privilege, ExtendedPrivilege from user where State='Authorized'", nbrow, row, result);
	if (!reason.empty())
	{
		return std::make_pair(users, reason);
	}

	while (row != nullptr)
	{
		// serial the name of the shard
		CLoginCookie cookie;
		cookie.setFromString(row[1]);

		users.push_back({
		    .uid = row[0],
		    .cookie = cookie,
		    .privilege = row[2],
		    .extendedPrivilege = row[3],
		});
		row = mysql_fetch_row(result);
	}

	return std::make_pair(users, "");
}

pair<bool, string> CMysqlPersistence::existsShardById(const sint32& shardid) {
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	string reason = sqlQuery("select ShardId from shard where ShardId=" + toString(shardid), nbrow, row, result);
	if (!reason.empty())
	{
		return std::make_pair(false, reason);
	}

	return std::make_pair(nbrow != 0, reason);
}

string CMysqlPersistence::updateUserWaitingOnShard(const string& uid, const sint32& shardid) {
	return sqlQuery("update user set State='Waiting', ShardId=" + toString(shardid) + " where UId=" + uid);
}

string CMysqlPersistence::logoutUserById(const string& uid) {
	return sqlQuery("update user set state='Offline', ShardId=-1, Cookie='' where UId=" + uid);
}

string CMysqlPersistence::logoutUserByCookie(const CLoginCookie& cookie) {
	return sqlQuery("update user set state='Offline', ShardId=-1, Cookie='' where Cookie='" + sqlEscape(cookie.setToString()) + "'");
}

pair<optional<string>, string> CMysqlPersistence::findUserLoginById(const string& uid) {
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	string reason = sqlQuery("select Login from user where UId=" + uid, nbrow, row, result);
	if (!reason.empty())
	{
		return std::make_pair(std::nullopt, reason);
	}

	if (nbrow == 0)
	{
		return std::make_pair(std::nullopt, reason);
	}

	return std::make_pair(std::make_optional(row[0]), reason);
}

pair<vector<NotOfflineUserProjection>, string> CMysqlPersistence::findNotOfflineUsers()
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	vector<NotOfflineUserProjection> users;
	string reason = sqlQuery("select UId, State, Cookie from user where State!='Offline'", nbrow, row, result);
	if (!reason.empty())
	{
		return std::make_pair(users, reason);
	}

	while (row != nullptr)
	{
		// serial the name of the shard
		CLoginCookie cookie;
		cookie.setFromString(row[2]);

		users.push_back({
			.uid = row[0],
			.state = row[1],
			.cookie = cookie
		});
		row = mysql_fetch_row(result);
	}

	return std::make_pair(users, "");
}

