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



#ifndef CL_BOT_CHAT_PAGE_MISSION_H
#define CL_BOT_CHAT_PAGE_MISSION_H

#include "bot_chat_page.h"
#include "obs_huge_list.h"
#include "game_share/mission_desc.h"

class CDBCtrlSheet;

/** A page from which the user can choose a mission
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date August 2003
  */
class CBotChatPageMission : public CBotChatPage
{
public:
	// ctor
	CBotChatPageMission();
	// from CBotChatPage
	virtual void begin();
	virtual void end();
	virtual void init();
	// a mission has been selected -> popup the confirmation dialog
	void selectMission(CDBCtrlSheet *missionSheet);
	// current selected mission has been accepted
	void acceptMission();

	// Set the current mission type, for special display
	void	setMissionClientType(MISSION_DESC::TClientMissionType mType) {_MType= mType;}

private:
	// an observer to update big mission list from littles pages in server database
	CHugeListObs _MissionPagesObs;
	CDBCtrlSheet *_CurrSel;
	MISSION_DESC::TClientMissionType _MType;
};







#endif
