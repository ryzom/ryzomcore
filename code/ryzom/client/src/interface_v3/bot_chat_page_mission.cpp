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
#include "bot_chat_page_mission.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "dbgroup_list_sheet_trade.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_container.h"
#include "dbctrl_sheet.h"
#include "nel/gui/view_text_id.h"
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;

// name of the dialog containing the mission interface
static const char *WIN_BOT_CHAT_PAGE_MISSION = "ui:interface:bot_chat_missions";
// window to accept the mission
static const char *WIN_BOT_CHAT_ACCEPT_MISSION = "ui:interface:bot_chat_accept_mission";


// ***************************************************************************************
CBotChatPageMission::CBotChatPageMission()
{
	_MissionPagesObs.setListType(CHugeListObs::Missions);
	_CurrSel = NULL;
}

// *******************************************************************************************
void CBotChatPageMission::init()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	if (NLGUI::CDBManager::getInstance()->getDbBranch("SERVER:CHOOSE_MISSIONS"))
		NLGUI::CDBManager::getInstance()->addBranchObserver("SERVER:CHOOSE_MISSIONS", &_MissionPagesObs);
}

// ***************************************************************************************
void CBotChatPageMission::begin()
{
	CBotChatPage::begin();
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// clear intro text
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":CHOOSE_MISSION")->setValue32(0);
	_MissionPagesObs.setMissionClientType(_MType);
	_MissionPagesObs.start();

	// Select the Mission Aspect according to mission type
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION:MISSION_TYPE")->setValue32(_MType);

	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_PAGE_MISSION));
	if (gc)
	{
		// show the ui
		gc->setActive(true);
		// unselect any list
		CDBGroupListSheetText *buyListSheet = dynamic_cast<CDBGroupListSheetText *>(gc->getGroup("missions"));
		if (buyListSheet)
		{
			buyListSheet->unselect();
		}
	}

}

// ***************************************************************************************
void CBotChatPageMission::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_MISSION, false);
	activateWindow(WIN_BOT_CHAT_ACCEPT_MISSION, false);
}

// ***************************************************************************************
void CBotChatPageMission::selectMission(CDBCtrlSheet *missionSheet)
{
	if (!missionSheet) return;
	if (missionSheet->getGrayed()) return;
	// show the dialog with good infos
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_ACCEPT_MISSION));

	// copy text id for title
	{
		CCDBNodeLeaf *titleTextLeaf = dynamic_cast<CCDBNodeLeaf *>(missionSheet->getRootBranch()->getNode(ICDBNode::CTextId("TEXT")));
		//
		CViewTextID	*viewTitleTextID = dynamic_cast<CViewTextID *>(gc->getView("text_title_id"));
		if (viewTitleTextID && titleTextLeaf)
			viewTitleTextID->setTextId(titleTextLeaf->getValue32());
	}

	// copy text id for details
	CCDBNodeLeaf *detailTextLeaf = dynamic_cast<CCDBNodeLeaf *>(missionSheet->getRootBranch()->getNode(ICDBNode::CTextId("DETAIL_TEXT")));
	//
	CViewTextID	*viewTextID = dynamic_cast<CViewTextID *>(gc->getView("text_id"));
	if(viewTextID && detailTextLeaf)
	{
		viewTextID->setTextId(detailTextLeaf->getValue32());
	}
	// copy icon
	CCDBNodeLeaf *iconLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION:ICON");
	if (iconLeaf)
	{
		iconLeaf->setValue32(missionSheet->getSheetId());
	}
	//
	if (!gc) return;
	CWidgetManager::getInstance()->setTopWindow(gc);
	gc->setActive(true);
	gc->updateCoords();
	gc->center();
	gc->setModalParentList(WIN_BOT_CHAT_PAGE_MISSION);
	_CurrSel = missionSheet;
}

// ***************************************************************************************
void CBotChatPageMission::acceptMission()
{
	if (!_CurrSel) return;
	sint index = _CurrSel->getIndexInParent();
	if (index < 0) return;
	// send msg to server
	NLMISC::CBitMemStream out;
	static const char *msgName;
	if(_MType==MISSION_DESC::ZCCharge)
		msgName= "BOTCHAT:DUTY_APPLY";
	else
		msgName= "BOTCHAT:PICK_MISSION";
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		uint8 missionIndex = (uint8) index;
		out.serial(missionIndex);
		NetMngr.push(out);
	}
	else
	{
		nlwarning(" unknown message name %s", msgName);
	}
	// close the selection box
	activateWindow(WIN_BOT_CHAT_ACCEPT_MISSION, false);
	/// close the botchat
	//CBotChatManager::getInstance()->setCurrPage(NULL);
	_CurrSel = NULL;
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

// handler for bot chat mission
class CAHSelectMission : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* params */)
	{
		CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (sheet) BotChatPageAll->ChooseMission->selectMission(sheet);
	}
};
REGISTER_ACTION_HANDLER(CAHSelectMission, "select_mission");

// the player has accepted a mission and now select it
class CAHAcceptMission : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->ChooseMission->acceptMission();
	}
};
REGISTER_ACTION_HANDLER(CAHAcceptMission, "accept_mission");


//=====================================================================================
/** Tests for missions pages ..
  */
NLMISC_COMMAND( testMissionPage, "<tmp> debug mission page", "")
{
	if (args.size() != 1) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:0:ICON")->setValue32(CSheetId("generic_craft.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:0:TEXT")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:0:DETAIL_TEXT")->setValue32(11);

	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:1:ICON")->setValue32(CSheetId("generic_fight.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:1:TEXT")->setValue32(2);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:1:DETAIL_TEXT")->setValue32(12);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:2:ICON")->setValue32(CSheetId("generic_forage.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:2:TEXT")->setValue32(3);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:2:DETAIL_TEXT")->setValue32(13);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:3:ICON")->setValue32(CSheetId("generic_generic.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:3:TEXT")->setValue32(4);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:3:DETAIL_TEXT")->setValue32(14);

	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:4:ICON")->setValue32(CSheetId("generic_rite.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:4:TEXT")->setValue32(5);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:4:DETAIL_TEXT")->setValue32(15);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:5:ICON")->setValue32(CSheetId("generic_travel.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:5:TEXT")->setValue32(6);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:5:DETAIL_TEXT")->setValue32(16);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:6:ICON")->setValue32(CSheetId("generic_craft.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:6:TEXT")->setValue32(7);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:6:DETAIL_TEXT")->setValue32(17);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:7:ICON")->setValue32(CSheetId("generic_fight.mission_icon").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:7:TEXT")->setValue32(8);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:7:DETAIL_TEXT")->setValue32(18);
	//
	sint32 pageId;
	fromString(args[0], pageId);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:SESSION")->setValue32(CBotChatManager::getInstance()->getSessionID());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:PAGE_ID")->setValue32(pageId);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHOOSE_MISSIONS:HAS_NEXT")->setValue32(0); // not relevant for test ..
	return true;
}
