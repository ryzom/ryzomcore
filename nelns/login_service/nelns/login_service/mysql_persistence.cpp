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
using std::optional;
using std::pair;
using std::string;

void CMysqlPersistence::init()
{
	sqlInit();
}

std::pair<std::optional<LoginUserProjection>, std::string> CMysqlPersistence::findUserByLogin(const std::string &login)
{
	CMysqlResult queryResult;
	MYSQL_ROW row;
	sint32 nbrow;
	string reason = sqlQuery("select uid, password, state from user where Login='" + login + "'", nbrow, row, queryResult);

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

	LoginUserProjection result {
		.uid = -1,
		.password = row[1],
		.state = row[2]
	};
	NLMISC::fromString(row[0], result.uid);

	return std::make_pair(std::make_optional(result), "");
}

std::string CMysqlPersistence::authorizeUser(sint32 uid, const NLNET::CLoginCookie &cookie)
{
	return sqlQuery("update user set state='Authorized', Cookie='" + cookie.setToString() + "' where UId=" + toString(uid));
}
