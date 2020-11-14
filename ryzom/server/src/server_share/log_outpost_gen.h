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



#ifndef _LOG_GEN_OUTPOST_H
#define _LOG_GEN_OUTPOST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Outpost
{
	TLogNoContext_Outpost();
	~TLogNoContext_Outpost();
};



void _log_Outpost_Challenge(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_);
#define log_Outpost_Challenge(outpostName, ownerGuildName, challengerGuildName) \
	_log_Outpost_Challenge(outpostName, ownerGuildName, challengerGuildName, __FILE__, __LINE__)

void _log_Outpost_ChallengeWin(const std::string &outpostName, const std::string &oldOwnerGuildName, const std::string &newOwnerGuildName, uint32 winLevel, const char *_filename_, uint _lineNo_);
#define log_Outpost_ChallengeWin(outpostName, oldOwnerGuildName, newOwnerGuildName, winLevel) \
	_log_Outpost_ChallengeWin(outpostName, oldOwnerGuildName, newOwnerGuildName, winLevel, __FILE__, __LINE__)

void _log_Outpost_ChallengeLost(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_);
#define log_Outpost_ChallengeLost(outpostName, ownerGuildName, challengerGuildName) \
	_log_Outpost_ChallengeLost(outpostName, ownerGuildName, challengerGuildName, __FILE__, __LINE__)

void _log_Outpost_BuyOption(const std::string &outpostName, const std::string &ownerGuildName, NLMISC::CSheetId buildingSheet, const char *_filename_, uint _lineNo_);
#define log_Outpost_BuyOption(outpostName, ownerGuildName, buildingSheet) \
	_log_Outpost_BuyOption(outpostName, ownerGuildName, buildingSheet, __FILE__, __LINE__)


#endif

