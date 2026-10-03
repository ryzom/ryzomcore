// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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



#include "stdpch.h"
// client
#include "game_share/chat_message.h"
#include "game_share/character_title.h"
#include "chat_text_manager.h"
#include "chat_link_ui.h"
#include "emoji_manager.h"
#include "emoji_picker.h"
#include "../client_chat_manager.h"
#include "nel/gui/group_container.h"
#include "chat_window.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_menu.h"
#include "nel/misc/utf_string_view.h"
#include "nel/misc/sstring.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/group_tab.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/ctrl_tooltip.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "interface_manager.h"
#include "people_interraction.h"
#include "misc.h"
#include "nel/misc/i18n.h"
#include "nel/misc/time_nl.h"
#include "../entity_cl.h"
#include "../entities.h"
#include "../user_entity.h"
#include "../connection.h"
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;

extern CClientChatManager ChatMngr;

CChatTextManager* CChatTextManager::_Instance = NULL;

// last selected chat from 'copy_chat_popup' action handler
static std::string LastSelectedChat;
static CSheetId LastSelectedChatSheetId = CSheetId::Unknown;
static CChatMessage LastSelectedMessage;
static bool LastSelectedHasMessage = false;
static CChatGroup::TGroupType LastSelectedGroup = CChatGroup::nbChatMode;
static CEntityId LastSelectedDynamicChannelId = CEntityId::Unknown;
static std::string LastSelectedReceiver;
static std::string LastSelectedSender;
static CRefPtr<CGroupEditBox> LastSelectedChatInput;

struct CChatQuoteDraft
{
	CChatQuoteDraft() : Group(CChatGroup::nbChatMode), DynamicChannelId(CEntityId::Unknown),
		HistoryOffset(0), MinHeightOffset(0), RequestId(0), SentAt(0), SentRevision(0), InputCleared(false) {}
	CRefPtr<CGroupEditBox> EditBox;
	CChatMessage Message;
	CChatGroup::TGroupType Group;
	CEntityId DynamicChannelId;
	string Receiver;
	sint32 HistoryOffset;
	sint32 MinHeightOffset;
	uint32 RequestId;
	TTime SentAt;
	string SentText;
	vector<CGroupEditBox::CTextTag> SentTags;
	uint64 SentRevision;
	bool InputCleared;
};
static list<CChatQuoteDraft> ChatQuoteDrafts;
static bool setQuotePreviewActive(CGroupEditBox *editBox, bool active, sint32 &historyOffset, sint32 &minHeightOffset);
static uint32 NextQuoteRequestId = 0;
static const TTime QuoteSendTimeout = 60 * 1000;

class CChatMessageParagraph : public CGroupParagraph
{
public:
	CChatMessageParagraph(const TCtorParam &param) :
		CGroupParagraph(param),
		HasMessage(false),
		Group(CChatGroup::nbChatMode),
		DynamicChannelId(CEntityId::Unknown)
	{}
	~CChatMessageParagraph();
	virtual void draw();
	bool HasMessage;
	CChatMessage Message;
	CChatGroup::TGroupType Group;
	CEntityId DynamicChannelId;
	std::string Receiver;
	std::string Sender;
	std::vector<CRefPtr<CViewBase> > MentionViews;
	CRefPtr<CCtrlTabButton> MentionTab;
	std::vector<CRefPtr<CViewBase> > ReactionViews;

	// False when there was no reaction row.
	bool clearReactionViews()
	{
		if (ReactionViews.empty())
			return false;
		for (uint i = 0; i < ReactionViews.size(); ++i)
			if (ReactionViews[i])
				delChild(ReactionViews[i]);
		ReactionViews.clear();
		return true;
	}
};

//=================================================================================
static bool getMentionInputRange(CGroupEditBox *editBox, uint32 &start, uint32 &end)
{
	if (!editBox)
		return false;
	const std::string &handler = editBox->getAHOnEnter();
	CChatWindow *chatWindow = handler == "chat_box_entry" ?
		getChatWndMgr().getChatWindowFromCaller(editBox) : NULL;
	if (handler != "contact_entry" && (!chatWindow || chatWindow->getEditBox() != editBox))
		return false;
	const ::u32string &text = editBox->getInputStringRef();
	const sint32 cursor = editBox->getCursorPos();
	if (cursor <= 0 || (uint32)cursor > text.size())
		return false;
	start = (uint32)cursor;
	while (start && CHAT_MESSAGE::isMentionNameChar(text[start - 1]))
		--start;
	if (!start || text[start - 1] != '@')
		return false;
	--start;
	if (start && !CHAT_MESSAGE::isMentionBoundary(text[start - 1]))
		return false;
	const ucstring utf16 = CUtfStringView(text).toUtf16();
	const uint32 utf16Start = (uint32)CUtfStringView(text.substr(0, start)).toUtf16().size();
	if (CHAT_MESSAGE::isMentionProtectedText(utf16, utf16Start))
		return false;
	end = (uint32)cursor;
	while (end < text.size() && CHAT_MESSAGE::isMentionNameChar(text[end]))
		++end;
	return true;
}

//=================================================================================
bool CChatTextManager::isMentionInput(CGroupEditBox *editBox) const
{
	uint32 start, end;
	return getMentionInputRange(editBox, start, end);
}

// ***************************************************************************
class CHandlerChatMention : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *caller, const std::string &params)
	{
		CGroupEditBox *editBox = dynamic_cast<CGroupEditBox *>(caller);
		if (!editBox)
			return;
		const std::string uiId = CSString(editBox->getId()).quote(true, false);
		uint32 start, end;
		if (CWidgetManager::getInstance()->getCaptureKeyboard() != editBox ||
			!editBox->isActiveThroughParents() || !getMentionInputRange(editBox, start, end))
		{
			CLuaManager::getInstance().executeLuaScript("SearchCommand:clear_mention(" + uiId + ")");
			return;
		}
		const ::u32string &text = editBox->getInputStringRef();
		::u32string prefix = text.substr(start + 1, editBox->getCursorPos() - start - 1);
		CCDBNodeLeaf *enabled = CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:CHAT_AUTOCOMPLETE", false);
		if (!enabled || !enabled->getValueBool())
		{
			CLuaManager::getInstance().executeLuaScript("SearchCommand:clear_mention(" + uiId + ")");
			return;
		}
		if (params.compare(0, 10, "candidate=") == 0 || params.compare(0, 5, "name=") == 0 ||
			params.compare(0, 12, "number_name=") == 0)
		{
			const bool candidate = params.compare(0, 10, "candidate=") == 0;
			const bool number = params.compare(0, 12, "number_name=") == 0;
			if (number)
			{
				if (prefix.empty() || prefix.back() < '1' || prefix.back() > '9')
					return;
				prefix.erase(prefix.size() - 1);
			}
			const std::string name = params.substr(candidate ? 10 : number ? 12 : 5);
			const ::u32string decoded = CUtfStringView(name).toUtf32();
			if (decoded.empty() || decoded.size() < prefix.size() ||
				compareCaseInsensitive(CUtfStringView(decoded.substr(0, prefix.size())).toUtf8(), CUtfStringView(prefix).toUtf8()) != 0)
				return;
			for (uint i = 0; i < decoded.size(); ++i)
				if (!CHAT_MESSAGE::isMentionNameChar(decoded[i]))
					return;
			if (candidate)
			{
				const std::string quotedName = CSString(name).quote(true, false);
				CLuaManager::getInstance().executeLuaScript("if SearchCommand:find(SearchCommand.valid_commands_list," + quotedName +
					") == nil then table.insert(SearchCommand.valid_commands_list," + quotedName + ") end");
				return;
			}
			const ::u32string replacement = ::u32string(1, '@') + decoded;
			const ::u32string completed = text.substr(0, start) + replacement + text.substr(end);
			if (CUtfStringView(completed).toUtf16().size() > CHAT_MESSAGE::MaxTextLength)
				return;
			const sint32 oldCursor = editBox->getCursorPos();
			const sint32 oldSelectCursor = CGroupEditBox::getSelectCursorPos();
			CGroupEditBoxBase *oldSelection = CGroupEditBoxBase::getCurrSelection();
			editBox->setCursorPos((sint32)start);
			CGroupEditBox::setSelectCursorPos((sint32)end);
			CGroupEditBoxBase::setCurrSelection(editBox);
			if (!editBox->writeString(CUtfStringView(replacement).toUtf8(), true, false, false))
			{
				editBox->setCursorPos(oldCursor);
				CGroupEditBox::setSelectCursorPos(oldSelectCursor);
				CGroupEditBoxBase::setCurrSelection(oldSelection);
				return;
			}
			editBox->invalidateCoords();
			editBox->setFocusOnText();
			editBox->setCursorPos((sint32)(start + replacement.size()));
			CGroupEditBox::setSelectCursorPos(editBox->getCursorPos());
			CGroupEditBoxBase::setCurrSelection(NULL);
			return;
		}
		std::vector<std::string> names;
		for (uint i = 0; i < PeopleInterraction.FriendList.getNumPeople(); ++i)
			names.push_back(PeopleInterraction.FriendList.getName(i));
		std::string players;
		for (uint i = 0; i < names.size(); ++i)
		{
			if (i) players += ',';
			players += CSString(names[i]).quote(true, false);
		}
		if (params == "number")
		{
			if (prefix.empty() || prefix.back() < '1' || prefix.back() > '9')
				return;
			const std::string key = editBox->getId() + ":" + toString("%u", start) + ":" +
				CUtfStringView(prefix.substr(0, prefix.size() - 1)).toUtf8();
			CLuaManager::getInstance().executeLuaScript("SearchCommand:check_autocomplet_number(" + uiId + "," +
				toString("%u", prefix.back() - '0') + "," + CSString(key).quote(true, false) + ",{" + players + "})");
			return;
		}
		CLuaManager::getInstance().executeLuaScript("SearchCommand:search_mention(" + uiId + "," +
			CSString(CUtfStringView(prefix).toUtf8()).quote(true, false) + "," + toString("%u", start) +
			",{" + players + "}," + (params == "complete" ? "true" : "false") + ")");
		if (params == "complete")
			CLuaManager::getInstance().executeLuaScript("SearchCommand:check_autocomplet(" + uiId + ")");
	}
};
REGISTER_ACTION_HANDLER(CHandlerChatMention, "chat_mention");

static std::list<CChatMessageParagraph *> ChatMentionViews;
static std::map<std::string, bool> ChatMentionUnread;

// Players who reacted to a message with one emoji, in reaction order.
struct CChatReaction
{
	std::string Emoji;
	std::vector<CEntityId> Reactors;
	std::vector<std::string> Names;
};
// Keeps a reaction row of a chat line short.
static const uint ChatReactionEmojiLimit = 20;
static std::multimap<std::string, CChatMessageParagraph *> ChatMessageParagraphs;
static std::map<std::string, std::vector<CChatReaction> > ChatReactions;
// Message and channel the next reaction goes to.
static std::string ReactionMessageId;
static CChatGroup::TGroupType ReactionGroup = CChatGroup::nbChatMode;
static CEntityId ReactionDynamicChannelId;
static std::string ReactionReceiver;

//=================================================================================
static uint32 getLocalCharacterId()
{
	return NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
}

//=================================================================================
static void registerMessageParagraph(CChatMessageParagraph *paragraph)
{
	if (!paragraph->Message.MessageId.empty())
		ChatMessageParagraphs.insert(make_pair(paragraph->Message.MessageId, paragraph));
}

//=================================================================================
static void setReactionTarget(const string &messageId, CChatGroup::TGroupType group,
	const CEntityId &dynamicChannelId, const string &receiver)
{
	ReactionMessageId = messageId;
	ReactionGroup = group;
	ReactionDynamicChannelId = dynamicChannelId;
	ReactionReceiver = receiver;
}

struct CChatMentionTab : public IOnReceiveTextId
{
	CChatMentionTab() : TextModifier(NULL) {}
	~CChatMentionTab()
	{
		if (TextId != NULL && TextId->getOnReceiveTextId() == this)
			TextId->setOnReceiveTextId(TextModifier);
	}
	virtual void onReceiveTextId(std::string &str)
	{
		if (TextModifier)
			TextModifier->onReceiveTextId(str);
		Text = str;
		MarkedText = "@ " + str;
		str = MarkedText;
		setCase(MarkedText, TextId->getCaseMode());
	}
	CRefPtr<CCtrlTabButton> Tab;
	CRefPtr<CViewTextID> TextId;
	IOnReceiveTextId *TextModifier;
	CRGBA PushedColor, OverColor;
	std::string Text, MarkedText;
};
static std::list<CChatMentionTab> ChatMentionTabs;

//=================================================================================
static bool hasUnreadChatMention(const CCtrlTabButton *tab)
{
	for (std::list<CChatMessageParagraph *>::const_iterator it = ChatMentionViews.begin(); it != ChatMentionViews.end(); ++it)
	{
		std::map<std::string, bool>::const_iterator unread = ChatMentionUnread.find((*it)->Message.MessageId);
		if ((*it)->MentionTab == tab && unread != ChatMentionUnread.end() && unread->second)
			return true;
	}
	return false;
}

//=================================================================================
static void updateMentionTabs()
{
	for (std::list<CChatMentionTab>::iterator it = ChatMentionTabs.begin(); it != ChatMentionTabs.end();)
	{
		if (it->Tab == NULL)
		{
			it = ChatMentionTabs.erase(it);
			continue;
		}
		CViewText *text = it->Tab->getViewText();
		if (!hasUnreadChatMention(it->Tab))
		{
			if (text && text->getText() == it->MarkedText)
				text->setHardText(it->Text);
			it->Tab->setTextColorNormal(CRGBA::stringToRGBA(CWidgetManager::getInstance()->getParser()->getDefine("chat_group_tab_color_normal").c_str()));
			it->Tab->setTextColorPushed(it->PushedColor);
			it->Tab->setTextColorOver(it->OverColor);
			it = ChatMentionTabs.erase(it);
			continue;
		}
		if (text && (it->MarkedText.empty() || text->getText() != it->MarkedText))
		{
			it->Text = text->getHardText();
			it->MarkedText = "@ " + text->getText();
			text->setText(it->MarkedText);
			it->MarkedText = text->getText();
		}
		const CRGBA color = CRGBA::stringToRGBA(CWidgetManager::getInstance()->getParser()->getDefine("chat_group_tab_color_mention").c_str());
		it->Tab->setTextColorNormal(color);
		it->Tab->setTextColorPushed(color);
		it->Tab->setTextColorOver(color);
		++it;
	}
}

//=================================================================================
CChatMessageParagraph::~CChatMessageParagraph()
{
	typedef std::multimap<std::string, CChatMessageParagraph *>::iterator TParagraphIt;
	const std::pair<TParagraphIt, TParagraphIt> paragraphs = ChatMessageParagraphs.equal_range(Message.MessageId);
	for (TParagraphIt it = paragraphs.first; it != paragraphs.second; ++it)
		if (it->second == this)
		{
			ChatMessageParagraphs.erase(it);
			break;
		}
	if (ChatMessageParagraphs.find(Message.MessageId) == ChatMessageParagraphs.end())
		ChatReactions.erase(Message.MessageId);
	ChatMentionViews.remove(this);
	bool retained = false;
	for (std::list<CChatMessageParagraph *>::const_iterator it = ChatMentionViews.begin(); it != ChatMentionViews.end(); ++it)
		if ((*it)->Message.MessageId == Message.MessageId)
			retained = true;
	if (!retained)
		ChatMentionUnread.erase(Message.MessageId);
	updateMentionTabs();
}

//=================================================================================
void CChatMessageParagraph::draw()
{
	CGroupParagraph::draw();
	std::map<std::string, bool>::iterator unread = ChatMentionUnread.find(Message.MessageId);
	if (unread == ChatMentionUnread.end() || !unread->second || CWidgetManager::getInstance()->getModalWindow())
		return;
	for (CInterfaceGroup *parent = this; parent; parent = parent->getParent())
		if (!parent->getActive())
			return;
	sint32 clipX, clipY, clipW, clipH;
	getClip(clipX, clipY, clipW, clipH);
	CWidgetManager *widgets = CWidgetManager::getInstance();
	CInterfaceGroup *window = widgets->getWindow(this);
	if (!window)
		return;
	// A mention is read only when all its wrapped lines are visible and unobscured.
	for (uint i = 0; i < MentionViews.size(); ++i)
	{
		CViewText *view = dynamic_cast<CViewText *>((CViewBase *)MentionViews[i]);
		if (!view || !view->getActive() || view->getWReal() <= 0 || view->getHReal() <= 0)
			continue;
		bool readable = true, hasCharacters = false;
		for (uint line = 0; readable && line < view->getNumLine(); ++line)
		{
			const sint start = view->getLineStartIndex(line);
			sint end;
			bool previousLineEnd;
			view->getLineEndIndex(line, end, previousLineEnd);
			if (start < 0 || end <= start)
				continue;
			float x0, y0, x1, y1, height;
			view->getCharacterPositionFromIndex(start, false, x0, y0, height);
			view->getCharacterPositionFromIndex(end, previousLineEnd, x1, y1, height);
			// NeL applies FirstLineX to character positions on continuation lines,
			// while line-end positions omit it even on the first line.
			if (line != 0)
				x0 -= view->getFirstLineX();
			if (line == 0)
				x1 += view->getFirstLineX();
			const sint32 left = (sint32)floorf(view->getXReal() + x0);
			const sint32 right = (sint32)ceilf(view->getXReal() + x1);
			const sint32 bottom = (sint32)floorf(view->getYReal() + y0);
			const sint32 top = (sint32)ceilf(view->getYReal() + y0 + view->getFontHeight());
			if (right <= left)
				continue;
			hasCharacters = true;
			readable = left >= clipX && right <= clipX + clipW && bottom >= clipY && top <= clipY + clipH &&
				widgets->getWindowUnder((left + right - 1) / 2, (bottom + top - 1) / 2) == window;
			// A popup can cover only an edge of the line and miss its midpoint.
			const vector<CWidgetManager::SMasterGroup> &masterGroups = widgets->getAllMasterGroup();
			for (uint group = 0; readable && group < masterGroups.size(); ++group)
			{
				if (!masterGroups[group].Group->getActive())
					continue;
				for (uint priority = 0; readable && priority < WIN_PRIORITY_MAX; ++priority)
				{
					const list<CInterfaceGroup *> &windows = masterGroups[group].PrioritizedWindows[priority];
					for (list<CInterfaceGroup *>::const_iterator it = windows.begin(); readable && it != windows.end(); ++it)
					{
						CInterfaceGroup *other = *it;
						if (other == window || !other->getActive() || !other->getUseCursor() ||
							!other->isIn(left, bottom, right - left, top - bottom))
							continue;
						const sint32 overlapLeft = max(left, other->getXReal());
						const sint32 overlapRight = min(right, other->getXReal() + other->getWReal());
						const sint32 overlapBottom = max(bottom, other->getYReal());
						const sint32 overlapTop = min(top, other->getYReal() + other->getHReal());
						if (overlapRight > overlapLeft && overlapTop > overlapBottom)
							readable = widgets->getWindowUnder((overlapLeft + overlapRight - 1) / 2,
								overlapBottom + (overlapTop - overlapBottom + 1) / 2) == window;
					}
				}
			}
		}
		if (readable && hasCharacters)
		{
			unread->second = false;
			updateMentionTabs();
			return;
		}
	}
}

//=================================================================================
bool CChatTextManager::hasUnreadMention(const CCtrlTabButton *tab) const
{
	return hasUnreadChatMention(tab);
}

//=================================================================================
void CChatTextManager::setMentionTab(CViewBase *message, CCtrlTabButton *tab)
{
	CChatMessageParagraph *paragraph = dynamic_cast<CChatMessageParagraph *>(message);
	if (!paragraph || paragraph->MentionViews.empty())
		return;
	paragraph->MentionTab = tab;
	std::map<std::string, bool>::const_iterator unread = ChatMentionUnread.find(paragraph->Message.MessageId);
	if (unread == ChatMentionUnread.end() || !unread->second)
		return;
	if (tab && hasUnreadMention(tab))
	{
		bool present = false;
		for (std::list<CChatMentionTab>::const_iterator it = ChatMentionTabs.begin(); it != ChatMentionTabs.end(); ++it)
			if (it->Tab == tab)
				present = true;
		if (!present)
		{
			CChatMentionTab state;
			state.Tab = tab;
			state.PushedColor = tab->getTextColorPushed();
			state.OverColor = tab->getTextColorOver();
			ChatMentionTabs.push_back(state);
			CChatMentionTab &stored = ChatMentionTabs.back();
			stored.TextId = dynamic_cast<CViewTextID *>(tab->getViewText());
			if (stored.TextId != NULL)
			{
				stored.TextModifier = stored.TextId->getOnReceiveTextId();
				stored.TextId->setOnReceiveTextId(&stored);
			}
		}
		updateMentionTabs();
	}
}

//=================================================================================
CChatTextManager::CChatTextManager() :
	_TextFontSize(NULL),
	_TextMultilineSpace(NULL),
	_TextShadowed(NULL),
	_ShowTimestamps(NULL),
	_EmojiMode(NULL),
	_EmojiSize(NULL)
{
}

//=================================================================================
CChatTextManager::~CChatTextManager()
{
	ChatMessageParagraphs.clear();
	ChatReactions.clear();
	ChatMentionUnread.clear();
	updateMentionTabs();
	ChatMentionViews.clear();
	delete _TextFontSize;
	_TextFontSize = NULL;
	delete _TextMultilineSpace;
	_TextMultilineSpace = NULL;
	delete _TextShadowed;
	_TextShadowed = NULL;
	delete _ShowTimestamps;
	_ShowTimestamps = NULL;
	delete _EmojiMode;
	_EmojiMode = NULL;
	delete _EmojiSize;
	_EmojiSize = NULL;
}
//=================================================================================
uint CChatTextManager::getTextFontSize() const
{
	if (!_TextFontSize)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		_TextFontSize = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:FONT_SIZE", false);
		if (!_TextFontSize) return 12;
	}
	return (uint) _TextFontSize->getValue32();
}

//=================================================================================
uint CChatTextManager::getTextMultiLineSpace() const
{
	if (!_TextMultilineSpace)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		_TextMultilineSpace = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:MULTI_LINE_SPACE", false);
		if (!_TextMultilineSpace) return 1;
	}
	return (uint) _TextMultilineSpace->getValue32();
}

//=================================================================================
bool CChatTextManager::isTextShadowed() const
{
	if (!_TextShadowed)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		_TextShadowed = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHADOWED_TEXT", false);
		if (!_TextShadowed) return false;
	}
	return _TextShadowed->getValueBool();
}

//=================================================================================
uint CChatTextManager::getEmojiMode() const
{
	if (!_EmojiMode)
	{
		_EmojiMode = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:EMOJI_MODE", false);
		if (!_EmojiMode) return EmojiImage;
	}
	sint32 v = _EmojiMode->getValue32();
	if (v < EmojiText || v > EmojiImage) return EmojiImage;
	return (uint)v;
}

//=================================================================================
uint CChatTextManager::getEmojiSize() const
{
	if (!_EmojiSize)
	{
		_EmojiSize = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:EMOJI_SIZE", false);
		if (!_EmojiSize) return EmojiSmall;
	}
	sint32 v = _EmojiSize->getValue32();
	if (v < EmojiSmall || v > EmojiLarge) return EmojiSmall;
	return (uint)v;
}

//=================================================================================
sint32 CChatTextManager::getEmojiPixelSize() const
{
	// Small is the font size, large the tile size, medium in between.
	// Not named 'small': the Windows SDK defines it as a macro.
	const sint32 smallPx = (sint32)getTextFontSize();
	const sint32 largePx = EmojiTilePixels;
	switch (getEmojiSize())
	{
		case EmojiLarge:	return largePx;
		case EmojiMedium:	return smallPx < largePx ? (smallPx + largePx) / 2 : largePx;
		case EmojiSmall:
		default:			return smallPx;
	}
}

//=================================================================================
bool CChatTextManager::showTimestamps() const
{
	if (!_ShowTimestamps)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		_ShowTimestamps = CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHOW_TIMES_IN_CHAT_CB", false);
		if (!_ShowTimestamps) return false;
	}
	return _ShowTimestamps->getValueBool();
}

//=================================================================================
static CInterfaceGroup *parseCommandTag(string &line)
{
	string::size_type start = line.find("/$$");
	if (start == string::npos) return NULL;
	string::size_type end = line.find("$$/", start + 3);
	if (end == string::npos) return NULL;
	std::string commandLine = line.substr(start + 3, end - start - 3);
	line = line.substr(0, start) + line.substr(end +3);
	vector<string> params;
	explode(commandLine, std::string("|"), params);
	if (params.size() != 4)
	{
		nlwarning("4 parameters wanted for command tag : template|caption|ah|ah_params");
		return NULL;
	}
	//
	static int commandId = 0;
	pair<string, string> uiTemplateParams[4] =
	{
		make_pair(string("id"), NLMISC::toString("command%d", commandId ++)),
		make_pair(string("caption"), params[1]),
		make_pair(string("ah"), params[2]),
		make_pair(string("ah_params"), params[3])
	};
	return CWidgetManager::getInstance()->getParser()->createGroupInstance(params[0], "", uiTemplateParams, 4);
}

static CInterfaceGroup *buildLineWithCommand(CInterfaceGroup *commandGroup, CViewText *text)
{
	nlassert(commandGroup);
	nlassert(text);
	CInterfaceGroup *group = new CInterfaceGroup(CViewBase::TCtorParam());
	static int groupId = 0;
	group->setId(NLMISC::toString("%d", groupId++));
	static volatile bool sizeref = 1;
	static volatile sint32 w = 0;
	group->setSizeRef(sizeref);
	group->setW(w);
	group->setResizeFromChildH(true);
	//
	group->addGroup(commandGroup);
	commandGroup->setParent(group);
	text->setParent(group);
	text->setParentPos(commandGroup);
	text->setPosRef(Hotspot_TL);
	text->setParentPosRef(Hotspot_TR);
	group->addView(text);
	return group;
}

static inline bool	isUrlTag(const string &s, string::size_type index, string::size_type textSize)
{
	// Format http://, https://
	// or markdown style (title)[http://..]
	if(textSize > index+7)
	{
		bool markdown = false;
		string::size_type i = index;
		// advance index to url section if markdown style link is detected
		if (s[i] == '(')
		{
			// scan for ')[http://'
			while(i < textSize-9)
			{
				if (s[i] == ')' && s[i+1] == '[')
				{
					i += 2;
					markdown = true;
					break;
				}
				else
				if (s[i] == ')')
				{
					i += 1;
					break;
				}
				i++;
			}
		}

		if (textSize > i + 7)
		{
			bool isUrl = (toLowerAscii(s.substr(i, 7)) == "http://" || toLowerAscii(s.substr(i, 8)) == "https://");
			// match "text http://" and not "texthttp://"
			if (isUrl && i > 0 && !markdown)
			{
				// '}' is in the list because of color tags, ie "@{FFFF}http://..."
#ifdef NL_ISO_CPP0X_AVAILABLE
				const vector<ucchar> chars{ ' ', '"', '\'', '(', '[', '}' };
#else
				static std::vector<ucchar> chars;

				if (chars.empty())
				{
					chars.push_back(' ');
					chars.push_back('"');
					chars.push_back('\'');
					chars.push_back('(');
					chars.push_back('[');
					chars.push_back('}');
				}
#endif
				isUrl = std::find(chars.begin(), chars.end(), s[i - 1]) != chars.end();
			}
			return isUrl;
		}
	}

	return false;
}

// ***************************************************************************
// isUrlTag must match
static inline void getUrlTag(const string &s, string::size_type &index, string &url, string &title)
{
	bool isMarkdown = false;
	string::size_type textSize = s.size();
	string::size_type pos;

	// see if we have markdown format
	if (s[index] == '(')
	{
		index++;
		pos = index;
		while(pos < textSize-9)
		{
			if (s[pos] == ')' && s[pos + 1] == '[')
			{
				isMarkdown = true;
				title = s.substr(index, pos - index);
				index = pos + 2;
				break;
			}
			else if (s[pos] == ')')
			{
				break;
			}

			pos++;
		}
	}

	char chOpen = ' ';
	char chClose = ' ';
	if (isMarkdown)
	{
		chOpen = '[';
		chClose = ']';
	}
	else if (index > 0)
	{
		chOpen = s[index - 1];
		if (chOpen == '\'') chClose = '\'';
		else if (chOpen == '"') chClose = '"';
		else if (chOpen == '(') chClose = ')';
		else if (chOpen == '[') chClose = ']';
		else chClose = ' ';
	}

	if (chOpen == chClose)
	{
		pos = s.find_first_of(chClose, index);

		// handle common special case: 'text http://.../, text'
		if (pos != string::npos && index > 0)
		{
			if (s[index-1] == ' ' && (s[pos-1] == ',' || s[pos-1] == '.'))
			{
				pos--;
			}
		}
	}
	else
	{
		// scan for nested open/close tags
		pos = index;
		sint nested = 0;
		while(pos < textSize)
		{
			if (s[pos] == chOpen)
			{
				nested++;
			}
			else if (s[pos] == chClose)
			{
				if (nested == 0)
				{
					break;
				}
				else
				{
					nested--;
				}
			}

			pos++;
		}
	}

	// fallback to full string length as we did match http:// already and url spans to the end probably
	if (pos == string::npos)
	{
		pos = textSize;
	}

	url = s.substr(index, pos - index);
	index = pos;

	// skip ']' closing char
	if (isMarkdown) index++;
}

//=================================================================================
static void prependTimestamp(string &msg)
{
	string cur_time;
	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:SHOW_CLOCK_12H", false);
	if (node && node->getValueBool())
		cur_time = CInterfaceManager::getTimestampHuman("[%I:%M:%S %p] ");
	else
		cur_time = CInterfaceManager::getTimestampHuman();

	string::size_type codePos = msg.find("@{");
	if (codePos != string::npos)
	{
		// Prepend the current time (do it after the color if the color at first position.
		if (codePos == 0)
		{
			codePos = msg.find(string("}"));
			msg = msg.substr(0, codePos + 1) + cur_time + msg.substr(codePos + 1, msg.length() - codePos);
		}
		else
		{
			msg = cur_time + msg;
		}
	}
	else
	{
		msg = cur_time + msg;
	}
}

//=================================================================================
CViewBase *CChatTextManager::createMsgText(const string &cstMsg, NLMISC::CRGBA col, bool justified /*=false*/, bool plaintext /*=false*/)
{
	string msg = cstMsg;
	CInterfaceGroup *commandGroup = parseCommandTag(msg);

	if (showTimestamps())
		prependTimestamp(msg);

	// must wrap all lines to CGroupParagraph because CGroupList will calculate
	// width from previous line which ends up as CViewText otherwise
	return createMsgTextComplex(msg, col, justified, plaintext, commandGroup);
}

//=================================================================================
string CChatTextManager::getMessageText(const CChatMessage &message, CChatGroup::TGroupType group) const
{
	string original;
	for (std::vector<CChatMessagePart>::const_iterator it = message.Parts.begin(); it != message.Parts.end(); ++it)
		original += it->Type == CChatMessagePart::Text ? it->TextValue.toUtf8() :
			CHAT_SHARE::getPartName(*it);
	bool disableTranslation = false;
	if (group < CChatGroup::nbChatMode)
	{
		CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(
			"UI:SAVE:TRANSLATION:" + toUpper(CChatGroup::groupTypeToString(group)) + ":DISABLE", false);
		disableTranslation = node && node->getValueBool();
	}
	if (message.TranslatedParts.empty())
	{
		// Legacy translations can arrive inside the original text field.
		if (disableTranslation)
		{
			string::size_type start = original.find("{:");
			string::size_type end = original.find("}@{", start);
			if (start != string::npos && end != string::npos && end >= start + 5)
				return original.substr(0, start) + original.substr(start + 5, end - start - 5);
		}
		return original;
	}

	string translated;
	for (std::vector<CChatMessagePart>::const_iterator it = message.TranslatedParts.begin(); it != message.TranslatedParts.end(); ++it)
		translated += it->Type == CChatMessagePart::Text ? it->TextValue.toUtf8() :
			CHAT_SHARE::getPartName(*it);
	if (message.TranslationLanguage.empty())
	{
		if (disableTranslation)
		{
			string::size_type start = translated.find("{:");
			string::size_type end = translated.find("}@{", start);
			if (start != string::npos && end != string::npos && end >= start + 5)
				return translated.substr(0, start) + translated.substr(start + 5, end - start - 5);
		}
		return translated;
	}
	if (disableTranslation)
		return original;
	if (message.SourceLanguage.size() != 2)
		return translated;
	return "{:" + message.SourceLanguage + ":" + original + "}@{ " + translated;
}

//=================================================================================
static std::vector<std::pair<size_t, size_t> > getDisplayedMentions(const CChatMessage &message,
	const string &displayed, size_t offset)
{
	std::vector<std::pair<size_t, size_t> > ranges;
	if (message.Mentions.empty() || message.Parts.empty())
		return ranges;
	const uint32 localCharacter = getLocalCharacterId();
	const ucstring text = ucstring::makeFromUtf8(displayed);
	for (uint i = 0; i < message.Mentions.size(); ++i)
	{
		const CChatMessageMention &mention = message.Mentions[i];
		const bool mentioned = mention.Scope == CChatMessageMention::Player ?
			mention.PlayerId.getShortId() == localCharacter : message.SenderId.getShortId() != localCharacter;
		if (!mentioned || mention.Part >= message.Parts.size())
			continue;
		const ucstring token = message.Parts[mention.Part].TextValue.substr(mention.Start, mention.Length);
		if (token.empty())
			continue;
		ucstring::size_type found = text.find(token);
		while (found != ucstring::npos)
		{
			if (CHAT_MESSAGE::getMentionEnd(text, (uint32)found) == found + token.size())
				ranges.push_back(make_pair(offset + text.substr(0, found).toUtf8().size(), token.toUtf8().size()));
			found = text.find(token, found + token.size());
		}
	}
	sort(ranges.begin(), ranges.end());
	ranges.erase(unique(ranges.begin(), ranges.end()), ranges.end());
	return ranges;
}

//=================================================================================
static void registerMentionParagraph(CChatMessageParagraph *paragraph, const CChatMessage &message)
{
	if (!paragraph->MentionViews.empty())
	{
		ChatMentionViews.push_back(paragraph);
		ChatMentionUnread.insert(make_pair(message.MessageId, true));
	}
}

//=================================================================================
CViewBase *CChatTextManager::createMsgText(const string &prefix, const CChatMessage &message,
	NLMISC::CRGBA col, bool justified, CChatGroup::TGroupType group)
{
	string body;
	if (group == CChatGroup::tell)
		body = getMessageText(message, group);
	else
		getStringCategoryIfAny(getMessageText(message, group), body);
	if (group != CChatGroup::tell && group != CChatGroup::system)
		strFindReplace(body, "{break}", "");
	string quoteLine, quoteHeader;
	bool quotedReference = false;
	if (!message.Quote.MessageId.empty() && !message.Quote.Parts.empty())
	{
		string author = CEntityCL::removeTitleAndShardFromName(message.Quote.SenderName.toUtf8());
		if (author.empty())
			author = message.Quote.SenderName.toUtf8();
		quoteHeader = author + ": \xC2\xBB";
		quoteLine = quoteHeader;
		for (std::vector<CChatMessagePart>::const_iterator it = message.Quote.Parts.begin();
			it != message.Quote.Parts.end(); ++it)
		{
			quoteLine += it->Type == CChatMessagePart::Text ? it->TextValue.toUtf8() :
				CHAT_SHARE::getPartName(*it);
			quotedReference |= it->Type != CChatMessagePart::Text;
		}
		quoteLine += "\xC2\xAB\n\xE2\x86\xB3 ";
	}
	bool hasReference = false;
	for (std::vector<CChatMessagePart>::const_iterator it = message.Parts.begin(); it != message.Parts.end(); ++it)
		hasReference |= it->Type != CChatMessagePart::Text;
	if (hasReference || quotedReference)
	{
		bool disableTranslation = false;
		if (group < CChatGroup::nbChatMode)
		{
			CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(
				"UI:SAVE:TRANSLATION:" + toUpper(CChatGroup::groupTypeToString(group)) + ":DISABLE", false);
			disableTranslation = node && node->getValueBool();
		}
		const bool hasTranslation = !message.TranslatedParts.empty() && !message.TranslationLanguage.empty() &&
			!disableTranslation && message.SourceLanguage.size() == 2;
		bool inverse = false, hideFlag = false;
		if (hasTranslation)
		{
			const string language = toUpperAscii(message.SourceLanguage);
			CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(
				"UI:SAVE:TRANSLATION:" + language + ":INVERSE_DISPLAY", false);
			inverse = node && node->getValueBool();
			node = NLGUI::CDBManager::getInstance()->getDbProp(
				"UI:SAVE:TRANSLATION:" + language + ":HIDE_FLAG", false);
			hideFlag = node && node->getValueBool();
		}
		const std::vector<CChatMessagePart> &parts = hasTranslation && inverse ? message.Parts :
			!message.TranslatedParts.empty() && (message.TranslationLanguage.empty() || !disableTranslation) ?
			message.TranslatedParts : message.Parts;
		std::vector<string> texts(parts.size());
		string visibleParts;
		bool firstText = true;
		for (uint i = 0; i < parts.size(); ++i)
		{
			if (parts[i].Type == CChatMessagePart::Text)
			{
				string value = parts[i].TextValue.toUtf8();
				if (firstText && group != CChatGroup::tell)
				{
					string withoutCategory;
					getStringCategoryIfAny(value, withoutCategory);
					value.swap(withoutCategory);
				}
				if (group != CChatGroup::tell && group != CChatGroup::system)
					strFindReplace(value, "{break}", "");
				texts[i] = value;
				visibleParts += value;
				firstText = false;
			}
			else
			{
				visibleParts += CHAT_SHARE::getPartName(parts[i]);
				firstText = false;
			}
		}
		if (hasTranslation || (body.size() >= visibleParts.size() &&
			body.compare(body.size() - visibleParts.size(), visibleParts.size(), visibleParts) == 0))
		{
			string copyText = hasTranslation ? prefix : prefix + body.substr(0, body.size() - visibleParts.size());
			string lead = (quotedReference ? quoteHeader : quoteLine) +
				(quotedReference ? string() : copyText);
			if (!quoteLine.empty())
			{
				string colorTag;
				CChatWindow::encodeColorTag(col, colorTag);
				lead.insert(0, colorTag);
			}
			if (showTimestamps())
				prependTimestamp(lead);
			CChatMessageParagraph *paragraph = new CChatMessageParagraph(CViewBase::TCtorParam());
			paragraph->setId("line");
			paragraph->setSizeRef("w");
			paragraph->setResizeFromChildH(true);
			paragraph->HasMessage = true;
			paragraph->Message = message;
			if (!lead.empty())
				addMsgText(paragraph, lead, col, justified);
			if (quotedReference)
			{
				for (std::vector<CChatMessagePart>::const_iterator it = message.Quote.Parts.begin();
					it != message.Quote.Parts.end(); ++it)
				{
					if (it->Type == CChatMessagePart::Text)
						addMsgText(paragraph, it->TextValue.toUtf8(), col, justified);
					else
						paragraph->addChildLink(CHAT_SHARE::createAttachmentView(*it, justified));
				}
				string quoteEnd = "\xC2\xAB\n\xE2\x86\xB3 " + copyText;
				string colorTag;
				CChatWindow::encodeColorTag(col, colorTag);
				quoteEnd.insert(0, colorTag);
				addMsgText(paragraph, quoteEnd, col, justified);
			}
			if (hasTranslation && !hideFlag)
			{
				const std::vector<CChatMessagePart> &tooltipParts = inverse ? message.TranslatedParts : message.Parts;
				string tooltip;
				for (std::vector<CChatMessagePart>::const_iterator it = tooltipParts.begin(); it != tooltipParts.end(); ++it)
					tooltip += it->Type == CChatMessagePart::Text ? it->TextValue.toUtf8() :
						CHAT_SHARE::getPartName(*it);
				CCtrlButton *flag = new CCtrlButton(CViewBase::TCtorParam());
				const string texture = "flag-" + toLowerAscii(message.SourceLanguage) + ".tga";
				flag->setTexture(texture);
				flag->setTextureOver(texture);
				flag->setTexturePushed(texture);
				flag->setDefaultContextHelp(tooltip);
				flag->setId("tr");
				paragraph->addChild(flag);
			}
			for (uint i = 0; i < parts.size(); ++i)
			{
				if (parts[i].Type == CChatMessagePart::Text)
				{
					const std::vector<std::pair<size_t, size_t> > mentions = getDisplayedMentions(message, texts[i], 0);
					addMsgText(paragraph, texts[i], col, justified, &mentions);
				}
				else
					paragraph->addChildLink(CHAT_SHARE::createAttachmentView(parts[i], justified));
			}
			paragraph->setRightClickHandler("copy_chat_popup");
			paragraph->setRightClickHandlerParams((quoteLine.empty() ? lead : copyText) + visibleParts);
			registerMentionParagraph(paragraph, message);
			registerMessageParagraph(paragraph);
			updateReactionViews(paragraph);
			return paragraph;
		}
	}
	string text = quoteLine + prefix + body;
	if (!quoteLine.empty())
	{
		string colorTag;
		CChatWindow::encodeColorTag(col, colorTag);
		text.insert(0, colorTag);
	}
	std::vector<std::pair<size_t, size_t> > mentions =
		getDisplayedMentions(message, body, text.size() - body.size());
	CInterfaceGroup *commandGroup = parseCommandTag(text);
	if (commandGroup)
		mentions.clear();
	if (showTimestamps())
	{
		const size_t before = text.size();
		prependTimestamp(text);
		for (uint i = 0; i < mentions.size(); ++i)
			mentions[i].first += text.size() - before;
	}
	CViewBase *view = createMsgTextComplex(text, col, justified, false, commandGroup, &mentions);
	CChatMessageParagraph *paragraph = dynamic_cast<CChatMessageParagraph *>(view);
	if (paragraph)
	{
		paragraph->HasMessage = true;
		paragraph->Message = message;
		paragraph->setRightClickHandlerParams(prefix + body);
		registerMentionParagraph(paragraph, message);
		registerMessageParagraph(paragraph);
		updateReactionViews(paragraph);
	}
	return view;
}

//=================================================================================
void CChatTextManager::setMessageTarget(CViewBase *view, CChatGroup::TGroupType group,
	const CEntityId &dynamicChannelId, const string &receiver)
{
	CChatMessageParagraph *paragraph = dynamic_cast<CChatMessageParagraph *>(view);
	if (!paragraph)
		return;
	paragraph->Group = group;
	paragraph->DynamicChannelId = dynamicChannelId;
	paragraph->Receiver = receiver;
}

//=================================================================================
void CChatTextManager::setMessageSender(CViewBase *view, const string &sender)
{
	CChatMessageParagraph *paragraph = dynamic_cast<CChatMessageParagraph *>(view);
	if (paragraph)
		paragraph->Sender = sender;
}

//=================================================================================
const CChatMessage *CChatTextManager::getSelectedMessage() const
{
	return LastSelectedHasMessage ? &LastSelectedMessage : NULL;
}

//=================================================================================
bool CChatTextManager::getSelectedMessageTarget(CChatGroup::TGroupType &group,
	CEntityId &dynamicChannelId, string &receiver) const
{
	group = LastSelectedGroup;
	dynamicChannelId = LastSelectedDynamicChannelId;
	receiver = LastSelectedReceiver;
	return CHAT_MESSAGE::isValidTarget(group, dynamicChannelId, receiver);
}

//=================================================================================
string CChatTextManager::getQuoteMessageId(const CGroupEditBox *editBox) const
{
	for (list<CChatQuoteDraft>::const_iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end(); ++it)
		if (it->EditBox == editBox)
			return it->Message.MessageId;
	return string();
}

//=================================================================================
bool CChatTextManager::getQuoteTarget(const CGroupEditBox *editBox,
	CChatGroup::TGroupType &group, CEntityId &dynamicChannelId, string &receiver) const
{
	for (list<CChatQuoteDraft>::const_iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end(); ++it)
		if (it->EditBox == editBox)
		{
			group = it->Group;
			dynamicChannelId = it->DynamicChannelId;
			receiver = it->Receiver;
			return CHAT_MESSAGE::isValidTarget(group, dynamicChannelId, receiver);
		}
	return false;
}

//=================================================================================
uint32 CChatTextManager::beginQuoteSend(CGroupEditBox *editBox)
{
	if (!editBox)
		return 0;
	list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin();
	for (; it != ChatQuoteDrafts.end(); ++it)
		if (it->EditBox == editBox)
			break;
	if (it == ChatQuoteDrafts.end())
	{
		// Positions without a quote use the same acknowledgement and draft checks.
		CChatQuoteDraft draft;
		draft.EditBox = editBox;
		ChatQuoteDrafts.push_back(draft);
		it = ChatQuoteDrafts.end();
		--it;
	}
	if (it->RequestId != 0)
		return 0;
	if (++NextQuoteRequestId == 0)
		++NextQuoteRequestId;
	it->RequestId = NextQuoteRequestId;
	it->SentAt = CTime::getLocalTime();
	it->SentText = editBox->getInputString();
	it->SentTags = editBox->getTextTags();
	it->SentRevision = editBox->getInputRevision();
	it->InputCleared = false;
	return it->RequestId;
}

//=================================================================================
void CChatTextManager::applyReaction(const CChatMessage &message)
{
	const CChatMessageReaction &reaction = message.Parts[0].ReactionValue;
	typedef std::multimap<std::string, CChatMessageParagraph *>::iterator TParagraphIt;
	const std::pair<TParagraphIt, TParagraphIt> paragraphs = ChatMessageParagraphs.equal_range(reaction.MessageId);
	if (paragraphs.first == paragraphs.second)
		return;
	std::vector<CChatReaction> &reactions = ChatReactions[reaction.MessageId];
	uint index = 0;
	while (index < reactions.size() && reactions[index].Emoji != reaction.Emoji)
		++index;
	if (index == reactions.size())
	{
		if (reaction.Remove || reactions.size() >= ChatReactionEmojiLimit)
			return;
		reactions.push_back(CChatReaction());
		reactions.back().Emoji = reaction.Emoji;
	}
	CChatReaction &entry = reactions[index];
	// Zulip users have no character id, their name tells them apart.
	const string name = CEntityCL::removeTitleAndShardFromName(message.SenderName.toUtf8());
	uint reactor = 0;
	while (reactor < entry.Reactors.size() && (entry.Reactors[reactor] != message.SenderId ||
		(message.SenderId == CEntityId::Unknown && entry.Names[reactor] != name)))
		++reactor;
	if (reaction.Remove)
	{
		if (reactor == entry.Reactors.size())
			return;
		entry.Names.erase(entry.Names.begin() + reactor);
		entry.Reactors.erase(entry.Reactors.begin() + reactor);
		if (entry.Reactors.empty())
			reactions.erase(reactions.begin() + index);
	}
	else
	{
		if (reactor != entry.Reactors.size())
			return;
		entry.Reactors.push_back(message.SenderId);
		entry.Names.push_back(name);
	}
	for (TParagraphIt it = paragraphs.first; it != paragraphs.second; ++it)
		updateReactionViews(it->second);
}

//=================================================================================
void CChatTextManager::updateReactionViews(CChatMessageParagraph *paragraph)
{
	const bool hadRow = paragraph->clearReactionViews();
	std::map<std::string, std::vector<CChatReaction> >::const_iterator reactions =
		ChatReactions.find(paragraph->Message.MessageId);
	if (reactions == ChatReactions.end() || reactions->second.empty())
	{
		if (hadRow)
			paragraph->invalidateCoords();
		return;
	}
	CViewText *first = paragraph->getNumChildren() ? dynamic_cast<CViewText *>(paragraph->getChild(0)) : NULL;
	const CRGBA color = first ? first->getColor() : CRGBA::White;
	const CRGBA ownColor = CRGBA::stringToRGBA(
		CWidgetManager::getInstance()->getParser()->getDefine("chat_message_color_mention").c_str());
	const uint32 localCharacter = getLocalCharacterId();
	// The row starts on its own line below the message.
	CViewBase *lineBreak = createMsgTextSimple("\n", color, false, NULL);
	paragraph->addChild(lineBreak);
	paragraph->ReactionViews.push_back(lineBreak);
	for (std::vector<CChatReaction>::const_iterator it = reactions->second.begin(); it != reactions->second.end(); ++it)
	{
		bool own = false;
		string names;
		for (uint i = 0; i < it->Reactors.size(); ++i)
		{
			own |= it->Reactors[i].getShortId() == localCharacter;
			names += (i ? ", " : "") + it->Names[i];
		}
		const string code = ":" + it->Emoji + ":";
		string::size_type length = 0;
		const CEmojiManager::CEntry *entry = NULL;
		string label = code;
		if (CEmojiManager::getInstance().matchAt(code, 0, code.size(), length, entry) && length == code.size() &&
			!entry->Texture.empty())
		{
			// Clicking the emoji adds or takes back the own reaction.
			CCtrlButton *button = new CCtrlButton(CViewBase::TCtorParam());
			button->setId("reaction");
			button->setTexture(entry->Texture);
			button->setTextureOver(entry->Texture);
			button->setTexturePushed(entry->Texture);
			button->setScale(true);
			button->setW(getEmojiPixelSize());
			button->setH(getEmojiPixelSize());
			button->setModulateGlobalColorAll(false);
			button->setDefaultContextHelp(names);
			button->setActionOnLeftClick("chat_reaction");
			button->setParamsOnLeftClick(it->Emoji);
			paragraph->addChild(button);
			paragraph->ReactionViews.push_back(button);
			label.clear();
		}
		CViewBase *count = createMsgTextSimple(label + " " + toString(it->Reactors.size()) + "   ",
			own ? ownColor : color, false, NULL);
		paragraph->addChild(count);
		paragraph->ReactionViews.push_back(count);
	}
	paragraph->invalidateCoords();
}

//=================================================================================
void CChatTextManager::react(const string &emoji)
{
	if (ReactionMessageId.empty())
		return;
	// Picking an emoji the player already used takes the reaction back.
	bool remove = false;
	std::map<std::string, std::vector<CChatReaction> >::const_iterator reactions = ChatReactions.find(ReactionMessageId);
	if (reactions != ChatReactions.end())
	{
		const uint32 localCharacter = getLocalCharacterId();
		for (std::vector<CChatReaction>::const_iterator it = reactions->second.begin(); it != reactions->second.end(); ++it)
			if (it->Emoji == emoji)
				for (uint i = 0; !remove && i < it->Reactors.size(); ++i)
					remove = it->Reactors[i].getShortId() == localCharacter;
	}
	CChatMessageRequest request;
	request.Text = ucstring::makeFromUtf8(":" + emoji + ":");
	CChatMessageReference reference;
	reference.Type = CChatMessageReference::Reaction;
	reference.Length = (uint16)request.Text.size();
	reference.ReactionValue.MessageId = ReactionMessageId;
	reference.ReactionValue.Emoji = emoji;
	reference.ReactionValue.Remove = remove;
	request.References.push_back(reference);
	const bool queued = request.isValid() && (ReactionGroup == CChatGroup::tell ?
		ChatMngr.tell(ReactionReceiver, request) : ChatMngr.chat(request, ReactionGroup, ReactionDynamicChannelId));
	if (!queued)
		CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
}

//=================================================================================
void CChatTextManager::clearSentInput(uint32 requestId)
{
	for (list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end(); ++it)
		if (requestId != 0 && it->RequestId == requestId)
		{
			if (it->EditBox && !it->InputCleared)
			{
				it->EditBox->setInputString(string());
				if (!it->Message.MessageId.empty())
					setQuotePreviewActive(it->EditBox, false, it->HistoryOffset, it->MinHeightOffset);
				it->InputCleared = true;
			}
			return;
		}
}

//=================================================================================
// Give a refused request back to its input, unless the player already typed a new one.
static void restoreSentInput(CChatQuoteDraft &draft)
{
	CGroupEditBox *editBox = draft.EditBox;
	if (!draft.InputCleared || !editBox)
		return;
	draft.InputCleared = false;
	if (!editBox->getInputStringRef().empty())
	{
		draft.Message = CChatMessage();
		return;
	}
	editBox->setInputString(draft.SentText);
	for (uint i = 0; i < draft.SentTags.size(); ++i)
	{
		const CGroupEditBox::CTextTag &tag = draft.SentTags[i];
		editBox->addTextTag(tag.Start, tag.Length, tag.Type, tag.Color, tag.Reference);
	}
	editBox->setCursorPos((sint32)editBox->getInputStringRef().size());
	if (!draft.Message.MessageId.empty() &&
		!setQuotePreviewActive(editBox, true, draft.HistoryOffset, draft.MinHeightOffset))
		draft.Message = CChatMessage();
}

//=================================================================================
void CChatTextManager::finishQuoteSend(uint32 requestId, bool accepted)
{
	for (list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end(); ++it)
		if (requestId != 0 && it->RequestId == requestId)
		{
			if (!it->EditBox)
			{
				ChatQuoteDrafts.erase(it);
				return;
			}
			it->RequestId = 0;
			if (accepted && it->InputCleared)
			{
				CGroupEditBox *editBox = it->EditBox;
				ChatQuoteDrafts.erase(it);
				if (editBox->getAHOnEnter() == "chat_link_tell")
					CAHManager::getInstance()->runActionHandler("chat_link_tell", editBox, "accepted");
				return;
			}
			if (accepted)
			{
				CGroupEditBox *editBox = it->EditBox;
				const vector<CGroupEditBox::CTextTag> &tags = editBox->getTextTags();
				bool unchanged = it->SentRevision == editBox->getInputRevision() &&
					it->SentText == editBox->getInputString() && it->SentTags.size() == tags.size();
				for (uint i = 0; unchanged && i < tags.size(); ++i)
				{
					if (it->SentTags[i].Start != tags[i].Start ||
						it->SentTags[i].Length != tags[i].Length ||
						it->SentTags[i].Type != tags[i].Type ||
						it->SentTags[i].Reference != tags[i].Reference ||
						it->SentTags[i].Color != tags[i].Color)
						unchanged = false;
				}
				if (unchanged)
				{
					editBox->setInputString(string());
					clearQuote(editBox);
					if (editBox->getAHOnEnter() == "chat_link_tell")
						CAHManager::getInstance()->runActionHandler("chat_link_tell", editBox, "accepted");
					return;
				}
			}
			else
			{
				restoreSentInput(*it);
				CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
			}
			if (it->Message.MessageId.empty())
				ChatQuoteDrafts.erase(it);
			return;
		}
}

//=================================================================================
void CChatTextManager::checkQuoteSendTimeout()
{
	const TTime now = CTime::getLocalTime();
	bool timedOut = false;
	for (list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end();)
	{
		if (!it->EditBox)
		{
			it = ChatQuoteDrafts.erase(it);
			continue;
		}
		if (it->RequestId != 0 && now - it->SentAt >= QuoteSendTimeout)
		{
			it->RequestId = 0;
			restoreSentInput(*it);
			timedOut = true;
			if (it->Message.MessageId.empty())
			{
				it = ChatQuoteDrafts.erase(it);
				continue;
			}
		}
		++it;
	}
	if (timedOut)
		CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
}

//=================================================================================
void CChatTextManager::failPendingQuoteSends()
{
	bool pending = false;
	for (list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end();)
	{
		if (it->RequestId != 0)
		{
			it->RequestId = 0;
			restoreSentInput(*it);
			pending = true;
			if (it->Message.MessageId.empty())
			{
				it = ChatQuoteDrafts.erase(it);
				continue;
			}
		}
		++it;
	}
	if (pending)
		CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
}

//=================================================================================
static bool setQuotePreviewActive(CGroupEditBox *editBox, bool active, sint32 &historyOffset, sint32 &minHeightOffset)
{
	CInterfaceGroup *widget = editBox ? editBox->getParent() : NULL;
	CInterfaceGroup *content = widget ? widget->getParent() : NULL;
	CInterfaceGroup *preview = content ? content->getGroup("quote_preview") : NULL;
	CInterfaceGroup *viewport = widget ? dynamic_cast<CInterfaceGroup*>(widget->getParentPos()) : NULL;
	if (!preview || !viewport || !content)
		return false;
	if (active)
	{
		content->updateCoords();
		const sint32 available = max(sint32(1), viewport->getHReal());
		const sint32 height = min(preview->getH(false), available);
		preview->setH(height);
		historyOffset = min(height, available - 1);
	}
	const sint32 height = preview->getH(false);
	const sint32 offset = active ? historyOffset : -historyOffset;
	// Shrink the history viewport; offset its bottom anchors to keep the input row fixed.
	viewport->setH(viewport->getH(false) - offset);
	widget->setY(widget->getY() - offset);
	const vector<CCtrlBase*> &controls = content->getControls();
	for (uint i = 0; i < controls.size(); ++i)
		if (controls[i]->getParentPos() == viewport && (controls[i]->getParentPosRef() & Hotspot_Bx))
			controls[i]->setY(controls[i]->getY() - offset);
	const vector<CViewBase*> &views = content->getViews();
	for (uint i = 0; i < views.size(); ++i)
		if (views[i]->getParentPos() == viewport && (views[i]->getParentPosRef() & Hotspot_Bx))
			views[i]->setY(views[i]->getY() - offset);
	CGroupContainer *container = dynamic_cast<CGroupContainer*>(editBox->getEnclosingContainer());
	if (container)
	{
		if (active)
			minHeightOffset = min(height, max(sint32(0), container->getH(false) - container->getPopupMinH()));
		container->setPopupMinH(container->getPopupMinH() + (active ? minHeightOffset : -minHeightOffset));
	}
	preview->setActive(active);
	if (!active)
		preview->setH(44);
	content->invalidateCoords();
	return true;
}

//=================================================================================
void CChatTextManager::quoteSelectedMessage(CGroupEditBox *editBox)
{
	const CChatMessage *message = getSelectedMessage();
	CChatGroup::TGroupType group;
	CEntityId dynamicChannelId;
	string receiver;
	if (!message || message->MessageId.empty() || !editBox ||
		!getSelectedMessageTarget(group, dynamicChannelId, receiver))
		return;
	CInterfaceGroup *widget = editBox->getParent();
	CInterfaceGroup *content = widget ? widget->getParent() : NULL;
	if (!content)
		return;
	CInterfaceGroup *preview = content->getGroup("quote_preview");
	if (!preview)
	{
		vector<pair<string, string> > params;
		params.push_back(make_pair(string("id"), string("quote_preview")));
		params.push_back(make_pair(string("edit_box"), editBox->getId()));
		preview = CWidgetManager::getInstance()->getParser()->createGroupInstance(
			"chat_quote_preview", content->getId(), params);
		if (!preview)
			return;
		preview->setActive(false);
		content->addGroup(preview);
		preview->setParent(content);
		preview->setParentPos(content);
		preview->setParentSize(content);
		preview->setY(widget->getYReal() + widget->getHReal() - content->getYReal());
	}
	CViewText *author = dynamic_cast<CViewText*>(preview->getView("author"));
	CViewText *text = dynamic_cast<CViewText*>(preview->getView("text"));
	if (!author || !text)
		return;
	string authorName = CEntityCL::removeTitleAndShardFromName(message->SenderName.toUtf8());
	if (authorName.empty())
		authorName = message->SenderName.toUtf8();
	author->setText(authorName);
	string previewText;
	for (std::vector<CChatMessagePart>::const_iterator it = message->Parts.begin();
		it != message->Parts.end(); ++it)
		previewText += it->Type == CChatMessagePart::Text ? it->TextValue.toUtf8() :
			CHAT_SHARE::getPartName(*it);
	text->setText(previewText);
	clearQuote(editBox);
	CChatQuoteDraft draft;
	draft.EditBox = editBox;
	draft.Message = *message;
	draft.Group = group;
	draft.DynamicChannelId = dynamicChannelId;
	draft.Receiver = receiver;
	if (!setQuotePreviewActive(editBox, true, draft.HistoryOffset, draft.MinHeightOffset))
		return;
	ChatQuoteDrafts.push_back(draft);
	editBox->setFocusOnText();
}

//=================================================================================
void CChatTextManager::clearQuote(CGroupEditBox *editBox)
{
	sint32 historyOffset = 0;
	sint32 minHeightOffset = 0;
	bool reserved = false;
	for (list<CChatQuoteDraft>::iterator it = ChatQuoteDrafts.begin(); it != ChatQuoteDrafts.end();)
	{
		if (it->EditBox == NULL || it->EditBox == editBox)
		{
			// A cleared input already hid its quote preview.
			if (it->EditBox == editBox && !it->Message.MessageId.empty() && !it->InputCleared)
			{
				historyOffset = it->HistoryOffset;
				minHeightOffset = it->MinHeightOffset;
				reserved = true;
			}
			it = ChatQuoteDrafts.erase(it);
		}
		else
			++it;
	}
	if (reserved)
		setQuotePreviewActive(editBox, false, historyOffset, minHeightOffset);
}

//=================================================================================
string CChatTextManager::getSelectedPlayerName() const
{
	if (!UserEntity || LastSelectedSender.empty() || LastSelectedSender[0] == '~')
		return string();
	const string playerName = CEntityCL::removeTitleAndShardFromName(LastSelectedSender);
	if (playerName.empty() || compareCaseInsensitive(playerName, UserEntity->getDisplayName()) == 0)
		return string();
	if (LastSelectedHasMessage && LastSelectedMessage.SenderId != CEntityId::Unknown)
	{
		if (LastSelectedMessage.SenderId.getType() != RYZOMID::player)
			return string();
	}
	else
	{
		CEntityCL *entity = EntitiesMngr.getEntityByName(playerName, false, true);
		if (!entity || entity->Type != CEntityCL::Player)
			return string();
	}
	return playerName;
}

//=================================================================================
bool CChatTextManager::isChatInput(CGroupEditBox *editBox) const
{
	if (!editBox)
		return false;
	const string &handler = editBox->getAHOnEnter();
	if (handler == "contact_entry")
		return true;
	CChatWindow *chatWindow = handler == "chat_box_entry" ?
		getChatWndMgr().getChatWindowFromCaller(editBox) : NULL;
	return chatWindow && chatWindow->getEditBox() == editBox;
}

//=================================================================================
CViewBase *CChatTextManager::createMsgTextSimple(const string &msg, NLMISC::CRGBA col, bool justified, CInterfaceGroup *commandGroup)
{
	CViewText *vt = new CViewText(CViewText::TCtorParam());
	// get parameters from config.xml
	vt->setShadow(isTextShadowed());
	vt->setShadowOutline(false);
	vt->setFontSize(getTextFontSize());
	vt->setMultiLine(true);
	vt->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
	vt->setMultiLineSpace(getTextMultiLineSpace());
	vt->setModulateGlobalColor(false);

	// if text contain any color code, set the text formated and white,
	// otherwise, set text normal and apply global color
	if (msg.find("@{") != string::npos)
	{
		vt->setTextFormatTaged(msg);
		vt->setColor(NLMISC::CRGBA::White);
	}
	else
	{
		vt->setText(msg);
		vt->setColor(col);
	}

	if (!commandGroup)
	{
		return vt;
	}
	else
	{
		return buildLineWithCommand(commandGroup, vt);
	}
}

//=================================================================================
void CChatTextManager::addTextSegment(CGroupParagraph *para, const string &msg,
	string::size_type from, string::size_type to, NLMISC::CRGBA col, bool justified,
	const char *id)
{
	if (from >= to)
		return;

	string seg = CViewText::getFormatTagPrefixAt(msg, (uint)from);
	seg.append(msg, from, to - from);
	if (getEmojiMode() == EmojiUnicode)
		seg = CEmojiManager::getInstance().substituteShortcodes(seg);

	CViewBase *vt = createMsgTextSimple(seg, col, justified, NULL);
	if (id)
		vt->setId(id);
	para->addChild(vt);
}

//=================================================================================
// Emoji image with its name as tooltip. A CCtrlToolTip does not capture the
// pointer, so clicks still reach the chat line and its links.
class CCtrlEmoji : public CCtrlToolTip
{
public:
	CCtrlEmoji(const TCtorParam &param) : CCtrlToolTip(param) {}

	bool setTexture(const std::string &texture)
	{
		_TextureId.setTexture(texture.c_str());
		return !_TextureId.empty();
	}

	virtual void draw()
	{
		// Only the alpha of the interface colour applies to the tile.
		CRGBA col = CRGBA::White;
		col.A = (uint8)(((sint32)col.A *
			((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A + 1)) >> 8);

		CViewRenderer &rVR = *CViewRenderer::getInstance();
		rVR.drawRotFlipBitmap(_RenderLayer, _XReal, _YReal, _WReal, _HReal,
			0, false, _TextureId, col);
	}

private:
	CViewRenderer::CTextureId	_TextureId;
};

//=================================================================================
CViewBase *CChatTextManager::createEmojiView(const string &texture, const string &name)
{
	if (texture.empty())
		return NULL;

	CCtrlEmoji *bm = new CCtrlEmoji(CViewBase::TCtorParam());
	bm->setId("emoji");
	if (!bm->setTexture(texture))
	{
		delete bm;
		return NULL;
	}
	sint32 size = getEmojiPixelSize();
	bm->setW(size);
	bm->setH(size);
	bm->setModulateGlobalColor(false);
	if (!name.empty())
		bm->setDefaultContextHelp(":" + name + ":");
	return bm;
}

//=================================================================================
void CChatTextManager::addMsgText(CGroupParagraph *paragraph, const string &msg,
	NLMISC::CRGBA col, bool justified, const std::vector<std::pair<size_t, size_t> > *mentions)
{
	CEmojiManager &emoji = CEmojiManager::getInstance();
	const bool useEmojiImages = getEmojiMode() == EmojiImage && emoji.mayContainEmoji(msg);
	const string lower = toLowerAscii(msg);
	const bool hasUrl = lower.find("http://") != string::npos || lower.find("https://") != string::npos;
	string::size_type pos = 0;
	size_t mentionIndex = 0;
	for (string::size_type i = 0; i < msg.size();)
	{
		// Step over format tags instead of scanning inside them. A tooltip tag
		// holds arbitrary text, so "@{Hsee :smile: here}" contains something
		// that looks exactly like a shortcode, and matching it would cut the
		// tag in half. The same goes for a URL written inside one.
		const uint tagLength = CViewText::getFormatTagLength(msg, (uint)i);
		if (tagLength)
		{
			i += tagLength;
			continue;
		}
		while (mentions && mentionIndex < mentions->size() && (*mentions)[mentionIndex].first < i)
			++mentionIndex;
		string::size_type emojiLength = 0;
		const CEmojiManager::CEntry *emojiEntry = NULL;
		if (mentions && mentionIndex < mentions->size() && (*mentions)[mentionIndex].first == i)
		{
			const size_t length = (*mentions)[mentionIndex].second;
			++mentionIndex;
			if (!length || length > msg.size() - i)
				continue;
			addTextSegment(paragraph, msg, pos, i, col, justified);
			const CRGBA color = CRGBA::stringToRGBA(
				CWidgetManager::getInstance()->getParser()->getDefine("chat_message_color_mention").c_str());
			CViewBase *highlight = createMsgTextSimple(msg.substr(i, length), color, justified, NULL);
			paragraph->addChild(highlight);
			static_cast<CChatMessageParagraph *>(paragraph)->MentionViews.push_back(highlight);
			i += length;
			pos = i;
		}
		else if (useEmojiImages && emoji.matchAt(msg, i, msg.size(), emojiLength, emojiEntry))
		{
			addTextSegment(paragraph, msg, pos, i, col, justified);
			CViewBase *emojiView = createEmojiView(emojiEntry->Texture, emojiEntry->Name);
			if (emojiView)
				paragraph->addChild(emojiView);
			else
			{
				// No tile for this one, so fall back a step rather than showing
				// nothing: the unicode form, still carrying the format state.
				string segment = CViewText::getFormatTagPrefixAt(msg, (uint)i);
				segment += emojiEntry->Utf8;
				paragraph->addChild(createMsgTextSimple(segment, col, justified, NULL));
			}
			i += emojiLength;
			pos = i;
		}
		else if (hasUrl && isUrlTag(msg, i, msg.size()))
		{
			addTextSegment(paragraph, msg, pos, i, col, justified);
			string url, title;
			getUrlTag(msg, i, url, title);
			if (!url.empty())
			{
				CViewLink *link = new CViewLink(CViewBase::TCtorParam());
				link->setId("link");
				link->setUnderlined(true);
				link->setShadow(isTextShadowed());
				link->setShadowOutline(false);
				link->setFontSize(getTextFontSize());
				link->setMultiLine(true);
				link->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
				link->setMultiLineSpace(getTextMultiLineSpace());
				link->setModulateGlobalColor(false);
				link->setColor(col);
				link->LinkTitle = title.empty() ? url : title;
				link->setText(link->LinkTitle);
				if (url.find_first_of('\'') != string::npos)
				{
					string clean;
					for (string::size_type j = 0; j < url.size(); ++j)
						clean += url[j] == '\'' ? "%27" : string(1, url[j]);
					url.swap(clean);
				}
				link->setActionOnLeftClick("lua");
				link->setParamsOnLeftClick("game:chatUrl('" + url + "')");
				paragraph->addChildLink(link);
				pos = i;
			}
		}
		else
			++i;
	}
	addTextSegment(paragraph, msg, pos, msg.size(), col, justified, "text");
}

//=================================================================================
CViewBase *CChatTextManager::createMsgTextComplex(const string &msg, NLMISC::CRGBA col, bool justified, bool plaintext,
	CInterfaceGroup *commandGroup, const std::vector<std::pair<size_t, size_t> > *mentions)
{
	string::size_type textSize = msg.size();

	CGroupParagraph *para = new CChatMessageParagraph(CViewBase::TCtorParam());
	para->setId("line");
	para->setSizeRef("w");
	para->setResizeFromChildH(true);

	// use right click because left click might be used to activate chat window
	para->setRightClickHandler("copy_chat_popup");
	para->setRightClickHandlerParams(msg);

	if (plaintext)
	{
		addTextSegment(para, msg, 0, msg.size(), col, justified, "text");
		return para;
	}

	string::size_type pos = 0;

	string::size_type startTr = msg.find("{:");
	string::size_type endOfOriginal = msg.find("}@{");

	// Original/Translated case, example: {:enHello the world!}@{ Bonjour le monde !
	if (startTr != string::npos && endOfOriginal != string::npos)
	{
		string lang = toUpperAscii(msg.substr(startTr+2, 2));

		bool inverse = false;
		bool hideFlag = false;
		CCDBNodeLeaf *nodeInverse = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:TRANSLATION:" + lang + ":INVERSE_DISPLAY", false);
		if (nodeInverse)
			inverse = nodeInverse->getValueBool();
		CCDBNodeLeaf *nodeHideFlag = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:TRANSLATION:" + lang + ":HIDE_FLAG", false);
		if (nodeHideFlag)
			hideFlag = nodeHideFlag->getValueBool();
		
		addTextSegment(para, msg, 0, startTr, col, justified);

		string texture = "flag-"+toLowerAscii(msg.substr(startTr+2, 2))+".tga";
		string original = msg.substr(startTr+5, endOfOriginal-startTr-5);
		string translation = msg.substr(endOfOriginal+3);
		CCtrlButton *ctrlButton = new CCtrlButton(CViewBase::TCtorParam());
		ctrlButton->setTexture(texture);
		ctrlButton->setTextureOver(texture);
		ctrlButton->setTexturePushed(texture);
		if (!inverse)
		{
		  ctrlButton->setDefaultContextHelp(original);
		  pos = endOfOriginal+4;
		}
		else
		{
		  ctrlButton->setDefaultContextHelp(translation);
		  pos = startTr+5;
		  textSize = endOfOriginal;
		}
		ctrlButton->setId("tr");
		if (hideFlag) {
		  delete ctrlButton;
		} else {
		  para->addChild(ctrlButton);
		}
	}

	if (pos < textSize)
	{
		std::vector<std::pair<size_t, size_t> > visibleMentions;
		if (mentions)
		{
			for (uint i = 0; i < mentions->size(); ++i)
			{
				const size_t start = (*mentions)[i].first;
				const size_t length = (*mentions)[i].second;
				if (start >= pos && start < textSize && length <= textSize - start)
					visibleMentions.push_back(make_pair(start - pos, length));
			}
		}
		addMsgText(para, msg.substr(pos, textSize - pos), col, justified, &visibleMentions);
	}

	return para;
}

//=================================================================================
CChatTextManager &CChatTextManager::getInstance()
{
	if( !_Instance )
		_Instance = new CChatTextManager();
	return *_Instance;
}

//=================================================================================
void CChatTextManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

//=================================================================================
void CChatTextManager::reset ()
{
	while (!ChatQuoteDrafts.empty())
		clearQuote(ChatQuoteDrafts.front().EditBox);
	LastSelectedChatInput = NULL;
	ChatMessageParagraphs.clear();
	ChatReactions.clear();
	ChatMentionUnread.clear();
	updateMentionTabs();
	ChatMentionViews.clear();
	_TextFontSize = NULL;
	_TextMultilineSpace = NULL;
	_TextShadowed = NULL;
	_ShowTimestamps = NULL;
	_EmojiMode = NULL;
	_EmojiSize = NULL;
}

// ***************************************************************************
// Called when we right click on a chat line
class	CHandlerCopyChatPopup: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &params )
	{
		if (pCaller == NULL) return;

		LastSelectedChat = params;
		LastSelectedHasMessage = false;
		LastSelectedGroup = CChatGroup::nbChatMode;
		LastSelectedDynamicChannelId = CEntityId::Unknown;
		LastSelectedReceiver.clear();
		LastSelectedSender.clear();
		LastSelectedChatInput = NULL;
		CChatMessageParagraph *paragraph = dynamic_cast<CChatMessageParagraph *>(pCaller);
		if (paragraph)
		{
			LastSelectedHasMessage = paragraph->HasMessage;
			if (paragraph->HasMessage)
				LastSelectedMessage = paragraph->Message;
			LastSelectedGroup = paragraph->Group;
			LastSelectedDynamicChannelId = paragraph->DynamicChannelId;
			LastSelectedReceiver = paragraph->Receiver;
			LastSelectedSender = paragraph->Sender;
		}
		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(pCaller);
		LastSelectedChatInput = chat ? chat->getEditBox() : NULL;
		if (LastSelectedChatInput == NULL)
		{
			CInterfaceGroup *container = pCaller->getParentContainer();
			LastSelectedChatInput = container ? dynamic_cast<CGroupEditBox*>(container->getGroup("eb")) : NULL;
		}

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->enableTempOver();
		LastSelectedChatSheetId = CSheetId::Unknown;
		if (pGP && !UserPrivileges.empty())
		{
			CCtrlBase *ctrl = CWidgetManager::getInstance()->getCapturePointerRight();
			if (ctrl && ctrl->getParent() == pGP)
				CHAT_SHARE::getAttachmentSheetId(ctrl, LastSelectedChatSheetId);
		}
		CGroupMenu *menu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId(
			"ui:interface:chat_copy_action_menu"));
		if (menu && menu->getRootMenu())
		{
			const sint sheetIdLine = menu->getRootMenu()->getLineFromId("copy_sheet_id");
			if (sheetIdLine >= 0)
				menu->getRootMenu()->setHiddenLine(sheetIdLine, LastSelectedChatSheetId == CSheetId::Unknown);
			CChatGroup::TGroupType group;
			CEntityId dynamicChannelId;
			string receiver;
			// Quotes and reactions go to the channel of the selected message.
			const bool hasTarget = LastSelectedHasMessage && !LastSelectedMessage.MessageId.empty() &&
				getChatTextMngr().getSelectedMessageTarget(group, dynamicChannelId, receiver);
			const sint line = menu->getRootMenu()->getLineFromId("quote");
			if (line >= 0)
			{
				const bool selectable = hasTarget && LastSelectedChatInput != NULL;
				menu->getRootMenu()->setSelectable(line, selectable);
				menu->getRootMenu()->setGrayedLine(line, !selectable);
			}
			const sint reactLine = menu->getRootMenu()->getLineFromId("react");
			if (reactLine >= 0)
			{
				menu->getRootMenu()->setSelectable(reactLine, hasTarget);
				menu->getRootMenu()->setGrayedLine(reactLine, !hasTarget);
			}
			const sint tellLine = menu->getRootMenu()->getLineFromId("tell");
			if (tellLine >= 0)
			{
				const bool selectable = !getChatTextMngr().getSelectedPlayerName().empty();
				menu->getRootMenu()->setSelectable(tellLine, selectable);
				menu->getRootMenu()->setGrayedLine(tellLine, !selectable);
			}
			const sint friendLine = menu->getRootMenu()->getLineFromId("add_friend");
			if (friendLine >= 0)
			{
				// Privileged accounts and players already in the friend list cannot be added.
				const string playerName = getChatTextMngr().getSelectedPlayerName();
				const CHARACTER_TITLE::ECharacterTitle title =
					CHARACTER_TITLE::toCharacterTitle(CEntityCL::getTitleFromName(LastSelectedSender));
				const bool selectable = !playerName.empty() &&
					(title < CHARACTER_TITLE::BeginGmTitle || title > CHARACTER_TITLE::EndGmTitle) &&
					PeopleInterraction.FriendList.getIndexFromName(playerName) == -1;
				menu->getRootMenu()->setSelectable(friendLine, selectable);
				menu->getRootMenu()->setGrayedLine(friendLine, !selectable);
			}
		}

		CWidgetManager::getInstance()->enableModalWindow (pCaller, "ui:interface:chat_copy_action_menu");
	}
};
REGISTER_ACTION_HANDLER( CHandlerCopyChatPopup, "copy_chat_popup");

// ***************************************************************************
class CHandlerQuoteChat : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *, const string &params)
	{
		CWidgetManager *widgets = CWidgetManager::getInstance();
		if (params != "from_modal")
		{
			CGroupEditBox *editBox = dynamic_cast<CGroupEditBox*>(widgets->getElementFromId(params));
			getChatTextMngr().clearQuote(editBox);
			if (editBox)
				editBox->setFocusOnText();
			return;
		}
		CRefPtr<CGroupEditBox> editBox = LastSelectedChatInput;
		widgets->disableModalWindow();
		getChatTextMngr().quoteSelectedMessage(editBox);
	}
};
REGISTER_ACTION_HANDLER(CHandlerQuoteChat, "quote_chat");

// ***************************************************************************
// Called when we right click on a chat line and choose 'copy' from context menu
class	CHandlerCopyChat: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &params )
	{
		if (pCaller == NULL) return;

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->disableTempOver();

		CAHManager::getInstance()->runActionHandler("copy_to_clipboard", NULL, LastSelectedChat);
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerCopyChat, "copy_chat");

// ***************************************************************************
// Called when we right click on an attachment and choose 'copy sheet id' from context menu
class CHandlerCopyChatSheetId: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* params */)
	{
		if (pCaller == NULL) return;

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->disableTempOver();

		if (!UserPrivileges.empty() && LastSelectedChatSheetId != CSheetId::Unknown)
			CAHManager::getInstance()->runActionHandler("copy_to_clipboard", NULL, LastSelectedChatSheetId.toString());
		LastSelectedChatSheetId = CSheetId::Unknown;
		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerCopyChatSheetId, "copy_chat_sheet_id");

// ***************************************************************************
// Called when we right click on a chat line and choose 'react' from context menu
class CHandlerChatReact : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* params */)
	{
		if (pCaller == NULL) return;

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->disableTempOver();

		CWidgetManager::getInstance()->disableModalWindow();
		CChatGroup::TGroupType group;
		CEntityId dynamicChannelId;
		string receiver;
		if (LastSelectedHasMessage && !LastSelectedMessage.MessageId.empty() &&
			getChatTextMngr().getSelectedMessageTarget(group, dynamicChannelId, receiver))
		{
			setReactionTarget(LastSelectedMessage.MessageId, group, dynamicChannelId, receiver);
			CEmojiPicker::getInstance().openForReaction();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerChatReact, "chat_react");

// ***************************************************************************
// Called when we click on an emoji of the reaction row
class CHandlerChatReaction : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &params)
	{
		CChatMessageParagraph *paragraph = pCaller ? dynamic_cast<CChatMessageParagraph *>(pCaller->getParent()) : NULL;
		if (!paragraph || params.empty() ||
			!CHAT_MESSAGE::isValidTarget(paragraph->Group, paragraph->DynamicChannelId, paragraph->Receiver))
			return;
		setReactionTarget(paragraph->Message.MessageId, paragraph->Group, paragraph->DynamicChannelId, paragraph->Receiver);
		getChatTextMngr().react(params);
	}
};
REGISTER_ACTION_HANDLER( CHandlerChatReaction, "chat_reaction");

// ***************************************************************************
class CHandlerChatAddFriend: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* params */)
	{
		if (pCaller == NULL) return;

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->disableTempOver();

		const string playerName = getChatTextMngr().getSelectedPlayerName();
		if (!playerName.empty())
		{
			sint playerIndex = PeopleInterraction.IgnoreList.getIndexFromName(playerName);
			if (playerIndex != -1)
				PeopleInterraction.askMoveContact(playerIndex, &PeopleInterraction.IgnoreList, &PeopleInterraction.FriendList);
			else
				PeopleInterraction.askAddContact(playerName, &PeopleInterraction.FriendList);
		}

		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerChatAddFriend, "chat_add_friend");

