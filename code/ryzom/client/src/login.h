// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef CL_LOGIN_H
#define CL_LOGIN_H

#include "nel/misc/types_nl.h"
#include "game_share/http_client.h"
#include <string>
#include <vector>

struct CShard
{
	CShard(const std::string &version, bool online, uint32 shardId, const std::string &name, uint32 nbPlayers, const std::string &wsAddr, const std::string &emergencyPatchURL)
	{
		Version		= version;
		Online		= online;
		ShardId		= shardId;
		Name		= name;
		NbPlayers	= nbPlayers;
		WsAddr		= wsAddr;
		EmergencyPatchURL	= emergencyPatchURL;
	}

	std::string Version;
	bool		Online;
	uint32		ShardId;
	std::string Name;
	uint32		NbPlayers;
	std::string WsAddr;
	std::vector<std::string>	PatchURIs;
	std::string EmergencyPatchURL;
};

extern std::string LoginLogin, LoginPassword;
extern uint32 LoginShardId;


extern uint32 AvailablePatchs;



std::string checkLogin(const std::string &login, const std::string &password, const std::string &clientApp);
std::string selectShard(uint32 shardId, std::string &cookie, std::string &addr);
std::string getBGDownloaderCommandLine();

// connection with the server. (login, shard list, etc.).
bool login();
void loginIntro();

// force patch for the mainland part
void mainLandPatch();

extern std::vector<CShard> Shards;
extern sint32		ShardSelected;

// TODO : nico : put this in an external file, this way it isn't included by the background downloader
#ifndef RY_BG_DOWNLOADER

/*
 * HTTP client preconfigured to connect to the startup login host
 */
class CStartupHttpClient : public CHttpClient
{
public:

	bool connectToLogin();
};

extern CStartupHttpClient HttpClient;

#endif // RY_BG_DOWNLOADER

#endif // CL_LOGIN_H

/* End of login.h */
