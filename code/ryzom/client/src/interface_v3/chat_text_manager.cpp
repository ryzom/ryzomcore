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

//=================================================================================
CViewBase *CChatTextManager::createMsgText(const ucstring &cstMsg, NLMISC::CRGBA col, bool justified /*=false*/)
{
	ucstring msg = cstMsg;
	CInterfaceGroup *commandGroup = parseCommandTag(msg);
	CViewText *vt = new CViewText(CViewText::TCtorParam());
	// get parameters from config.xml
	vt->setShadow(isTextShadowed());
	vt->setFontSize(getTextFontSize());
	vt->setMultiLine(true);
	vt->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
	vt->setMultiLineSpace(getTextMultiLineSpace());
	vt->setModulateGlobalColor(false);

	ucstring cur_time;
	if (showTimestamps())
	{
		cur_time = CInterfaceManager::getTimestampHuman();
	}

	// if text contain any color code, set the text formated and white,
	// otherwise, set text normal and apply global color
	size_t codePos = msg.find(ucstring("@{"));
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
		

		vt->setTextFormatTaged(msg);
		vt->setColor(NLMISC::CRGBA::White);
	}
	else
	{
		msg = cur_time + msg;
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
}
