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
#include "action_handler_base.h"
#include "action_handler_misc.h"

#include "nel/gui/interface_expr.h"
#include "interface_manager.h"

#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "dbctrl_sheet.h"
#include "interface_3d_scene.h"
#include "character_3d.h"
#include "nel/gui/group_container.h"
#include "people_interraction.h"

#include "../r2/editor.h"

using namespace std;
using namespace NLMISC;

// ------------------------------------------------------------------------------------------------
class CAHActiveMenu : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();

		// get the parent container
		CGroupContainer *gc = NULL;
		CCtrlBase *cb = pCaller;
		while (cb)
		{
			gc = dynamic_cast<CGroupContainer *>(cb);
			if (gc) break;
			cb = cb->getParent();
		}

		// update GC_POPUP flag
		if (gc)
		{
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_POPUP")->setValue64((gc->isPopuped() || gc->getLayerSetup() == 0) ? 1 : 0);
		}
		else
		{
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_POPUP")->setValue64(0);
		}

		// update GC_HAS_HELP flag
		if(gc && !gc->getHelpPage().empty())
		{
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_HAS_HELP")->setValue64(1);
		}
		else
		{
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_HAS_HELP")->setValue64(0);
		}

		// open the menu
		if (CDBCtrlSheet::getDraggedSheet() == NULL)
		{
			CWidgetManager::getInstance()->enableModalWindow (pCaller, getParam(Params, "menu"));
		}
	}
};
REGISTER_ACTION_HANDLER (CAHActiveMenu, "active_menu");

// ------------------------------------------------------------------------------------------------
class CAHSetKeyboardFocus : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string target = getParam (Params, "target");
		CGroupEditBox *geb;
		if (pCaller == NULL)
			geb = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId (target));
		else
			geb = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId (pCaller->getId(), target));
		if (geb == NULL)
		{
			nlwarning("<CAHSetKeyboardFocus::execute> Can't get target edit box %s, or bad type", target.c_str());
			return;
		}
		CWidgetManager::getInstance()->setCaptureKeyboard(geb);
		string selectAllStr = getParam (Params, "select_all");
		bool selectAll = CInterfaceElement::convertBool(selectAllStr.c_str());
		if (selectAll)
		{
			geb->setSelectionAll();
		}
	}
};
REGISTER_ACTION_HANDLER (CAHSetKeyboardFocus, "set_keyboard_focus");

// ------------------------------------------------------------------------------------------------
class CAHResetKeyboardFocus : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CWidgetManager::getInstance()->resetCaptureKeyboard();
	}
};
REGISTER_ACTION_HANDLER (CAHResetKeyboardFocus, "reset_keyboard_focus");

// ------------------------------------------------------------------------------------------------
class CAHSetEditBoxCommand : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CGroupEditBox *menuEB = CGroupEditBox::getMenuFather();
		if (menuEB) menuEB->setCommand(getParam(Params, "value"), nlstricmp(getParam(Params, "execute"), "true") ? true : false);
	}
};
REGISTER_ACTION_HANDLER (CAHSetEditBoxCommand, "set_edit_box_command");

// ------------------------------------------------------------------------------------------------
class CAHSetServerString : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sValue = getParam(Params,"value");
		string sTarget = getParam(Params,"target");

		if (sTarget.empty()) return;

		if (sTarget.rfind(':') == string::npos)
		{
			if (pCaller == NULL) return;
			sTarget = pCaller->getId() + ":" + sTarget;
		}
		else
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			string elt = sTarget.substr(0,sTarget.rfind(':'));
			CInterfaceElement *pIE;
			if (pCaller != NULL)
				pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), elt);
			else
				pIE = CWidgetManager::getInstance()->getElementFromId(elt);
			if (pIE == NULL) return;
			sTarget = pIE->getId() + ":" + sTarget.substr(sTarget.rfind(':')+1,sTarget.size());
		}

		CInterfaceExprValue evValue;
		if (CInterfaceExpr::eval(sValue, evValue, NULL))
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (evValue.toInteger())
				pIM->addServerString (sTarget, (uint32)evValue.getInteger());
		}
	}
};
REGISTER_ACTION_HANDLER (CAHSetServerString, "set_server_string");

// ------------------------------------------------------------------------------------------------
class CAHSetServerID : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sValue = getParam(Params,"value");
		string sTarget = getParam(Params,"target");
		string sRemoveTitle = getParam(Params,"remove_title");

		if (sTarget.empty()) return;

		if (sTarget.rfind(':') == string::npos)
		{
			if (pCaller == NULL) return;
			sTarget = pCaller->getId() + ":" + sTarget;
		}
		else
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			string elt = sTarget.substr(0,sTarget.rfind(':'));
			CInterfaceElement *pIE;
			if (pCaller != NULL)
				pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), elt);
			else
				pIE = CWidgetManager::getInstance()->getElementFromId(elt);
			if (pIE == NULL) return;
			sTarget = pIE->getId() + ":" + sTarget.substr(sTarget.rfind(':')+1,sTarget.size());
		}

		CInterfaceExprValue evValue;
		if (CInterfaceExpr::eval(sValue, evValue, NULL))
		{
			bool bRemoveTitle = false;
			if (!sRemoveTitle.empty())
				fromString(sRemoveTitle, bRemoveTitle);

			CInterfaceManager *pIM = CInterfaceManager::getInstance();

			if (bRemoveTitle)
			{
				CStringPostProcessRemoveTitle *pSPPRT = new CStringPostProcessRemoveTitle;

				if (evValue.toInteger())
					pIM->addServerID (sTarget, (uint32)evValue.getInteger(), pSPPRT);
			}
			else
			{
				if (evValue.toInteger())
					pIM->addServerID (sTarget, (uint32)evValue.getInteger(), NULL);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHSetServerID, "set_server_id");

// ------------------------------------------------------------------------------------------------
class CAHResetCamera : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sTarget = getParam(Params,"target");

		if (sTarget.empty()) return;

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceElement *pIE;
		if (pCaller != NULL)
			pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), sTarget);
		else
			pIE = CWidgetManager::getInstance()->getElementFromId(sTarget);
		CInterface3DCamera *pCam = dynamic_cast<CInterface3DCamera*>(pIE);
		if (pCam == NULL) return;
		pCam->reset();
	}
};
REGISTER_ACTION_HANDLER (CAHResetCamera, "reset_camera");

///////////////////////////////
// VIRTUAL DESKTOP MANAGMENT //
///////////////////////////////


// ------------------------------------------------------------------------------------------------
class CAHSetVirtualDesktop : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sVDesk = getParam(Params,"vdesk");

		if (sVDesk.empty()) return;
		sint32 nVDesk;
		fromString(sVDesk, nVDesk);

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->setMode((uint8)nVDesk);

		PeopleInterraction.refreshActiveUserChats();
	}
};
REGISTER_ACTION_HANDLER (CAHSetVirtualDesktop, "set_virtual_desktop");

// ------------------------------------------------------------------------------------------------
class CAHResetVirtualDesktop : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sVDesk = getParam(Params,"vdesk");

		if (sVDesk.empty()) return;
		sint32 nVDesk;
		fromString(sVDesk, nVDesk);

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->resetMode((uint8)nVDesk);

		PeopleInterraction.refreshActiveUserChats();
	}
};
REGISTER_ACTION_HANDLER (CAHResetVirtualDesktop, "reset_virtual_desktop");

// ------------------------------------------------------------------------------------------------
class CAHMilkoMenuResetInterface : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sParam("mode=");
		if(R2::getEditor().getMode() == R2::CEditor::TestMode)
			sParam = "R2TestMode";

		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQResetUI"), "milko_menu_do_reset_interface", sParam);
	}
};
REGISTER_ACTION_HANDLER (CAHMilkoMenuResetInterface, "milko_menu_reset_interface");

// ------------------------------------------------------------------------------------------------
class CAHMilkoMenuDoResetInterface : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string& Params)
	{
		// get param
		string mode;
		fromString(getParam(Params, "mode"), mode);

		// run procedure
		vector<string> v;
		if (mode == "R2TestMode")
			CWidgetManager::getInstance()->runProcedure ("proc_reset_r2ed_interface", NULL, v);
		else
			CWidgetManager::getInstance()->runProcedure("proc_reset_interface", NULL, v);
	}
};
REGISTER_ACTION_HANDLER(CAHMilkoMenuDoResetInterface, "milko_menu_do_reset_interface");

// ------------------------------------------------------------------------------------------------
class CAHResetInterface : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		uint32 i;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		const vector<CWidgetManager::SMasterGroup> &rVMG = CWidgetManager::getInstance()->getAllMasterGroup();
		for (uint32 nMasterGroup = 0; nMasterGroup < rVMG.size(); nMasterGroup++)
		{
			const CWidgetManager::SMasterGroup &rMG = rVMG[nMasterGroup];
			const vector<CInterfaceGroup*> &rV = rMG.Group->getGroups();
			// Active all containers (that can be activated)
			for (i = 0; i < rV.size(); ++i)
			{
				CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(rV[i]);
				if (pGC == NULL) continue;
				if (pGC->isSavable())
				{
					// Yoyo: DO NOT force activation of containers who don't want to save their Active state.
					// Usually driven by server.
					if(pGC->isActiveSavable())
						pGC->setActive(true);
				}
			}

			CWidgetManager::getInstance()->checkCoords();
			CWidgetManager::getInstance()->getMasterGroup((uint8)nMasterGroup).centerAllContainers();

			// Pop in and close all containers
			for (i = 0; i < rV.size(); ++i)
			{
				CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(rV[i]);
				if (pGC == NULL) continue;
				if (pGC->isSavable())
				{
					if (pGC->isPopable()&&pGC->isPopuped())
						pGC->popin();

					// Can close ?
					if (pGC->isOpenable()&&pGC->isOpen())
						pGC->close();
				}
			}

			CWidgetManager::getInstance()->getMasterGroup((uint8)nMasterGroup).deactiveAllContainers();
		}
	}
};
REGISTER_ACTION_HANDLER (CAHResetInterface, "reset_interface");

// ------------------------------------------------------------------------------------------------
class CAHConvertServerEntities : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sDstPath = getParam(Params, "dest");
		if (sDstPath.empty()) return;
		string sEntityNb = getParam(Params, "entity");
		uint32 nEntityNb = 0;
		if (!sEntityNb.empty())
			fromString(sEntityNb, nEntityNb);

		CCharacterSummary cs;
		SCharacter3DSetup::setupCharacterSummaryFromSERVERDB(cs, (uint8)nEntityNb);
		SCharacter3DSetup::setupDBFromCharacterSummary(sDstPath, cs);

	}
};
REGISTER_ACTION_HANDLER (CAHConvertServerEntities, "convert_server_entities");

/*// ------------------------------------------------------------------------------------------------
class CAHPopup : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sCont = getParam(Params,"cont");
		CInterfaceExprValue eVal;
		if (!CInterfaceExpr::eval(sCont, eVal, NULL)) return;
		sCont = eVal.getString();
		if (sCont.empty()) return;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(sCont));
		if (pGC == NULL) return;
		if (pGC->isPopuped()) return;
		pGC->setHighLighted(false);
		// pop the window
		pGC->popupCurrentPos();
		if (pGC->getPopupW() != -1)
		{
			pGC->setX(pGC->getPopupX());
			pGC->setY(pGC->getPopupY());
			pGC->setW(pGC->getPopupW());
			// must resize the children to get correct height
			pGC->setChildrenH(pGC->getPopupChildrenH());
		}
		pGC->invalidateCoords(2);
	}
};
REGISTER_ACTION_HANDLER (CAHPopup, "popup");
*/


