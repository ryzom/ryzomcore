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

#ifndef NL_CONNEXION_H
#define NL_CONNEXION_H

#include <string>
#include <vector>

struct CShard
{
	CShard(const std::string &version, bool online, uint32 shardId, const std::string &name, uint32 nbPlayers, const std::string &wsAddr, const std::string &patchURL)
	{
		Version = version;
		Online = online;
		ShardId = shardId;
		Name = name;
		NbPlayers = nbPlayers;
		WsAddr = wsAddr;
		PatchURL = patchURL;
	}

	std::string Version;
	bool Online;
	uint32 ShardId;
	std::string Name;
	uint32 NbPlayers;
	std::string WsAddr;
	std::string PatchURL;
};

std::string checkLogin(const std::string &login, const std::string &password, const std::string &clientApp);
std::string selectShard(uint32 shardId, std::string &cookie, std::string &addr);

extern std::vector<CShard> Shards;



#endif // NL_CONNEXION_H

/* End of connexion.h */

