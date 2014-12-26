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


#include "nel/gui/action_handler.h"
#include "interface_manager.h"
#include "bot_chat_manager.h"
#include "../sheet_manager.h"
#include "skill_manager.h"
#include "dbctrl_sheet.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "group_quick_help.h"
#include "nel/gui/view_text_id.h"
#include "../user_entity.h"
#include "../entities.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/dbview_bar.h"
#include "../debug_client.h"
#include "interface_3d_scene.h"
#include "character_3d.h"
#include "item_special_effect.h"
#include "item_consumable_effect.h"
#include "bonus_malus.h"
// Game share
#include "game_share/slot_types.h"
#include "game_share/item_family.h"
#include "game_share/skills.h"
#include "game_share/armor_types.h"
#include "game_share/weapon_types.h"
#include "game_share/damage_types.h"
#include "game_share/ecosystem.h"
#include "game_share/mp_category.h"
#include "game_share/item_origin.h"
#include "game_share/shield_types.h"
#include "game_share/trade_slot_type.h"
#include "../string_manager_client.h"
#include "game_share/sphrase_com.h"
#include "sbrick_manager.h"
#include "sphrase_manager.h"
#include "action_handler_help.h"
#include "nel/misc/i18n.h"
#include "nel/misc/algo.h"
#include "nel/net/email.h"
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/prerequisit_infos.h"
#include "game_share/resistance_type.h"
#include "../r2/editor.h"
#include "../init.h"
#include "../browse_faq.h"
#include "people_list.h"
#include "people_interraction.h"

extern CSheetManager	SheetMngr;

extern NLMISC::CLog		g_log;

using namespace std;
using namespace NLMISC;
using namespace STRING_MANAGER;


///////////////////////////////////
// STATIC FUNCTIONS DECLARATIONS //
///////////////////////////////////
static void setupCreatorName(CSheetHelpSetup &setup);
static void setHelpText(CSheetHelpSetup &setup, const ucstring &text);
static void setHelpTextID(CSheetHelpSetup &setup, sint32 id);
static void fillSabrinaPhraseListBrick(const CSPhraseCom &phrase, IListSheetBase *listBrick);
static void setupListBrickHeader(CSheetHelpSetup &setup);
static void hideListBrickHeader(CSheetHelpSetup &setup);

static void setupHelpPage(CInterfaceGroup *window, const string &url);
static void setupHelpTitle(CInterfaceGroup	*group, const ucstring &title);
static void setHelpCtrlSheet(CSheetHelpSetup &setup, uint32 sheetId);

// Setup help for an item in a window (type is known)
static void setupItemHelp(CSheetHelpSetup &setup);
static void setupPactHelp(CSheetHelpSetup &setup);
static void setupSkillToTradeHelp(CSheetHelpSetup &setup);
static void setupSabrinaBrickHelp(CSheetHelpSetup &setup, bool auraDisabled= false);
void setupSabrinaPhraseHelp(CSheetHelpSetup &setup, const class CSPhraseCom &phrase, uint32 phraseSheetId);
static void setupMissionHelp(CSheetHelpSetup &setup);

// ***************************************************************************
#define INFO_LIST_ITEM					"list_item"
#define INFO_LIST_BRICK					"list_brick"
#define INFO_LIST_BRICK_HEADER			"list_brick_header"
#define INFO_LIST_BRICK_REQUIREMENT		"list_brick_requirement"
#define INFO_GROUP_MP_STAT				"mp_stats"
#define INFO_GROUP_CHAR_3D				"char3d"
#define INFO_ITEM_PREVIEW				"item_preview"
#define INFO_ITEM_PREVIEW_SCENE_3D		"scene_item_preview"
#define ITEM_PREVIEW_WIDTH				200

// ***************************************************************************
std::deque<uint>							CInterfaceHelp::_ActiveWindows;
std::vector<CInterfaceHelp::CInfoWindow>	CInterfaceHelp::_InfoWindows;
bool										CInterfaceHelp::_InfoWindowInit= false;
CInterfaceHelp::CFittedWeaponWeightObserver CInterfaceHelp::_FittedWeaponWeightObserver;


// ***************************************************************************
void	CInterfaceHelp::CInfoWindow::infoReceived()
{
	// refresh text
	if(CtrlSheet && Window)
	{
		CSheetHelpSetup setup;
		setup.setupDefaultIDs();
		setup.HelpWindow = Window;
		setup.SrcSheet = CtrlSheet;
		refreshItemHelp(setup);
	}
}

// ***************************************************************************
void	CInterfaceHelp::CInfoWindow::missionInfoReceived(const CPrerequisitInfos &infos)
{
	// refresh text
	if(CtrlSheet && Window)
	{
		CSheetHelpSetup setup;
		setup.setupDefaultIDs();
		setup.HelpWindow = Window;
		setup.SrcSheet = CtrlSheet;
		refreshMissionHelp(setup, infos);
	}
}

// ***************************************************************************
void	CInterfaceHelp::initWindows()
{
	if(_InfoWindowInit)
		return;

	_InfoWindowInit= true;

	// Get the Max window allowed.
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	sint maxHelpWindow;
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("MAX_HELP_WINDOW"), maxHelpWindow);

	// Allow Max 256. More may be a script error...
	clamp(maxHelpWindow, 0, 256);

	for(sint i=0;i<maxHelpWindow;i++)
	{
		CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:sheet_help"+toString(i)));
		// if the window exist, insert
		if(group)
		{
			CInfoWindow	newInfo;
			newInfo.Window= group->getId();
			newInfo.KeepButton= group->getId() + ":header_opened:keep";
			_InfoWindows.push_back(newInfo);
		}
		else
			break;
	}
	// add observers for the update of phrase help texts (depends of weight of equipped weapons)
	for (uint i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp(std::string(LOCAL_INVENTORY) + ":HAND:" + toString(i), false);
		if(pNodeLeaf)
		{
			ICDBNode::CTextId textId;
			pNodeLeaf->addObserver(&_FittedWeaponWeightObserver, textId);
		}
	}
}



// ***************************************************************************
void	CInterfaceHelp::release()
{
	if (_InfoWindowInit)
	{
		_InfoWindowInit = false;
		_InfoWindows.clear();
	}
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// add observers for the update of phrase help texts (depends of weight of equipped weapons)
	for (uint i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp(std::string(LOCAL_INVENTORY) + ":HAND:" + toString(i), false);
		if(pNodeLeaf)
		{
			ICDBNode::CTextId textId;
			pNodeLeaf->removeObserver(&_FittedWeaponWeightObserver, textId);
		}
	}
}

// ***************************************************************************
void CInterfaceHelp::CFittedWeaponWeightObserver::update(ICDBNode* node)
{
	CInterfaceHelp::updateWindowSPhraseTexts();
}

// ***************************************************************************
CInterfaceGroup	*CInterfaceHelp::activateNextWindow(CDBCtrlSheet *elt, sint forceKeepWindow)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// update WindowList if possible
	initWindows();
	sint	maxHelpWindow= (sint)_InfoWindows.size();


	// Update Active window list
	uint i;
	for(i=0;i<_ActiveWindows.size();)
	{
		CInterfaceGroup	*group= _InfoWindows[_ActiveWindows[i]].Window;
		// if the window has been closed, remove it from list
		if(!group->getActive())
		{
			_ActiveWindows.erase(_ActiveWindows.begin()+i);
		}
		else
			i++;
	}

	bool showSlotAndCreator = false;
	// If an active window get the same object, abort, but make it top.
	for(i=0;i<_ActiveWindows.size();i++)
	{
		CInterfaceGroup	*group= _InfoWindows[_ActiveWindows[i]].Window;
		CDBCtrlSheet		*ctrlSrc= elt;
		// get the ctrl sheet in this help window.
		CDBCtrlSheet		*ctrlDst= dynamic_cast<CDBCtrlSheet*>(group->getCtrl(":ctrl_slot"));
		if(ctrlDst && ctrlSrc)
		{
			// if same Aspect
			if( ctrlSrc->sameAspect(ctrlDst) )
			{
				bool	ok= true;

				// for items, must also test if they have the same itemSlotId, cause relies also on "ItemInfo system"
				if(elt->getType() == CCtrlSheetInfo::SheetType_Item)
				{
					showSlotAndCreator = true;

					CDBCtrlSheet		*realCtrlDst= _InfoWindows[_ActiveWindows[i]].CtrlSheet;
					if(!realCtrlDst)
						ok= false;
					else if( getInventory().getItemSlotId(ctrlSrc) != getInventory().getItemSlotId(realCtrlDst) )
						ok= false;
				}

				// if success to find same element
				if(ok)
				{
					// then don't neet to open a new window, but make the older top.
					CWidgetManager::getInstance()->setTopWindow(group);
					return NULL;
				}
			}
		}
	}

	// If some free window possible, search which to take
	sint	newIndexWindow= -1;
	bool	mustPlace= true;
	bool	mustAddToActiveWindows= true;
	// if an active window is not in KeepMode, get it.
	for(i=0;i<_ActiveWindows.size();i++)
	{
		// must also test forceKeep for special Action Help which open Brick Help
		if(!_InfoWindows[_ActiveWindows[i]].KeepMode && forceKeepWindow!=(sint)_ActiveWindows[i])
		{
			newIndexWindow= _ActiveWindows[i];
			mustPlace= false;
			mustAddToActiveWindows= false;
			break;
		}
	}

	// If not found get new closed one.
	if(newIndexWindow==-1)
	{
		if(_ActiveWindows.size() < (uint)maxHelpWindow)
		{
			// flag each active window
			static std::vector<bool>	windowActive;
			windowActive.clear();
			windowActive.resize(maxHelpWindow, false);
			for(i=0;i<_ActiveWindows.size();i++)
			{
				windowActive[_ActiveWindows[i]]= true;
			}

			// Search the first window closed, and that is in KeepMode (if possible)
			sint	newIndexWindowKeep= -1;
			for(i=0;i<(uint)maxHelpWindow;i++)
			{
				if(!windowActive[i])
				{
					// Keep the first window not active
					if(newIndexWindow==-1)
						newIndexWindow= i;
					// Keep the first window not active, that is in KeepMode
					if(_InfoWindows[i].KeepMode && newIndexWindowKeep==-1)
						newIndexWindowKeep= i;
				}
			}
			// Fail?
			if( newIndexWindow==-1 )
				return NULL;
			// success to take a "keep" one?
			if(newIndexWindowKeep!=-1)
				newIndexWindow= newIndexWindowKeep;
		}
		else
		{
			// All the info window are opened (and should not be in KeepMode....), take the last recently opened.
			newIndexWindow= _ActiveWindows.front();
			// free space
			_ActiveWindows.pop_front();
		}
	}

	// get the next window
	CInterfaceGroup	*group= _InfoWindows[newIndexWindow].Window;
	nlassert(group);

	CInterfaceElement *ctrl = group->getElement(group->getId()+":content:ctrl_slot");
	if (ctrl) ctrl->setActive(showSlotAndCreator);
	ctrl = group->getElement(group->getId()+":content:creator");
	if (ctrl) ctrl->setActive(showSlotAndCreator);
	ctrl = group->getElement(group->getId()+":content:creator_header");
	if (ctrl) ctrl->setActive(showSlotAndCreator);

	// activate it, set top, copy item watched
	group->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(group);
	_InfoWindows[newIndexWindow].CtrlSheet= elt;
	// insert in list
	if(mustAddToActiveWindows)
		_ActiveWindows.push_back(newIndexWindow);

	// If Item or mission, must add a callBack when itemInfo (or missionInfo) OK.
	if (elt)
	{
		if( elt->getType() == CCtrlSheetInfo::SheetType_Item )
		{
			_InfoWindows[newIndexWindow].ItemSheet= elt->getSheetId();
			_InfoWindows[newIndexWindow].ItemSlotId= getInventory().getItemSlotId(elt);

			// Add the waiter only if really needed (not for raw materials)
			const CItemSheet	*itemSheet= elt->asItemSheet();
			if(itemSheet && itemSheet->Family != ITEMFAMILY::RAW_MATERIAL )
				getInventory().addItemInfoWaiter(&_InfoWindows[newIndexWindow]);
		}
		else if ( elt->getType() == CCtrlSheetInfo::SheetType_Mission )
		{
			_InfoWindows[newIndexWindow].MissionSlotId = (uint16)elt->getIndexInDB();
			CBotChatManager::getInstance()->addMissionInfoWaiter(&_InfoWindows[newIndexWindow]);
		}
	}

	// recompute at next pass
	group->invalidateCoords();

	// Hide some components.
	// Hide the creator name
	CSheetHelpSetup setup;
	setup.setupDefaultIDs();
	setup.SrcSheet = elt;
	setup.HelpWindow = group;
	//setupCreatorName(setup);

	// Hide elements by defaults
	resetSheetHelp(setup);

	return group;
}


// ***************************************************************************
void			CInterfaceHelp::removeWaiterItemInfo(uint i)
{
	if(i<_InfoWindows.size())
		getInventory().removeItemInfoWaiter(&_InfoWindows[i]);
}

// ***************************************************************************
void			CInterfaceHelp::removeWaiterMissionInfo(uint i)
{
	if(i<_InfoWindows.size())
		CBotChatManager::getInstance()->removeMissionInfoWaiter(&_InfoWindows[i]);
}

// ***************************************************************************
void			CInterfaceHelp::changeKeepMode(uint i)
{
	if(i<_InfoWindows.size())
	{
		_InfoWindows[i].KeepMode= !_InfoWindows[i].KeepMode;
		bool	state= _InfoWindows[i].KeepMode;
		if(_InfoWindows[i].KeepButton)
			_InfoWindows[i].KeepButton->setPushed(state);
	}
}

// ***************************************************************************
void			CInterfaceHelp::setKeepMode(uint i, bool state)
{
	if(i<_InfoWindows.size())
	{
		_InfoWindows[i].KeepMode= state;
		if(_InfoWindows[i].KeepButton)
			_InfoWindows[i].KeepButton->setPushed(state);
	}
}


// ***************************************************************************
void			CInterfaceHelp::closeAll()
{
	// update WindowList if possible
	initWindows();
	sint	maxHelpWindow= (sint)_InfoWindows.size();

	_ActiveWindows.clear();
	// For all windows
	for(uint i=0;i<(uint)maxHelpWindow;i++)
	{
		_InfoWindows[i].Window->setActive(false);
	}
}


// ***************************************************************************
void			CInterfaceHelp::resetWindowPos(sint y)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// update WindowList if possible
	initWindows();
	sint	maxHelpWindow= (sint)_InfoWindows.size();

	uint32	w, h;
	CViewRenderer::getInstance()->getScreenSize(w,h);

	// For all windows, reset pos
	for(uint i=0;i<(uint)maxHelpWindow;i++)
	{
		_InfoWindows[i].Window->setX(i*_InfoWindows[i].Window->getW(false));
		_InfoWindows[i].Window->setY((sint)h+y);
		_InfoWindows[i].Window->invalidateCoords();
	}
}

// ***************************************************************************
void			CInterfaceHelp::serialInfoWindows(NLMISC::IStream &f)
{
	f.serialVersion(0);

	vector<CInfoWindowSave>	infoWindowSave;
	if(f.isReading())
	{
		// Setup pos by default (for case where number of windows differ)
		resetWindowPos(-100);

		f.serialCont(infoWindowSave);
		uint	minSize= (uint)min(infoWindowSave.size(), _InfoWindows.size());
		for(uint i=0;i<minSize;i++)
		{
			_InfoWindows[i].Window->setX(infoWindowSave[i].X);
			_InfoWindows[i].Window->setY(infoWindowSave[i].Y);
			_InfoWindows[i].Window->invalidateCoords();
		}
	}
	else
	{
		infoWindowSave.resize(_InfoWindows.size());
		for(uint i=0;i<infoWindowSave.size();i++)
		{
			infoWindowSave[i].X= _InfoWindows[i].Window->getX();
			infoWindowSave[i].Y= _InfoWindows[i].Window->getY();
		}
		f.serialCont(infoWindowSave);
	}
}

// ***************************************************************************
void			CInterfaceHelp::debugOpenedInfoWindows()
{
	for(uint i=0;i<_ActiveWindows.size();i++)
	{
		uint	index= _ActiveWindows[i];
		uint	sheetId= 0;
		if(_InfoWindows[index].CtrlSheet)
			sheetId= _InfoWindows[index].CtrlSheet->getSheetId();
		if(sheetId)
		{
			nlinfo("debugInfoWindow: %s", CSheetId(sheetId).toString().c_str() );
		}
	}
}

// ***************************************************************************
void			CInterfaceHelp::updateWindowSPhraseTexts()
{
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();

	for(uint i=0;i<_ActiveWindows.size();i++)
	{
		uint	index= _ActiveWindows[i];
		CDBCtrlSheet	*ctrl= _InfoWindows[index].CtrlSheet;
		CInterfaceGroup	*group= _InfoWindows[index].Window;
		if(group && ctrl && (ctrl->isSPhraseId() || ctrl->isSPhrase()) )
		{
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = ctrl;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getCtrl("ctrl_slot"));

			if(ctrl->isSPhraseId())
			{
				// reset up the complete window
				setupSabrinaPhraseHelp(setup, pPM->getPhrase(ctrl->getSPhraseId()), 0);
			}
			else if(ctrl->isSPhrase())
			{
				CSPhraseCom			phrase;
				uint32				phraseSheetId= setup.SrcSheet->getSheetId();
				pPM->buildPhraseFromSheet(phrase, phraseSheetId);
				setupSabrinaPhraseHelp(setup, phrase, phraseSheetId);
			}
		}
	}
}

// ***************************************************************************
/** Close all the helps.
 */
class CHandlerCloseHelp : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceHelp::closeAll();
	}
};
REGISTER_ACTION_HANDLER( CHandlerCloseHelp, "close_help");


// ***************************************************************************
/** Build the help window for an item and open it.
 */
class CHandlerOpenItemHelp : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (cs != NULL && cs->getSheetId()!=0 )
		{
			// get the forceKeep param (for Building info)
			sint	forceKeepWindow= -1;
			string	forceKeepWindowStr= getParam(sParams, "force_keep");
			if(!forceKeepWindowStr.empty())
				fromString(forceKeepWindowStr, forceKeepWindow);

			// open the next window
			CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs, forceKeepWindow);
			if (!group) return;
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = cs;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getElement(group->getId()+":content:ctrl_slot"));
			setupItemHelp(setup);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenItemHelp, "open_item_help");


// ***************************************************************************
/** Build the help window for a pact and open it.
 */
class CHandlerOpenPactHelp : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (cs != NULL && cs->getSheetId()!=0 )
		{
			CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs);
			if (!group) return;
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = cs;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getElement(group->getId()+":content:ctrl_slot"));
			setupPactHelp(setup);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenPactHelp, "open_pact_help");


// ***************************************************************************
/** Build the help window for a targeted entity to know what the title means
 */
class CHandlerOpenTitleHelp : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		// display web profile if necessary
		if (getParam(sParams, "from") == "contact")
		{
			if (pCaller == NULL)
				return;

			CInterfaceGroup *fatherGC = pCaller->getParent();
			if (fatherGC == NULL)
				return;
			fatherGC = fatherGC->getParent();
			if (fatherGC == NULL)
				return;
			string str = fatherGC->getId().substr(0,fatherGC->getId().rfind('_'));
			str = str.substr(str.rfind(':')+1, str.size());
			CPeopleList *peopleList = PeopleInterraction.getPeopleListFromContainerID(str);
			if (peopleList == NULL)
				return;

			sint index = peopleList->getIndexFromContainerID(fatherGC->getId());
			if (index == -1)
				return;
			ucstring name = peopleList->getName(index);
			if ( ! name.empty())
			{
				CAHManager::getInstance()->runActionHandler("show_hide", pCaller, "profile|pname="+name.toUtf8()+"|ptype="+toString((int)CEntityCL::Player));
			}
			return;
		}
		else if (getParam(sParams, "from") == "target")
		{
			// Require info on the target
			CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
			if (selection == NULL) return;
			//if(selection->isNPC())
			{
				ucstring name = selection->getEntityName();
				if(name.empty())
				{
					// try to get the name from the string manager (for npc)
					uint32 nDBid = selection->getNameId();
					if (nDBid != 0)
					{
						STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
						pSMC->getString (nDBid, name);
						ucstring copyName = name;
						name = CEntityCL::removeTitleAndShardFromName(name);
						if (name.empty())
						{
							CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(selection);
							bool woman = false;
							if (pChar != NULL)
								woman = pChar->getGender() == GSGENDER::female;

							// extract the replacement id
							ucstring strNewTitle = CEntityCL::getTitleFromName(copyName);

							// retrieve the translated string
							if (!strNewTitle.empty())
								name = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(strNewTitle, woman);
							else
								name.clear();
						}
					}
				}
				if(!name.empty())
					CAHManager::getInstance()->runActionHandler("show_hide", pCaller, "profile|pname="+name.toUtf8()+"|ptype="+toString((int)selection->Type));
				return;
			}
		}

		CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(NULL);
		if (!group) return;

		// prepare the help window
		CSheetHelpSetup setup;
		setup.setupDefaultIDs();
		setup.HelpWindow = group;
		setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getElement(group->getId()+":content:ctrl_slot"));

		// Get name and title
		// ------------------
		ucstring name;
		ucstring title;
		bool reservedTitle = false;
		string sFrom = getParam(sParams, "from");
		if (sFrom == "target")
		{
			// Require info on the target
			CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
			if (selection == NULL) return;
			name = CEntityCL::removeTitleAndShardFromName(selection->getEntityName());
			title = selection->getTitle();
			reservedTitle = selection->hasReservedTitle();
		}
		else if (sFrom == "user")
		{
			// Require info on the local_player
			name = CEntityCL::removeTitleAndShardFromName(UserEntity->getEntityName());
			title = UserEntity->getTitle();
			reservedTitle = UserEntity->hasReservedTitle();
		}
		else return;

		// Get Title info (bricks and skills needed)
		// -----------------------------------------
		CUnblockTitlesSheet *pUTS = dynamic_cast<CUnblockTitlesSheet*>(SheetMngr.get(CSheetId("unblock.titles")));
		if (pUTS == NULL)
		{
			nlwarning("cant find unblock.titles");
			return;
		}

		// get title id number from the title id
		uint titleIDnb;
		for (titleIDnb = 0; titleIDnb < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++titleIDnb)
		{
			bool women = UserEntity && UserEntity->getGender()==GSGENDER::female;
			if (CStringManagerClient::getTitleLocalizedName(CHARACTER_TITLE::toString((CHARACTER_TITLE::ECharacterTitle)titleIDnb),women) == title)
				break;
		}

		// Retrieve all infos about the title
		const CUnblockTitlesSheet::STitleUnblock *pTU = NULL;
		if (titleIDnb != CHARACTER_TITLE::NB_CHARACTER_TITLE)
			pTU = &pUTS->TitlesUnblock[titleIDnb];


		// Display all infos found
		// -----------------------
		ucstring titleText = CI18N::get("uihelpTitleFormat");
		strFindReplace(titleText, "%name", name.toString());

		// Display title
		ucstring::size_type p1 = title.find('(');
		if (p1 != ucstring::npos)
		{
			ucstring::size_type p2 = title.find(')', p1+1);
			if (p2 != ucstring::npos)
				title = title.substr(p1+1, p2-p1-1);
		}
		strFindReplace(titleText, "%title", title);

		// Display all skills needed to obtain this title
		ucstring sSkillsNeeded;
		if (!title.empty() && pTU == NULL)
			sSkillsNeeded = CI18N::get("uiTitleCantObtain");

		if (pTU != NULL)
		{
			sSkillsNeeded = CI18N::get("uiTitleSkillHeader");
			if (pTU->SkillsNeeded.size() == 0 || reservedTitle)
			{
				sSkillsNeeded += CI18N::get("uiTitleSkillNoNeed");
			}
			else
			{
				for (uint i = 0; i < pTU->SkillsNeeded.size(); ++i)
				{
					for (uint j = 0; j < pTU->SkillsNeeded[i].size(); ++j)
					{
						uint skillNb;
						for (skillNb = 0; skillNb < SKILLS::NUM_SKILLS; ++skillNb)
							if (pTU->SkillsNeeded[i][j] == SKILLS::toString((SKILLS::ESkills)skillNb))
								break;

						if (skillNb != SKILLS::NUM_SKILLS)
						{
							sSkillsNeeded += CStringManagerClient::getSkillLocalizedName((SKILLS::ESkills)skillNb);
							sSkillsNeeded += "  (" + toString(pTU->SkillsLevelNeeded[i][j]) + ")";
							sSkillsNeeded += "\n";
						}
						else
						{
							nlwarning("cant find skill : %s", pTU->SkillsNeeded[i][j].c_str());
						}
					}
					if( i != pTU->SkillsNeeded.size()-1 )
					{
						sSkillsNeeded += CI18N::get("uiTitleSkillOr");
						sSkillsNeeded +="\n";
					}
				}
			}
		}
		strFindReplace(titleText, "%skills", sSkillsNeeded);

		// Display all bricks needed to obtain this title
		ucstring sBricksNeeded;
		if (pTU != NULL)
		{
			sBricksNeeded = CI18N::get("uiTitleBrickHeader");
			if (pTU->BricksNeeded.size() == 0 || reservedTitle)
			{
				sBricksNeeded += CI18N::get("uiTitleBrickNoNeed");
			}
			else
			{
				// Get the list of "requirement bricks"
				string listID = group->getId() + setup.PrefixForExtra + INFO_LIST_BRICK_REQUIREMENT;
				IListSheetBase	*listBrick= dynamic_cast<IListSheetBase*>(group->getElement(listID));
				if(listBrick)
				{
					CSPhraseCom phrase;
					phrase.Bricks = pTU->BricksNeeded;
					fillSabrinaPhraseListBrick(phrase, listBrick);
				}
			}
		}
		strFindReplace(titleText, "%bricks", sBricksNeeded);

		// setup the text
		setHelpText(setup, titleText);
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpTitleInfo"));

		// hide the ctrl sheet
		setHelpCtrlSheet(setup, 0);

	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenTitleHelp, "open_title_help");

// ***************************************************************************
/** Build the help for a skill that is to trade
 */
class CHandlerOpenSkillToTradeHelp : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (cs != NULL)
		{
			CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs);
			if (!group) return;
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = cs;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getElement(group->getId()+":content:ctrl_slot"));
			setupSkillToTradeHelp(setup);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenSkillToTradeHelp, "open_skill_to_trade_help");

// ***************************************************************************
/** Build the help window for a pact/item/brick and open it.
 */
class CHandlerOpenHelpAuto : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (!cs)
		{
			nlwarning("<CHandlerOpenHelpAuto::execute> no caller sheet found.");
			return;
		}
		if (cs->getSheetId()!=0 )
		{
			// create group to display help
			CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs);
			if (!group) return;
			// setup the item.
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = cs;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getElement(group->getId()+":content:ctrl_slot"));
			setupSheetHelp(setup);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenHelpAuto, "open_help_auto");

// ***************************************************************************
/** Browse an URL into a CGroupHTML
 */
class CHandlerBrowse : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		string container = getParam (sParams, "name");
		CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId(container);
		CInterfaceGroup *elementGroup = dynamic_cast<CInterfaceGroup*>(element);

		string urls = getParam (sParams, "url");

		bool show = getParam (sParams, "show") != "0";

		bool localizePage= getParam (sParams, "localize")=="1";

		// Action Handler?
		if (strncmp (urls.c_str(), "ah:", 3) == 0)
		{
			// Find next action handler
			string::size_type start = 3;
			string::size_type end = urls.find ("&&", start);
			if (end == string::npos)
				end = urls.size();
			while (start < end)
			{
				// Extract the url
				string url = urls.substr(start, end-start);

				// Run an action handler
				string::size_type index = url.find_first_of("&");
				if (index == string::npos)
					index = url.size();
				string action = url.substr(0, index);
				string params;
				if (index<url.size())
					params = url.substr(index+1, url.size()-index-1);

				// Replace '&'
				if(action=="command")
				{
					// Replace : by ' '
					index = 0;
					while ((index = params.find_first_of("&", index)) != string::npos)
					{
						params[index] = ' ';
					}
				}
				else
				{
					// Replace : by '|'
					index = 0;
					while ((index = params.find_first_of("&", index)) != string::npos)
					{
						params[index] = '|';
					}
				}

				// Replace %HH encoding with ASCII values (AFTER '&' replacing, to possibly have reals '&')
				for(uint i=0;i<params.size();i++)
				{
					if(params[i]=='%' && i<params.size()-2)
					{
						if(isxdigit(params[i+1]) && isxdigit(params[i+2]))
						{
							// read value from heax decimal
							uint8	val= 0;
							params[i+1]= tolower(params[i+1]);
							params[i+2]= tolower(params[i+2]);
							if(isdigit(params[i+1]))	val= params[i+1]-'0';
							else						val= 10+ params[i+1]-'a';
							val*=16;
							if(isdigit(params[i+2]))	val+= params[i+2]-'0';
							else						val+= 10+ params[i+2]-'a';

							// write
							params[i]= val;
							// erase heax value
							params.erase(i+1, 2);
						}
					}
				}

				ucstring ucparams(params);
				CInterfaceManager::parseTokens(ucparams);
				params = ucparams.toUtf8();
				// go. NB: the action handler himself may translate params from utf8
				CAHManager::getInstance()->runActionHandler(action, elementGroup, params);

				// Next name
				start = end+2;
				end = urls.find ("&&", start);
				if (end == string::npos)
					end = urls.size();
			}
		}
		else
		{
			// Get the group HTML
			if (element)
			{
				// Group HTML ?
				CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(element);
				if (groupHtml)
				{
					if (show)
					{
						// Look for a parent container
						CInterfaceGroup *parent = groupHtml->getParent();
						while (parent)
						{
							if (parent->getParent() && (parent->getParent()->getId() == "ui:interface"))
							{
								parent->setActive(true);
								break;
							}

							parent = parent->getParent();
						}
					}

					// localize if wanted
					if(localizePage)
						strFindReplace(urls, "_wk.", string("_")+ClientCfg.getHtmlLanguageCode()+"." );

					// Browse the url
					groupHtml->browse(urls.c_str());
					// Set top of the page
					CCtrlScroll *pScroll = groupHtml->getScrollBar();
					if (pScroll != NULL)
						pScroll->moveTrackY(10000);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerBrowse, "browse");

// ***************************************************************************
/** Browse Undo into a CGroupHTML
 */
class CHandlerBrowseUndo : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		string container = getParam (sParams, "name");
		CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(container));
		if (groupHtml)
		{
			groupHtml->browseUndo();
		}
	};
};
REGISTER_ACTION_HANDLER( CHandlerBrowseUndo, "browse_undo");

// ***************************************************************************
/** Browse Redo into a CGroupHTML
 */
class CHandlerBrowseRedo : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		string container = getParam (sParams, "name");
		CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(container));
		if (groupHtml)
		{
			groupHtml->browseRedo();
		}
	};
};
REGISTER_ACTION_HANDLER( CHandlerBrowseRedo, "browse_redo");

// ***************************************************************************
/** Browse Redo into a CGroupHTML
 */
class CHandlerBrowseRefresh : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		string container = getParam (sParams, "name");
		CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(container));
		if (groupHtml)
		{
			groupHtml->refresh();
		}
	};
};
REGISTER_ACTION_HANDLER( CHandlerBrowseRefresh, "browse_refresh");



// ***************************************************************************
/** Build the help window for a pact/item/brick and open it.
 */
class CHandlerHTMLSubmitForm : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		string container = getParam (sParams, "name");

		uint form;
		fromString(getParam (sParams, "form"), form);

		string submit_button = getParam (sParams, "submit_button");

		CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId(container);
		{
			// Group HTML ?
			CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(element);
			if (groupHtml)
			{
				// Submit the form the url
				groupHtml->submitForm (form, submit_button.c_str ());
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerHTMLSubmitForm, "html_submit_form");

//////////////////////////////////
// STATIC FUNCTIONS DEFINITIONS //
//////////////////////////////////

// ***************************************************************************
static void setupHelpPage(CInterfaceGroup *window, const string &url)
{
	// must be a container
	CGroupContainer	*gc= dynamic_cast<CGroupContainer*>(window);
	if(gc)
		gc->setHelpPage(url);
}

// ***************************************************************************
void setHelpText(CSheetHelpSetup &setup, const ucstring &text)
{
	ucstring	copyStr= text;
	// remove trailing \n
	while(!copyStr.empty() && copyStr[copyStr.size()-1]=='\n')
	{
		copyStr.resize(copyStr.size()-1);
	}

	if (!setup.HelpWindow) return;
	CViewText *viewText= dynamic_cast<CViewText *>(setup.HelpWindow->getView(setup.ViewText));
	if(viewText)
	{
		viewText->setTextFormatTaged(copyStr);
	}
	CInterfaceGroup *viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextGroup);
	if (viewTextGroup) viewTextGroup->setActive(true);
	if (!setup.ScrollTextIdGroup.empty())
	{
		viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextIdGroup);
		if (viewTextGroup) viewTextGroup->setActive(false);
	}
}

// ***************************************************************************
void setHelpCtrlSheet(CSheetHelpSetup &setup, uint32 sheetId)
{
	if(setup.DestSheet)
		setup.DestSheet->setSheetId (sheetId);
}

// ***************************************************************************
void setHelpTextID(CSheetHelpSetup &setup, sint32 id)
{
	if (!setup.HelpWindow) return;
	CViewTextID	*viewTextID = dynamic_cast<CViewTextID *>(setup.HelpWindow->getView(setup.ViewTextID));
	if(viewTextID)
	{
		viewTextID->setTextId(id);
	}
	if (!setup.ScrollTextGroup.empty())
	{
		CInterfaceGroup *viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextGroup);
		if (viewTextGroup) viewTextGroup->setActive(false);
	}
	CInterfaceGroup *viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextIdGroup);
	if (viewTextGroup) viewTextGroup->setActive(true);
}

// ***************************************************************************
static void setupHelpTitle(CInterfaceGroup	*group, const ucstring &title)
{
	CGroupContainer	*pGC= dynamic_cast<CGroupContainer*>(group);
	if(!group)
		return;
	pGC->setUCTitle(title);
}

// ***************************************************************************
static void setupSkillToTradeHelp(CSheetHelpSetup &setup)
{
	if (!setup.HelpWindow) return;

	// get the calling item
	if (!setup.SrcSheet)
	{
		nlwarning("<CHandlerOpenBrickHelp::execute> no caller sheet found.");
		return;
	}

	SKILLS::ESkills skill = setup.SrcSheet->getSkill();
	if (skill >= SKILLS::unknown)
	{
		nlwarning("bad skill");
		return;
	}


	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}

	ucstring	skillText;

	// Name in title
	const ucstring title(CStringManagerClient::getSkillLocalizedName(skill));
	setupHelpTitle(setup.HelpWindow, title);

	// search all job that have minimum required level for that skill
//	CInterfaceManager *im = CInterfaceManager::getInstance();
//	for (uint career = 0; career < JOBS::NUM_CAREER_DB_SLOTS; ++career)
//	{
//		for (uint job = 0; job < 8; ++job)
//		{
//			std::string dbPath = toString("CHARACTER_INFO:CAREER%d:JOB%d:JOB_CAP", (int) career, (int) job);
//			uint level = (uint) NLGUI::CDBManager::getInstance()->getDbProp(dbPath)->getValue32();
//			if (level != 0) // has the player this job ?
//			{
//				// check if level in this job is enough to get the skills
//				// TODO finish it
//				/*
//				JOBS::TJob job = JOBS::getJobFromDBIndex(career, job);
//				if (job != JOBS::Unknown)
//				{
//				}*/
//			}
//		}
//	}

	// setup skill desc if available.
	const ucstring desc(CStringManagerClient::getSkillLocalizedDescription(skill));
	if( !desc.empty() )
	{
		skillText+= "\n" + desc;
	}

	setHelpText(setup, skillText);
}


// ***************************************************************************
static	string	toReadableFloat(float val)
{
	sint	iv= sint(val * 10);
	if((iv%10)==0)
	{
		return toString(iv/10);
	}
	else
	{
		return toString("%.1f", val);
	}
}

// ***************************************************************************
static	string	toPercentageText(float val)
{
	sint	iv= sint(val * 1000);
	if((iv%10)==0)
	{
		return toString(iv/10);
	}
	else
	{
		val= float(iv)/10;
		return toString("%.1f", val);
	}
}

// ***************************************************************************
void	getItemDefenseText(CDBCtrlSheet *item, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	// Parry/Dodge
	strFindReplace(itemText, "%dodge", toString(itemInfo.DodgeModifier) );
	strFindReplace(itemText, "%parry", toString(itemInfo.ParryModifier) );

	// Display All protections
	strFindReplace( itemText, "%protect", toPercentageText(itemInfo.ProtectionFactor) );
	strFindReplace( itemText, "%p_slash", toString(itemInfo.MaxSlashingProtection) );
	strFindReplace( itemText, "%p_pierce", toString(itemInfo.MaxPiercingProtection) );
	strFindReplace( itemText, "%p_blunt", toString(itemInfo.MaxBluntProtection) );
}


void	getDamageText(CDBCtrlSheet *item, const CItemSheet*pIS, ucstring &itemText, bool displayAsMod)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	string	strMod;
	if(displayAsMod)
		strMod= "+";
	strFindReplace(itemText, "%dmg", strMod + toString(itemInfo.CurrentDamage));
	if(displayAsMod)
		strMod= "+";
	strFindReplace(itemText, "%max_dmg", strMod + toString(itemInfo.MaxDamage));

	if(pIS->Family!=ITEMFAMILY::AMMO)
	{
		// Display the Dodge/Parry Modifier
		strFindReplace(itemText, "%dodge", toString(itemInfo.DodgeModifier) );
		strFindReplace(itemText, "%parry", toString(itemInfo.ParryModifier) );

		// Display the Adversary Dodge/Parry Modifier
		strFindReplace(itemText, "%adv_dodge", toString(itemInfo.AdversaryDodgeModifier) );
		strFindReplace(itemText, "%adv_parry", toString(itemInfo.AdversaryParryModifier) );
	}
}

void	getSpeedText(CDBCtrlSheet *item, ucstring &itemText, bool displayAsMod)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	string	strMod;
	if(displayAsMod)
		strMod= itemInfo.HitRate>=0?"+":"";
	strFindReplace(itemText, "%speed", strMod + toReadableFloat(itemInfo.HitRate));
}

void	getRangeText(CDBCtrlSheet *item, ucstring &itemText, bool displayAsMod)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	string	strMod;
	if(displayAsMod)
		strMod= itemInfo.Range>=0?"+":"";
	strFindReplace(itemText, "%range", strMod + toReadableFloat(itemInfo.Range/1000.f));
}

void	getHPAndSapLoadText(CDBCtrlSheet *item, const CItemSheet*pIS, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	if(pIS->Family != ITEMFAMILY::RAW_MATERIAL)
	{
		// must find first hpmax! (else replace error)
		strFindReplace(itemText, "%hpmax", toString(itemInfo.HpMax) );
		strFindReplace(itemText, "%hp", toString(itemInfo.Hp) );
		// SapLoad
		strFindReplace(itemText, "%sapmax", toString(itemInfo.SapLoadMax) );
		strFindReplace(itemText, "%sap", toString(itemInfo.SapLoadCurrent) );
	}
}


void	getBuffText(CDBCtrlSheet *item, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	const string valIds[]={"Hp", "Sap", "Sta", "Focus"};
	sint32		 vals[]= {itemInfo.HpBuff, itemInfo.SapBuff, itemInfo.StaBuff, itemInfo.FocusBuff};
	uint		numVals= sizeof(vals) / sizeof(vals[0]);
	ucstring	bufInfo;

	// For each buf, append a line if !=0
	for(uint i=0;i<numVals;i++)
	{
		sint32	modifier= vals[i];
		if(modifier!=0)
		{
			ucstring	line= CI18N::get( "uihelpItem" + valIds[i] + (modifier>0?"Bonus":"Malus") );
			strFindReplace(line, "%val", toString(modifier) );
			bufInfo+= line;
		}
	}

	// append a \n before
	if(!bufInfo.empty())
		bufInfo = "\n" + bufInfo + "\n";

	// Display the buff info.
	strFindReplace(itemText, "%buffs", bufInfo);
}

void	getMagicProtection(CDBCtrlSheet	*item, ucstring &itemText)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	ucstring	mProtInfo;

	// Header (always here, because at least max absorb)
	mProtInfo= CI18N::get("uihelpMagicProtectFormatHeader");

	// For each protection
	for(uint i=0;i<CClientItemInfo::MaxMagicProtectionByJewel;i++)
	{
		if(itemInfo.MagicProtection[i] != PROTECTION_TYPE::None)
		{
			// Protection info
			ucstring	str= CI18N::get("uihelpMagicProtectFormat");
			strFindReplace(str, "%t", CI18N::get("pt"+PROTECTION_TYPE::toString(itemInfo.MagicProtection[i])) );
			strFindReplace(str, "%v", toString(itemInfo.MagicProtectionFactor[i]) );
			mProtInfo+= str;
		}
	}

	// add Max damage absorbed
	{
		// Mul item quality by a constant
		uint	maxAbsorb= item->getQuality();
		CCDBNodeLeaf	*nodeFactor= NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("player_protect_absorbfactor"), false);
		if(nodeFactor)
			maxAbsorb= maxAbsorb*nodeFactor->getValue32()/100;

		// Add to text
		ucstring	str= CI18N::get("uihelpMagicProtectMaxAbsorbFormat");
		strFindReplace(str, "%v", toString(maxAbsorb) );
		mProtInfo+= str;
	}

	// replace in item info
	strFindReplace(itemText, "%magic_protection", mProtInfo);
}

void	getMagicResistance(CDBCtrlSheet	*item, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	ucstring	mResistInfo;

	// Header (always here, because at least max absorb)
	mResistInfo= CI18N::get("uihelpMagicResistFormatHeader");

	// For each resistance
	uint32	resist[RESISTANCE_TYPE::NB_RESISTANCE_TYPE];
	nlctassert(RESISTANCE_TYPE::NB_RESISTANCE_TYPE==5);
	resist[RESISTANCE_TYPE::Desert]= itemInfo.DesertMagicResistance;
	resist[RESISTANCE_TYPE::Forest]= itemInfo.ForestMagicResistance;
	resist[RESISTANCE_TYPE::Lacustre]= itemInfo.LacustreMagicResistance;
	resist[RESISTANCE_TYPE::Jungle]= itemInfo.JungleMagicResistance;
	resist[RESISTANCE_TYPE::PrimaryRoot]= itemInfo.PrimaryRootMagicResistance;
	for(uint i=0;i<RESISTANCE_TYPE::NB_RESISTANCE_TYPE;i++)
	{
		if(resist[i] != 0)
		{
			// Resist info
			ucstring	str= CI18N::get("uihelpMagicResistFormat");
			strFindReplace(str, "%t", CI18N::get("rs"+RESISTANCE_TYPE::toString((RESISTANCE_TYPE::TResistanceType)i) ));
			strFindReplace(str, "%v", toReadableFloat(float(resist[i])/100) );
			mResistInfo+= str;
		}
	}

	// replace in item info
	strFindReplace(itemText, "%magic_resistance", mResistInfo);
}

void	getActionMalus(CDBCtrlSheet *item, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	strFindReplace(itemText, "%actmalus", toPercentageText(itemInfo.WearEquipmentMalus) );
}

void	getBulkText(CDBCtrlSheet *item, const CItemSheet*pIS, ucstring &itemText)
{
	// Display direct value: because cannot know where this item will be drop!! (bag, mektoub etc...)
	float	slotBulkTotal= max((sint32)1, item->getQuantity()) * pIS->Bulk;

	// If stackable and bulk not 0, display in form "1 (10)". where (10) is the total of quantity*bulk
	if(pIS->Stackable>1 && pIS->Bulk>0)
		strFindReplace(itemText, "%bulk", toString("%.2f  (%.2f)", pIS->Bulk, slotBulkTotal) );
	// else simple form
	else
		strFindReplace(itemText, "%bulk", toString("%.2f", slotBulkTotal) );
}

void	getWeightText(CDBCtrlSheet *item, const CItemSheet*pIS, ucstring &itemText)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	CCDBNodeLeaf *pWG = NLGUI::CDBManager::getInstance()->getDbProp(item->getSheet()+":WEIGHT",false);
	if(pWG)
	{
		// must mul weight by quantity
		sint32	slotWeight= pWG->getValue32();
		sint32	slotWeightTotal= max((sint32)1, item->getQuantity()) * slotWeight;

		// if stackable and weight not 0, display in form "1 (10)". where (10) is the total of quantity*weight
		if(pIS->Stackable>1 && slotWeight>0)
		{
			string	sws;
			string	swt;
			if( (slotWeight%DB_WEIGHT_SCALE) == 0)	sws= toString("%d", slotWeight/DB_WEIGHT_SCALE );
			else									sws= toString("%.2f", float(slotWeight)/DB_WEIGHT_SCALE);
			if( (slotWeightTotal%DB_WEIGHT_SCALE) == 0)	swt= toString("%d", slotWeightTotal/DB_WEIGHT_SCALE );
			else										swt= toString("%.2f", float(slotWeightTotal)/DB_WEIGHT_SCALE);

			// combine
			strFindReplace(itemText, "%weight", toString("%s  (%s)", sws.c_str(), swt.c_str() ));
		}
		// else display in simple form
		else
		{
			if( (slotWeightTotal%DB_WEIGHT_SCALE) == 0)
				strFindReplace(itemText, "%weight", toString("%d", slotWeightTotal/DB_WEIGHT_SCALE ));
			else
				strFindReplace(itemText, "%weight", toString("%.2f", float(slotWeightTotal)/DB_WEIGHT_SCALE) );
		}
	}
	else
		strFindReplace(itemText, "%weight", "???" );
}

void	getMagicBonus(CDBCtrlSheet *item, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	nlctassert(CClientItemInfo::NumMagicFactorType==4);
	const string valIds[CClientItemInfo::NumMagicFactorType]={"OffElemental", "OffAffliction", "DefHeal", "DefAffliction"};
	ucstring	mbInfo;

	// For each magic bonus, test first if equal
	sint32	allCastSpeedFactor= sint(itemInfo.CastingSpeedFactor[0]*100);
	sint32	allMagicPowerFactor= sint(itemInfo.MagicPowerFactor[0]*100);
	bool	equal= true;
	for(uint i=1;i<CClientItemInfo::NumMagicFactorType;i++)
	{
		sint32		cs= sint(itemInfo.CastingSpeedFactor[i]*100);
		sint32		mp= sint(itemInfo.MagicPowerFactor[i]*100);
		if( cs!=allCastSpeedFactor ||
			mp!=allMagicPowerFactor )
		{
			equal= false;
			break;
		}
	}

	// if all are equal
	if(equal)
	{
		// if 0, just display nothing
		if(allCastSpeedFactor!=0 || allMagicPowerFactor!=0)
		{
			// else display "all"
			ucstring	line= CI18N::get( "uihelpItemMagicBonusAll");
			strFindReplace(line, "%cs", toString("%+d", allCastSpeedFactor) );
			strFindReplace(line, "%mp", toString("%+d", allMagicPowerFactor) );
			mbInfo+= line;
		}
	}
	else
	{
		// then display info separated for each
		for(uint i=0;i<CClientItemInfo::NumMagicFactorType;i++)
		{
			sint32		cs= sint(itemInfo.CastingSpeedFactor[i]*100);
			sint32		mp= sint(itemInfo.MagicPowerFactor[i]*100);
			if(cs!=0 || mp!=0)
			{
				ucstring	line= CI18N::get( string("uihelpItemMagicBonus") + valIds[i] );
				strFindReplace(line, "%cs", toString("%+d", cs) );
				strFindReplace(line, "%mp", toString("%+d", mp) );
				mbInfo+= line;
			}
		}
	}

	// append a \n before
	if(mbInfo.size())
	{
		// add spell level header
		ucstring	spellRuleFmt= CI18N::get("uihelpItemMagicBonusHeader");
		strFindReplace(spellRuleFmt, "%mglvl", toString(item->getQuality()));
		mbInfo= spellRuleFmt + mbInfo;
	}

	// Display the buff info.
	strFindReplace(itemText, "%magic_bonus", mbInfo);
}

void	getItemRequirementText(CDBCtrlSheet *item, const CItemSheet*pIS, ucstring &itemText)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	bool requiredNeeded= false;
	ucstring	fmt, fmtc;

	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	if( itemInfo.RequiredCharac != CHARACTERISTICS::Unknown && itemInfo.RequiredCharacLevel != 0 )
	{
		// Build the req string
		if(pIM->isItemCaracRequirementMet(itemInfo.RequiredCharac, (sint32)itemInfo.RequiredCharacLevel))
			fmtc = CI18N::get("uihelpItemCaracReqMetFmt");
		else
			fmtc = CI18N::get("uihelpItemCaracReqNotMetFmt");
		strFindReplace(fmtc, "%d", toString((uint)itemInfo.RequiredCharacLevel));
		strFindReplace(fmtc, "%s", CI18N::get(toString("uiCaracId%d", (uint)itemInfo.RequiredCharac)) );

		//strFindReplace(itemText, "%caracreq", fmtc );
		requiredNeeded = true;
	}
	else
	{
		//strFindReplace(itemText, "%caracreq", "" );
	}

	if( itemInfo.RequiredSkillLevel > 0 )
	{
		if( itemInfo.RequiredSkill != SKILLS::unknown )
		{
			if (CSkillManager::getInstance()->checkBaseSkillMetRequirement(itemInfo.RequiredSkill, itemInfo.RequiredSkillLevel))
				fmt = CI18N::get("uihelpItemSkillReqMetFmt");
			else
				fmt = CI18N::get("uihelpItemSkillReqNotMetFmt");

			strFindReplace(fmt, "%d", toString((uint)itemInfo.RequiredSkillLevel));
			const ucstring skillName(STRING_MANAGER::CStringManagerClient::getSkillLocalizedName(itemInfo.RequiredSkill));
			strFindReplace(fmt, "%s", skillName);
		}
		else
		{
			if (CSkillManager::getInstance()->checkBaseSkillMetRequirement(SKILLS::unknown, itemInfo.RequiredSkillLevel))
				fmt = CI18N::get("uihelpItemAnySkillReqMetFmt");
			else
				fmt = CI18N::get("uihelpItemAnySkillReqNotMetFmt");

			strFindReplace(fmt, "%d", toString((uint)itemInfo.RequiredSkillLevel));
		}

		strFindReplace(itemText, "%skillreq", fmt );
		requiredNeeded = true;
	}
	else
	{
		strFindReplace(itemText, "%skillreq", "" );
	}

	if( itemInfo.RequiredSkillLevel2 > 0 )
	{
		if( itemInfo.RequiredSkill2 != SKILLS::unknown )
		{
			if (CSkillManager::getInstance()->checkBaseSkillMetRequirement(itemInfo.RequiredSkill2, itemInfo.RequiredSkillLevel2))
				fmt = CI18N::get("uihelpItemSkillReqMetFmt");
			else
				fmt = CI18N::get("uihelpItemSkillReqNotMetFmt");

			strFindReplace(fmt, "%d", toString((uint)itemInfo.RequiredSkillLevel2));
			const ucstring skillName(STRING_MANAGER::CStringManagerClient::getSkillLocalizedName(itemInfo.RequiredSkill2));
			strFindReplace(fmt, "%s", skillName);
		}
		else
		{
			if (CSkillManager::getInstance()->checkBaseSkillMetRequirement(SKILLS::unknown, itemInfo.RequiredSkillLevel2))
				fmt = CI18N::get("uihelpItemAnySkillReqMetFmt");
			else
				fmt = CI18N::get("uihelpItemAnySkillReqNotMetFmt");

			strFindReplace(fmt, "%d", toString((uint)itemInfo.RequiredSkillLevel2));
		}

		strFindReplace(itemText, "%skillreq2", fmt );
		requiredNeeded = true;
	}
	else
	{
		strFindReplace(itemText, "%skillreq2", "" );
	}

	if( requiredNeeded )
		strFindReplace(itemText, "%caracreq", fmtc );
	else
		strFindReplace(itemText, "%caracreq", CI18N::get("uihelpItemCaracReqNone") );

#if 0
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	CHARACTERISTICS::TCharacteristics	caracType;
	float								caracValue= 0.f;

	bool	req = pIS->hasCharacRequirement(item->getQuality(), caracType, caracValue);

	bool	req2 = (itemInfo.RequiredCharac != CHARACTERISTICS::Unknown && itemInfo.MinRequiredCharacLevel > 0);
	bool	skillReq = (itemInfo.MinRequiredSkillLevel > 0 && itemInfo.RequiredSkill != SKILLS::unknown);

	// check item specific req
	if (req2)
	{
		if (req)
		{
			if (itemInfo.RequiredCharac == caracType && itemInfo.MinRequiredCharacLevel > caracValue)
			{
				caracValue = itemInfo.MinRequiredCharacLevel;
				req2 = false;
			}
		}
		else
		{
			caracValue = itemInfo.MinRequiredCharacLevel;
			caracType = itemInfo.RequiredCharac;
			req2 = false;
			req = true;
		}
	}

	if(req)
	{
		// Build the req string
		ucstring	fmt;
		if(pIM->isItemCaracRequirementMet(caracType, (sint32)caracValue))
			fmt= CI18N::get("uihelpItemCaracReqMetFmt");
		else
			fmt= CI18N::get("uihelpItemCaracReqNotMetFmt");
		strFindReplace(fmt, "%d", toString((uint)caracValue));
		strFindReplace(fmt, "%s", CI18N::get(toString("uiCaracId%d", (uint)caracType)) );

		if (req2)
		{
			fmt += CI18N::get("uihelpItemCaracReqAnd");
			if ( pIM->isItemCaracRequirementMet(itemInfo.RequiredCharac, (sint32)itemInfo.MinRequiredCharacLevel) )
				fmt += CI18N::get("uihelpItemCaracReqMetFmt");
			else
				fmt += CI18N::get("uihelpItemCaracReqNotMetFmt");
			strFindReplace(fmt, "%d", toString((uint)itemInfo.MinRequiredCharacLevel));
			strFindReplace(fmt, "%s", CI18N::get(toString("uiCaracId%d", (uint)itemInfo.RequiredCharac)) );
		}

		strFindReplace(itemText, "%caracreq", fmt );
	}
	else if (!skillReq)
	{
		// empty
		strFindReplace(itemText, "%caracreq", CI18N::get("uihelpItemCaracReqNone") );
	}
	else // skillReq and no charac req
	{
		// empty
		strFindReplace(itemText, "%caracreq", "" );
	}

	if (skillReq)
	{
		// Build the req string
		ucstring	fmt;
		if (req)
			fmt = CI18N::get("uihelpItemCaracReqAnd");

		if (CSkillManager::getInstance()->checkBaseSkillMetRequirement(itemInfo.RequiredSkill, itemInfo.MinRequiredSkillLevel))
			fmt += CI18N::get("uihelpItemSkillReqMetFmt");
		else
			fmt += CI18N::get("uihelpItemSkillReqNotMetFmt");
		strFindReplace(fmt, "%d", toString((uint)itemInfo.MinRequiredSkillLevel));
		const ucstring skillName = STRING_MANAGER::CStringManagerClient::getSkillLocalizedName(itemInfo.RequiredSkill);
		strFindReplace(fmt, "%s", skillName);

		strFindReplace(itemText, "%skillreq", fmt );
	}
	else
	{
		strFindReplace(itemText, "%skillreq", "" );
	}
#endif
}

void	getSkillModVsType(CDBCtrlSheet	*item, const CItemSheet*pIS, ucstring &itemText)
{
	// retrieve the current itemInfo
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(item) );

	ucstring	sMod;
	// check skill mod
	if(!itemInfo.TypeSkillMods.empty())
	{
		for (uint i = 0 ; i < itemInfo.TypeSkillMods.size() ; ++i)
		{
			EGSPD::CClassificationType::TClassificationType type = itemInfo.TypeSkillMods[i].Type;

			sMod= CI18N::get("uihelpSkillModVsType");
			strFindReplace(sMod, "%mod", NLMISC::toString(itemInfo.TypeSkillMods[i].Modifier));
			strFindReplace(sMod, "%type", CStringManagerClient::getClassificationTypeLocalizedName(type));
			sMod += "\n\n";

			// TODO : process ALL mod
			break;
		}
	}

	strFindReplace(itemText, "%skill_mod_vs_type", sMod);
}

void getArmorBonus(CDBCtrlSheet *item, ucstring &itemText, const CItemSheet*pIS)
{
	ucstring armor_bonus("");
	sint32 level = 0;

	if (pIS->Armor.ArmorType == ARMORTYPE::HEAVY)
		level = item->getQuality();
	else if (pIS->Armor.ArmorType == ARMORTYPE::MEDIUM)
		level = item->getQuality() / 2;

	if (pIS->Armor.ArmorType == ARMORTYPE::HEAVY || pIS->Armor.ArmorType == ARMORTYPE::MEDIUM)
		armor_bonus = "@{FFFF}(+@{2F2F}" + toString(level) + " @{FFFF}" + CI18N::get("uiHP") + ")";

	strFindReplace(itemText, "%armor_bonus",  armor_bonus);
}

// ***************************************************************************
void getItemText (CDBCtrlSheet *item, ucstring &itemText, const CItemSheet*pIS)
{
	if ((item == NULL) || (pIS == NULL))
		return;

	// *** Select the correct format according to item family.
	switch(pIS->Family)
	{
	case ITEMFAMILY::ARMOR : itemText= CI18N::get("uihelpItemArmorFormat"); break;
	case ITEMFAMILY::MELEE_WEAPON : itemText= CI18N::get("uihelpItemMeleeWeaponFormat"); break;
	case ITEMFAMILY::RANGE_WEAPON : itemText= CI18N::get("uihelpItemRangeWeaponFormat"); break;
	case ITEMFAMILY::AMMO : itemText= CI18N::get("uihelpItemAmmoFormat"); break;
	case ITEMFAMILY::RAW_MATERIAL : itemText= CI18N::get("uihelpItemMPFormat"); break;
	case ITEMFAMILY::SHIELD : itemText= CI18N::get("uihelpItemShieldFormat"); break;
	case ITEMFAMILY::CRAFTING_TOOL : itemText= CI18N::get("uihelpItemCraftingToolFormat"); break;
	case ITEMFAMILY::HARVEST_TOOL : itemText= CI18N::get("uihelpItemHarvestToolFormat"); break;
	case ITEMFAMILY::TAMING_TOOL : itemText= CI18N::get("uihelpItemTamingToolFormat"); break;
	case ITEMFAMILY::JEWELRY : itemText= CI18N::get("uihelpItemJewelFormat"); break;
	case ITEMFAMILY::CRYSTALLIZED_SPELL	 : itemText= CI18N::get("uihelpItemCrystalSpell"); break;
	case ITEMFAMILY::ITEM_SAP_RECHARGE : itemText= CI18N::get("uihelpItemChargeSpell"); break;
	case ITEMFAMILY::PET_ANIMAL_TICKET : itemText= CI18N::get("uihelpItemAnimal"); break;
	case ITEMFAMILY::TELEPORT : itemText= CI18N::get("uihelpItemTeleport"); break;
	case ITEMFAMILY::COSMETIC : itemText= CI18N::get("uihelpItemCosmetic"); break;
	case ITEMFAMILY::SCROLL : itemText= CI18N::get("uihelpItemScroll"); break;
	case ITEMFAMILY::SCROLL_R2 : itemText = CI18N::get("uihelpItemScrollR2"); break;
	case ITEMFAMILY::CONSUMABLE : itemText= CI18N::get("uihelpItemConsumableFormat"); break;
	default: itemText= CI18N::get("uihelpItemDefaultFormat");
	};


	// *** Replace Common part
	strFindReplace(itemText, "%origin", CI18N::get("io"+ITEM_ORIGIN::enumToString(pIS->ItemOrigin)) );
	strFindReplace(itemText, "%quality", toString(item->getQuality()) );
	strFindReplace(itemText, "%quantity", toString(item->getQuantity()) );
	// display Weight
	getWeightText(item, pIS, itemText);
	// display Bulk
	getBulkText(item, pIS, itemText);
	// Get the SapLoad...
	getHPAndSapLoadText(item, pIS, itemText);
	// Get carac and skill Requirement
	getItemRequirementText(item, pIS, itemText);
	// Get Item effect
	CItemSpecialEffectHelper::getInstance()->getItemSpecialEffectText(pIS, itemText);

	// Description
	const ucstring desc(CStringManagerClient::getItemLocalizedDescription(pIS->Id));
	if(!desc.empty())
	{
		strFindReplace(itemText, "%desc", "@{FFF9}" + CI18N::get("uiMissionDesc") + "\n@{FFFF}" + desc + "\n" );
	}
	else
		strFindReplace(itemText, "%desc", ucstring() );

	// Custom text
	const	CClientItemInfo	&itemInfo = getInventory().getItemInfo(getInventory().getItemSlotId(item) );
	if (!itemInfo.CustomText.empty())
	{
		strFindReplace(itemText, "%custom_text", "\n@{FFFF}" + itemInfo.CustomText + "\n");
		ucstring itemMFC = CI18N::get("uiItemTextMessageFromCrafter");
		strFindReplace(itemText, "%mfc", itemMFC);
	}
	else
		strFindReplace(itemText, "%custom_text", ucstring() );

	if ( pIS->Family == ITEMFAMILY::COSMETIC )
	{
		EGSPD::CPeople::TPeople people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( ITEM_ORIGIN::enumToString( pIS->ItemOrigin ) );
		if ( UserEntity->getGender() != pIS->Cosmetic.Gender || UserEntity->people() != people )
			strFindReplace(itemText, "%cansell", CI18N::get("uihelpItemCosmeticDontFit") );
		else
			strFindReplace(itemText, "%cansell", ucstring() );
	}
	else if(pIS->DropOrSell )
		strFindReplace(itemText, "%cansell", ucstring() );
	else
		strFindReplace(itemText, "%cansell", CI18N::get("uihelpItemCantSell") );

	// *** Replace special for each type
	switch(pIS->Family)
	{
	case ITEMFAMILY::ARMOR :
		{
			strFindReplace(itemText, "%armor", CI18N::get("at"+ARMORTYPE::toString(pIS->Armor.ArmorType)));
			// Armor bonus based on armor type
			getArmorBonus(item, itemText, pIS);
			// Protection
			getItemDefenseText(item, itemText);
			// Player buffs
			getBuffText(item, itemText);
			// action malus
			getActionMalus(item, itemText);
		}
		break;
	case ITEMFAMILY::MELEE_WEAPON :
		{
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->MeleeWeapon.Skill) );
			strFindReplace(itemText, "%cat", CI18N::get("wt"+WEAPONTYPE::toString(pIS->MeleeWeapon.WeaponType)) );
			strFindReplace(itemText, "%dmtype", CI18N::get("dt"+DMGTYPE::toString(pIS->MeleeWeapon.DamageType)) );
			strFindReplace(itemText, "%reach", toString(pIS->MeleeWeapon.MeleeRange) );
			// Damage / Speed
			getDamageText(item, pIS, itemText, false);
			getSpeedText(item, itemText, false);
			// Player buffs
			getBuffText(item, itemText);
			// action malus
			getActionMalus(item, itemText);
			// magical bonus
			getMagicBonus(item, itemText);
			// skill bonus against specific types
			getSkillModVsType(item, pIS, itemText);
		}
		break;
	case ITEMFAMILY::RANGE_WEAPON :
		{
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->RangeWeapon.Skill) );
			strFindReplace(itemText, "%cat", CI18N::get("wt"+WEAPONTYPE::toString(pIS->RangeWeapon.WeaponType)) );
			// Damage / Speed / Range
			getDamageText(item, pIS, itemText, true);
			getSpeedText(item, itemText, false);
			getRangeText(item, itemText, false);
			// Player buffs
			getBuffText(item, itemText);
			// action malus
			getActionMalus(item, itemText);
			// magical bonus
			getMagicBonus(item, itemText);
			// skill bonus against specific types
			getSkillModVsType(item, pIS, itemText);
		}
		break;
	case ITEMFAMILY::AMMO :
		{
			// Localization
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->Ammo.Skill) ) ;
			strFindReplace(itemText, "%dmtype", CI18N::get("dt"+DMGTYPE::toString(pIS->Ammo.DamageType)) );
			// WARNING: here 999 is hardcoded because of the new global limit of stacks (999), Ammo.Magazine should not be used anymore
			if (item->getUseQuantity())
				strFindReplace(itemText, "%magazine", toString(item->getQuantity()) + " / " + toString(999 /*pIS->Ammo.Magazine*/) );
			else
				strFindReplace(itemText, "%magazine", toString(999 /*pIS->Ammo.Magazine*/) );

			// Damage / Speed / Range
			getDamageText(item, pIS, itemText, false);
			getSpeedText(item, itemText, true);
			getRangeText(item, itemText, true);
		}
		break;
	case ITEMFAMILY::RAW_MATERIAL :
		{
			// Basics
			strFindReplace(itemText, "%ecosystem", CI18N::get("ecosys"+ECOSYSTEM::toString(pIS->Mp.Ecosystem)) ) ;
			strFindReplace(itemText, "%family", RM_FAMILY::toLocalString(pIS->Mp.Family) ) ;
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->Mp.HarvestSkill) ) ;

			// MpColor
			if( RM_COLOR::validColor(pIS->Mp.MpColor) )
				strFindReplace(itemText, "%mpcolor", RM_COLOR::toLocalString(pIS->Mp.MpColor) ) ;
			else
				strFindReplace(itemText, "%mpcolor", "???" );

			// Craft some part?
			if(pIS->canBuildSomeItemPart())
			{
				ucstring	fmt= CI18N::get("uihelpItemMPCraft");
				ucstring	ipList;
				pIS->getItemPartListAsText(ipList);
				strFindReplace(fmt, "%ip", ipList);
				strFindReplace(itemText, "%craft", fmt);
			}
			// Craft Mp requirement?
			else if(pIS->isUsedAsCraftRequirement())
				strFindReplace(itemText, "%craft", CI18N::get("uihelpItemMPCraftRequirement"));
			// No Craft at all
			else
				strFindReplace(itemText, "%craft", CI18N::get("uihelpItemMPNoCraft"));
		}
		break;
	case ITEMFAMILY::SHIELD :
		{
			//strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(SHIELDTYPE::shieldTypeToSkill(pIS->Shield.ShieldType)) );
			strFindReplace(itemText, "%cat", CI18N::get("st"+SHIELDTYPE::toString(pIS->Shield.ShieldType)) );
			// Protection
			getItemDefenseText(item, itemText);
			// Player buffs
			getBuffText(item, itemText);
			// action malus
			getActionMalus(item, itemText);
		}
		break;
	// Crafting Tool: the skill is not valid, since depends on what is built.
	case ITEMFAMILY::CRAFTING_TOOL :
		{
			strFindReplace(itemText, "%tool", CI18N::get("tool"+TOOL_TYPE::toString(pIS->Tool.CraftingToolType)) );
			// Player buffs
			getBuffText(item, itemText);
		}
		break;
	case ITEMFAMILY::HARVEST_TOOL :
		{
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->Tool.Skill) );
			// Player buffs
			getBuffText(item, itemText);
		}
		break;
	case ITEMFAMILY::TAMING_TOOL :
		{
			strFindReplace(itemText, "%skill", CStringManagerClient::getSkillLocalizedName(pIS->Tool.Skill) );
			strFindReplace(itemText, "%cmdrange", toString(pIS->Tool.CommandRange) );
			strFindReplace(itemText, "%maxpacker", toString(pIS->Tool.MaxDonkey) );
		}
		break;
	case ITEMFAMILY::JEWELRY :
		{
			// Player buffs
			getBuffText(item, itemText);
			// Magic protection
			getMagicProtection(item, itemText);
			// Magic Resistances
			getMagicResistance(item, itemText);
		}
		break;
	case ITEMFAMILY::CONSUMABLE :
		{
			strFindReplace(itemText, "%consumption_time", toString(pIS->Consumable.ConsumptionTime));
			strFindReplace(itemText, "%overdose_timer_min", toString(pIS->Consumable.OverdoseTimer/60));
			strFindReplace(itemText, "%overdose_timer_sec", toString(pIS->Consumable.OverdoseTimer % 60));
			// Get Item Consumable infos
			CItemConsumableEffectHelper::getInstance()->getItemConsumableEffectText(pIS, itemText, item->getQuality());
		}
		break;
	case ITEMFAMILY::SCROLL_R2:
		{
			strFindReplace(itemText, "%r2_description_text", toString(itemInfo.R2ItemDescription));
			strFindReplace(itemText, "%r2_comment_text", toString(itemInfo.R2ItemComment));
		}
		break;
	case ITEMFAMILY::PET_ANIMAL_TICKET:
		{
			string nr = (itemInfo.PetNumber > 0) ? toString(itemInfo.PetNumber) : "(slot)" + toString(item->getIndexInDB());
			strFindReplace(itemText, "%petnumber", nr);
		}
		break;
	default:
		{
			strFindReplace(itemText, "%no_rent", pIS->IsItemNoRent ? CI18N::get("uihelpItemNoRent") : string(""));
			strFindReplace(itemText, "%descnr", pIS->IsItemNoRent ? CI18N::get("uihelpItemNoRentDesc") : string(""));
		}
		break;
	};

#ifdef NL_DEBUG
	INVENTORIES::TInventory inventory = (INVENTORIES::TInventory)item->getInventoryIndex();
	sint32 slot = item->getIndexInDB();
	string debugText = NLMISC::toString("inventory: %s\nslot: %d\n", INVENTORIES::toString(inventory).c_str(), slot);
	ucstring debugText2;
	debugText2.fromUtf8(debugText);
	itemText = debugText2 + itemText;
#endif
}


// ***************************************************************************
static void	setupEnchantedItem(CSheetHelpSetup &setup, ucstring &itemText)
{
	// if don't find the tag in the text (eg: if not useful), no-op
	static const	ucstring	enchantTag("%enchantment");
	if( itemText.find(enchantTag) == ucstring::npos )
		return;

	// retrieve the current itemInfo
	CDBCtrlSheet	*ctrl= setup.SrcSheet;
	CInterfaceGroup	*group= setup.HelpWindow;
	if(!ctrl || !group)
		return;
	const	CClientItemInfo	&itemInfo= getInventory().getItemInfo(getInventory().getItemSlotId(ctrl) );
	IListSheetBase	*listBrick= dynamic_cast<IListSheetBase*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_LIST_BRICK));

	// if the item is enchanted
	if( !itemInfo.Enchantment.empty())
	{
		CSPhraseManager	*pPM= CSPhraseManager::getInstance();

		// fill the enchantement info
		ucstring	enchantInfo;
		const CItemSheet	*pIS= ctrl->asItemSheet();
		if(pIS && pIS->Family == ITEMFAMILY::CRYSTALLIZED_SPELL)
			pPM->buildPhraseDesc(enchantInfo, itemInfo.Enchantment, 0, false, "uihelpPhraseCrystalSpellFormat");
		else
			pPM->buildPhraseDesc(enchantInfo, itemInfo.Enchantment, 0, false, "uihelpPhraseEnchantmentFormat");

		// replace
		strFindReplace(itemText, enchantTag, enchantInfo );

		// if exist, setup text header
		if(listBrick)
			setupListBrickHeader(setup);

		// fill the bricks
		fillSabrinaPhraseListBrick(itemInfo.Enchantment, listBrick);
	}
	else
	{
		// must hide the listBrick
		if(listBrick)
			listBrick->setActive(false);

		// hide the list brick header
		hideListBrickHeader(setup);

		// hide the text
		strFindReplace(itemText, enchantTag, ucstring());
	}
}


// ***************************************************************************
static void	setupRawMaterialStats(CSheetHelpSetup &setup)
{
	// retrieve the current itemInfo
	CDBCtrlSheet	*ctrl= setup.SrcSheet;
	CInterfaceGroup	*group= setup.HelpWindow;
	if(!ctrl || !group)
		return;

	// get the group for raw material stat
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_GROUP_MP_STAT));
	if(!groupMp)
		return;

	// if the item is a Mp
	const CItemSheet	*pIS= ctrl->asItemSheet();
	if(pIS && pIS->Family == ITEMFAMILY::RAW_MATERIAL)
	{
		// If the MP can craft some item part
		if(pIS->canBuildSomeItemPart())
		{
			// activate the mp stat group
			groupMp->setActive(true);

			// Initialize the itempart selection combo box
			CDBGroupComboBox	*pCB= dynamic_cast<CDBGroupComboBox*>(groupMp->getElement(groupMp->getId()+":item_part_choice" ));
			if( pCB )
			{
				pCB->resetTexts();
				for(uint i=0;i<RM_FABER_TYPE::NUM_FABER_TYPE;i++)
				{
					RM_FABER_TYPE::TRMFType		faberType= RM_FABER_TYPE::TRMFType(i);

					if(pIS->canBuildItemPart(faberType))
					{
						pCB->addText(RM_FABER_TYPE::toLocalString(faberType));
					}
				}

				// force reset, but try to keep the precedent selection
				// (useful to test same item-part from different MPs)
				sint32	precSel= pCB->getSelection();
				pCB->setSelection(1);
				pCB->setSelection(0);
				if(precSel>=0 && precSel<(sint32)pCB->getNumTexts())
				{
					pCB->setSelection(precSel);
				}
			}
		}
		else
		{
			// just hide it
			groupMp->setActive(false);
		}
	}
	else
	{
		// just hide it
		groupMp->setActive(false);
	}
}

// ***************************************************************************
void resetSheetHelp(CSheetHelpSetup &setup)
{
	CInterfaceGroup	*group= setup.HelpWindow;
	if(!group)
		return;

	// Hide the list of items by default
	IListSheetBase	*listItem= dynamic_cast<IListSheetBase*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_LIST_ITEM));
	if(listItem)
	{
		listItem->setActive(false);
	}

	// Hide the item preview by default
	CInterfaceElement	*elt= group->getElement(group->getId()+setup.PrefixForExtra+INFO_ITEM_PREVIEW);
	if(elt)
	{
		elt->setActive(false);
	}

	// Hide the list of brick by default
	IListSheetBase	*listBrick= dynamic_cast<IListSheetBase*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_LIST_BRICK));
	if(listBrick)
	{
		listBrick->setActive(false);
	}
	// Hide the mpstats by default
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_GROUP_MP_STAT));
	if(groupMp)
		groupMp->setActive(false);
	// Hide the list of brick requirement by default
	listBrick= dynamic_cast<IListSheetBase*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_LIST_BRICK_REQUIREMENT));
	if(listBrick)
	{
		listBrick->setActive(false);
	}
	// Hide the listBrick header by default
	CViewText	*view= dynamic_cast<CViewText*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_LIST_BRICK_HEADER));
	if(view)
	{
		view->setActive(false);
	}

	CInterfaceGroup		*groupCosmetic = dynamic_cast<CInterfaceGroup*>(group->getElement(group->getId()+setup.PrefixForExtra+INFO_GROUP_CHAR_3D));
	if(groupCosmetic)
		groupCosmetic->setActive(false);
}


// ***************************************************************************
static void setupItemHelp(CSheetHelpSetup &setup)
{
	if (!setup.HelpWindow) return;

	// get the calling item
	if (!setup.SrcSheet || setup.SrcSheet->getType()!=CCtrlSheetInfo::SheetType_Item )
	{
		nlwarning("<CHandlerOpenItemHelp::execute> no caller sheet found");
		return;
	}

	// If the sheet is 0, don't open!
	if(setup.SrcSheet->getSheetId()==0)
		return;

	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}

	// NB: for raw materials only, must do each once only, must not do it at refresh, cause combo reseted
	setupRawMaterialStats(setup);

	// update the item Help
	refreshItemHelp(setup);
}


// ***************************************************************************
void setupCosmetic(CSheetHelpSetup &setup, CItemSheet *pIS)
{
	nlassert(pIS);
	if(pIS->Family!=ITEMFAMILY::COSMETIC)
		return;

	EGSPD::CPeople::TPeople people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( ITEM_ORIGIN::enumToString( pIS->ItemOrigin ) );
	if ( !( UserEntity->getGender() != pIS->Cosmetic.Gender || UserEntity->people() != people ) )
	{
		CInterfaceGroup		*groupCosmetic = dynamic_cast<CInterfaceGroup*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_GROUP_CHAR_3D));
		if(groupCosmetic)
			groupCosmetic->setActive(true);
		// display the character head in the help window
		CCharacterSummary cs;
		SCharacter3DSetup::setupCharacterSummaryFromSERVERDB( cs );
		// we dont want to display helmets
		cs.VisualPropA.PropertySubData.HatModel = SCharacter3DSetup::getDB ( "SERVER:USER:HAIR_TYPE" );
		cs.VisualPropA.PropertySubData.HatColor = SCharacter3DSetup::getDB ("SERVER:USER:HAIR_COLOR");
		if ( pIS->ItemType == ITEM_TYPE::HAIR_MALE || pIS->ItemType == ITEM_TYPE::HAIR_FEMALE )
			cs.VisualPropA.PropertySubData.HatModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::HEAD_SLOT );
		else if ( pIS->ItemType == ITEM_TYPE::TATOO_MALE || pIS->ItemType == ITEM_TYPE::TATOO_FEMALE )
			cs.VisualPropC.PropertySubData.Tattoo = pIS->Cosmetic.VPValue;
		else if ( pIS->ItemType == ITEM_TYPE::HAIRCOLOR_MALE || pIS->ItemType == ITEM_TYPE::HAIRCOLOR_FEMALE )
			cs.VisualPropA.PropertySubData.HatColor = pIS->Cosmetic.VPValue;
		else
			nlwarning("<setupItemHelp> Invalid cosmetic item type '%s'",ITEM_TYPE::toString( pIS->ItemType ).c_str() );

		SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D",cs );
	}
}

// ***************************************************************************
void setupItemPreview(CSheetHelpSetup &setup, CItemSheet *pIS)
{
	nlassert(pIS);
	
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeBranch *dbBranch = NLGUI::CDBManager::getInstance()->getDbBranch( setup.SrcSheet->getSheet() );
	
	
	CInterfaceElement *elt = setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_ITEM_PREVIEW);
	if (elt == NULL)
		return;

	CInterfaceGroup	*ig = dynamic_cast<CInterfaceGroup*>(elt);

	if ( ! ig)
	{
		return;
	}

	static sint32 helpWidth = setup.HelpWindow->getW();
	bool scene_inactive = ! NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:SHOW_3D_ITEM_PREVIEW")->getValueBool();
	if (scene_inactive || 
		(pIS->Family != ITEMFAMILY::ARMOR && 
		 pIS->Family != ITEMFAMILY::MELEE_WEAPON && 
		 pIS->Family != ITEMFAMILY::RANGE_WEAPON && 
		 pIS->Family != ITEMFAMILY::SHIELD))
	{			
		setup.HelpWindow->setW(helpWidth);
		ig->setActive(false);
		return;
	}
	else
	{
		setup.HelpWindow->setW(helpWidth + ITEM_PREVIEW_WIDTH);
		ig->setActive(true);
	}
	
	CInterface3DScene *sceneI = dynamic_cast<CInterface3DScene *>(ig->getGroup("scene_item_preview"));
	if (!sceneI)
	{
		nlwarning("Can't retrieve character 3d view, or bad type");
		ig->setActive(false);
		return;
	}

	CInterface3DCharacter *char3DI = NULL;
	if (sceneI->getCharacter3DCount() != 0)
		char3DI = sceneI->getCharacter3D(0);

	if (char3DI == NULL)
	{
		nlwarning("Can't retrieve char 3D Interface");
		ig->setActive(false);
		return;
	}

	CInterface3DCamera *camera = sceneI->getCamera(0);
	if (camera == NULL)
	{
		nlwarning("Can't retrieve camera");
		ig->setActive(false);
		return;
	}


	SCharacter3DSetup c3Ds;

	CCharacterSummary cs;
	SCharacter3DSetup::setupCharacterSummaryFromSERVERDB( cs );

	float camHeight = -0.85f;

	if (pIS->Family == ITEMFAMILY::ARMOR)
	{
		if (pIS->ItemType == ITEM_TYPE::LIGHT_BOOTS || pIS->ItemType == ITEM_TYPE::MEDIUM_BOOTS || pIS->ItemType == ITEM_TYPE::HEAVY_BOOTS)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropB.PropertySubData.FeetModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::FEET_SLOT );
			cs.VisualPropB.PropertySubData.FeetColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			camHeight = -1.15f;
		}
		else if (pIS->ItemType == ITEM_TYPE::LIGHT_GLOVES || pIS->ItemType == ITEM_TYPE::MEDIUM_GLOVES || pIS->ItemType == ITEM_TYPE::HEAVY_GLOVES)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropB.PropertySubData.HandsModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::HANDS_SLOT );
			cs.VisualPropB.PropertySubData.HandsColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			//cs.VisualPropB.PropertySubData.HandsColor = pIS->Color;
		}
		else if (pIS->ItemType == ITEM_TYPE::LIGHT_SLEEVES || pIS->ItemType == ITEM_TYPE::MEDIUM_SLEEVES || pIS->ItemType == ITEM_TYPE::HEAVY_SLEEVES)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropA.PropertySubData.ArmModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::ARMS_SLOT );
			cs.VisualPropA.PropertySubData.ArmColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			//cs.VisualPropA.PropertySubData.ArmColor = pIS->Color;
			camHeight = -0.55f;
		}
		else if (pIS->ItemType == ITEM_TYPE::LIGHT_PANTS || pIS->ItemType == ITEM_TYPE::MEDIUM_PANTS || pIS->ItemType == ITEM_TYPE::HEAVY_PANTS)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropA.PropertySubData.TrouserModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::LEGS_SLOT );
			cs.VisualPropA.PropertySubData.TrouserColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			camHeight = -1.00f;
		}
		else if (pIS->ItemType == ITEM_TYPE::LIGHT_VEST || pIS->ItemType == ITEM_TYPE::MEDIUM_VEST || pIS->ItemType == ITEM_TYPE::HEAVY_VEST)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropA.PropertySubData.JacketModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::CHEST_SLOT );
			cs.VisualPropA.PropertySubData.JacketColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			camHeight = -0.55f;
		}
		else if (pIS->ItemType == ITEM_TYPE::HEAVY_HELMET)
		{
			CCDBNodeLeaf *color = dbBranch->getLeaf( setup.SrcSheet->getSheet()+":USER_COLOR", false );
			cs.VisualPropA.PropertySubData.HatModel = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::HEAD_SLOT );
			cs.VisualPropA.PropertySubData.HatColor = color->getValue32();
			SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
			camHeight = -0.35f;
		}	
	}
	else if (pIS->Family == ITEMFAMILY::SHIELD)
	{
		cs.VisualPropA.PropertySubData.WeaponLeftHand = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::LEFT_HAND_SLOT );
		if (cs.VisualPropA.PropertySubData.WeaponRightHand != 0)
		{
			CItemSheet *pES = SheetMngr.getItem(SLOTTYPE::RIGHT_HAND_SLOT, cs.VisualPropA.PropertySubData.WeaponRightHand);
			if (pES->ItemType == ITEM_TYPE::TWO_HAND_AXE || pES->ItemType == ITEM_TYPE::TWO_HAND_MACE || pES->ItemType == ITEM_TYPE::TWO_HAND_SWORD ||
				pES->ItemType == ITEM_TYPE::MAGICIAN_STAFF || pES->ItemType == ITEM_TYPE::AUTOLAUCH || pES->ItemType == ITEM_TYPE::LAUNCHER || pES->ItemType == ITEM_TYPE::RIFLE)
				cs.VisualPropA.PropertySubData.WeaponRightHand = 0;
		}
		SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);

	}
	else if (pIS->Family == ITEMFAMILY::MELEE_WEAPON || pIS->Family == ITEMFAMILY::RANGE_WEAPON)
	{
		cs.VisualPropA.PropertySubData.WeaponRightHand = CVisualSlotManager::getInstance()->sheet2Index( CSheetId(setup.SrcSheet->getSheetId()), SLOTTYPE::RIGHT_HAND_SLOT );
		cs.VisualPropA.PropertySubData.WeaponLeftHand = 0;
		SCharacter3DSetup::setupDBFromCharacterSummary("UI:TEMP:CHAR3D", cs);
	}
	else
		nlwarning("<setupItemPreview> Invalid armour or weapon item type '%s'", ITEM_TYPE::toString( pIS->ItemType ).c_str() );

	if (camera == NULL)
		return;

	camera->setTgtZ(camHeight);
	char3DI->setAnim(CAnimationStateSheet::Idle);
}

// ***************************************************************************
void refreshItemHelp(CSheetHelpSetup &setup)
{
	// Setup creator name view
	setupCreatorName(setup);

	// **** setup the item Text info
	ucstring	itemText;
	CEntitySheet *pES = SheetMngr.get ( CSheetId(setup.SrcSheet->getSheetId()) );
	if ((pES != NULL) && (pES->type() == CEntitySheet::ITEM))
	{
		CItemSheet *pIS = (CItemSheet*)pES;

		// ---- Common
		ucstring title = setup.SrcSheet->getItemActualName();
		setupHelpTitle(setup.HelpWindow, title );
		getItemText (setup.SrcSheet, itemText, pIS);

		// ---- Enchanted items only
		setupEnchantedItem(setup, itemText);

		// ---- Cosmetic only
		setupCosmetic (setup, pIS);

		// ---- item preview
		setupItemPreview(setup, pIS);
	}

	// if this is a R2 plot item, Add comment and description

// BORIS : 06/09/2006 : removed because seams to build a double 'description' in the item info windows
//	const CItemSheet *pIS= setup.SrcSheet->asItemSheet();
//	if (pIS)
//	{
//		if (pIS->Family == ITEMFAMILY::SCROLL_R2)
//		{
//			const R2::TMissionItem *mi = R2::getEditor().getPlotItemInfos((uint32) setup.SrcSheet->getSheetId());
//			if (mi)
//			{
//				itemText += CI18N::get("uiRingPlotItemDesc");
//				itemText += mi->Description.empty() ? CI18N::get("uiRingPlotItemEmpty")
//													: mi->Description;
//				//itemText += ucstring("\n@{6F6F}") + CI18N::get("uiRingPlotItemComment") + ucstring("\n");
//				/*
//				itemText += mi->Comment.empty() ? CI18N::get("uiRingPlotItemEmpty")
//												: (ucstring("\n") + mi->Comment);
//												*/
//			}
//		}
//	}


	// **** setup the text
	setHelpText(setup, itemText);
}


// ***************************************************************************
static void setupPactHelp(CSheetHelpSetup &setup)
{
	if (!setup.HelpWindow) return;

	// get the calling item
	if (!setup.SrcSheet)
	{
		nlwarning("<CHandlerOpenBrickHelp::execute> no caller sheet found.");
		return;
	}

	// If the sheet is 0, don't open!
	if(setup.SrcSheet->getSheetId()==0)
		return;

	const CPactSheet *pact = setup.SrcSheet->asPactSheet();
	if (!pact)
	{
		nlwarning("<CHandlerOpenBrickHelp::execute> can't get pact.");
		return;
	}

	// Level of the pact is in the quality.
	sint32 pactLevel = setup.SrcSheet->getQuality();

	if (pactLevel < 0 || pactLevel >= (sint32) pact->PactLose.size())
	{
		nlwarning("<CHandlerOpenBrickHelp::execute> bad level for pact.");
		return;
	}

	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}


	const CPactSheet::SPact &pactLose = pact->PactLose[pactLevel];
	// **** setup the brick Text info
	ucstring	pactText;

	// TODO Localisation
	setupHelpTitle(setup.HelpWindow, pactLose.Name);

	pactText= CI18N::get("uihelpPactFormat");
	strFindReplace(pactText, "%lvl", toString(pactLevel));
	strFindReplace(pactText, "%hp", toString(pactLose.LoseHitPointsLevel));
	strFindReplace(pactText, "%sta", toString(pactLose.LoseStaminaLevel));
	strFindReplace(pactText, "%sap", toString(pactLose.LoseSapLevel));
	strFindReplace(pactText, "%skill", toString(pactLose.LoseSkillsLevel));

	// **** setup the text
	setHelpText(setup, pactText);
	return;
};

// ***************************************************************************
static void setupMissionHelp(CSheetHelpSetup &setup)
{
	// get the calling item
	if (!setup.SrcSheet)
	{
		nlwarning("<setupMissionHelp> no caller sheet found.");
		return;
	}

	// setup the item.
//	CDBCtrlSheet		*ctrlMission= dynamic_cast<CDBCtrlSheet*>(setup.HelpWindow->getCtrl("ctrl_slot"));
	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}

	// get detail text id from db
	if (!setup.SrcSheet->getRootBranch()) return;
	CCDBNodeLeaf *detailTextLeaf = dynamic_cast<CCDBNodeLeaf *>(setup.SrcSheet->getRootBranch()->getNode(ICDBNode::CTextId("DETAIL_TEXT")));
	if (!detailTextLeaf) return;

	// Change the title according to Mission Client Type
	MISSION_DESC::TIconId	iconId= (MISSION_DESC::TIconId)setup.SrcSheet->getSheetId();
	MISSION_DESC::TClientMissionType	mType= MISSION_DESC::getClientMissionType(iconId);
	if(mType==MISSION_DESC::Mission)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpMission"));
	else if(mType==MISSION_DESC::ZCRequirement)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpZCRequirement"));
	else if(mType==MISSION_DESC::BuildingRequirement)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpBuildingRequirement"));
	else if(mType==MISSION_DESC::ZCCharge)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpZCCharge"));
	else if(mType==MISSION_DESC::Building)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpBuilding"));
	else if(mType==MISSION_DESC::RMBuy)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpRMBuy"));
	else if(mType==MISSION_DESC::RMUpgrade)
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpRMUpgrade"));
	else
		setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpMission") );


	// **** setup the text id
	setHelpTextID(setup, detailTextLeaf->getValue32());

	CViewTextID	*viewTextTitleID = dynamic_cast<CViewTextID *>(setup.HelpWindow->getView("text_title_id"));
	if (viewTextTitleID != NULL)
	{
		CCDBNodeLeaf *titleTextLeaf = dynamic_cast<CCDBNodeLeaf*>(setup.SrcSheet->getRootBranch()->getNode(ICDBNode::CTextId("TEXT")));
		if (titleTextLeaf == NULL) return;
		viewTextTitleID->setTextId(titleTextLeaf->getValue32());
	}

};

// ***************************************************************************
void refreshMissionHelp(CSheetHelpSetup &setup, const CPrerequisitInfos &infos)
{
	static NLMISC::CRGBA orange(250,150,0);
	static NLMISC::CRGBA darkGreen(0,150,0);

	if (infos.Prerequisits.size() > 15)
	{
		// blabla
	}

	bool conditionValidated = false;

	// NB : firts prerequisit MUST be an 'and'
	for (uint i = 0 ; i < infos.Prerequisits.size() ; )
	{
		nlassert(infos.Prerequisits[i].IsMandatory);
		conditionValidated = infos.Prerequisits[i].Validated;

		// check 'or' conditions, if any or (or enclosing 'and') is validated then global block is matched
		uint orIndexMax;
		for ( orIndexMax = i+1 ; orIndexMax < infos.Prerequisits.size() ; ++orIndexMax )
		{
			if (infos.Prerequisits[orIndexMax].IsMandatory)
				break;

			if (infos.Prerequisits[orIndexMax].Validated)
				conditionValidated = true;

		}
		// fill text, choose color according to conditions and block
		for (uint j = i ; j < orIndexMax ; ++j )
		{
			const std::string text = setup.HelpWindow->getId() + ":content:scroll_text_id:text_list:" + NLMISC::toString("text_%u",j+1);
			CViewText *viewText = dynamic_cast<CViewText *>(setup.HelpWindow->getElement(text));
			if (viewText)
			{
				viewText->setActive(true);
				if (infos.Prerequisits[j].IsMandatory)
					viewText->setHardText("uiMissionAnd");
				else
					viewText->setHardText("uiMissionOr");
			}

			const std::string textId = setup.HelpWindow->getId() + ":content:scroll_text_id:text_list:" + NLMISC::toString("text_id_prereq_%u",j+1);

			CViewTextID	*viewTextID = dynamic_cast<CViewTextID *>(setup.HelpWindow->getElement(textId));
			if(viewTextID)
			{
				// not validated : change color to red
				if (!infos.Prerequisits[j].Validated)
				{
					if (!conditionValidated)
						viewTextID->setColor(orange);
					else
						viewTextID->setColor(CRGBA::White);
				}
				else
					viewTextID->setColor(darkGreen);

				viewTextID->setActive(true);
				viewTextID->setTextId(infos.Prerequisits[j].Description);
			}
		}

		// go to next 'and' statement (or end)
		i = orIndexMax;
	}

	// inactivate other lines
	for (uint i = (uint)infos.Prerequisits.size(); i < 15	; ++i)
	{
		const std::string text = setup.HelpWindow->getId() + ":content:scroll_text_id:text_list:" + NLMISC::toString("text_%u",i+1);
		CViewText *viewText = dynamic_cast<CViewText *>(setup.HelpWindow->getElement(text));
		if (viewText)
			viewText->setActive(false);

		const std::string textId = setup.HelpWindow->getId() + ":content:scroll_text_id:text_list:" + NLMISC::toString("text_id_prereq_%u",i+1);
		CViewTextID	*viewTextID = dynamic_cast<CViewTextID *>(setup.HelpWindow->getElement(textId));
		if(viewTextID)
			viewTextID->setActive(false);
	}

	if (!setup.ScrollTextGroup.empty())
	{
		CInterfaceGroup *viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextGroup);
		if (viewTextGroup) viewTextGroup->setActive(false);
	}
	CInterfaceGroup *viewTextGroup = setup.HelpWindow->getGroup(setup.ScrollTextIdGroup);
	if (viewTextGroup) viewTextGroup->setActive(true);
}

// ***************************************************************************
class CPlayerShardNameRemover : public IOnReceiveTextId
{
	virtual	void	onReceiveTextId(ucstring &str)
	{
		str= CEntityCL::removeShardFromName(str);
	}
};
static CPlayerShardNameRemover	PlayerShardNameRemover;

// ***************************************************************************
void setupCreatorName(CSheetHelpSetup &setup)
{
	if (!setup.HelpWindow) return;
	CViewTextID *vtid = dynamic_cast<CViewTextID*>(setup.HelpWindow->getView(setup.CreatorViewTextID));
	CViewText	*vthd = dynamic_cast<CViewText*>(setup.HelpWindow->getView("creator_header"));
	if (vtid != NULL)
	{
		bool bIsRM = false;
		if (setup.SrcSheet)
		{
			const CItemSheet *pIS= dynamic_cast<const CItemSheet*>(SheetMngr.get(CSheetId(setup.SrcSheet->getSheetId())));
			bIsRM = (pIS && pIS->Family == ITEMFAMILY::RAW_MATERIAL);
		}

		// if a RM or not an item, disable the view
		if(!setup.SrcSheet || bIsRM || setup.SrcSheet->getType()!=CCtrlSheetInfo::SheetType_Item	)
		{
			// important else a brick could display a creator name....
			vtid->setActive(false);
			if(vthd)
				vthd->setActive(false);
		}
		else
		{
			// get the CreatorTextID
			uint32	itemSlotId= getInventory().getItemSlotId(setup.SrcSheet);
			uint32	creatorTextId= getInventory().getItemInfo(itemSlotId).CreatorName;

			vtid->setActive(true);
			vtid->setTextId(creatorTextId);
			vtid->setOnReceiveTextId(&PlayerShardNameRemover);
			if(vthd)
				vthd->setActive(creatorTextId!=0);
		}
	}
}

// ***************************************************************************
// ***************************************************************************
// Outpost Building Help
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void fillOutpostBuildingListItem(const std::vector<NLMISC::CSheetId> &mps, IListSheetBase *listItem, uint32 qualityLevel)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(listItem)
	{
		listItem->setActive(true);
		string	branchBase= listItem->getDbBranchName();
		// setup mps
		uint i;
		for(i=0;i<mps.size();i++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:SHEET", branchBase.c_str(), i));
			if(node)
				node->setValue32(mps[i].asInt());
			node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:QUALITY", branchBase.c_str(), i), false);
			if(node)
				node->setValue32(qualityLevel);
			node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:PREREQUISIT_VALID", branchBase.c_str(), i), false);
			if(node)
				node->setValue32(1);
		}
		// Reset other to 0.
		for(;;i++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:SHEET", branchBase.c_str(), i), false);
			if(node)
				node->setValue32(0);
			else
				break;
		}
	}
}

// ***************************************************************************
void setupOutpostBuildingHelp(CSheetHelpSetup &setup)
{
	// get the calling item
	if (!setup.SrcSheet)
	{
		nlwarning("<setupOutpostBuildingHelp> no caller sheet found.");
		return;
	}

	// setup the item.
	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}

	const COutpostBuildingSheet *pOBS = setup.SrcSheet->asOutpostBuildingSheet();
	if (pOBS == NULL)
	{
		nlwarning("<setupOutpostBuildingHelp> can't get outpost building sheet.");
		return;
	}

	setupHelpTitle(setup.HelpWindow, CI18N::get("uihelpOutpostBuilding"));

	ucstring	sOBText;

	sOBText = CI18N::get("uihelpOBFormat_"+COutpostBuildingSheet::toString(pOBS->OBType));

	{
		ucstring timeText;
		timeText = toString(pOBS->CostTime/60) + CI18N::get("uiBotChatTimeMinute");
		if ((pOBS->CostTime % 60) != 0)
			timeText += toString(pOBS->CostTime%60) + CI18N::get("uiBotChatTimeSecond");

		strFindReplace(sOBText, "%costtime", timeText);
	}

	strFindReplace(sOBText, "%costdapper", toString(pOBS->CostDapper));

	// Set name of the building
	strFindReplace(sOBText, "%name", STRING_MANAGER::CStringManagerClient::getOutpostBuildingLocalizedName(pOBS->Id));

	// For driller, set lvl
	strFindReplace(sOBText, "%lvl", toString(pOBS->MPLevelOfHighestExtractRate));

	// **** setup the text
	setHelpText(setup, sOBText);

	// **** raw materials stats for driller to buy
	if(pOBS->OBType==COutpostBuildingSheet::OB_Driller)
	{
		// Get the list of bricks container
		IListSheetBase	*listItem= dynamic_cast<IListSheetBase*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_LIST_ITEM));

		// setup the bricks
		fillOutpostBuildingListItem(pOBS->Mps, listItem, pOBS->MPLevelOfHighestExtractRate);
	}
}

// ***************************************************************************
// ***************************************************************************
// SBrick / Phrase help
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static bool		getAuraDisabledState(CDBCtrlSheet *cs)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!cs)
		return false;

	// Get the DISABLED DBprop
	string	db= cs->getSheet() + ":DISABLED";
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(db, false);
	return node && node->getValue32()!=0;
}

// ***************************************************************************
static sint		getBonusMalusSpecialTT(CDBCtrlSheet *cs)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(!cs)
		return 0;

	// Get the SPECIAL_TOOLTIP DBprop
	string	db= cs->getSheet() + ":SPECIAL_TOOLTIP";
	return NLGUI::CDBManager::getInstance()->getDbValue32 (db);
}


// ***************************************************************************
void getSabrinaBrickText(CSBrickSheet *pBR, ucstring &brickText)
{
	if(!pBR)
		return;

	// *** get the formated text according to Brick type.
	if( pBR->isFaber() && pBR->isMandatory() )
	{
		brickText= CI18N::get("uihelpBrickFaberFormat");
	}
	else
	{
		brickText= CI18N::get("uihelpBrickFormat");
	}

	// *** Basics
	// Level
	strFindReplace(brickText, "%lvl", toString(pBR->Level));
	// Kill the whole text between %ks, if the skill is unknown
	const ucstring killSkill("%ks");
	if( pBR->getSkill()==SKILLS::unknown )
	{
		ucstring::size_type	pos0= brickText.find(killSkill);
		if(pos0 != ucstring::npos)
		{
			ucstring::size_type	pos1= brickText.find(killSkill, pos0 + killSkill.size() );
			if(pos1 != ucstring::npos)
				brickText.replace(pos0, pos1+killSkill.size()-pos0, ucstring() );
		}
	}
	else
	{
		// remove %ks tag
		while(strFindReplace(brickText, "%ks", ""));

		// Skill, or array of skill for combat
		if(pBR->UsedSkills.size()==1)
			strFindReplace(brickText, "%skill", CStringManagerClient::getSkillLocalizedName(pBR->getSkill()));
		else
		{
			ucstring	fullSkillText;
			bool		first= true;
			for(uint i=0;i<pBR->UsedSkills.size();i++)
			{
				SKILLS::ESkills		skill= pBR->UsedSkills[i];
				if(skill!=SKILLS::unknown)
				{
					if(!first)
						fullSkillText+= CI18N::get("uihelpBrickCombatSkillSeparator");
					first= false;
					fullSkillText+= CStringManagerClient::getSkillLocalizedName(skill);
				}
			}

			strFindReplace(brickText, "%skill", fullSkillText);
		}
	}

	// Cost
	strFindReplace(brickText, "%cost", toString(pBR->SabrinaCost));
	// Header Cost: cost or credit
	if(pBR->SabrinaCost>=0)
		strFindReplace(brickText, "%hcost", CI18N::get("uihelpSabrinaCost") );
	else
		strFindReplace(brickText, "%hcost", CI18N::get("uihelpSabrinaCredit") );

	// Relative Cost
	// Kill the whole text between %krc, if the relative cost is 0
	if(pBR->SabrinaRelativeCost==0.f)
	{
		const ucstring killRC("%krc");
		ucstring::size_type	pos0= brickText.find(killRC);
		if(pos0 != ucstring::npos)
		{
			ucstring::size_type	pos1= brickText.find(killRC, pos0 + killRC.size() );
			if(pos1 != ucstring::npos)
				brickText.replace(pos0, pos1+killRC.size()-pos0, ucstring() );
		}
	}
	else
	{
		// remove %krc tag
		while(strFindReplace(brickText, "%krc", ""));


		strFindReplace(brickText, "%relative_cost", toPercentageText(pBR->SabrinaRelativeCost)+string("%"));
		// Header Cost: cost or credit
		if(pBR->SabrinaRelativeCost>=0.f)
			strFindReplace(brickText, "%hrel_cost", CI18N::get("uihelpSabrinaRelCost") );
		else
			strFindReplace(brickText, "%hrel_cost", CI18N::get("uihelpSabrinaRelCredit") );
	}

	// Description
	strFindReplace(brickText, "%desc", CStringManagerClient::getSBrickLocalizedDescription(pBR->Id) );

	// *** Faber
	if( pBR->isFaber() && pBR->isMandatory() )
	{
		// Display the ToolType required.
		strFindReplace(brickText, "%tool", CI18N::get("tool"+TOOL_TYPE::toString(pBR->FaberPlan.ToolType)));

		// --- Display MP itempart information
		if(pBR->FaberPlan.ItemPartMps.empty())
		{
			strFindReplace(brickText, "%mpinfo", CI18N::get("uihelpMpNone"));
		}
		else
		{
			ucstring	mpInfo;
			for(uint i=0;i<pBR->FaberPlan.ItemPartMps.size();i++)
			{
				CSBrickSheet::CFaberPlan::CItemPartMP		&mpSlot= pBR->FaberPlan.ItemPartMps[i];

				// Display the part this slot build.
				mpInfo+= "@{T4}";
				mpInfo+= RM_FABER_TYPE::toLocalString(mpSlot.FaberTypeFilter);
				mpInfo+= "\n";
			}
			// replace in brickText
			strFindReplace(brickText, "%mpinfo", mpInfo);
		}

		// --- Display MP formula information
		if(pBR->FaberPlan.FormulaMps.empty())
		{
			strFindReplace(brickText, "%mpformula", CI18N::get("uihelpMpNone"));
		}
		else
		{
			ucstring mpInfo;
			for(uint i=0;i<pBR->FaberPlan.FormulaMps.size();i++)
			{
				CSBrickSheet::CFaberPlan::CFormulaMP		&mpSlot= pBR->FaberPlan.FormulaMps[i];

				// Display the required item
				mpInfo+= "@{T4}";
				mpInfo+= STRING_MANAGER::CStringManagerClient::getItemLocalizedName(mpSlot.ItemRequired);
				mpInfo+= "\n";
			}
			// replace in brickText
			strFindReplace(brickText, "%mpformula", mpInfo);
		}
	}

	// *** Magic
	ucstring	magicResistStr;
	// Has Some Magic Resistance setuped?
	if( pBR->isMagic() && pBR->MagicResistType!=RESISTANCE_TYPE::None)
	{
		magicResistStr= CI18N::get("uihelpBrickMagicResist");
		strFindReplace(magicResistStr, "%t", CI18N::get("rs"+RESISTANCE_TYPE::toString(pBR->MagicResistType) ));
	}
	strFindReplace(brickText, "%magicresist", magicResistStr);
}


// ***************************************************************************
/*
 *	Used both by setupSabrinaPhraseHelp() and setupEnchantedItem()
 */
void fillSabrinaPhraseListBrick(const CSPhraseCom &phrase, IListSheetBase *listBrick)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CSBrickManager		*pBM= CSBrickManager::getInstance();

	if(listBrick)
	{
		listBrick->setActive(true);
		string	branchBase= listBrick->getDbBranchName();
		// setup phrase bricks
		uint i;
		for(i=0;i<phrase.Bricks.size();i++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:SHEET", branchBase.c_str(), i));
			if(node)
				node->setValue32(phrase.Bricks[i].asInt());

			// For requirements bricks, update the LOCKED state
			node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:LOCKED", branchBase.c_str(), i));
			if(node)
			{
				if(pBM->isBrickKnown(phrase.Bricks[i]))
					node->setValue32(0);
				else
					// 2 to redify it
					node->setValue32(2);
			}
		}
		// Reset other to 0.
		for(;;i++)
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("%s:%d:SHEET", branchBase.c_str(), i), false);
			if(node)
				node->setValue32(0);
			else
				break;
		}
	}
}

void hideListBrickHeader(CSheetHelpSetup &setup)
{
//	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get the header text
	CViewText	*view= dynamic_cast<CViewText*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_LIST_BRICK_HEADER));
	if(view)
	{
		view->setActive(false);
	}
}

void setupListBrickHeader(CSheetHelpSetup &setup)
{
	// get the header text
	CViewText	*view= dynamic_cast<CViewText*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_LIST_BRICK_HEADER));
	if(view)
	{
		view->setActive(true);
		view->setTextFormatTaged(CI18N::get("uihelpPhraseHeaderBricks"));
	}
}


// ***************************************************************************
/*	phraseSheetId: not null if comes from a .sphrase Sheet, used to show progression info
 */
void setupSabrinaPhraseHelp(CSheetHelpSetup &setup, const CSPhraseCom &phrase, uint32 phraseSheetId)
{
	CSPhraseManager	  *pPM = CSPhraseManager::getInstance();

	if(!setup.SrcSheet || phrase.empty())
		return;

	if(!setup.HelpWindow)
		return;

	// setup the item.
	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}

	// **** setup the phrase Text info
	setupHelpTitle(setup.HelpWindow, phrase.Name);

	// get the phraseText
	ucstring	phraseText;
	// if required, add the .sphrase requirements.
	// NB: don't add if from bot chat validation (useless cause already filtered by server)
	pPM->buildPhraseDesc(phraseText, phrase, phraseSheetId, !setup.FromBotChat);


	// **** If interesting to do it, setup the brick list
	if( pPM->allowListBrickInHelp(phrase) )
	{
		// Get the list of bricks container
		IListSheetBase	*listBrick= dynamic_cast<IListSheetBase*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_LIST_BRICK));

		// if exist, setup text header
		if(listBrick)
			setupListBrickHeader(setup);

		// setup the bricks
		fillSabrinaPhraseListBrick(phrase, listBrick);
	}


	// **** For .sphrase only, setup the requirement bricks
	if(phraseSheetId!=0)
	{
		IListSheetBase	*listBrick= dynamic_cast<IListSheetBase*>(setup.HelpWindow->getElement(setup.HelpWindow->getId()+setup.PrefixForExtra+INFO_LIST_BRICK_REQUIREMENT));
		if(listBrick)
		{
			CSPhraseCom		dummyPhrase;
			pPM->buildPhraseBrickRequirement(phraseSheetId, dummyPhrase.Bricks);
			if(!dummyPhrase.Bricks.empty())
			{
				phraseText+= CI18N::get("uihelpPhraseBrickRequirement");
				fillSabrinaPhraseListBrick(dummyPhrase, listBrick);
			}
		}
	}


	// **** setup the final text
	setHelpText(setup, phraseText);
}


// ***************************************************************************
static void setupSabrinaBrickHelp(CSheetHelpSetup &setup, bool auraDisabled)
{
	if (!setup.HelpWindow) return;

	// get the calling item
	if (!setup.SrcSheet || !setup.SrcSheet->isSBrick() )
	{
		nlwarning("<CHandlerOpenBrickHelp::execute> no caller sheet found");
		return;
	}

	// If the sheet is 0, don't open!
	if(setup.SrcSheet->getSheetId()==0)
		return;

	// setup the item.
	if(setup.DestSheet)
	{
		setup.SrcSheet->copyAspect(setup.DestSheet);
		setup.DestSheet->setActive(true);
	}


	// **** setup the brick Text info
	ucstring	brickText;
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*pBR= pBM->getBrick(CSheetId(setup.SrcSheet->getSheetId()));
	if(pBR)
	{
		const ucstring title(CStringManagerClient::getSBrickLocalizedName(pBR->Id));
		setupHelpTitle(setup.HelpWindow, title);

		// add brick info
		getSabrinaBrickText(pBR, brickText);

		// Append special Aura Info
		if(auraDisabled)
		{
			brickText+= CI18N::get("uihelpAuraDisabled");
		}
	}


	// **** setup the text
	setHelpText(setup, brickText);
}

// ***************************************************************************
void CSheetHelpSetup::setupDefaultIDs()
{
	ViewText = "text";
	ViewTextID = "text_id";
	ScrollTextGroup = "scroll_text";
	ScrollTextIdGroup = "scroll_text_id";
	CreatorViewTextID = "creator";
	PrefixForExtra= ":content:scroll_text:text_list:";
	FromBotChat= false;
}

// ***************************************************************************
void setupSheetHelp(CSheetHelpSetup &setup)
{
	if (!setup.SrcSheet) return;
	switch(setup.SrcSheet->getType())
	{
		case CCtrlSheetInfo::SheetType_Skill:	setupSkillToTradeHelp(setup);	break;
		case CCtrlSheetInfo::SheetType_Item:	setupItemHelp(setup);			break;
		case CCtrlSheetInfo::SheetType_Pact:	setupPactHelp(setup);			break;
		case CCtrlSheetInfo::SheetType_Mission:	setupMissionHelp(setup);		break;
		case CCtrlSheetInfo::SheetType_SPhrase:
			{
				CSPhraseCom			phrase;
				CSPhraseManager		*pPM= CSPhraseManager::getInstance();
				uint32				phraseSheetId= setup.SrcSheet->getSheetId();
				pPM->buildPhraseFromSheet(phrase, phraseSheetId);
				setupSabrinaPhraseHelp(setup, phrase, phraseSheetId);
				break;
			}
		case CCtrlSheetInfo::SheetType_OutpostBuilding:  setupOutpostBuildingHelp(setup); break;
		default:
			nlwarning("<CHandlerOpenHelpAuto> Bad item type.");
		break;
	}
}


// ***************************************************************************
class CHandlerOpenPhraseIdHelp : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (cs != NULL && cs->getType()==CCtrlSheetInfo::SheetType_SPhraseId)
		{
			// Get the CSPhraseCom pointed.
			sint32	id= cs->getSPhraseId();
			if(id!=0)
			{
				CSPhraseManager	*pPM= CSPhraseManager::getInstance();
				CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs);
				if (!group) return;
				CSheetHelpSetup setup;
				setup.setupDefaultIDs();
				setup.HelpWindow = group;
				setup.SrcSheet = cs;
				setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getCtrl("ctrl_slot"));
				setupSabrinaPhraseHelp(setup, pPM->getPhrase(id), 0);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenPhraseIdHelp, "open_phraseid_help");


// ***************************************************************************
class CHandlerOpenSBrickHelp : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		CDBCtrlSheet	*cs = dynamic_cast<CDBCtrlSheet*>(pCaller);

		// No Info if bad control / no sheetid
		if(!cs || cs->getSheetId()==0)
			return;
		CSheetId	brickSheetId= CSheetId(cs->getSheetId());
		// No Info for the special "Remove Me" brick
		if(brickSheetId == pBM->getInterfaceRemoveBrick() )
			return;
		// No Info for special "XP catalyzer" or "PVP oupost" interface brick in bonus malus window
		string	brickName= brickSheetId.toString();
		string	xpCatBrickPrefix= "big_xpcat_";
		string	ringXpCatBrickPrefix= "big_ring_xpcat_";
		string	pvpOutpostBrickPrefix= "big_outpost_pvp_";
		if(brickName.compare(0, xpCatBrickPrefix.size(), xpCatBrickPrefix)==0)
			return;
		if(brickName.compare(0, ringXpCatBrickPrefix.size(), ringXpCatBrickPrefix)==0)
			return;
		if(brickName.compare(0, pvpOutpostBrickPrefix.size(), pvpOutpostBrickPrefix)==0)
			return;

		// Else, Ok open the window
		{
			// get the forceKeep param (for Action info)
			sint	forceKeepWindow= -1;
			string	forceKeepWindowStr= getParam(Params, "force_keep");
			if(!forceKeepWindowStr.empty())
				fromString(forceKeepWindowStr, forceKeepWindow);
			// get the Aura Disabled param
			bool	auraDisabled= false;
			string	auraDisabledStr= getParam(Params, "test_aura_disabled");
			bool tmpAuraDisabled;
			fromString(auraDisabledStr, tmpAuraDisabled);
			if(tmpAuraDisabled)
				auraDisabled= getAuraDisabledState(cs);

			// open
			CInterfaceGroup	*group = CInterfaceHelp::activateNextWindow(cs, forceKeepWindow);
			if (!group) return;
			CSheetHelpSetup setup;
			setup.setupDefaultIDs();
			setup.HelpWindow = group;
			setup.SrcSheet = cs;
			setup.DestSheet = dynamic_cast<CDBCtrlSheet*>(group->getCtrl("ctrl_slot"));
			setupSabrinaBrickHelp(setup, auraDisabled);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerOpenSBrickHelp, "open_sbrick_help");



// ***************************************************************************
class CHandlerOnCloseHelp : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		// Remove the waiter for special ItemInfo
		uint index;
		fromString(Params, index);
		CInterfaceHelp::removeWaiterItemInfo(index);
		CInterfaceHelp::removeWaiterMissionInfo(index);

		// unpuhsed the "Keep" button.
		CInterfaceHelp::setKeepMode(index, false);
	}
};
REGISTER_ACTION_HANDLER( CHandlerOnCloseHelp, "on_close_help");


// ***************************************************************************
class CHandlerHelpKeep : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		// flag
		uint index;
		fromString(Params, index);
		CInterfaceHelp::changeKeepMode(index);
	}
};
REGISTER_ACTION_HANDLER( CHandlerHelpKeep, "help_keep");


// ***************************************************************************
class CHandlerHelpResetPos : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		sint y;
		fromString(getParam(Params, "y"), y);
		// update WindowList if possible
		CInterfaceHelp::resetWindowPos(y);
	}
};
REGISTER_ACTION_HANDLER( CHandlerHelpResetPos, "help_reset_pos");



//-----------------------------------------------
//	setConsoModSuccessTooltip
//-----------------------------------------------
void setConsoModSuccessTooltip( CDBCtrlSheet *cs )
{
	if(!cs)
		return;

	CInterfaceManager * pIM = CInterfaceManager::getInstance();

	CCDBNodeLeaf * nodeSM = NULL;
	ucstring ustr;
	if( CSheetId(cs->getSheetId()).toString() == "mod_melee_success.sbrick" )
	{
		ustr = CI18N::get("uittModMeleeSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:MELEE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_range_success.sbrick" )
	{
		ustr = CI18N::get("uittModRangeSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:RANGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_craft_success.sbrick" )
	{
		ustr = CI18N::get("uittModCraftSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:CRAFT", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_defense_success.sbrick" )
	{
		ustr = CI18N::get("uittModDefenseSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:DODGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_dodge_success.sbrick" )
	{
		ustr = CI18N::get("uittModDodgeSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:DODGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_parry_success.sbrick" )
	{
		ustr = CI18N::get("uittModParrySuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:PARRY", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::common_ecosystem)+":FORAGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_desert_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModDesertForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::desert)+":FORAGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_forest_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModForestForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::forest)+":FORAGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_lacustre_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModLacustreForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::lacustre)+":FORAGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_jungle_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModJungleForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::jungle)+":FORAGE", false);
	}
	else
	if( CSheetId(cs->getSheetId()).toString() == "mod_primary_root_forage_success.sbrick" )
	{
		ustr = CI18N::get("uittModPrimaryRootForageSuccess");
		nodeSM = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SUCCESS_MODIFIER:ECO:"+toString((uint8)ECOSYSTEM::primary_root)+":FORAGE", false);
	}

	if( nodeSM )
	{
		if( nodeSM->getValue32() < 0 )
			strFindReplace(ustr, "%modifier", "@{E42F}-"+toString(nodeSM->getValue32())+"@{FFFF}");
		else
			strFindReplace(ustr, "%modifier", "@{0F0F}"+toString(nodeSM->getValue32())+"@{FFFF}");

		// replace the context help that is required.
		CWidgetManager::getInstance()->setContextHelpText(ustr);
	}
}


// ***************************************************************************
class CHandlerAuraModifierTooltip : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CDBCtrlSheet *cs= dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(!cs)
			return;

		// set value of consumable's tootltip
		setConsoModSuccessTooltip(cs);

		// special tooltip? (pvp outpost and xp catalyzer)
		sint	specialTTId= getBonusMalusSpecialTT(cs);
		if(specialTTId==BONUS_MALUS::XpCatalyser)
			CWidgetManager::getInstance()->setContextHelpText(CI18N::get("uittXpBonus"));
		else if(specialTTId==BONUS_MALUS::OutpostPVPOn)
			CWidgetManager::getInstance()->setContextHelpText(CI18N::get("uittPvpOutpostOn"));
		else if(specialTTId==BONUS_MALUS::OutpostPVPOutOfZone)
			CWidgetManager::getInstance()->setContextHelpText(CI18N::get("uittPvpOutpostOutOfZone"));
		else if(specialTTId==BONUS_MALUS::OutpostPVPInRound)
			CWidgetManager::getInstance()->setContextHelpText(CI18N::get("uittPvpOutpostInRound"));
		else if(specialTTId==BONUS_MALUS::DeathPenalty)
		{
			CCDBNodeLeaf * node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:DEATH_XP_MALUS", false);
			if( node )
			{
				ucstring txt = CI18N::get("uittDeathPenalty");
				strFindReplace(txt, "%dp", toString((100*node->getValue16())/254));
				CWidgetManager::getInstance()->setContextHelpText(txt);
			}
		}
		// if disabled.
		else if( getAuraDisabledState(cs) )
		{
			// get the normal string, and append a short info.
			ucstring	str;
			cs->getContextHelp(str);

			str+= CI18N::get("uittAuraDisabled");

			// and replace the context help that is required.
			CWidgetManager::getInstance()->setContextHelpText(str);
		}
		// else keep the default one
	}
};
REGISTER_ACTION_HANDLER( CHandlerAuraModifierTooltip, "aura_modifier_tooltip");

// ***************************************************************************
class CHandlerUserPaToolTip : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		uint8 index;
		fromString(Params, index);
		--index; // Param is 1-based so subtract 1
		if (index >= MAX_INVENTORY_ANIMAL)
		{
			return;
		}

		ucstring txt;
		CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:NAME", index));
		if (node && CStringManagerClient::instance()->getDynString(node->getValue32(), txt))
		{
			CWidgetManager::getInstance()->setContextHelpText(CEntityCL::removeTitleFromName(txt));
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerUserPaToolTip, "userpa_name_tooltip");

// ***************************************************************************
class CHandlerAnimalDeadPopupTooltip : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		// Find the mount's db leaf
		CCDBNodeBranch *animalsNode = safe_cast<CCDBNodeBranch*>(NLGUI::CDBManager::getInstance()->getDB()->getNode( ICDBNode::CTextId( "SERVER:PACK_ANIMAL" ), false ));
		BOMB_IF( ! animalsNode, "! animalsNode", return; );
		sint32 minTimeRemaining = -1;
		uint nbAnimals = (uint)animalsNode->getNbNodes();
		for ( uint i=0; i!=nbAnimals; ++i )
		{
			CCDBNodeLeaf	*statusNode = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d", i) + ":STATUS", false);
			if (statusNode && ANIMAL_STATUS::isDead((ANIMAL_STATUS::EAnimalStatus)statusNode->getValue32()) )
			{
				ICDBNode *beastNode = animalsNode->getNode( i );
//				CCDBNodeLeaf *uidLeaf = safe_cast<CCDBNodeLeaf*>(beastNode->getNode( ICDBNode::CTextId( "UID" ) ));
				CCDBNodeLeaf *despawnLeaf = safe_cast<CCDBNodeLeaf*>(beastNode->getNode( ICDBNode::CTextId( "DESPAWN" ) ));
				if( minTimeRemaining == -1 )
				{
					minTimeRemaining = despawnLeaf->getValue32();
				}
				else
				if( minTimeRemaining > despawnLeaf->getValue32() )
				{
					minTimeRemaining = despawnLeaf->getValue32();
				}
			}
		}

		ucstring str;
		BOMB_IF( minTimeRemaining < 0, "at least one animal should be dead", return; );

		str += CI18N::get("uittAnimalDeadPopupToolTip");
		str += " : ";
		str += toString(minTimeRemaining);

		// replace the context help that is required.
		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER( CHandlerAnimalDeadPopupTooltip, "animal_dead_popup_tooltip");



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// MILKO STUFF
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

#include "../motion/user_controls.h"
#include "../entities.h"
#include "people_interraction.h"
#include "../net_manager.h"

// ***************************************************************************
class CAHMilkoKick: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
		if (selection != NULL)
		{
			sint n = PeopleInterraction.TeamList.getIndexFromName(selection->getEntityName());
			if (n >= 0)
			{
				const string msgName = "TEAM:KICK";
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					uint8 teamMember = (uint8)n;
					out.serialEnum(teamMember);
					NetMngr.push(out);
					//nlinfo("impulseCallBack : %s %d sent", msgName.c_str(), teamMember);
				}
				else
					nlwarning("unknown message named '%s'.", msgName.c_str());
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CAHMilkoKick, "milko_kick");


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// RAW MATERIAL STATS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static	void	onMpChangeItemPart(CInterfaceGroup *wnd, uint32 itemSheetId, const std::string &statPrefixId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint	i;

	if(!wnd || !itemSheetId)
		return;

	// get the item sheet
	const CItemSheet	*pIS= dynamic_cast<const CItemSheet	*>(SheetMngr.get(CSheetId(itemSheetId)));
	if(!pIS || pIS->Family!=ITEMFAMILY::RAW_MATERIAL)
		return;

	// get the group for raw material stat
	CInterfaceGroup		*groupMp= dynamic_cast<CInterfaceGroup*>(wnd->getElement(wnd->getId()+statPrefixId+INFO_GROUP_MP_STAT));
	if(!groupMp)
		return;

	// get the combo box
	CDBGroupComboBox	*pCB= dynamic_cast<CDBGroupComboBox*>(groupMp->getElement(groupMp->getId()+":item_part_choice" ));
	if( !pCB )
		return;

	// get the selection
	uint	comboSelection= pCB->getSelection();

	// get the related FaberType
	RM_FABER_TYPE::TRMFType		faberType= RM_FABER_TYPE::MPL;
	uint	bitCount= 0;
	for(i=0;i<RM_FABER_TYPE::NUM_FABER_TYPE;i++)
	{
		if(pIS->Mp.ItemPartBF & (uint64)(1 << i))
		{
			if(bitCount==comboSelection)
			{
				faberType= RM_FABER_TYPE::TRMFType(i);
				break;
			}
			bitCount++;
		}
	}

	// get the item part
	if(!pIS->canBuildItemPart(faberType))
		return;
	const CItemSheet::CMpItemPart		&itemPart= pIS->getItemPart(faberType);


	// **** Build Icon
	// get the icon
	CViewBitmap	*viewBmp= dynamic_cast<CViewBitmap*>(groupMp->getElement(groupMp->getId()+":icon" ));
	if(viewBmp)
	{
		// texture name in config.xml
		viewBmp->setTexture(CWidgetManager::getInstance()->getParser()->getDefine( RM_FABER_TYPE::toIconDefineString(faberType) ));
	}


	// **** Build text
	// get the text
	CViewText	*viewText= dynamic_cast<CViewText*>(groupMp->getElement(groupMp->getId()+":text" ));
	if(viewText)
	{
		ucstring	mpCraft;

		// add the Origin filter.
		string	originFilterKey= "iompf" + ITEM_ORIGIN::enumToString((ITEM_ORIGIN::EItemOrigin)itemPart.OriginFilter);
		mpCraft+= CI18N::get(originFilterKey);

		viewText->setText(mpCraft);
	}


	// **** Build Stat bars
	// default: hide all
	for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
	{
		// get the stat group
		CInterfaceGroup		*groupStat= dynamic_cast<CInterfaceGroup*>(groupMp->getElement(groupMp->getId()+toString(":stat%d",i) ));
		if(groupStat)
			groupStat->setActive(false);
	}
	// enable only one that are relevant for this item part
	uint	groupIndex= 0;
	for(i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
	{
		RM_FABER_STAT_TYPE::TRMStatType		statType= RM_FABER_STAT_TYPE::TRMStatType(i);

		// if this stat is not relevant for this item part, don't display it!
		if(!RM_FABER_STAT_TYPE::isStatRelevant(faberType, statType))
			continue;

		// get the next stat group
		CInterfaceGroup		*groupStat= dynamic_cast<CInterfaceGroup*>(groupMp->getElement(groupMp->getId()+toString(":stat%d",groupIndex) ));
		if(groupStat)
		{
			groupStat->setActive(true);
			// fill text and bar according to stat
			CViewText	*statTitle= dynamic_cast<CViewText*>(groupStat->getElement(groupStat->getId()+":text" ));
			CDBViewBar	*statValue= dynamic_cast<CDBViewBar*>(groupStat->getElement(groupStat->getId()+":bar" ));
			if(statTitle)
				statTitle->setText(RM_FABER_STAT_TYPE::toLocalString(statType));
			if(statValue)
				statValue->setValue(itemPart.Stats[i]);
		}

		groupIndex++;
	}

}


// ***************************************************************************
// MP Item Part selection in a info window
class CAHItemHelpMpChangeItemPart : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		// get the info window associated
		uint infoWindowIndex;
		fromString(Params, infoWindowIndex);
		if(infoWindowIndex>=CInterfaceHelp::_InfoWindows.size())
			return;
		CInterfaceHelp::CInfoWindow		&infoWindow= CInterfaceHelp::_InfoWindows[infoWindowIndex];

		// just to have the correct 'PrefixForExtra' string.
		CSheetHelpSetup		dummy;
		dummy.setupDefaultIDs();

		// common method for info and botchat
		if(infoWindow.CtrlSheet)
		{
			onMpChangeItemPart(infoWindow.Window, infoWindow.CtrlSheet->getSheetId(), dummy.PrefixForExtra);
		}
	}
};
REGISTER_ACTION_HANDLER( CAHItemHelpMpChangeItemPart, "item_help_mp_change_item_part");


// ***************************************************************************
// same, but for BotChat dialog box
class CAHItemBotChatMpChangeItemPart : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// retreive direct params
		string	wndStr= getParam(Params, "wnd");
		string	dbitem= getParam(Params, "dbitem");
		string	prefix= getParam(Params, "prefix");

		CInterfaceGroup		*wnd= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(wndStr));
		CCDBNodeLeaf		*node= NLGUI::CDBManager::getInstance()->getDbProp(dbitem);

		// common method for info and botchat
		if(wnd && node)
		{
			onMpChangeItemPart(wnd, node->getValue32(), prefix);
		}
	}
};
REGISTER_ACTION_HANDLER( CAHItemBotChatMpChangeItemPart, "item_botchat_mp_change_item_part");



// ***************************************************************************
// ***************************************************************************
// Stat report
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************

void updateStatReport ()
{
	// After 30 game minutes, send a stat report
	const uint64 time4StatReport = 60*30*1000;
	if ((ingameTime0 () <= time4StatReport) && (ingameTime1 () > time4StatReport))
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler ("proc", NULL, "proc_stat_report");
	}
}

// ***************************************************************************

extern string getSystemInformation();
class CAHSendStatReport : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->displaySystemInfo(CI18N::get ("uiSendingStatReport"));
		string s = getDebugInformation();
		s += getSystemInformation();

		string progname;
		char name[1024] = "";
#ifdef NL_OS_WINDOWS
		GetModuleFileName (NULL, name, 1023);
#else
		// TODO for Linux
#endif
		progname = CFile::getFilename(name);
		progname += " ";
		progname += "Statistic Report";

		bool res = NLNET::sendEmail ("", "", "", progname, s, "");
		if (res)
			nlinfo ("Stat report sent");
		else
			nlwarning ("Can't send stat report");
	}
};
REGISTER_ACTION_HANDLER (CAHSendStatReport, "send_stat_report");

// ***************************************************************************
class CHandlerMkInMode : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CCDBNodeLeaf *pVal = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:MK_MODE", false);
		if(pVal)
		{
			sint32 mode = pVal->getValue32() + 1;
			if (mode > 5)
				mode = 1;
			pVal->setValue32(mode);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerMkInMode, "mk_inc_mode");

// ***************************************************************************
class CHandlerBrowseFAQ : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		#ifdef NL_OS_WINDOWS
			NL3D::UDriver *Driver = CViewRenderer::getInstance()->getDriver();
			if (Driver)
			{
				HWND wnd = (HWND) Driver->getDisplay();
				ShowWindow(wnd, SW_MINIMIZE);
			}
		#endif
		browseFAQ(ClientCfg.ConfigFile);
	}
};
REGISTER_ACTION_HANDLER( CHandlerBrowseFAQ, "browse_faq");
