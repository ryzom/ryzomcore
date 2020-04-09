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



#include "stdpch.h"
#include "bot_chat_page_ring_sessions.h"
#include "../user_entity.h"
#include "../entities.h"



#define WIN_BOT_CHAT_PAGE_RING_SESSION "ui:interface:ring_sessions"

// *************************************************
CBotChatPageRingSessions::CBotChatPageRingSessions()
{
	RingAccessPointPos.set(0.f, 0.f, 0.f);
}


// *************************************************
void CBotChatPageRingSessions::begin()
{
	CBotChatPage::begin();
	activateWindow(WIN_BOT_CHAT_PAGE_RING_SESSION, true);
	if (UserEntity->trader() != CLFECOMMON::INVALID_SLOT)
	{
		RingAccessPointPos = EntitiesMngr.entity(UserEntity->trader())->pos();
	}
}

// *************************************************
void CBotChatPageRingSessions::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_RING_SESSION, false);
}




