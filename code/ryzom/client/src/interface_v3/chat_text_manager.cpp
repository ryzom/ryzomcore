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



#include "stdpch.h"
// client
#include "chat_text_manager.h"
#include "nel/gui/view_text.h"
#include "nel/gui/group_paragraph.h"
#include "interface_manager.h"

using namespace std;
using namespace NLMISC;

CChatTextManager* CChatTextManager::_Instance = NULL;

//=================================================================================
CChatTextManager::CChatTextManager() :
	_TextFontSize(NULL),
	_TextMultilineSpace(NULL),
	_TextShadowed(NULL),
	_ShowTimestamps(NULL)
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
static CInterfaceGroup *parseCommandTag(ucstring &line)
{
	string::size_type start = line.find(ucstring("/$$"));
	if (start == string::npos) return NULL;
	string::size_type end = line.find(ucstring("$$/"), start + 3);
	if (end == string::npos) return NULL;
	std::string commandLine;
	ucstring ucCommandLine = line.substr(start + 3, end - start - 3);
	ucCommandLine.toString(commandLine);
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

static inline bool	isUrlTag(const ucstring &s, ucstring::size_type index, ucstring::size_type textSize)
{
	// Format http://, https://
	// or markdown style (title)[http://..]
	if(textSize > index+7)
	{
		bool markdown = false;
		ucstring::size_type i = index;
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
			bool isUrl = (toLower(s.substr(i, 7)) == ucstring("http://") || toLower(s.substr(i, 8)) == ucstring("https://"));
			// match "text http://" and not "texthttp://"
			if (isUrl && i > 0 && !markdown)
			{
				// '}' is in the list because of color tags, ie "@{FFFF}http://..."
				const ucchar chars[] = {' ', '"', '\'', '(', '[', '}'};
				isUrl = std::find(std::begin(chars), std::end(chars), s[i - 1]) != std::end(chars);
			}
			return isUrl;
		}
	}

	return false;
}

// ***************************************************************************
// isUrlTag must match
static inline void getUrlTag(const ucstring &s, ucstring::size_type &index, ucstring &url, ucstring &title)
{
	bool isMarkdown = false;
	ucstring::size_type textSize = s.size();
	ucstring::size_type pos;

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

	ucchar chOpen = ' ';
	ucchar chClose = ' ';
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
		if (pos != ucstring::npos && index > 0)
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
	if (pos == ucstring::npos)
	{
		pos = textSize;
	}

	url = s.substr(index, pos - index);
	index = pos;

	// skip ']' closing char
	if (isMarkdown) index++;
}

//=================================================================================
static void prependTimestamp(ucstring &msg)
{
	ucstring cur_time;
	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:SHOW_CLOCK_12H", false);
	if (node && node->getValueBool())
		cur_time = CInterfaceManager::getTimestampHuman("[%I:%M:%S %p] ");
	else
		cur_time = CInterfaceManager::getTimestampHuman();

	ucstring::size_type codePos = msg.find(ucstring("@{"));
	if (codePos != ucstring::npos)
	{
		// Prepend the current time (do it after the color if the color at first position.
		if (codePos == 0)
		{
			codePos = msg.find(ucstring("}"));
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
CViewBase *CChatTextManager::createMsgText(const ucstring &cstMsg, NLMISC::CRGBA col, bool justified /*=false*/, bool plaintext /*=false*/)
{
	ucstring msg = cstMsg;
	CInterfaceGroup *commandGroup = parseCommandTag(msg);

	if (showTimestamps())
		prependTimestamp(msg);

	// must wrap all lines to CGroupParagraph because CGroupList will calculate
	// width from previous line which ends up as CViewText otherwise
	return createMsgTextComplex(msg, col, justified, plaintext, commandGroup);
}

//=================================================================================
CViewBase *CChatTextManager::createMsgTextSimple(const ucstring &msg, NLMISC::CRGBA col, bool justified, CInterfaceGroup *commandGroup)
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
	if (msg.find(ucstring("@{")) != ucstring::npos)
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
CViewBase *CChatTextManager::createMsgTextComplex(const ucstring &msg, NLMISC::CRGBA col, bool justified, bool plaintext, CInterfaceGroup *commandGroup)
{
	ucstring::size_type textSize = msg.size();

	CGroupParagraph *para = new CGroupParagraph(CViewBase::TCtorParam());
	para->setId("line");
	para->setSizeRef("w");
	para->setResizeFromChildH(true);

	if (plaintext)
	{
		CViewBase *vt = createMsgTextSimple(msg, col, justified, NULL);
		vt->setId("text");
		para->addChild(vt);

		return para;
	}


	// quickly check if text has links or not
	bool hasUrl;
	{
		ucstring s = toLower(msg);
		hasUrl = (s.find(ucstring("http://")) || s.find(ucstring("https://")));
	}

	ucstring::size_type pos = 0;
	for (ucstring::size_type i = 0; i< textSize;)
	{
		if (hasUrl && isUrlTag(msg, i, textSize))
		{
			if (pos != i)
			{
				CViewBase *vt = createMsgTextSimple(msg.substr(pos, i - pos), col, justified, NULL);
				para->addChild(vt);
			}

			ucstring url;
			ucstring title;
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
					vt->LinkTitle = title.toUtf8();
					vt->setText(title);
				}
				else
				{
					vt->LinkTitle = url.toUtf8();
					vt->setText(url);
				}

				if (url.find_first_of('\'') != string::npos)
				{
					ucstring clean;
					for(string::size_type i = 0; i< url.size(); ++i)
					{
						if (url[i] == '\'')
							clean += ucstring("%27");
						else
							clean += url[i];
					}
					url = clean;
				}
				vt->setActionOnLeftClick("lua");
				vt->setParamsOnLeftClick("game:chatUrl('" + url.toUtf8() + "')");

				para->addChildLink(vt);

				pos = i;
			}
		}
		else
		{
			++i;
		}
	}

	if (pos < textSize)
	{
		CViewBase *vt = createMsgTextSimple(msg.substr(pos, textSize - pos), col, justified, NULL);
		vt->setId("text");
		para->addChild(vt);
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
	_TextFontSize = NULL;
	_TextMultilineSpace = NULL;
	_TextShadowed = NULL;
	_ShowTimestamps = NULL;
}
