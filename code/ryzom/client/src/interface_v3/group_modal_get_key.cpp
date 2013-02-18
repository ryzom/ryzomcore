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

#include "group_modal_get_key.h"
#include "interface_manager.h"
#include "nel/gui/ctrl_button.h"

#include "nel/misc/events.h"

#include "../actions_client.h"
#include "macrocmd_manager.h"
#include "macrocmd_key.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************

#define VIEW_TEXT_KEY		"ui:interface:assign_key:keytext"
#define VIEW_TEXT_INUSE		"ui:interface:assign_key:inuse"
#define CTRL_BUTTON_OK		"ui:interface:assign_key:ok_cancel:ok"

// ***************************************************************************

NLMISC_REGISTER_OBJECT(CViewBase, CGroupModalGetKey, std::string, "modal_get_key");

CGroupModalGetKey::CGroupModalGetKey(const TCtorParam &param)
: CGroupModal(param)
{
	Combo.Key = KeyCount;
	Combo.KeyButtons = noKeyButton;
}

// ***************************************************************************
void CGroupModalGetKey::setActive (bool state)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (state == true)
		CWidgetManager::getInstance()->setCaptureKeyboard (this);
	else
		CWidgetManager::getInstance()->setCaptureKeyboard (NULL);

	CViewText *pVT= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId( VIEW_TEXT_KEY ));
	if (pVT != NULL) pVT->setText(string(""));
	pVT= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId( VIEW_TEXT_INUSE ));
	if (pVT != NULL) pVT->setText(string(""));
	CCtrlBaseButton *pCB= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId( CTRL_BUTTON_OK ));
	if (pCB != NULL) pCB->setFrozen(true);

	CGroupModal::setActive(state);
}

// ***************************************************************************
bool CGroupModalGetKey::handleEvent (const NLGUI::CEventDescriptor &event)
{
	if (event.getType() == NLGUI::CEventDescriptor::key)
	{
		NLGUI::CEventDescriptorKey &edk = (NLGUI::CEventDescriptorKey &)event;
		if (edk.getKeyEventType() == NLGUI::CEventDescriptorKey::keydown)
		{
//			if ((edk.getKey() != KeyCONTROL) && (edk.getKey() != KeyMENU) && (edk.getKey() != KeySHIFT))
//			{
	  			Combo.Key = edk.getKey();
				Combo.KeyButtons = noKeyButton;
				if (edk.getKeyAlt()) Combo.KeyButtons = (TKeyButton)((uint8)Combo.KeyButtons | altKeyButton);
				if (edk.getKeyCtrl()) Combo.KeyButtons = (TKeyButton)((uint8)Combo.KeyButtons | ctrlKeyButton);
				if (edk.getKeyShift()) Combo.KeyButtons = (TKeyButton)((uint8)Combo.KeyButtons | shiftKeyButton);

				// Setup the text !
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CViewText *pVT= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId( VIEW_TEXT_KEY ));
				if (pVT != NULL) pVT->setText(Combo.toUCString());

				// Check if in use
				CActionsManager *pCurAM = NULL;
				CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

				if ((Caller == "newkey") || (Caller == "editkey"))
					pCurAM = pMCM->NewKey->CurAM;
				if (Caller == "editcmd")
					pCurAM = pMCM->EditCmd->CurAM;

				if (pCurAM != NULL)
				{
					const CActionsManager::TComboActionMap &keyShortcut = pCurAM->getComboActionMap();
					CActionsManager::TComboActionMap::const_iterator it = keyShortcut.find(Combo);
					pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId( VIEW_TEXT_INUSE ));
					if (it != keyShortcut.end())
					{
						const CBaseAction *baseAction = pCurAM->getBaseAction(it->second);
						if (baseAction && pCurAM->isActionPresentInContext(it->second))
						{
							ucstring shortcutName = baseAction->getActionLocalizedText(it->second);
							if (pVT != NULL) pVT->setText(shortcutName);
						}
					}
					else
					{
						if (pVT != NULL) pVT->setText(string(""));
					}
				}

				// Show the ok button
				CCtrlBaseButton *pCB= dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId( CTRL_BUTTON_OK ));
				if (pCB != NULL) pCB->setFrozen(false);
			}
//		}

		return true; // Catch all key events
	}
	return CGroupModal::handleEvent(event);
}


