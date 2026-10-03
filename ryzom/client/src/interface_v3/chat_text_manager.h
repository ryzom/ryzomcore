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
#include "game_share/chat_group.h"
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

class CChatMessage;
class CChatMessageParagraph;

namespace NLGUI
{
	class CViewBase;
	class CInterfaceGroup;
	class CGroupEditBox;
	class CGroupParagraph;
	class CCtrlTabButton;
}

namespace NLMISC{
	class CEntityId;
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
	// UI:SAVE:CHAT:EMOJI_MODE
	enum TEmojiMode
	{
		EmojiText    = 0,
		EmojiUnicode = 1,
		EmojiImage   = 2
	};

	// UI:SAVE:CHAT:EMOJI_SIZE
	enum TEmojiSize
	{
		EmojiSmall  = 0,
		EmojiMedium = 1,
		EmojiLarge  = 2
	};

	//\name Text parameters. They are read from the interface database (the configuration of text is doned in config.xml)
	//@{
		uint		 getTextFontSize() const;
		uint		 getTextMultiLineSpace() const;
		bool		 isTextShadowed() const;
		uint		 getEmojiMode() const;
		uint		 getEmojiSize() const;
		sint32		 getEmojiPixelSize() const;

		static const sint32 EmojiTilePixels = 32;
	//@}
	/** Build a new text multiline using the current chat text settings
	  * \param msg the actual text
	  * \param col the color of the text
	  * \param justified Should be true for justified text (stretch spaces of line to fill the full width)
	  * \param plaintext Text will not be parsed for uri markup links
	  */
	NLGUI::CViewBase *createMsgText(const std::string &msg, NLMISC::CRGBA col, bool justified = false, bool plaintext = false);
	NLGUI::CViewBase *createMsgText(const std::string &prefix, const CChatMessage &message, NLMISC::CRGBA col,
		bool justified = false, CChatGroup::TGroupType group = CChatGroup::nbChatMode);
	std::string getMessageText(const CChatMessage &message, CChatGroup::TGroupType group) const;
	bool isChatInput(NLGUI::CGroupEditBox *editBox) const;
	void setMessageTarget(NLGUI::CViewBase *view, CChatGroup::TGroupType group,
		const NLMISC::CEntityId &dynamicChannelId, const std::string &receiver = std::string());
	void setMessageSender(NLGUI::CViewBase *view, const std::string &sender);
	const CChatMessage *getSelectedMessage() const;
	bool getSelectedMessageTarget(CChatGroup::TGroupType &group,
		NLMISC::CEntityId &dynamicChannelId, std::string &receiver) const;
	std::string getQuoteMessageId(const NLGUI::CGroupEditBox *editBox) const;
	bool getQuoteTarget(const NLGUI::CGroupEditBox *editBox, CChatGroup::TGroupType &group,
		NLMISC::CEntityId &dynamicChannelId, std::string &receiver) const;
	// Update the reaction rows of the reacted message.
	void applyReaction(const CChatMessage &message);
	// React to the message chosen in the chat menu or in its reaction row.
	void react(const std::string &emoji);
	uint32 beginQuoteSend(NLGUI::CGroupEditBox *editBox);
	// Empty the input once the request is queued; it comes back if the server refuses it.
	void clearSentInput(uint32 requestId);
	void finishQuoteSend(uint32 requestId, bool accepted);
	void checkQuoteSendTimeout();
	void failPendingQuoteSends();
	void quoteSelectedMessage(NLGUI::CGroupEditBox *editBox);
	void clearQuote(NLGUI::CGroupEditBox *editBox);
	std::string getSelectedPlayerName() const;
	bool isMentionInput(NLGUI::CGroupEditBox *editBox) const;
	void setMentionTab(NLGUI::CViewBase *message, NLGUI::CCtrlTabButton *tab);
	bool hasUnreadMention(const NLGUI::CCtrlTabButton *tab) const;
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
	// Append msg[from, to) with the format tags in effect at from.
	void addTextSegment(NLGUI::CGroupParagraph *para, const std::string &msg,
		std::string::size_type from, std::string::size_type to,
		NLMISC::CRGBA col, bool justified, const char *id = NULL);

	// NULL when the atlas has no tile for it.
	NLGUI::CViewBase *createEmojiView(const std::string &texture, const std::string &name);
	// Rebuild the reaction row below a chat line.
	void updateReactionViews(CChatMessageParagraph *paragraph);
	void addMsgText(NLGUI::CGroupParagraph *paragraph, const std::string &msg, NLMISC::CRGBA col, bool justified,
		const std::vector<std::pair<size_t, size_t> > *mentions = NULL);
	NLGUI::CViewBase *createMsgTextComplex(const std::string &msg, NLMISC::CRGBA col, bool justified, bool plaintext,
		NLGUI::CInterfaceGroup *commandGroup, const std::vector<std::pair<size_t, size_t> > *mentions = NULL);
};

// shortcut to get text manager instance
inline CChatTextManager &getChatTextMngr() { return CChatTextManager::getInstance(); }

#endif
