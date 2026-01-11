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



#ifndef _LOG_GEN_CHAT_H
#define _LOG_GEN_CHAT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Chat
{
	TLogNoContext_Chat();
	~TLogNoContext_Chat();
};



void _log_Chat_Chat(std::string channel, const NLMISC::CEntityId &senderCharId, const std::string &chatText, const std::list< NLMISC::CEntityId > &receiverCharIds, const char *_filename_, uint _lineNo_);
#define log_Chat_Chat(channel, senderCharId, chatText, receiverCharIds) \
	_log_Chat_Chat(channel, senderCharId, chatText, receiverCharIds, __FILE__, __LINE__)

void _log_Chat_Tell(const NLMISC::CEntityId &senderCharId, const NLMISC::CEntityId &receiverCharId, const std::string &chatText, const char *_filename_, uint _lineNo_);
#define log_Chat_Tell(senderCharId, receiverCharId, chatText) \
	_log_Chat_Tell(senderCharId, receiverCharId, chatText, __FILE__, __LINE__)


#endif

