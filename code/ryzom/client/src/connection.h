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



#ifndef CL_CONNECTION_H
#define CL_CONNECTION_H

#include "nel/misc/types_nl.h"
//#include "game_share/jobs.h"
#include "game_share/mainland_summary.h"

extern uint8 ServerPeopleActive;
extern uint8 ServerCareerActive;

extern std::vector<CMainlandSummary>	Mainlands;
extern uint8 PlayerSelectedSlot;
extern std::string	PlayerSelectedFileName;
extern TSessionId	PlayerSelectedMainland;							// This is the mainland selected at the SELECT perso!!
extern ucstring		PlayerSelectedHomeShardName;					// The home shard name (aniro, leanon etc....)
extern ucstring		PlayerSelectedHomeShardNameWithParenthesis;		// Same with parenthesis
extern std::vector<CCharacterSummary>	CharacterSummaries;
extern std::string UserPrivileges;

extern ucstring NewKeysCharNameWanted;
extern ucstring NewKeysCharNameValidated;
extern std::string GameKeySet;
extern std::string RingEditorKeySet;



bool hasPrivilegeDEV();
bool hasPrivilegeSGM();
bool hasPrivilegeGM();
bool hasPrivilegeSG();
bool hasPrivilegeG();
bool hasPrivilegeEM();
bool hasPrivilegeEG();
bool hasPrivilegeVG();


// connection with the server. (login, shard list, etc.).
bool connection(const std::string &cookie, const std::string &fsaddr);

// reselect character after reconnection
bool reconnection();

enum TInterfaceState
{
	AUTO_LOGIN,			// -> GLOBAL_MENU, QUIT (if connection errors)
	GLOBAL_MENU,		// -> SELECT_CHARACTER, QUIT (if connection errors)
	GOGOGO_IN_THE_GAME,	// -> launch the game
	QUIT_THE_GAME		// -> quit the game
};

// All the functions associated to the Finite State Machine
TInterfaceState autoLogin (const std::string &cookie, const std::string &fsaddr, bool firstConnection);


std::string buildPlayerNameForSaveFile(const ucstring &playerNameIn);


void globalMenuMovieShooter();

void updateBGDownloaderUI();

// compute patcher priority, depending on the presence of one or more mainland characters : in this case, give the patch a boost
void updatePatcherPriorityBasedOnCharacters();

#endif // CL_CONNECTION_H

/* End of connection.h */
