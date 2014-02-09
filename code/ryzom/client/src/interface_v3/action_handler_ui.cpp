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

// Interface
#include "interface_manager.h"
#include "bot_chat_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_container.h"
#include "macrocmd_manager.h"
#include "chat_window.h"
#include "people_interraction.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_html.h"
#include "inventory_manager.h"

// Client Game
#include "../entities.h"
#include "../actions_client.h"

// Game share specific includes
#include "game_share/constants.h"


// ***************************************************************************

using namespace std;
using namespace NLMISC;

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// CONTAINER ACTION HANDLERS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// test if a container is currently allowed to be displayed
static bool isContainerAuthorized(CGroupContainer *pGC)
{
	nlassert(pGC);
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	std::string shortId = pGC->getShortId();
	// special case to prevent opening the guild inventory window if there's
	// no guild or not in guild hall.
	if (shortId == "inv_guild")
	{
		if (NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:NAME") == 0
			|| !getInventory().isInventoryPresent(INVENTORIES::guild))
		{
			return false; // can't open it right now
		}
	}
	if (shortId == "inv_room")
	{
		if (!getInventory().isInventoryPresent(INVENTORIES::player_room))
		{
			return false;
		}
	}
	return true;
}

// ***************************************************************************
// open
// Arg : a container name
// Open a container
// ***************************************************************************
class CAHUIOpen : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;
		pGC->open();
	}
};
REGISTER_ACTION_HANDLER( CAHUIOpen, "open" );

// ***************************************************************************
// close
// Arg : a container name
// Close a container
// ***************************************************************************
class CAHUIClose : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		pGC->close();
	}
};
REGISTER_ACTION_HANDLER( CAHUIClose, "close" );

// ***************************************************************************
// open_close
// Arg : a container name
// Toggle - Close a container if opened and open it if closed
// ***************************************************************************
class CAHUIOpenClose : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;
		pGC->setOpen (!pGC->isOpen());
	}
};
REGISTER_ACTION_HANDLER( CAHUIOpenClose, "open_close" );

// ***************************************************************************
// popup
// Arg : a container name
// Popup a container
// ***************************************************************************
class CAHUIPopup : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!pGC->isPopable())
		{
			nlwarning("%s cannot be popup", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;
		//
		pGC->popup();
		//
		CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
		CWidgetManager::getInstance()->setCapturePointerRight(NULL);
	}
};
REGISTER_ACTION_HANDLER( CAHUIPopup, "popup" );

// ***************************************************************************
// popin
// Arg : a container name
// Popin a container (restore it)
// ***************************************************************************
class CAHUIPopin : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!pGC->isPopable())
		{
			nlwarning("%s cannot be popin", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;
		// memorize popup position
		pGC->setPopupX(pGC->getX());
		pGC->setPopupY(pGC->getY());
		pGC->setPopupW(pGC->getW());
		pGC->setPopupH(pGC->getH());
		//
		pGC->popin();
		CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
		CWidgetManager::getInstance()->setCapturePointerRight(NULL);
	}
};
REGISTER_ACTION_HANDLER( CAHUIPopin, "popin" );

// ***************************************************************************
// popup_popin
// Arg : a container name
// Toggle Popup/Popin a container
// ***************************************************************************
class CAHUIPopupPopin : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!pGC->isPopable())
		{
			nlwarning("%s cannot be popup/popin", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;
		if (pGC->isPopuped())
			CAHManager::getInstance()->runActionHandler("popin", NULL, Params);
		else
			CAHManager::getInstance()->runActionHandler("popup", NULL, Params);
	}
};
REGISTER_ACTION_HANDLER( CAHUIPopupPopin, "popup_popin" );

// ***************************************************************************
// show_on_press
// Arg : a container name
// Show a container on a key down (else hide it)
// ***************************************************************************
class CAHUIShowOnPress : public IActionHandler
{
public:
	CAHUIShowOnPress()
	{
		_FirstTime = true;
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;

		CActionsManager *pAM = &Actions;

		// If key is up
		if (!pAM->valide(CAction::CName("show_on_press",Params.c_str())))
		{
			pGC->setActive(false);
			_FirstTime = true;
		}
		else // Key is down
		{
			// For the first time ?
			/*if (_FirstTime)
			{
				_FirstTime = false;
			}
			else // Not the first time*/
			{
				// Show the container
				pGC->setActive(true);
			}
		}
	}
private:
	bool _FirstTime;
};
REGISTER_ACTION_HANDLER( CAHUIShowOnPress, "show_on_press" );


// ***************************************************************************
// show
// Arg : a container name
// Show a container
// ***************************************************************************
class CAHUIShow : public IActionHandler
{
public:
	CAHUIShow()
	{
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;

		pGC->setActive(true);
		CWidgetManager::getInstance()->setTopWindow(pGC);
	}
};
REGISTER_ACTION_HANDLER( CAHUIShow, "show" );

static string currentWebApp;

// ***************************************************************************
// hide
// Arg : a container name
// Hide a container
// ***************************************************************************
class CAHUIHide : public IActionHandler
{
public:
	CAHUIHide()
	{
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", Params));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", Params.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;

		pGC->setActive(false);
	}
};
REGISTER_ACTION_HANDLER( CAHUIHide, "hide" );

string urlencode(const string &param)
{
	CSString str = param;
	str = str.replace("+", toString("%%%2x", '+').c_str());
	str = str.replace("'", toString("%%%2x", '\'').c_str());
	str = str.replace("-", toString("%%%2x", '-').c_str());
	str = str.replace("\"", toString("%%%2x", '"').c_str());
	str = str.replace(" ", "+");
	return str;
}

// ***************************************************************************
// show_hide
// Arg : a container name
// Toggle - Show a container if hidden and hide it if shown
// ***************************************************************************
class CAHUIShowHide : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string webapp, window = Params;
		vector<string> res;
		explode(Params, string("|"), res);
		if(res[0]=="webig" || res[0]=="mailbox" || res[0]=="guild_forum" || res[0]=="profile")
		{
			window = "webig";
			if(res[0]=="mailbox")
				webapp = "mail";
			else if(res[0]=="guild_forum")
				webapp = "forum";
			else if(res[0]=="profile")
				webapp = "profile&pname="+urlencode(getParam(Params,"pname"))+"&ptype="+getParam(Params,"ptype");
			else
				webapp = "web";
		}

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface", window));
		if (pGC == NULL)
		{
			nlwarning("%s is not a container", window.c_str());
			return;
		}
		if (!isContainerAuthorized(pGC)) return;

		if(window == "webig")
		{
			if(pGC->getActive() && currentWebApp == webapp)
			{
				pGC->setActive(false);
				currentWebApp.clear();
			}
			else
			{
				pGC->setActive(true);
				currentWebApp = webapp;
			}
			if(!webapp.empty() && pGC->getActive())
			{
				CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:webig:content:html"));
				if (pGH == NULL)
				{
					nlwarning("%s is not a group html", window.c_str());
					return;
				}
				pGH->setURL("http://"+ClientCfg.WebIgMainDomain+"/index.php?app="+webapp);
			}
		}
		else
		{
			// normal open/close swap
			pGC->setActive(!pGC->getActive());
		}
	}
};
REGISTER_ACTION_HANDLER( CAHUIShowHide, "show_hide" );

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// GAME ACTION HANDLERS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
// next_sheath
// Arg : none
// Set the Next sheath
// ***************************************************************************
/*
class CAHNextSheath : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pNLCurSetWrite = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active"));
		CCDBNodeLeaf *pNLCurSetRead = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_active"));
		CCDBNodeLeaf *pNLNbSet = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_nb"));
		sint64 nVal = pNLCurSetRead->getValue64() - INVENTORIES::sheath1;
		sint64 nMax = pNLNbSet->getValue64();
		nVal++;
		if (nVal >= nMax) nVal = 0;
		nVal += INVENTORIES::sheath1;
		pNLCurSetWrite->setValue64(nVal);
	}
};
REGISTER_ACTION_HANDLER( CAHNextSheath, "next_sheath" );
*/
// ***************************************************************************
// previous_sheath
// Arg : none
// Set the Previous sheath
// ***************************************************************************
/*
class CAHPreviousSheath : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pNLCurSetWrite = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active"));
		CCDBNodeLeaf *pNLCurSetRead = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_active"));
		CCDBNodeLeaf *pNLNbSet = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_nb"));
		sint64 nVal = pNLCurSetRead->getValue64() - INVENTORIES::sheath1;
		sint64 nMax = pNLNbSet->getValue64();
		if (nVal == 0) nVal = nMax;
		nVal--;
		nVal += INVENTORIES::sheath1;
		pNLCurSetWrite->setValue64(nVal);
	}
};
REGISTER_ACTION_HANDLER( CAHPreviousSheath, "previous_sheath" );
*/
// ***************************************************************************
// set_sheath
// Arg : the sheath number (int)
// Set the sheath ##
// ***************************************************************************
/*
class CAHSetSheath : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pNLCurSetWrite = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active"));
		CCDBNodeLeaf *pNLNbSet = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_nb"));
		sint64 nVal;
		fromString(Params, nVal);
		nVal -= INVENTORIES::sheath1;
		sint64 nMax = pNLNbSet->getValue64();
		if (nVal < 0) nVal = 0;
		if (nVal >= nMax) nVal = nMax-1;
		nVal += INVENTORIES::sheath1;
		pNLCurSetWrite->setValue64(nVal);
	}
};
REGISTER_ACTION_HANDLER( CAHSetSheath, "set_sheath" );
*/
// ***************************************************************************
// talk_untalk
// Arg : none
// Talk or end dialog with the current target
// ***************************************************************************
class CAHTalkUntalk : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
/*		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if(pIM == NULL) return;
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());

		if (selection && selection->Type == CEntityCL::Player)
		{
			if (!selection->getName().empty())
			{
				// popup the main window and set its target to be the player
				CChatWindow *cw = PeopleInterraction.MainChat.Window;
				if (cw && cw->getContainer())
				{
					cw->getContainer()->setActive(true);
					cw->getContainer()->setOpen(true);
					CInterfaceManager *im = CInterfaceManager::getInstance();
					CWidgetManager::getInstance()->setTopWindow(cw->getContainer());
					cw->enableBlink(1);
					CWidgetManager::getInstance()->setCaptureKeyboard(cw->getEditBox());
					PeopleInterraction.MainChat.Filter.setTargetPlayer(selection->getName());
				}
			}
		}
		else
		{
			CBotChatManager::getInstance()->setCurrPage(NULL);

			if (UserEntity->mode()==MBEHAV::COMBAT
			||  UserEntity->mode()==MBEHAV::COMBAT_FLOAT)
			{
				return; // Cant talk
			}
			// Not in combat mode.
			else
			{
				// talk ?
				if ((UserEntity->selection() != UserEntity->slot()) &&
					(selection) && (selection->properties().talkableTo()))
				{
					CVectorD vect1 = selection->pos();
					CVectorD vect2 = UserEntity->pos();
					double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
					if(distanceSquare <= MaxTalkingDistSquare)
					{
						CAHManager::getInstance()->runActionHandler("context_talk",NULL);
					}
				}
			}
		}*/
	}
};
REGISTER_ACTION_HANDLER( CAHTalkUntalk, "talk_untalk" );

// ***************************************************************************
// mount_unmount
// Arg : none
// Mount or unseat if we can the current target
// ***************************************************************************
class CAHMountUnmount : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if(pIM == NULL) return;
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());

		// If mode Combat (no talk, no give, no mount)
		if(UserEntity->mode()==MBEHAV::COMBAT
		|| UserEntity->mode()==MBEHAV::COMBAT_FLOAT)
		{
			return; // Cant mount
		}
		// Mount
		else if(UserEntity->isRiding())
		{
			// We are currently mounted so unmount
			CAHManager::getInstance()->runActionHandler("context_unseat",NULL);
		}
		// Not in combat mode.
		else
		{
			// check if we can mount
			if ((selection) && (selection->properties().mountable()))
			{
				CVectorD vect1 = selection->pos();
				CVectorD vect2 = UserEntity->pos();
				double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
				if(distanceSquare <= MaxTalkingDistSquare)
				{
					// Ok lets mount
					CAHManager::getInstance()->runActionHandler("context_mount",NULL);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CAHMountUnmount, "mount_unmount" );

// ***************************************************************************
// exchange
// Arg : none
// Exchange with the current target
// ***************************************************************************
class CAHExchange : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if(pIM == NULL) return;
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());

		if(UserEntity->mode()==MBEHAV::COMBAT
		|| UserEntity->mode()==MBEHAV::COMBAT_FLOAT)
		{
			return; // Cant exchange
		}
		// Mount
		else
		{
			if (selection && selection->properties().canExchangeItem())
				if (!UserEntity->isBusy())
					CAHManager::getInstance()->runActionHandler("context_exchange",NULL);
		}
	}
};
REGISTER_ACTION_HANDLER( CAHExchange, "exchange" );

// ***************************************************************************
// set_top_window
// Arg : window full name
// ***************************************************************************
class CAHUISetTopWindow : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sWin = getParam(Params,"win");
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(sWin));
		if (pGC != NULL && isContainerAuthorized(pGC)) CWidgetManager::getInstance()->setTopWindow(pGC);
	}
};
REGISTER_ACTION_HANDLER( CAHUISetTopWindow, "set_top_window" );


// ***************************************************************************
// dock_undock_chat
// ***************************************************************************

class CAHUIDockUndocChat : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// change the DB (if exist)
		CCDBNodeLeaf *node= NLGUI::CDBManager::getInstance()->getDbProp(toString("UI:SAVE:ISDETACHED:")+Params, false);
		if(node)
		{
			// swap
			node->setValueBool(!node->getValueBool());
		}
	}
};
REGISTER_ACTION_HANDLER( CAHUIDockUndocChat, "dock_undock_chat" );

