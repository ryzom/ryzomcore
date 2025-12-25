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

#ifndef NELNS_LOGIN_SERVICE_PERSISTENCE_H
#define NELNS_LOGIN_SERVICE_PERSISTENCE_H

#include "nel/net/login_cookie.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <nel/misc/types_nl.h>

struct LoginUserProjection
{
	sint32 uid;
	std::string password;
	std::string state;
};

struct AuthorizedUserProjection
{
	std::string uid;
	NLNET::CLoginCookie cookie;
	std::string privilege;
	std::string extendedPrivilege;
};

struct NotOfflineUserProjection
{
	std::string uid;
	std::string state;
	NLNET::CLoginCookie cookie;
};

struct OnlineShardProjection
{
	uint32 sid;
	ucstring name;
	uint8 nbplayers;
};

struct ShardProjection
{
	uint32 sid;
	ucstring name;
	uint8 nbplayers;
};

class IPersistence
{
public:

	IPersistence () = default;
	virtual ~IPersistence () = default;

	virtual void init() = 0;

	virtual std::pair<std::optional<LoginUserProjection>, std::string> findUserByLogin(const std::string& login) = 0;

	virtual std::string authorizeUser(sint32 uid, const NLNET::CLoginCookie & cookie) = 0;

	virtual std::pair<std::optional<LoginUserProjection>, std::string> createUser(const std::string& login, const std::string& cpassword) = 0;

	virtual std::pair<std::vector<AuthorizedUserProjection>, std::string> findAuthorizedUsers() = 0;

	virtual std::string updateUserWaitingOnShard(const std::string& uid, const sint32& shardid) = 0;

	virtual std::string logoutUserById(const std::string& uid) = 0;

	virtual std::string logoutUserByCookie(const NLNET::CLoginCookie& cookie) = 0;

	virtual std::pair<std::optional<std::string>, std::string> findUserLoginById(const std::string& uid) = 0;

	virtual std::pair<std::vector<NotOfflineUserProjection>, std::string> findNotOfflineUsers() = 0;


	virtual std::pair<std::vector<OnlineShardProjection>, std::string> findOnlineShardsByApplication(const std::string& application) = 0;

	virtual std::pair<bool, std::string> existsShardById(const sint32& shardid) = 0;
};

#endif // NELNS_LOGIN_SERVICE_PERSISTENCE_H
