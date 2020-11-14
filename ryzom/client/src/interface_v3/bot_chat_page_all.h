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



#ifndef CL_BOT_CHAT_PAGE_ALL_H
#define CL_BOT_CHAT_PAGE_ALL_H

class CBotChatPageTrade;
class CBotChatPagePlayerGift;
class CBotChatPageCreateGuild;
class CBotChatPageNews;
class CBotChatPageDynamicMission;
class CBotChatPageMission;
class CBotChatPageMissionEnd;
class CBotChatPageRingSessions;

/** container class that contains all bot chat pages
  */
class CBotChatPageAll
{
public:
	CBotChatPageTrade			*Trade;
	CBotChatPagePlayerGift		*PlayerGift;
	CBotChatPageMission			*ChooseMission;
	CBotChatPageCreateGuild		*CreateGuild;
	CBotChatPageNews			*News;
	CBotChatPageMissionEnd		*MissionEnd;
	CBotChatPageDynamicMission  *DynamicMission;
	CBotChatPageRingSessions	*RingSessions;
public:
	CBotChatPageAll();
	~CBotChatPageAll();
	// init pages
	void init();
	// some init to call after connection:ready is sent to server
	void initAfterConnectionReady();
};


extern CBotChatPageAll *BotChatPageAll;




#endif
