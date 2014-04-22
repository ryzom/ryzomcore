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
#include "bot_chat_page_dynamic_mission.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"
#include "../string_manager_client.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "bot_chat_page_all.h"
#include "bot_chat_manager.h"
#include "../client_cfg.h"
#include "../net_manager.h"

using namespace std;

// name of the dialog containing the dynamic mission interface
static const char *WIN_BOT_CHAT_PAGE_DYNAMIC_MISSION = "ui:interface:bot_chat_dynamic_mission";
// db entry for dynamic mission description
static const char *DM_DESCRIPTION_DB_PATH = "SERVER:BOTCHAT:DM_DESCRIPTION";
// db entry for dynamic mission title
static const char *DM_TITLE_DB_PATH = "SERVER:BOTCHAT:DM_TITLE";
// db entry for dynamic mission validation (e.g the user can accept or regen it)
static const char *DM_VALID_DB_PATH = "UI:TEMP:DYNAMIC_MISSION_VALID";
// db entry for a choice
#define DM_CHOICE "SERVER:BOTCHAT:DM_CHOICE"


using namespace STRING_MANAGER;
using NLMISC::toString;

// *************************************************************************************************
CBotChatPageDynamicMission::CBotChatPageDynamicMission()
{
	std::fill(_ChoiceCB, _ChoiceCB + DYNAMIC_MISSION_NUM_CHOICES, (CDBGroupComboBox *) NULL);
	for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
	{
		for(uint l = 0; l < DYNAMIC_MISSION_MAX_NUM_OPTIONS; ++l)
		{
			_TextReceived[k][l] = false;
		}
	}
	std::fill(_Choice, _Choice + DYNAMIC_MISSION_NUM_CHOICES, -1);
	_MissionValid = false;
}

// *************************************************************************************************
void CBotChatPageDynamicMission::invalidateMission()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(DM_DESCRIPTION_DB_PATH)->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp(DM_TITLE_DB_PATH)->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp(DM_VALID_DB_PATH)->setValue32(0);
	_MissionValid = false;
}

// *************************************************************************************************
void CBotChatPageDynamicMission::begin()
{
	CBotChatPage::begin();
	// clear db entries for dynamic missions
	CInterfaceManager *im = CInterfaceManager::getInstance();
	invalidateMission();
	// clear all choices options
	for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString(DM_CHOICE "%d:TITLE", (int) k))->setValue32(0);
		for(uint l = 0; l < DYNAMIC_MISSION_MAX_NUM_OPTIONS; ++l)
		{
			NLGUI::CDBManager::getInstance()->getDbProp(toString(DM_CHOICE "%d:%d:TEXT", (int) k, (int) l))->setValue32(0);
		}
	}
	activateWindow(WIN_BOT_CHAT_PAGE_DYNAMIC_MISSION, true);
	CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_PAGE_DYNAMIC_MISSION));
	if (!ig)
	{
		std::fill(_ChoiceCB, _ChoiceCB + DYNAMIC_MISSION_NUM_CHOICES, (CDBGroupComboBox *) NULL);
	}
	else
	{
		for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
		{
			// get pointers on combo boxs
			_ChoiceCB[k] = dynamic_cast<CDBGroupComboBox *>(ig->getGroup(toString("choice%d", (int) k)));
			// empty the choice list (no datas received yet)
			if (_ChoiceCB[k])
			{
				_ChoiceCB[k]->setActive(false);
				_ChoiceCB[k]->resetTexts();
			}
			for(uint l = 0; l < DYNAMIC_MISSION_MAX_NUM_OPTIONS; ++l)
			{
				_TextReceived[k][l] = false;
			}
		}
	}
	std::fill(_Choice, _Choice + DYNAMIC_MISSION_NUM_CHOICES, -1);
	for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:TEMP:DYNAMIC_MISSION:CHOICE%d", (int) k))->setValue32(-1);
	}
}

// *************************************************************************************************
void CBotChatPageDynamicMission::end()
{
	// if a menu is currently poped, disable it
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CWidgetManager::getInstance()->disableModalWindow();
	activateWindow(WIN_BOT_CHAT_PAGE_DYNAMIC_MISSION, false);
}

// *************************************************************************************************
void CBotChatPageDynamicMission::init()
{

}

// *************************************************************************************************
void CBotChatPageDynamicMission::update()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();

	for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
	{
		if (_ChoiceCB[k])
		{
			uint l;
			if (_ChoiceCB[k]->getNumTexts() != 0 || ClientCfg.Local)
			{
				_ChoiceCB[k]->setActive(true);
			}
			// check for texts waiting to be received
			for(l = 0; l < _ChoiceCB[k]->getNumTexts(); ++l)
			{
				if (!_TextReceived[k][l])
				{
					if (ClientCfg.Local)
					{
						_ChoiceCB[k]->setText(l, ucstring(toString("Dynamic mission %d:%d", (int) k, (int) l)));
						_TextReceived[k][l] = true;
					}
					else
					{
						uint32 textID = (uint32) NLGUI::CDBManager::getInstance()->getDbProp(toString(DM_CHOICE "%d:%d:TEXT", (int) k, (int) l))->getValue32();
						// see if text has been receive
						ucstring result;
						bool received = CStringManagerClient::instance()->getDynString(textID, result);
						if (received)
						{
							_ChoiceCB[k]->setText(l, result);
							_TextReceived[k][l] = true;
						}
					}
				}
			}
			for(l = _ChoiceCB[k]->getNumTexts(); l < DYNAMIC_MISSION_NUM_CHOICES; ++l)
			{
				// see if text id  has been received
				uint32 textID = (uint32) NLGUI::CDBManager::getInstance()->getDbProp(toString(DM_CHOICE "%d:%d:TEXT", (int) k, (int) l))->getValue32();
				if (textID == 0 && !ClientCfg.Local) break;
				// see if text has been received
				ucstring result;
				bool received = CStringManagerClient::instance()->getDynString(textID, result);
				if (received)
				{
					_ChoiceCB[k]->addText(result);
					_TextReceived[k][l] = true;
				}
				else
				{
					// add a text to show the player that the text is being received
					_ChoiceCB[k]->addText(NLMISC::CI18N::get("uiWaitingChoiceFromServer"));
				}
			}
		}
	}
	if (ClientCfg.Local)
	{
		NLGUI::CDBManager::getInstance()->getDbProp(DM_VALID_DB_PATH)->setValue32(1);
	}
	else
	{
		if (!_MissionValid)
		{
			// activate 'regen' and 'accept' button when the description has been received
			uint32 textID = NLGUI::CDBManager::getInstance()->getDbProp(DM_TITLE_DB_PATH)->getValue32();
			if (textID != 0)
			{
				ucstring result;
				if (CStringManagerClient::instance()->getDynString(textID, result))
				{
					textID = NLGUI::CDBManager::getInstance()->getDbProp(DM_DESCRIPTION_DB_PATH)->getValue32();
					if (textID != 0)
					{
						if (CStringManagerClient::instance()->getDynString(textID, result))
						{
							NLGUI::CDBManager::getInstance()->getDbProp(DM_VALID_DB_PATH)->setValue32(1);
							_MissionValid = true;
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************************
void CBotChatPageDynamicMission::sendChoices()
{
	uint k;
	#ifdef NL_DEBUG
		for(k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
		{
			nlassert(_Choice[k] != -1);
		}
	#endif
	NLMISC::CBitMemStream out;
	static const char *msgName = "BOTCHAT:DM_CHOICE";
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		for(k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
		{
			uint8 u8Choice = (uint8) _Choice[k];
			out.serial(u8Choice);
		}
		NetMngr.push(out);
	}
	else
	{
		nlwarning(" unknown message name %s", msgName);
	}
}

// ***************************************************************************************
void CBotChatPageDynamicMission::selectionChanged(uint choice)
{
	if (choice > DYNAMIC_MISSION_NUM_CHOICES)
	{
		return;
	}
	if (!_ChoiceCB[choice]) return;
	_Choice[choice] = _ChoiceCB[choice]->getSelection();
	invalidateMission();
	for(uint k = 0; k < DYNAMIC_MISSION_NUM_CHOICES; ++k)
	{
		if (_Choice[k] == -1) return;
	}
	// all choices have been made, send msg to regen mission
	sendChoices();
}

// ***************************************************************************************
void CBotChatPageDynamicMission::regen()
{
	if (!_MissionValid) return;
	invalidateMission();
	// resend current choices, so that another mission is generated
	sendChoices();
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

// ***************************************************************************************
// the player has clicked on an item to buy it
class CAHChangeDMOption : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &params)
	{
		//get id of choice
		sint id;
		if (!NLMISC::fromString(getParam(params, "id"), id))
		{
			nlwarning("Bad choice list id");
			return;
		}
		if (id == -1) return;
		BotChatPageAll->DynamicMission->selectionChanged((uint) id);
	}
};
REGISTER_ACTION_HANDLER(CAHChangeDMOption, "change_dm_option");

// ***************************************************************************************
// regenerate current mission
class CAHRegenDM : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		BotChatPageAll->DynamicMission->regen();
	}
};
REGISTER_ACTION_HANDLER(CAHRegenDM, "regen_dm");


// ***************************************************************************************
// the player accepted the mission
class CAHAcceptDM : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* params */)
	{
		NLMISC::CBitMemStream out;
		static const char *msgName = "BOTCHAT:DM_ACCEPT";
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			NetMngr.push(out);
		}
		else
		{
			nlwarning(" unknown message name %s", msgName);
		}
		CBotChatManager::getInstance()->setCurrPage(NULL);
	}
};
REGISTER_ACTION_HANDLER(CAHAcceptDM, "accept_dm");



