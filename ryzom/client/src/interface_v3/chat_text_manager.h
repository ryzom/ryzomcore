// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2017  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
	class CInterfaceGroup;
	class CGroupParagraph;
}

namespace NLMISC{
	class CCDBNodeLeaf;
}

/** Class to get chat text parameters, and to build new text lines
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CEmojiManager;

class CChatTextManager
{
public:
	/// How emoji are shown in chat. Stored in UI:SAVE:CHAT:EMOJI_MODE.
	enum TEmojiMode
	{
		EmojiText    = 0,	// leave ":smile:" as written
		EmojiUnicode = 1,	// substitute the unicode character, drawn from the font
		EmojiImage   = 2	// draw the image from the emoji atlas
	};

	/// How big emoji are drawn. Stored in UI:SAVE:CHAT:EMOJI_SIZE.
	enum TEmojiSize
	{
		EmojiSmall  = 0,	// same height as the chat text
		EmojiMedium = 1,
		EmojiLarge  = 2		// the tile's own size
	};

	//\name Text parameters. They are read from the interface database (the configuration of text is doned in config.xml)
	//@{
		uint		 getTextFontSize() const;
		uint		 getTextMultiLineSpace() const;
		bool		 isTextShadowed() const;
		uint		 getEmojiMode() const;
		uint		 getEmojiSize() const;
		/// Height in pixels an emoji image is drawn at, for the current settings.
		sint32		 getEmojiPixelSize() const;
	//@}
	/** Build a new text multiline using the current chat text settings
	  * \param msg the actual text
	  * \param col the color of the text
	  * \param justified Should be true for justified text (stretch spaces of line to fill the full width)
	  * \param plaintext Text will not be parsed for uri markup links
	  */
	NLGUI::CViewBase *createMsgText(const std::string &msg, NLMISC::CRGBA col, bool justified = false, bool plaintext = false);
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
	mutable NLMISC::CCDBNodeLeaf    *_EmojiMode;
	mutable NLMISC::CCDBNodeLeaf    *_EmojiSize;

	// ctor, private because of singleton
	CChatTextManager();
	~CChatTextManager();

	bool showTimestamps() const;

	NLGUI::CViewBase *createMsgTextSimple(const std::string &msg, NLMISC::CRGBA col, bool justified, NLGUI::CInterfaceGroup *commandGroup);
	NLGUI::CViewBase *createMsgTextComplex(const std::string &msg, NLMISC::CRGBA col, bool justified, bool plaintext, NLGUI::CInterfaceGroup *commandGroup);

	/** Append msg[from, to) to the paragraph as one text view.
	  *
	  * The piece is prefixed with the format tags in effect at \p from, so that
	  * splitting a line around a link or an emoji does not lose the colour that
	  * was already set. Without this the remainder of a line reverts to the
	  * caller's colour and things like /em or an inline @{RGBA} come out wrong.
	  */
	void addTextSegment(NLGUI::CGroupParagraph *para, const std::string &msg,
		std::string::size_type from, std::string::size_type to,
		NLMISC::CRGBA col, bool justified, const char *id = NULL);

	/// Build the image view for one emoji, or NULL if it has no usable texture.
	NLGUI::CViewBase *createEmojiView(const std::string &texture);
};

// shortcut to get text manager instance
inline CChatTextManager &getChatTextMngr() { return CChatTextManager::getInstance(); }

#endif
