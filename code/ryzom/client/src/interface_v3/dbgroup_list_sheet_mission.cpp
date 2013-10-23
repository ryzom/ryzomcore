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

#include "game_share/mission_desc.h"

#include "dbgroup_list_sheet_mission.h"
#include "nel/gui/view_text_id_formated.h"
#include "nel/misc/cdb_leaf.h"
#include "interface_manager.h"

using namespace std;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetMission, std::string, "list_sheet_mission");


// **********************************************************************************
void CDBGroupListSheetMission::CSheetChildMission::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	// init the link to the database requesites
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL;
	string sTmp = Ctrl->getSheet()+":PREREQ_STATE";
	sTmp = "LOCAL:" + sTmp.substr(6,sTmp.size());
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp, false);
	nlassert(pNL != NULL);
	CurrentPreReqState.link ( sTmp.c_str() );

}

// **********************************************************************************
CViewText *CDBGroupListSheetMission::CSheetChildMission::createViewText() const
{
	// create a view text id because mission text is send by the server
	CViewTextIDFormated *vti = new CViewTextIDFormated(CViewBase::TCtorParam());
	if (Ctrl) vti->setDBLeaf(dynamic_cast<CCDBNodeLeaf *>(Ctrl->getRootBranch()->getNode(ICDBNode::CTextId("TEXT"))));
	else vti->setDBLeaf(NULL);
	vti->setFormatString(ucstring("$t"));
	return vti;
}

// **********************************************************************************
bool CDBGroupListSheetMission::CSheetChildMission::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	uint8 newPreReqState = (uint8)CurrentPreReqState.getSInt32();

	if (CachePreReqState != newPreReqState)
		return true;

	return false;
}

// ***************************************************************************
void CDBGroupListSheetMission::CSheetChildMission::update(CDBGroupListSheetText *pFather)
{
	uint8 newPreReqState = (uint8)CurrentPreReqState.getSInt32();

	CachePreReqState = newPreReqState;

	CSheetChild::update(pFather);
}

// **********************************************************************************
void CDBGroupListSheetMission::CSheetChildMission::updateViewText(CDBGroupListSheetText *pFather)
{
	CSheetChild::updateViewText(pFather);

	if ((Ctrl == NULL) || (Text == NULL)) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (CachePreReqState == MISSION_DESC::PreReqFail)
	{
		// If mission prerequesits failed for miscellaneaous reasons : COLOR RED
		CRGBA color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetRedifyColor).getValColor();

		Ctrl->setGrayed(true);
		Ctrl->setSheetColor(color);

		color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextRedifyColor).getValColor();
		Text->setColor(color);
	}
	else if (CachePreReqState == MISSION_DESC::PreReqFailAlreadyDone)
	{
		// If mission prerequesits failed because mission is already done : COLOR GREEN
		CRGBA color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetGreenifyColor).getValColor();

		Ctrl->setGrayed(true);
		Ctrl->setSheetColor(color);

		color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextGreenifyColor).getValColor();
		Text->setColor(color);
	}
	else if (CachePreReqState == MISSION_DESC::PreReqFailRunning)
	{
		// If mission prerequesits failed because the mission is in progress : COLOR GRAY
		CRGBA color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlSheetGrayColor).getValColor();

		Ctrl->setGrayed(true);
		Ctrl->setSheetColor(color);

		color = CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextGrayColor).getValColor();
		Text->setColor(color);
	}
	else
	{
		// The mission can be taken : COLOR WHITE
		Ctrl->setGrayed(false);
		Ctrl->setSheetColor(CRGBA::White);

		Text->setColor(CRGBA::White);
	}

}

// **********************************************************************************
bool CDBGroupListSheetMission::CSheetChildMission::isSheetValid(CDBGroupListSheetText * /* pFather */)
{
	if (!Ctrl) return false;
	return Ctrl->getSheetId() != 0;
}

