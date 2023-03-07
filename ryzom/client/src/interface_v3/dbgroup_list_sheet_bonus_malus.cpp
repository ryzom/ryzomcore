// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "dbgroup_list_sheet_bonus_malus.h"
#include "interface_manager.h"
#include "nel/misc/xml_macros.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
#define DISABLE_LEAF "DISABLED"

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetBonusMalus, std::string, "list_sheet_bonus_malus");

// ***************************************************************************
CDBGroupListSheetBonusMalus::CDBGroupListSheetBonusMalus(const TCtorParam &param)
:	CDBGroupListSheet(param),
	_RegenTextEnabled(true),
	_RegenTextY(-14), _RegenTextFontSize(8),
	_RegenTextColor(NLMISC::CRGBA::White),
	_RegenTextDisabledColor(NLMISC::CRGBA(127,127,127))
{
	// want leave space between controls in the list
	// Yoyo: I think it's better like this, + this is important for space consideration and because of XPCat/PVPOutpost
	//_ListLeaveSpace= false;
}

// ***************************************************************************
CDBGroupListSheetBonusMalus::CSheetChildTimer::CSheetChildTimer()
:	TimerDB(NULL), DisabledDB(NULL), TimerCache(0),
	_RegenTextColor(NLMISC::CRGBA::White),
	_RegenTextDisabledColor(NLMISC::CRGBA(127,127,127))
{
}

// ***************************************************************************
void CDBGroupListSheetBonusMalus::CSheetChildTimer::init(CDBGroupListSheet *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	CCDBNodeBranch *root = Ctrl->getRootBranch();
	if (root)
	{
		TimerDB = dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("DISABLED_TIME"), false));
		DisabledDB = dynamic_cast<CCDBNodeLeaf *>(root->getNode(ICDBNode::CTextId("DISABLED"), false));
	}

	if (Ctrl)
	{
		CDBGroupListSheetBonusMalus *owner = dynamic_cast<CDBGroupListSheetBonusMalus *>(pFather);
		if (owner)
		{
			_RegenTextColor = owner->_RegenTextColor;
			_RegenTextDisabledColor = owner->_RegenTextDisabledColor;
			Ctrl->setRegenText(owner->_RegenTextEnabled);
			Ctrl->setRegenTextY(owner->_RegenTextY);
			Ctrl->setRegenTextColor(owner->_RegenTextColor);
			Ctrl->setRegenTextFontSize(owner->_RegenTextFontSize);
			if (!owner->_RegenTextFct.empty())
				Ctrl->setRegenTextFct(owner->_RegenTextFct);
		}

		Ctrl->setRegenTextOutline(true);
	}
}

// ***************************************************************************
void CDBGroupListSheetBonusMalus::CSheetChildTimer::update(CDBGroupListSheet * /* pFather */)
{
	if(!TimerDB)
		return;

	NLMISC::TGameCycle tick = TimerDB->getValue32();
	if (TimerCache != tick)
	{
		TimerCache = TimerDB->getValue32();
		Ctrl->setRegenTickRange(CTickRange(LastGameCycle, TimerCache));
		if (DisabledDB)
		{
			if (DisabledDB->getValue32() == 0)
			{
				// active timer
				Ctrl->setGrayed(false);
				Ctrl->setRegenTextColor(_RegenTextColor);
			}
			else
			{
				// skill disabled timer
				Ctrl->setGrayed(true);
				Ctrl->setRegenTextColor(_RegenTextDisabledColor);
			}
		}
		else
		{
			Ctrl->setGrayed(true);
		}
	}
}

// ***************************************************************************
bool CDBGroupListSheetBonusMalus::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!CDBGroupListSheet::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr prop;
	XML_READ_BOOL(cur, "regen_text", _RegenTextEnabled, true);
	XML_READ_SINT(cur, "regen_text_y", _RegenTextY, -14);
	XML_READ_UINT(cur, "regen_text_fontsize", _RegenTextFontSize, 8);
	XML_READ_COLOR(cur, "regen_text_color", _RegenTextColor, NLMISC::CRGBA::White);
	XML_READ_COLOR(cur, "regen_text_disabled_color", _RegenTextDisabledColor, NLMISC::CRGBA(127, 127, 127));
	XML_READ_STRING(cur, "regen_text_fct", _RegenTextFct, "");

	return true;
}

