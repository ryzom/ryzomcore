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



#ifndef CL_BOT_CHAT_PAGE_H
#define CL_BOT_CHAT_PAGE_H

#define BOT_CHAT_BASE_DB_PATH "SERVER:BOTCHAT"

/** Base class for botchat pages.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date August 2003
  */
class CBotChatPage

{
public:
	virtual ~CBotChatPage() {}
	// start that page (display and setup ui)
	virtual void begin();
	// init page
	virtual void init() {}
	// update function : it is called at each frame.
	virtual void update() {}
	virtual void end() = 0;
	// tool fct : activate a window from its name
	void activateWindow(const char *windowName, bool active);
};




#endif
