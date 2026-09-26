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
#include "chat_text_manager.h"
#include "chat_link_ui.h"
#include "emoji_manager.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/view_link.h"
#include "nel/gui/view_text.h"
#include "nel/gui/ctrl_tooltip.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/group_paragraph.h"
#include "interface_manager.h"
#include "../connection.h"

using namespace std;
using namespace NLMISC;

CChatTextManager* CChatTextManager::_Instance = NULL;

// last selected chat from 'copy_chat_popup' action handler
static std::string LastSelectedChat;
static CSheetId LastSelectedChatSheetId = CSheetId::Unknown;

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
	delete _TextFontSize;
	_TextFontSize = NULL;
	delete _TextMultilineSpace;
	_TextMultilineSpace = NULL;
	delete _TextShadowed;
	_TextShadowed = NULL;
	delete _ShowTimestamps;
	_ShowTimestamps = NULL;
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
	// Small follows the chat font so emoji sit in the line, and large is the
	// atlas tile's own 32px. Medium is the midpoint between the two rather
	// than a fixed multiple of the font: at the default font size of 10 the
	// old "font * 3/2" gave 10 / 15 / 32, so the first step was barely
	// visible and the second was enormous. The midpoint makes the three
	// settings evenly spaced whatever the font size is.
	const sint32 small = (sint32)getTextFontSize();
	const sint32 large = EmojiTilePixels;
	switch (getEmojiSize())
	{
		case EmojiLarge:	return large;
		case EmojiMedium:	return small < large ? (small + large) / 2 : large;
		case EmojiSmall:
		default:			return small;
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

CViewBase *CChatTextManager::createMsgText(const string &cstPrefix, const CChatMessage &message,
	NLMISC::CRGBA col, bool justified)
{
	string prefix = cstPrefix;
	if (showTimestamps())
		prependTimestamp(prefix);

	CGroupParagraph *para = new CGroupParagraph(CViewBase::TCtorParam());
	para->setId("line");
	para->setSizeRef("w");
	para->setResizeFromChildH(true);

	string copyText = prefix;
	if (!prefix.empty())
		addMsgText(para, prefix, col, justified, 0, prefix.size());
	for (std::vector<CChatMessagePart>::const_iterator it = message.Parts.begin(); it != message.Parts.end(); ++it)
	{
		if (it->Type == CChatMessagePart::Text)
		{
			const string text = it->TextValue.toUtf8();
			// Same substitution a chat line gets, per part: each is passed to
			// addMsgText whole, so no index outlives it.
			CEmojiManager &emoji = CEmojiManager::getInstance();
			const string shown = (getEmojiMode() == EmojiUnicode && emoji.mayContainEmoji(text))
				? emoji.substituteShortcodes(text)
				: text;
			addMsgText(para, shown, col, justified, 0, shown.size());
			copyText += text;
		}
		else
		{
			CViewLink *view = CHAT_SHARE::createAttachmentView(*it, justified);
			para->addChildLink(view);
			copyText += view->LinkTitle;
		}
	}
	para->setRightClickHandler("copy_chat_popup");
	para->setRightClickHandlerParams(copyText);
	return para;
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

	// Carry the colour/tooltip state across the split. See
	// CViewText::getFormatTagPrefixAt: for an untagged line this is empty and
	// the piece keeps taking the plain path with the channel colour.
	string seg = CViewText::getFormatTagPrefixAt(msg, (uint)from);
	seg.append(msg, from, to - from);

	CViewBase *vt = createMsgTextSimple(seg, col, justified, NULL);
	if (id)
		vt->setId(id);
	para->addChild(vt);
}

//=================================================================================
/** One emoji image in a chat line, named on hover.
  *
  * A tooltip is context help, and context help lives on CCtrlBase, so this
  * cannot be the CViewBitmap it otherwise would be. The one ctrl that may sit
  * in a chat line is CCtrlToolTip: it is not capturable, so the paragraph keeps
  * its right-click-to-copy and a link under the same line still takes clicks.
  * Anything button-like would swallow the pointer wherever an emoji happened
  * to sit.
  *
  * Drawing is CViewBitmap's scaled path, minus the tiling and rotation that an
  * emoji never uses.
  */
class CCtrlEmoji : public CCtrlToolTip
{
public:
	CCtrlEmoji(const TCtorParam &param) : CCtrlToolTip(param) {}

	/// False if the atlas is missing or has no tile of that name.
	bool setTexture(const std::string &texture)
	{
		_TextureId.setTexture(texture.c_str());
		return !_TextureId.empty();
	}

	virtual void draw()
	{
		CRGBA col = CRGBA::White;
		// The tile carries its own colours, so it is not modulated by the
		// interface colour -- only faded with it, like the rest of the window.
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
		// Atlas missing or the tile is not in it. Caller falls back to text.
		delete bm;
		return NULL;
	}
	sint32 size = getEmojiPixelSize();
	bm->setW(size);
	bm->setH(size);
	bm->setModulateGlobalColor(false);
	// Name it on hover, in the form that can be typed back: ":fire:". Nameless
	// entries simply get no tooltip.
	if (!name.empty())
		bm->setDefaultContextHelp(":" + name + ":");
	return bm;
}

//=================================================================================
void CChatTextManager::addMsgText(CGroupParagraph *para, const string &msg, NLMISC::CRGBA col,
	bool justified, string::size_type pos, string::size_type textSize)
{
	// Image mode cuts the line around each emoji, so this has to know where
	// they are. Worked out here rather than passed in, so that every caller --
	// a chat line, a shared item's text -- gets them.
	CEmojiManager &emoji = CEmojiManager::getInstance();
	const bool useEmojiImages = getEmojiMode() == EmojiImage && emoji.mayContainEmoji(msg);

	// quickly check if text has links or not
	bool hasUrl;
	{
		string s = toLowerAscii(msg);
		hasUrl = (s.find("http://") || s.find("https://"));
	}

	for (string::size_type i = pos; i< textSize;)
	{
		// Step over format tags instead of scanning inside them. A tooltip tag
		// holds arbitrary text, so "@{Hsee :smile: here}" contains something
		// that looks exactly like a shortcode, and matching it would cut the
		// tag in half. The same goes for a URL written inside one.
		uint tagLen = CViewText::getFormatTagLength(msg, (uint)i);
		if (tagLen)
		{
			i += tagLen;
			continue;
		}

		string::size_type emojiLen = 0;
		const CEmojiManager::CEntry *emojiEntry = NULL;

		if (useEmojiImages && emoji.matchAt(msg, i, textSize, emojiLen, emojiEntry))
		{
			addTextSegment(para, msg, pos, i, col, justified);

			CViewBase *ev = createEmojiView(emojiEntry->Texture, emojiEntry->Name);
			if (ev)
			{
				para->addChild(ev);
			}
			else
			{
				// No tile for this one, so fall back a step rather than showing
				// nothing: the unicode form, still carrying the format state.
				string seg = CViewText::getFormatTagPrefixAt(msg, (uint)i);
				seg += emojiEntry->Utf8;
				para->addChild(createMsgTextSimple(seg, col, justified, NULL));
			}

			pos = i + emojiLen;
			i = pos;
		}
		else if (hasUrl && isUrlTag(msg, i, textSize))
		{
			addTextSegment(para, msg, pos, i, col, justified);

			string url;
			string title;
			getUrlTag(msg, i, url, title);
			if (url.size() > 0)
			{
				CViewLink *vt = new CViewLink(CViewBase::TCtorParam());
				vt->setId("link");
				vt->setUnderlined(true);
				vt->setShadow(isTextShadowed());
				vt->setShadowOutline(false);
				vt->setFontSize(getTextFontSize());
				vt->setMultiLine(true);
				vt->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
				vt->setMultiLineSpace(getTextMultiLineSpace());
				vt->setModulateGlobalColor(false);

				//NLMISC::CRGBA color;
				//color.blendFromui(col, CRGBA(255, 153, 0, 255), 100);
				//vt->setColor(color);
				vt->setColor(col);

				if (title.size() > 0)
				{
					vt->LinkTitle = title;
					vt->setText(vt->LinkTitle);
				}
				else
				{
					vt->LinkTitle = url;
					vt->setText(vt->LinkTitle);
				}

				if (url.find_first_of('\'') != string::npos)
				{
					string clean;
					for(string::size_type i = 0; i< url.size(); ++i)
					{
						if (url[i] == '\'')
							clean += "%27";
						else
							clean += url[i];
					}
					url = clean;
				}
				vt->setActionOnLeftClick("lua");
				vt->setParamsOnLeftClick("game:chatUrl('" + url + "')");

				para->addChildLink(vt);

				pos = i;
			}
		}
		else
		{
			++i;
		}
	}

	addTextSegment(para, msg, pos, textSize, col, justified, "text");
}

//=================================================================================
CViewBase *CChatTextManager::createMsgTextComplex(const string &originalMsg, NLMISC::CRGBA col, bool justified, bool plaintext, CInterfaceGroup *commandGroup)
{
	// In unicode mode the shortcodes become characters and the line stays a
	// single view, so nothing is split and there is no format state to carry.
	// That is why this mode cannot disturb text colour at all. It has to happen
	// before anything below indexes the line.
	CEmojiManager &emoji = CEmojiManager::getInstance();
	const uint emojiMode = getEmojiMode();
	const string msg = (emojiMode == EmojiUnicode && emoji.mayContainEmoji(originalMsg))
		? emoji.substituteShortcodes(originalMsg)
		: originalMsg;

	string::size_type textSize = msg.size();

	CGroupParagraph *para = new CGroupParagraph(CViewBase::TCtorParam());
	para->setId("line");
	para->setSizeRef("w");
	para->setResizeFromChildH(true);

	// use right click because left click might be used to activate chat window
	// Copy yields what was actually said, with ":name:" intact.
	para->setRightClickHandler("copy_chat_popup");
	para->setRightClickHandlerParams(originalMsg);

	if (plaintext)
	{
		CViewBase *vt = createMsgTextSimple(msg, col, justified, NULL);
		vt->setId("text");
		para->addChild(vt);

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

		CViewBase *vt = createMsgTextSimple(msg.substr(0, startTr), col, justified, NULL);
		para->addChild(vt);

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

	addMsgText(para, msg, col, justified, pos, textSize);

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
		LastSelectedChatSheetId = CSheetId::Unknown;

		CGroupParagraph *pGP = dynamic_cast<CGroupParagraph *>(pCaller);
		if (pGP) pGP->enableTempOver();
		if (pGP && !UserPrivileges.empty())
		{
			CCtrlBase *ctrl = CWidgetManager::getInstance()->getCapturePointerRight();
			if (ctrl && ctrl->getParent() == pGP)
				CHAT_SHARE::getAttachmentSheetId(ctrl, LastSelectedChatSheetId);
		}

		CGroupMenu *menu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:chat_copy_action_menu"));
		if (menu && menu->getRootMenu())
		{
			const sint line = menu->getRootMenu()->getLineFromId("copy_sheet_id");
			if (line >= 0)
				menu->getRootMenu()->setHiddenLine(line, LastSelectedChatSheetId == CSheetId::Unknown);
		}

		CWidgetManager::getInstance()->enableModalWindow (pCaller, "ui:interface:chat_copy_action_menu");
	}
};
REGISTER_ACTION_HANDLER( CHandlerCopyChatPopup, "copy_chat_popup");

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

