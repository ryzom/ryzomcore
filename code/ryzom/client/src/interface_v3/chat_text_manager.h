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



#ifndef	CHAT_TEXT_MANAGER_H
#define CHAT_TEXT_MANAGER_H

#include "nel/misc/rgba.h"

namespace NLGUI
{
	class CViewBase;
}

class ucstring;
namespace NLMISC{
	class CCDBNodeLeaf;
}

/** Class to get chat text parameters, and to build new text lines
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChatTextManager
{
public:
	//\name Text parameters. They are read from the interface database (the configuration of text is doned in config.xml)
	//@{
		uint		 getTextFontSize() const;
		uint		 getTextMultiLineSpace() const;
		bool		 isTextShadowed() const;
	//@}
	/** Build a new text multiline using the current chat text settings
	  * \param msg the actual text
	  * \param col the color of the text
	  * \param justified Should be true for justified text (stretch spaces of line to fill the full width)
	  */
	NLGUI::CViewBase *createMsgText(const ucstring &msg, NLMISC::CRGBA col, bool justified = false);
	// Singleton access
	static CChatTextManager &getInstance();

	// release memory
	static void releaseInstance();

	// Reset the manager
	void reset();

private:
	static CChatTextManager *_Instance;

	mutable NLMISC::CCDBNodeLeaf    *_TextFontSize;
	mutable NLMISC::CCDBNodeLeaf    *_TextMultilineSpace;
	mutable NLMISC::CCDBNodeLeaf    *_TextShadowed;
	mutable NLMISC::CCDBNodeLeaf    *_ShowTimestamps;

	// ctor, private because of singleton
	CChatTextManager();
	~CChatTextManager();

	bool showTimestamps() const;
};

// shortcut to get text manager instance
inline CChatTextManager &getChatTextMngr() { return CChatTextManager::getInstance(); }

#endif
