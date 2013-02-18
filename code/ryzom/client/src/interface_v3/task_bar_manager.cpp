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

#include "task_bar_manager.h"
#include "interface_manager.h"
#include "dbctrl_sheet.h"
#include "macrocmd_manager.h"
#include "dbgroup_list_sheet.h"

#include "nel/gui/action_handler.h"
#include "nel/gui/group_container.h"
#include "../actions_client.h"
#include "nel/gui/ctrl_button.h"

#include "interface_options_ryzom.h"

using namespace std;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CGroupContainerWindows, std::string, "container_windows");

// ***************************************************************************
CTaskBarManager::CTaskBarManager()
{
}


// ***************************************************************************
CTaskBarManager	*CTaskBarManager::_Instance= NULL;
CTaskBarManager *CTaskBarManager::getInstance()
{
	if(!_Instance)
		_Instance= new CTaskBarManager;
	return _Instance;
}

// ***************************************************************************
void CTaskBarManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
void	CTaskBarManager::CShortcutInfo::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serialEnum(SheetType);
	f.serial(DBSheet);
	f.serial(MacroId);
}

// ***************************************************************************
void	CTaskBarManager::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	// For All Shortcuts
	for(uint tbIndex=0;tbIndex<TBM_NUM_BARS;tbIndex++)
	{
		for(uint scut=0;scut<TBM_NUM_SHORTCUT_PER_BAR;scut++)
		{
			CShortcutInfo	info;

			// serial
			f.serial(info);

		}
	}

}

// ***************************************************************************
// CGroupContainerWindows
// ***************************************************************************

// ***************************************************************************
void CGroupContainerWindows::serialConfig(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serial(_ShowDesktops);
	if (f.isReading())
	{
		update();
	}
}

// ***************************************************************************
void CGroupContainerWindows::update(bool updatePos)
{
	CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(getCtrl("expand"));
	CInterfaceGroup *pIG = getGroup("mode_buttons");
	if ((pCB == NULL) || (pIG == NULL)) return;

	pCB->setPushed(_ShowDesktops);

	if (_ShowDesktops)
	{
		pIG->setActive(true);
		pCB->setX(124);
		setW(488);
		if (updatePos)
			setX(getX()-120);
	}
	else
	{
		pIG->setActive(false);
		pCB->setX(4);
		setW(368);
		if (updatePos)
			setX(getX()+120);
	}
}

// ***************************************************************************
class CHandlerTaskbarExpandOnOff: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainerWindows *pGCW = dynamic_cast<CGroupContainerWindows*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:windows"));
		if (pGCW == NULL) return;
		pGCW->setShowDesktops(!pGCW->getShowDesktops());
	}
};
REGISTER_ACTION_HANDLER( CHandlerTaskbarExpandOnOff, "taskbar_expand_on_off");



// ***************************************************************************
// Called when we want the gestion_windows menu to update its key binding
class CHandlerGWUpdateKeys: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:gestion_windows"));
		if (pGC == NULL) return;
		CActionsManager *pAM = &Actions;
		const CActionsManager::TActionComboMap &acmap = pAM->getActionComboMap();


		COptionsList *pOL = dynamic_cast<COptionsList*>(CWidgetManager::getInstance()->getOptions("gestion_windows_key_binding"));
		if (pOL == NULL) return;

		for (uint i = 0; i < pOL->getNumParams(); ++i)
		{
			string sTmp = pOL->getValue(i).getValStr();
			string sTxt = sTmp.substr(0,sTmp.find('|'));
			string sWin = sTmp.substr(sTmp.find('|')+1,sTmp.size());

			CActionsManager::TActionComboMap::const_iterator it = acmap.find(CAction::CName("show_hide",sWin.c_str()));
			string sFullTxt = string("ui:interface:gestion_windows:") + sTxt + ":key";
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(sFullTxt));
			if (pVT != NULL)
			{
				if (it != acmap.end())
					pVT->setText(it->second.toUCString());
				else
					pVT->setText(CI18N::get("uiNotAssigned"));
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerGWUpdateKeys, "gestion_windows_update_key_binding");

