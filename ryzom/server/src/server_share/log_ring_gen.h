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



#ifndef _LOG_GEN_RING_H
#define _LOG_GEN_RING_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"


#include "game_share/ring_session_manager_itf.h"


/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Ring
{
	TLogNoContext_Ring();
	~TLogNoContext_Ring();
};



void _log_Ring_EnterSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_);
#define log_Ring_EnterSession(charId, sessionId) \
	_log_Ring_EnterSession(charId, sessionId, __FILE__, __LINE__)

void _log_Ring_LeaveSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_);
#define log_Ring_LeaveSession(charId, sessionId) \
	_log_Ring_LeaveSession(charId, sessionId, __FILE__, __LINE__)


#endif

