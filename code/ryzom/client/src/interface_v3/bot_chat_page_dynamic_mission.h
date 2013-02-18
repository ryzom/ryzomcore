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



#ifndef CL_BOT_CHAT_PAGE_DYNAMIC_MISSION_H
#define CL_BOT_CHAT_PAGE_DYNAMIC_MISSION_H

#include "bot_chat_page.h"

namespace NLGUI
{
	class CDBGroupComboBox;
}

// number of choices that the player must make to create a dynamic mission
const uint DYNAMIC_MISSION_NUM_CHOICES = 3;
// max number of options per choice
const uint DYNAMIC_MISSION_MAX_NUM_OPTIONS = 8;

/** A page from which the user can create a dynamic mission
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date September 2003
  */
class CBotChatPageDynamicMission : public CBotChatPage
{
public:
	// ctor
	CBotChatPageDynamicMission();
	// from CBotChatPage
	virtual void begin();
	virtual void end();
	virtual void init();
	virtual void update();
	/** Must be called by action handler when selection in combo box has changed.
	  * This send a msg to the server and reset the description text id
	  */
	void		selectionChanged(uint choice);
	// force to regenerate current mission
	void		regen();
private:
	// The control for each choice list
	NLGUI::CDBGroupComboBox *_ChoiceCB[DYNAMIC_MISSION_NUM_CHOICES];
	// current choice for each group (-1 means that choice has not been made)
	sint			  _Choice[DYNAMIC_MISSION_NUM_CHOICES];
	// For each text ID, true if the text has been received
	bool			  _TextReceived[DYNAMIC_MISSION_NUM_CHOICES][DYNAMIC_MISSION_MAX_NUM_OPTIONS];

	//
	bool			  _MissionValid;
private:
	void		invalidateMission();
	//
	void		sendChoices();
};




#endif
