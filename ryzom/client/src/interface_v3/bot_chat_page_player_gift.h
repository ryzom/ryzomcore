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



#ifndef CL_BOT_CHAT_PAGE_PLAYER_GIFT_H
#define CL_BOT_CHAT_PAGE_PLAYER_GIFT_H

#include "bot_chat_page.h"

/** A page for the user to make a gift to the bot
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date August 2003
  */
class CBotChatPagePlayerGift : public CBotChatPage
{
public:
	virtual void begin();
	virtual void end();

	// if true, the player gift was validated. => end() must not restoreAllItems()
	bool	PlayerGiftValidated;
};







#endif
