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



#ifndef _LOG_GEN_PLAYER_H
#define _LOG_GEN_PLAYER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Player
{
	TLogNoContext_Player();
	~TLogNoContext_Player();
};



void _log_Player_Connect(uint32 userId, const std::string &fromAddr, const char *_filename_, uint _lineNo_);
#define log_Player_Connect(userId, fromAddr) \
	_log_Player_Connect(userId, fromAddr, __FILE__, __LINE__)

void _log_Player_Disconnect(uint32 userId, bool crashed, const char *_filename_, uint _lineNo_);
#define log_Player_Disconnect(userId, crashed) \
	_log_Player_Disconnect(userId, crashed, __FILE__, __LINE__)


#endif

