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

#ifndef NELNS_LOGIN_SERVICE_MYSQL_PERSISTENCE_H
#define NELNS_LOGIN_SERVICE_MYSQL_PERSISTENCE_H

#include <string>

#include <nelns/login_service/persistence.h>

class CMysqlPersistence : public IPersistence
{
public:
	CMysqlPersistence() = default;
	~CMysqlPersistence() override = default;

	void init() override;

	// user
	std::pair<std::optional<LoginUserProjection>, std::string> findUserByLogin(const std::string &login) override;

	std::string authorizeUser(sint32 uid, const NLNET::CLoginCookie &cookie) override;

	std::pair<std::optional<LoginUserProjection>, std::string> createUser(const std::string &login, const std::string &cpassword) override;

	std::pair<std::vector<AuthorizedUserProjection>, std::string> findAuthorizedUsers() override;

	std::string updateUserWaitingOnShard(const std::string &uid, const sint32 &shardid) override;

	std::string logoutUserById(const std::string &uid) override;

	std::string logoutUserByCookie(const NLNET::CLoginCookie& cookie) override;

	std::pair<std::optional<std::string>, std::string> findUserLoginById(const std::string &uid) override;

	std::pair<std::vector<NotOfflineUserProjection>, std::string> findNotOfflineUsers() override;

	// shard
	std::pair<std::vector<OnlineShardProjection>, std::string> findOnlineShardsByApplication(const std::string &application) override;

	std::pair<bool, std::string> existsShardById(const sint32 &shardid) override;
};

#endif // NELNS_LOGIN_SERVICE_PERSISTENCE_H
