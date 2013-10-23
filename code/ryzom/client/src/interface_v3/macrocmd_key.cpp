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
#include "macrocmd_key.h"
#include "macrocmd_manager.h"

#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_list.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/group_container.h"
#include "group_modal_get_key.h"
#include "nel/gui/interface_expr.h"

// tmp
#include "../r2/editor.h"


using namespace std;
using namespace NLMISC;

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// KEYS SHORTCUT KEYS SHORTCUT KEYS SHORTCUT KEYS SHORTCUT KEYS SHORTCUT KEYS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// To display all keys

#define WIN_KEYS							"ui:interface:keys"
#define WIN_KEYS_GAME						"ui:interface:keys_"
#define WIN_KEYS_EDIT						"ui:interface:keys_edit"
#define WIN_KEYS_EDIT_CONTENT				"ui:interface:keys_edit:content"
#define TEMPLATE_KEYS_GROUP					"tk_line"
#define TEMPLATE_KEYS_KEY_NAME				"name"
#define TEMPLATE_KEYS_SHORTCUT_NAME			"command"

// To edit a key already defined
/*
#define WIN_EDITKEY							"ui:interface:editkey"
#define VIEW_EDITKEY_TEXT_CATEGORY			"ui:interface:editkey:content:category"
#define VIEW_EDITKEY_TEXT_ACTION			"ui:interface:editkey:content:action"
#define VIEW_EDITKEY_TEXT_FIRST_PARAM		"ui:interface:editkey:content:param1"
#define VIEW_EDITKEY_TEXT_SECOND_PARAM		"ui:interface:editkey:content:param2"
#define VIEW_EDITKEY_TEXT_FIRST_PARAM_NAME	"ui:interface:editkey:content:param1_name"
#define VIEW_EDITKEY_TEXT_SECOND_PARAM_NAME	"ui:interface:editkey:content:param2_name"
#define VIEW_EDITKEY_TEXT_KEY				"ui:interface:editkey:content:txtkey"
*/
// To define a key

#define WIN_MODAL_GET_KEY	"ui:interface:assign_key"

// ***************************************************************************
// Display all keys
// ***************************************************************************


// ***************************************************************************
// Add the template key to the parent
void addKeyLine (CGroupList *pParent, const ucstring &keyName, const ucstring &shortcutName, bool grayed)
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();
	CMacroCmdManager	*pMCM = CMacroCmdManager::getInstance();
	uint	lineId= (uint)pMCM->NewKey->AllLines.size();
	string templateId = pParent->getId() + ":k" + NLMISC::toString(lineId);
	vector< pair<string, string> > vParams;
	vParams.push_back(make_pair(string("id"), templateId));
	vParams.push_back(make_pair(string("lineid"), toString(lineId)));
	CInterfaceGroup *pKeysLine = NULL;
	pKeysLine = CWidgetManager::getInstance()->getParser()->createGroupInstance (TEMPLATE_KEYS_GROUP, pParent->getId(), vParams);
	if (pKeysLine == NULL) return;

	// Put name
	CViewText *pViewKeyName = dynamic_cast<CViewText*>(pKeysLine->getView(TEMPLATE_KEYS_KEY_NAME));
	if (pViewKeyName != NULL)
	{
		pViewKeyName->setText (keyName);
		pViewKeyName->setColor(grayed?CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextGrayColor).getValColor():CRGBA::White);
	}

	CViewText *pViewShortcutName = dynamic_cast<CViewText*>(pKeysLine->getView(TEMPLATE_KEYS_SHORTCUT_NAME));
	if (pViewShortcutName != NULL)
	{
		pViewShortcutName->setText (shortcutName);
		pViewShortcutName->setColor(grayed?CWidgetManager::getInstance()->getSystemOption(CWidgetManager::OptionCtrlTextGrayColor).getValColor():CRGBA::White);
	}

	pKeysLine->setParent (pParent);
	pParent->addChild (pKeysLine);
}

// ***************************************************************************
struct CComboActionName
{
	CCombo				Combo;		// KeyCount <=> action name unbound
	CAction::CName		ActionName;
};
void buildActionToComboMap(uint8 nAM, CGroupList * /* pList */, string catName, map<ucstring, CComboActionName> &remaped)
{
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	CActionsManager *pAM = pMCM->ActionManagers[nAM];
	const CActionsManager::TComboActionMap &keyShortcut = pAM->getComboActionMap();

	// *** Order Keys by actions
	// parse the combo map
	CActionsManager::TComboActionMap::const_iterator it = keyShortcut.begin();
	while (it != keyShortcut.end())
	{
		const CAction::CName &rName = it->second;



		// if match the current category parsed
		const CCategory *pCat = pAM->getCategory(rName);
		if (pCat != NULL)
		if (pCat->Name == catName)
		{
			// see if action active in current context
			if (pAM->isActionPresentInContext(it->second))
			{
				pair<ucstring, CComboActionName>	value;
				// Don't take any risk: avoid any bug if the localisation is buggy and give same text for 2 differents CAction::CName
				// Use the localized text first, to have correct sort according to language
				value.first= pAM->getActionLocalizedText(rName) + rName.Name + rName.Argu;
				value.second.Combo= it->first;
				value.second.ActionName= it->second;
				remaped.insert(value);
			}
		}

		it++;
	}

	// *** Add unbound "ForceDisplay" actions
	const CActionsManager::TActionsForceDisplaySet &actionFD = pAM->getActionsForceDisplaySet();
	CActionsManager::TActionsForceDisplaySet::const_iterator	itFD= actionFD.begin();
	for(;itFD!=actionFD.end();itFD++)
	{
		const CAction::CName &rName = *itFD;
//		const CBaseAction *baseAction = pAM->getBaseAction(rName);

		// if match the current category parsed
		const CCategory *pCat = pAM->getCategory(rName);
		if (pCat != NULL)
		if (pCat->Name == catName)
		{
			// see if action active in current context
			if (pAM->isActionPresentInContext(rName))
			{
				pair<ucstring, CComboActionName>	value;
				// Don't take any risk: avoid any bug if the localisation is buggy and give same text for 2 differents CAction::CName
				// Use the localized text first, to have correct sort according to language
				value.first= pAM->getActionLocalizedText(rName) + rName.Name + rName.Argu;
				// if alredy added (ie combo bound)
				if(remaped.find(value.first)!=remaped.end())
					continue;
				// Add this unbound action
				value.second.Combo.Key= KeyCount;
				value.second.Combo.KeyButtons= noKeyButton;
				value.second.ActionName= rName;
				remaped.insert(value);
			}
		}

	}
}


// Get all the couple (combo,action) from the action manager nAM and insert them into pList (with the template)
void getAllComboAction(uint8 nAM, CGroupList *pList, const map<ucstring, CComboActionName> &remaped)
{
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	CActionsManager *pAM = pMCM->ActionManagers[nAM];
	// *** Fill Actions
	map<ucstring, CComboActionName>::const_iterator remapIT = remaped.begin();
	while (remapIT != remaped.end())
	{
		ucstring keyName;
		if(remapIT->second.Combo.Key==KeyCount)
			keyName= CI18N::get("uiNotAssigned");
		else
			keyName= remapIT->second.Combo.toUCString();
		const CBaseAction *baseAction = pAM->getBaseAction(remapIT->second.ActionName);
		if (baseAction)
		{
			ucstring shortcutName = baseAction->getActionLocalizedText(remapIT->second.ActionName);

			addKeyLine(pList, keyName, shortcutName, remapIT->second.Combo.Key==KeyCount);
			CModalContainerEditCmd::CLine	line;
			line.ComboAM= nAM;
			line.Combo= remapIT->second.Combo;
			line.ActionName= remapIT->second.ActionName;
			pMCM->NewKey->AllLines.push_back(line);
		}
		remapIT++;
	}
}

// ***************************************************************************
// Called when we activate the keys container
class	CHandlerKeysOpen: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Initialisation of category combo box
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();
		CMacroCmdManager	*pMCM = CMacroCmdManager::getInstance();

		for (uint nAM = 0; nAM < pMCM->ActionManagers.size(); ++nAM)
		{
			CActionsManager *pAM = pMCM->ActionManagers[nAM];
			const vector<CCategory> &rCats = pAM->getCategories();
			CGroupContainer *pGC;
			for (uint i = 0; i < rCats.size(); ++i)
			{
				string contName = string(WIN_KEYS_GAME)+rCats[i].Name;
				pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(contName));
				if(pGC != NULL)
				{
					pGC->setX(0);
					pGC->setY(0);
				}
			}
		}

		// Ensure the Key edit is closed
		pMCM->NewKey->deactivate();
		pMCM->NewKey->CurAM = NULL;
		pMCM->NewKey->AllLines.clear();

		// Get the group to add all (keys,shortcut) couple
		for (uint nAM = 0; nAM < pMCM->ActionManagers.size(); ++nAM)
		{
			CActionsManager *pAM = pMCM->ActionManagers[nAM];
			const vector<CCategory> &rCats = pAM->getCategories();
			for (uint i = 0; i < rCats.size(); ++i)
			{
				string contName = string(WIN_KEYS_GAME)+rCats[i].Name;
				CInterfaceGroup *pCategory = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(contName));
				CGroupList *pList = dynamic_cast<CGroupList*>(CWidgetManager::getInstance()->getElementFromId(contName + ":content"));
				if (pCategory != NULL && pList != NULL)
				{
					pList->clearGroups();
					pList->setDynamicDisplaySize(true);

					map<ucstring, CComboActionName> remaped;
					buildActionToComboMap(nAM, pList, rCats[i].Name, remaped);
					if (!remaped.empty())
					{
						getAllComboAction(nAM, pList, remaped);
						pCategory->setActive(true);
					}
					else
					{
						pCategory->setActive(false);
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerKeysOpen, "keys_open");

// ***************************************************************************
// Called when we push the new button
class	CHandlerKeysNew: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		pMCM->NewKey->CurrentEditCmdNb = -1;
		pMCM->NewKey->activate();
	}
};
REGISTER_ACTION_HANDLER( CHandlerKeysNew, "keys_new");

// ***************************************************************************
class	CHandlerKeysAssign: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		// Setup the editkey container from line
		if (pCaller == NULL) return;
		string sId = pCaller->getId();
		sId = sId.substr(sId.rfind('k')+1,sId.size());
		sint32 nLineNb;
		fromString(sId, nLineNb);
		if ((nLineNb < 0) || (nLineNb >= (sint32)pMCM->NewKey->AllLines.size())) return;

		pMCM->NewKey->CurrentEditCmdLine = pMCM->NewKey->AllLines[nLineNb];
		pMCM->NewKey->CurAM = pMCM->ActionManagers[pMCM->NewKey->AllLines[nLineNb].ComboAM];
		CGroupModalGetKey*pGetKey = dynamic_cast<CGroupModalGetKey*>(CWidgetManager::getInstance()->getElementFromId(WIN_MODAL_GET_KEY));
		pGetKey->Caller = "editkey";
		CWidgetManager::getInstance()->enableModalWindow(pCaller, WIN_MODAL_GET_KEY);
	}
};
REGISTER_ACTION_HANDLER( CHandlerKeysAssign, "keys_assign");


// ***************************************************************************
// Called when we right click on a shortcut line and choose edit from context menu
class	CHandlerKeysEdit: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		// Setup the editkey container from line
		if (pCaller == NULL) return;
		string sId = pCaller->getId();
		sId = sId.substr(sId.rfind('k')+1,sId.size());
		sint32 nLineNb;
		fromString(sId, nLineNb);
		if ((nLineNb < 0) || (nLineNb >= (sint32)pMCM->NewKey->AllLines.size())) return;

		pMCM->NewKey->CurrentEditCmdLine = pMCM->NewKey->AllLines[nLineNb];
		pMCM->NewKey->CurAM = pMCM->ActionManagers[pMCM->NewKey->AllLines[nLineNb].ComboAM];
		// The key must exist, else cannot edit the action
		CActionsManager::TComboActionMap::const_iterator it = pMCM->NewKey->CurAM->getComboActionMap().find(pMCM->NewKey->CurrentEditCmdLine.Combo);
		if (it != pMCM->NewKey->CurAM->getComboActionMap().end())
		{
			pMCM->NewKey->activateFrom (it->second.Name, it->second.Argu, nLineNb);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerKeysEdit, "keys_edit");

// ***************************************************************************
class	CHandlerKeysDelete: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		if (pCaller == NULL) return;
		string sId = pCaller->getId();
		sId = sId.substr(sId.rfind('k')+1,sId.size());
		sint32 nLineNb;
		fromString(sId, nLineNb);
		if ((nLineNb < 0) || (nLineNb >= (sint32)pMCM->NewKey->AllLines.size())) return;
		CActionsManager *pCurAM = pMCM->ActionManagers[pMCM->NewKey->AllLines[nLineNb].ComboAM];
		// if key not bound, skip (ie: this is a N/A action that is "ForceDisplay")
		if(pMCM->NewKey->AllLines[nLineNb].Combo.Key==NLMISC::KeyCount)
			return;
		// remove the combo
		pCurAM->removeCombo(pMCM->NewKey->AllLines[nLineNb].Combo);
		// reset display
		pMCM->refreshAllKeyDisplays();
	}
};
REGISTER_ACTION_HANDLER( CHandlerKeysDelete, "keys_delete");


// ***************************************************************************
// Edit Command (can be binded to a key)
// ***************************************************************************

// ***************************************************************************
CModalContainerEditCmd::CModalContainerEditCmd()
{
	Win = NULL;
	CurAM = NULL;
	_AllowAllActions= true;
}

// ***************************************************************************
void CModalContainerEditCmd::create(const std::string &name, bool bDefKey, bool allowAllActions)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string prefix = NLMISC::toUpper(name);
	CanDefineKey = bDefKey;
	_AllowAllActions = allowAllActions;
	prefix = string(DB_EDITCMD_PREFIX) + string(":") + prefix + string(":");
	DbComboSelCat = prefix + DB_EDITCMD_COMBO_SELECT_CATEGORY;
	DbComboSelAct = prefix + DB_EDITCMD_COMBO_SELECT_ACTION;
	DbComboSel1P  = prefix + DB_EDITCMD_COMBO_SELECT_FIRST_PARAM;
	DbComboSel2P  = prefix + DB_EDITCMD_COMBO_SELECT_SECOND_PARAM;
	DbComboDisp1P = prefix + DB_EDITCMD_COMBO_DISPLAY_FIRST_PARAM;
	DbComboDisp2P = prefix + DB_EDITCMD_COMBO_DISPLAY_SECOND_PARAM;

	// Create DB entry
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboSelCat);
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboSelAct);
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboSel1P);
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboSel2P);
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboDisp1P);
	NLGUI::CDBManager::getInstance()->getDbProp(DbComboDisp2P);

	vector< pair<string,string> > vArgs;
	vArgs.push_back(pair<string,string>("id",name));
	vArgs.push_back(pair<string,string>("db_sel_cat",DbComboSelCat));
	vArgs.push_back(pair<string,string>("db_sel_act",DbComboSelAct));
	vArgs.push_back(pair<string,string>("db_sel_1p",DbComboSel1P));
	vArgs.push_back(pair<string,string>("db_sel_2p",DbComboSel2P));
	vArgs.push_back(pair<string,string>("db_disp_1p",DbComboDisp1P));
	vArgs.push_back(pair<string,string>("db_disp_2p",DbComboDisp2P));

	Win = dynamic_cast<CGroupContainer*>( CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_EDITCMD, "ui:interface", vArgs));
	if (Win == NULL)
	{
		nlwarning ("cannot create %s", name.c_str());
		return;
	}
	WinName = "ui:interface:" + name;

	CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", Win);
	CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
	Win->setParent(pRoot);
	pRoot->addGroup(Win);

	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_KEY_GROUP));
	if (pIG != NULL) pIG->setActive (CanDefineKey);
}

// ***************************************************************************
void CModalContainerEditCmd::setTitle(const std::string &uistr)
{
	Win->setTitle(uistr);
}

// ***************************************************************************
void CModalContainerEditCmd::activate()
{
	CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
	CurrentEditCmdLine.ActionName.Name = "";
	CurrentEditCmdLine.ActionName.Argu = "";
	CurrentEditCmdNb = -1;
	Win->setActive(true);
	Win->launch();
	// Initialisation of category combo box
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CDBGroupComboBox *pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( WinName+WIN_EDITCMD_COMBO_CATEGORY ));
	if (pCB != NULL)
	{
		CurrentEditCmdCategories.clear();
		pCB->resetTexts();
		for (uint am=0; am < pMCM->ActionManagers.size(); am++)
		{
			const vector<CCategory> &rVCat = pMCM->ActionManagers[am]->getCategories();
			for (uint i=0; i < rVCat.size();i++)
			{
				const CCategory &cat = rVCat[i];
				// only macroisable categories (or if allow all)
				if(_AllowAllActions || cat.Macroisable)
				{
					// category should have at least one action that is possible in currenyt
					// context
					bool found = false;
					for (uint k = 0; k < cat.BaseActions.size(); ++k)
					{
						if (cat.BaseActions[k].isUsableInCurrentContext())
						{
							found = true;
							break;
						}
					}
					if (found)
					{
						pCB->addText( CI18N::get(rVCat[i].LocalizedName) );
						CurrentEditCmdCategories.push_back(rVCat[i].Name);
					}
				}
			}
		}
	}
	// Clean up all actions
	pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( WinName+WIN_EDITCMD_COMBO_ACTION ));
	if (pCB != NULL) pCB->resetTexts();
	// Clean up
	CurAM = NULL;
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelCat )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelAct )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel2P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp2P )->setValue32(-1);
	// reset name of params
	CViewText *pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_FIRST_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));
	pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_SECOND_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));
	invalidCurrentCommand();
}

// ***************************************************************************
void CModalContainerEditCmd::deactivate()
{
	if(Win)	Win->setActive(false);
}

// ***************************************************************************
void CModalContainerEditCmd::activateFrom (const std::string &cmdName, const std::string &cmdParams, sint nRef)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CMacroCmdManager	*pMCM = CMacroCmdManager::getInstance();

	CurrentEditCmdNb = nRef;
	Win->setActive(true);
	Win->launch();

	activate();

	CurrentEditCmdLine.ActionName.Name  = cmdName;
	CurrentEditCmdLine.ActionName.Argu = cmdParams;
	CurrentEditCmdNb = nRef;

	// Get current action manager and interface category index from cmdName
	uint i, j, k;
	bool bFound = false;
	const CBaseAction *pBA = NULL;
	CActionsManager *pAM = NULL;
	uint catCBIndex = 0;
	uint actCBIndex = 0;
	for (i=0; i < pMCM->ActionManagers.size(); ++i)
	{
		pAM = pMCM->ActionManagers[i];
		const vector<CCategory> &rVCat = pAM->getCategories();
		for (j=0; j < rVCat.size(); ++j)
		{
			// only macroisable category (or if allow all)
			if(_AllowAllActions || rVCat[j].Macroisable)
			{
				const vector<CBaseAction> &rVBA = rVCat[j].BaseActions;
				actCBIndex = 0;
				for (k=0; k < rVBA.size(); ++k)
				{
					// only macrosiable actions (or if allow all)
					if (_AllowAllActions || rVBA[k].Macroisable)
					{
						if(rVBA[k].Name == cmdName)
						{
							bFound = true;
							CurAM = pMCM->ActionManagers[i];
							pBA = &rVBA[k];
							break;
						}
						// actCBIndex is index in combo box here
						actCBIndex++;
					}
				}
				if (bFound) break;
				// catIndex is index in combo box here
				catCBIndex++;
			}
		}
		if (bFound) break;
	}
	if (!bFound) return;
	// Here pAM and catIndex are valid

	// Set category : Search the right category
	CDBGroupComboBox *pCB;
	pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(WinName+WIN_EDITCMD_COMBO_CATEGORY));
	pCB->setSelection(catCBIndex);
	onChangeCategory();
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();
	//CAHManager::getInstance()->runActionHandler("editcmd_change_category",NULL);
	pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(WinName+WIN_EDITCMD_COMBO_ACTION));
	pCB->setSelection(actCBIndex);
	onChangeAction();
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();
	//CAHManager::getInstance()->runActionHandler("editcmd_change_action",NULL);

	// Count number of displayed param
	uint nbRealParam = 0;
	for (i = 0; i < pBA->Parameters.size(); ++i)
	{
		const CBaseAction::CParameter &rP = pBA->Parameters[i];
		if (rP.Type != CBaseAction::CParameter::Hidden)
			nbRealParam++;
	}

	// Set params
	uint noParam = 0;
	string curStr = CurrentEditCmdLine.ActionName.Argu.substr();
	for (i = 0; i < pBA->Parameters.size(); ++i)
	{
		const CBaseAction::CParameter &rP = pBA->Parameters[i];
		string sTmp;

		// Get ith param (params are nameOfParam=argumentOfAction strings separated by |)
		// except for the last real param (which can then contains | chars) if it is the last param

		string::size_type pos = curStr.find('|');

		if ((pos == string::npos) ||
			(((noParam == nbRealParam-1) && (rP.Type != CBaseAction::CParameter::Hidden)) && (i == (pBA->Parameters.size()-1))))
		{
			sTmp = curStr;
			curStr = "";
		}
		else
		{
			
			sTmp = curStr.substr(0, pos);
			curStr = curStr.substr(pos+1, curStr.size());
		}

		// Remove 'name='
		if (sTmp.find('=') != string::npos)
			sTmp = sTmp.substr(sTmp.find('=')+1,sTmp.size());

		if (noParam >= 2) break; // NO MORE THAN 2 REAL PARAMS !

		if (rP.Type == CBaseAction::CParameter::Hidden)
		{
			// Skip it
		}
		else if (rP.Type == CBaseAction::CParameter::Constant)
		{
			if (noParam == 0)	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp1P )->setValue32(0);
			else				NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp2P )->setValue32(0);

			// Search the param value to get the position in combo box
			bool bValFound = false;
			uint nPinCB;
			for(nPinCB = 0; nPinCB < rP.Values.size(); ++nPinCB)
			{
				if (rP.Values[nPinCB].Value == sTmp)
				{
					bValFound = true;
					break;
				}
			}
			if (bValFound)
			{
				string sCombo;
				if (noParam == 0)	sCombo = WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_LIST;
				else				sCombo = WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_LIST;
				pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(sCombo));
				pCB->setSelection(nPinCB);
			}
		}
		else if ((rP.Type == CBaseAction::CParameter::User) || (rP.Type == CBaseAction::CParameter::UserName))
		{
			if (noParam == 0)	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp1P )->setValue32(1);
			else				NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp2P )->setValue32(1);

			string sEditBox;
			if (noParam == 0)	sEditBox = WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_EDITBOX;
			else				sEditBox = WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_EDITBOX;
			CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(sEditBox));
			// sTmp is utf8
			pEB->setInputStringAsUtf8(sTmp);
		}

		// Setup the param name if any
		if (rP.Type != CBaseAction::CParameter::Hidden)
		{
			string sText;
			if (noParam == 0)	sText = WinName+VIEW_EDITCMD_FIRST_PARAM_NAME;
			else				sText = WinName+VIEW_EDITCMD_SECOND_PARAM_NAME;
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(sText));
			if (pVT != NULL) pVT->setText(CI18N::get(pBA->Parameters[i].LocalizedName));
			noParam++;
		}
	}
	validCurrentCommand();

	// Update the key name
	const CActionsManager::TActionComboMap &actionCombo = pAM->getActionComboMap();
	CActionsManager::TActionComboMap::const_iterator it = actionCombo.find(CurrentEditCmdLine.ActionName);

	// Deactive the key definition (we are in edit key mode)
	CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_KEY));
	if (pBut != NULL) pBut->setActive(false);

	if (it != actionCombo.end())
	{
		CurrentEditCmdLine.Combo = it->second;

		// Activate the key definer text
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_TEXT_KEY));
		if (pVT != NULL) pVT->setActive(true);
		// setup the text of the key
		pVT->setText(it->second.toUCString());

		// There is already a shortcut so we can display ok button
		pBut = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
		if (pBut != NULL) pBut->setFrozen (false);
	}
}

// ***************************************************************************
// Given the cat index in comboBox, find action manager and the real cat index depending on the AM / macrosiable categories
bool CModalContainerEditCmd::getCatIndexAndAM (sint32 nInCatIndex, sint32 &nOutCatIndex, CActionsManager*&nOutAM)
{
	CMacroCmdManager	*pMCM = CMacroCmdManager::getInstance();
	if (nInCatIndex >= (sint32)CurrentEditCmdCategories.size())
		return false;
	string catName = CurrentEditCmdCategories[nInCatIndex];
	for (uint i=0; i < pMCM->ActionManagers.size(); ++i)
	{
		CActionsManager *pAM = pMCM->ActionManagers[i];
		const vector<CCategory> &rVCat = pAM->getCategories();
		for (uint j=0; j < rVCat.size(); ++j)
		{
			if (rVCat[j].Name == catName)
			{
				nOutCatIndex = j;
				nOutAM = pAM;
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************
// Give the action index in Combo box, find the action index in the category (skip not macroisable actions)
bool CModalContainerEditCmd::getBaseActionIndex(const CCategory &cat, sint32 nInActIndex, sint32 &nOutActIndex)
{
	if(nInActIndex<0)
		return false;
	for(nOutActIndex=0;	nOutActIndex<(sint32)cat.BaseActions.size();nOutActIndex++)
	{
		// only macroisable (or if allow all)
		if(_AllowAllActions || cat.BaseActions[nOutActIndex].Macroisable)
		{
			// must be valid in current context
			if (cat.BaseActions[nOutActIndex].isUsableInCurrentContext())
			{
				// ok, found?
				if(nInActIndex==0)	return true;
				else nInActIndex--;
			}
		}
	}
	return false;
}

// ***************************************************************************
// The current command is not valid (so we cant drag it into a macro nor define a key for it)
void CModalContainerEditCmd::invalidCurrentCommand()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewText *pVT;

	if (CurrentEditCmdNb == -1)
	{
		// Dont display key shortcut if we are in creation mode
		pVT= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId( WinName+VIEW_EDITCMD_TEXT_KEY ));
		if (pVT != NULL) pVT->setText(CI18N::get(VIEW_EDITCMD_TEXT_KEY_DEFAULT));

		// Deactivate the key definer text
		pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_TEXT_KEY));
		if (pVT != NULL) pVT->setActive(false);
	}

	// Deactivate the key definer button
	CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_KEY));
	if (pCB != NULL) pCB->setActive(false);

	// Deactivate ok button
	pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
	if (pCB != NULL) pCB->setFrozen (true);
}

// ***************************************************************************
// The current command becomes valid (display key shortcut if any)
void CModalContainerEditCmd::validCurrentCommand()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	sint32	catIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelCat )->getValue32();
	if(catIndex < 0) return;
	sint32	actIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelAct )->getValue32();
	if(actIndex < 0) return;

	CActionsManager *pAM;
	// From ComboBox to index in category/action
	if (!getCatIndexAndAM(catIndex,catIndex,pAM)) return;
	if (!getBaseActionIndex(pAM->getCategories()[catIndex], actIndex,actIndex)) return;

	// get the action
	const CBaseAction &rBA = pAM->getCategories()[catIndex].BaseActions[actIndex];

	// Build couple (name,param) that define a command
	CurrentEditCmdLine.ActionName.Name = rBA.Name;
	CurrentEditCmdLine.ActionName.Argu = "";
	uint noParam = 0;
	for (uint i = 0; i < rBA.Parameters.size(); ++i)
	{
		const CBaseAction::CParameter &rP = rBA.Parameters[i];

		if (noParam >= 2) break; // NO MORE THAN 2 REAL PARAMS !
		if ((i >= 1) && (!CurrentEditCmdLine.ActionName.Argu.empty())) CurrentEditCmdLine.ActionName.Argu+= "|";

		if (!rP.Name.empty())
			CurrentEditCmdLine.ActionName.Argu += rP.Name + "=";

		if (rP.Type == CBaseAction::CParameter::Hidden)
		{
			CurrentEditCmdLine.ActionName.Argu += rP.DefaultValue;
		}
		else if (rP.Type == CBaseAction::CParameter::Constant)
		{
			// If the param is constant get the string from the list
			// cannot use index directly because some options may be disabled in current context
			sint32 paramIndex;
			const std::vector<CBaseAction::CParameter::CValue> &rVVal = rP.Values;
			if (noParam == 0)	paramIndex = NLGUI::CDBManager::getInstance()->getDbProp(DbComboSel1P)->getValue32();
			else				paramIndex = NLGUI::CDBManager::getInstance()->getDbProp(DbComboSel2P)->getValue32();

			uint currIndex = 0;
			for (uint k = 0; k < rVVal.size(); ++k)
			{
				const CBaseAction::CParameter::CValue &rVal = rVVal[k];
				if (ActionsContext.matchContext(rVal.Contexts))
				{
					if (currIndex == (uint) paramIndex)
					{
						CurrentEditCmdLine.ActionName.Argu += rVal.Value;
						noParam++;
					}
					currIndex ++;
				}
			}
		}
		else if ((rP.Type == CBaseAction::CParameter::User) || (rP.Type == CBaseAction::CParameter::UserName))
		{
			// If the param is user or username get the string from the edit box
			string sWin;
			if (noParam == 0)	sWin = WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_EDITBOX;
			else				sWin = WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_EDITBOX;
			CGroupEditBox *pEB= dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId( sWin ));
			// convert to utf8
			if (pEB != NULL)
				CurrentEditCmdLine.ActionName.Argu += pEB->getInputStringAsUtf8();
			noParam++;
		}
	}
	// End of build

	// If we are in mode new shortcut display the button to setup a key
	if (WinName == "ui:interface:newkey")
	{
		if (CurrentEditCmdNb == -1)
		{
			// Activate the key definer button
			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_KEY));
			if (pCB != NULL) pCB->setActive(true);
			// Activate the key definer text
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_TEXT_KEY));
			if (pVT != NULL) pVT->setActive(true);
			// Does the command already exists ?
			const CActionsManager::TActionComboMap &actionCombo = pAM->getActionComboMap();
			CActionsManager::TActionComboMap::const_iterator it = actionCombo.find(CurrentEditCmdLine.ActionName);
			if (it != actionCombo.end())
			{
				CurrentEditCmdLine.Combo = it->second;
				// Yes ok let setup the text of the key
				pVT->setText(it->second.toUCString());
				// There is already a shortcut so we can display ok button
				CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
				if (pCB != NULL) pCB->setFrozen (false);
			}
			else
			{
				CurrentEditCmdLine.Combo.Key = KeyCount;
				CurrentEditCmdLine.Combo.KeyButtons = noKeyButton;
				// Display not assigned text
				pVT->setText(CI18N::get(VIEW_EDITCMD_TEXT_KEY_DEFAULT));
				// Do not display the ok button
				CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
				if (pCB != NULL) pCB->setFrozen (true);
			}
		}
		else
		{
			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
			if (pCB != NULL) pCB->setFrozen (false);
		}
	}

	// If we are in mode new command (from macro) just turn on the ok button
	if (WinName == "ui:interface:editcmd")
	{
		CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(WinName+CTRL_EDITCMD_BUTTON_OK));
		if (pCB != NULL) pCB->setFrozen(false);
	}
}

// ***************************************************************************
// Check if the param is valid depending on the type of the param
bool CModalContainerEditCmd::isParamValid (sint32 nParamIndex)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	sint32 paramType;
	if (nParamIndex == 0)	paramType = NLGUI::CDBManager::getInstance()->getDbProp(DbComboDisp1P)->getValue32();
	else					paramType = NLGUI::CDBManager::getInstance()->getDbProp(DbComboDisp2P)->getValue32();

	if (paramType == 0) // combo box list
	{
		sint32 paramListIndex;
		if (nParamIndex == 0)	paramListIndex = NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel1P )->getValue32();
		else					paramListIndex = NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel2P )->getValue32();

		if (paramListIndex < 0)	return false;
		else					return true;
	}
	else if (paramType == 1)
	{
		CGroupEditBox *pEB;
		if (nParamIndex == 0)	pEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId( WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_EDITBOX ));
		else					pEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId( WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_EDITBOX ));
		if (pEB == NULL) return false;

		// no need to translate utf8 or not here
		if (pEB->getInputStringRef().empty())		return false;
		else										return true;
	}
	return false;
}

// ***************************************************************************
void CModalContainerEditCmd::checkCurrentCommandValidity()
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();

	// Get the category selected
	sint32	catIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelCat )->getValue32();
	if(catIndex < 0) return;

	// Get the action index selected
	sint32	actIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelAct )->getValue32();
	if(actIndex < 0) return;

	CActionsManager *pAM;
	// From ComboBox to index in category/action
	if (!getCatIndexAndAM(catIndex,catIndex,pAM)) return;
	if (!getBaseActionIndex(pAM->getCategories()[catIndex], actIndex,actIndex)) return;

	// get action
	const vector<CBaseAction> &rVBA = pAM->getCategories()[catIndex].BaseActions;
	const vector<CBaseAction::CParameter> &rVParams = rVBA[actIndex].Parameters;
	// Count nb real param
	uint nbparam = 0;
	for (uint i = 0; i < rVParams.size(); ++i)
		if (rVParams[i].Type != CBaseAction::CParameter::Hidden)
			nbparam ++;
	if (nbparam == 0)
	{
		validCurrentCommand();
		return;
	}

	// Check parameters
	if (!isParamValid(0))
	{
		invalidCurrentCommand();
		return;
	}

	// First param is valid
	if (nbparam == 1) // If there is only one param requested so validate
	{
		validCurrentCommand();
	}
	else
	{ // 2 params
		nlassert(nbparam == 2);
		if (isParamValid(1))
			validCurrentCommand();
		else
			invalidCurrentCommand();
	}
}

// ***************************************************************************
void CModalContainerEditCmd::onChangeCategory()
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();

	// Get the category selected
	sint32	catIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelCat )->getValue32();
	if(catIndex < 0)
		return;

	// Update the combo box of actions
	CDBGroupComboBox *pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( WinName+WIN_EDITCMD_COMBO_ACTION ));
	if( pCB )
	{
		pCB->resetTexts();

		// From combo box index to real category index
		if (!getCatIndexAndAM(catIndex,catIndex,CurAM)) return;

		const vector<CBaseAction> &rVBA = CurAM->getCategories()[catIndex].BaseActions;

		// add only macroisable actions (or if allow all)
		for (uint i = 0; i < rVBA.size(); i++)
		{
			const CBaseAction &rBA = rVBA[i];
			if(_AllowAllActions || rBA.Macroisable)
			{
				if (rBA.isUsableInCurrentContext())
				{
					pCB->addText( CI18N::get(rBA.LocalizedName) );
				}
			}
		}
	}

	// reset the action and dont display params
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelAct )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel2P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp2P )->setValue32(-1);
	// reset name of params
	CViewText *pViewParamName;
	pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_FIRST_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));
	pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_SECOND_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));
	// Reset key
	invalidCurrentCommand();
}


// ***************************************************************************
void CModalContainerEditCmd::onChangeAction()
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();

	// Get the category selected
	sint32	catIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelCat )->getValue32();
	if(catIndex < 0)
		return;

	// Get the action index selected
	sint32	actIndex= NLGUI::CDBManager::getInstance()->getDbProp( DbComboSelAct )->getValue32();
	if(actIndex < 0)
		return;

	// From combo box index to cat/action index
	if (!getCatIndexAndAM(catIndex,catIndex,CurAM)) return;
	if (!getBaseActionIndex(CurAM->getCategories()[catIndex], actIndex,actIndex)) return;

	// Check parameters
	const vector<CBaseAction> &rVBA = CurAM->getCategories()[catIndex].BaseActions;
	const vector<CBaseAction::CParameter> &rVParams = rVBA[actIndex].Parameters;

	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboDisp2P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel1P )->setValue32(-1);
	NLGUI::CDBManager::getInstance()->getDbProp( DbComboSel2P )->setValue32(-1);
	// reset name of params
	CViewText *pViewParamName;
	pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_FIRST_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));
	pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(WinName+VIEW_EDITCMD_SECOND_PARAM_NAME));
	if (pViewParamName != NULL) pViewParamName->setText (string(""));

	uint noParam = 0;
	for (uint i = 0; i < rVParams.size(); ++i)
	{
		const CBaseAction::CParameter &rP = rVParams[i];
		if (rP.Type == CBaseAction::CParameter::Hidden)
			continue;
		string sDB;
		if (noParam >= 2) break; // NO MORE THAN 2 PARAMS !
		if (noParam == 0)
			sDB = DbComboDisp1P;
		else
			sDB = DbComboDisp2P;

		// Set the parameter name
		{
			string sViewText;
			if (noParam == 0)	sViewText = WinName+VIEW_EDITCMD_FIRST_PARAM_NAME;
			else				sViewText = WinName+VIEW_EDITCMD_SECOND_PARAM_NAME;

			pViewParamName = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(sViewText));
			if (pViewParamName != NULL)
				pViewParamName->setText (CI18N::get(rP.LocalizedName));
		}

		if (rP.Type == CBaseAction::CParameter::Constant)
		{
			string sWin;
			if (noParam == 0)	sWin = WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_LIST;
			else				sWin = WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_LIST;
			CDBGroupComboBox *pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( sWin ));
			if( pCB )
			{
				pCB->resetTexts();
				const std::vector<CBaseAction::CParameter::CValue> &rVVal = rP.Values;
				for (uint j = 0; j < rVVal.size(); j++)
				{
					const CBaseAction::CParameter::CValue &rVal = rVVal[j];

					if (ActionsContext.matchContext(rVal.Contexts))
					{
						if ((rVal.LocalizedValue.size() >= 2) &&
							(rVal.LocalizedValue[0] == 'u') && (rVal.LocalizedValue[1] == 'i'))
							pCB->addText(CI18N::get(rVal.LocalizedValue));
						else
							pCB->addText(ucstring(rVal.LocalizedValue));
					}
				}
			}
			NLGUI::CDBManager::getInstance()->getDbProp( sDB )->setValue32(0);
		}
		else if ((rP.Type == CBaseAction::CParameter::User) || (rP.Type == CBaseAction::CParameter::UserName))
		{
			string sWin;
			if (noParam == 0)	sWin = WinName+WIN_EDITCMD_COMBO_FIRST_PARAM_EDITBOX;
			else				sWin = WinName+WIN_EDITCMD_COMBO_SECOND_PARAM_EDITBOX;
			CGroupEditBox *pEB= dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId( sWin ));
			if( pEB )
			{
				pEB->setInputString(ucstring(""));
			}
			NLGUI::CDBManager::getInstance()->getDbProp( sDB )->setValue32(1);
		}
		noParam++;
	}
	// If action do not need any params validate the command
	if (noParam == 0)
		validCurrentCommand();
	else
		invalidCurrentCommand();
}

// ***************************************************************************
// Action handlers associated to the editcmd container
// ***************************************************************************

// ***************************************************************************
// Called at activate of macros container
class	CHandlerEditCmdOpen: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		// If we are in key shortcut mode
		if (Params == "newkey")
		{
			CGroupContainer *pGC = pMCM->NewKey->Win;
			//pMCM->NewKey->activate();

			if (pGC != NULL) pGC->setModalParentList(WIN_KEYS);
			if (pMCM->NewKey->CurrentEditCmdNb == -1)
				pMCM->NewKey->setTitle(WIN_EDITCMD_TITLE_NEW_KEY);
			else
				pMCM->NewKey->setTitle(WIN_EDITCMD_TITLE_EDIT_KEY);
		}

		// If we are in macro mode
		if (Params == "editcmd")
		{
			CGroupContainer *pGC = pMCM->EditCmd->Win;
			//pMCM->EditCmd->activate();
			if (pGC != NULL) pGC->setModalParentList(WIN_NEWMACRO);

			// Set right title depending if we are in new command
			if (pMCM->EditCmd->CurrentEditCmdNb == -1)
				pGC->setTitle (WIN_EDITCMD_TITLE_NEW_CMD);
			else // or not (edit cmd)
				pGC->setTitle (WIN_EDITCMD_TITLE_EDIT_CMD);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdOpen, "editcmd_open");

// ***************************************************************************
// Called when we change category
class	CHandlerEditCmdChangeCategory : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		if (Params == "editcmd")
			pMCM->EditCmd->onChangeCategory();
		if (Params == "newkey")
			pMCM->NewKey->onChangeCategory();
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdChangeCategory, "editcmd_change_category");

// ***************************************************************************
// Called when we change action
class	CHandlerEditCmdChangeAction : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		if (Params == "editcmd")
			pMCM->EditCmd->onChangeAction();
		if (Params == "newkey")
			pMCM->NewKey->onChangeAction();
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdChangeAction, "editcmd_change_action");

// ***************************************************************************
// Called when we change the first param
class	CHandlerEditCmdChangeFirstParam: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		if (Params == "editcmd")
			pMCM->EditCmd->checkCurrentCommandValidity();
		if (Params == "newkey")
			pMCM->NewKey->checkCurrentCommandValidity();
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdChangeFirstParam, "editcmd_change_first_param");

// ***************************************************************************
// Called when we change the second param
class	CHandlerEditCmdChangeSecondParam: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		if (Params == "editcmd")
			pMCM->EditCmd->checkCurrentCommandValidity();
		if (Params == "newkey")
			pMCM->NewKey->checkCurrentCommandValidity();
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdChangeSecondParam, "editcmd_change_second_param");

// ***************************************************************************
// Called when we want to define a key for that command
class	CHandlerEditCmdDefineKey: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		//checkCurrentCommandValidity();
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupModalGetKey*pGetKey = dynamic_cast<CGroupModalGetKey*>(CWidgetManager::getInstance()->getElementFromId(WIN_MODAL_GET_KEY));
		pGetKey->Caller = Params;
		CWidgetManager::getInstance()->enableModalWindow(pCaller, WIN_MODAL_GET_KEY);
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdDefineKey, "editcmd_cmd_def_key");

// ***************************************************************************
// Called when we validate the command
class	CHandlerEditCmdOK: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		// If we are in key shortcut mode
		if (Params == "newkey")
		{
			// Add the key !
			pMCM->NewKey->CurAM->addCombo(pMCM->NewKey->CurrentEditCmdLine.ActionName,
							pMCM->NewKey->CurrentEditCmdLine.Combo);
			// Refresh key containers
			pMCM->refreshAllKeyDisplays();
			pMCM->NewKey->deactivate();
		}
		// If we are in new command from macro mode
		if (Params == "editcmd")
		{
			if (pMCM->EditCmd->CurrentEditCmdNb != -1) // Edit mode ?
			{
				pMCM->CurrentEditMacro.delCommand(pMCM->EditCmd->CurrentEditCmdNb);
				pMCM->CurrentEditMacro.addCommand (pMCM->EditCmd->CurrentEditCmdLine.ActionName.Name.c_str(),pMCM->EditCmd->CurrentEditCmdLine.ActionName.Argu.c_str(),
													pMCM->EditCmd->CurrentEditCmdNb);
			}
			else
			{
				pMCM->CurrentEditMacro.addCommand(pMCM->EditCmd->CurrentEditCmdLine.ActionName.Name.c_str(),pMCM->EditCmd->CurrentEditCmdLine.ActionName.Argu.c_str());
			}
			CAHManager::getInstance()->runActionHandler("new_macro_open", pCaller);
			pMCM->EditCmd->deactivate();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEditCmdOK, "editcmd_ok");

// ***************************************************************************
// Called when the key defined is validated (with ok button on the assign_key modal)
class	CHandlerAssignKey: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		CWidgetManager::getInstance()->disableModalWindow();
		CGroupModalGetKey*pGetKey = dynamic_cast<CGroupModalGetKey*>(CWidgetManager::getInstance()->getElementFromId(WIN_MODAL_GET_KEY));
		if (pGetKey == NULL)
			return;

		// If we are in key shortcut mode
		if (pGetKey->Caller=="editkey")
		{
			pMCM->NewKey->CurrentEditCmdLine.Combo = pGetKey->Combo;
			pMCM->NewKey->CurAM->addCombo(pMCM->NewKey->CurrentEditCmdLine.ActionName,
							pMCM->NewKey->CurrentEditCmdLine.Combo);
			// Refresh all keys
			pMCM->refreshAllKeyDisplays();
		}

		// If we are in newkey mode
		if (pGetKey->Caller=="newkey")
		{
			pMCM->NewKey->CurrentEditCmdLine.Combo = pGetKey->Combo;

			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(pMCM->NewKey->WinName+VIEW_EDITCMD_TEXT_KEY));
			if (pVT != NULL) pVT->setText(pMCM->NewKey->CurrentEditCmdLine.Combo.toUCString());

			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(pMCM->NewKey->WinName+CTRL_EDITCMD_BUTTON_OK));
			if (pCB != NULL) pCB->setFrozen (false);
		}

		// If we are in mode macro
		if (pGetKey->Caller=="editcmd")
		{
			pMCM->EditCmd->CurrentEditCmdLine.Combo = pGetKey->Combo;
			pMCM->CurrentEditMacro.Combo = pMCM->EditCmd->CurrentEditCmdLine.Combo;
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_NEWMACRO_KEY));
			if (pVT != NULL) pVT->setText(pMCM->EditCmd->CurrentEditCmdLine.Combo.toUCString());
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerAssignKey, "ah_assign_key");

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(canEditKey)
{
	if(args.size()!=1)
	{
		nlwarning("<canEditKey> requires 1 arg");
		return false;
	}

	// cand edit?
	sint	lineNb= (sint)args[0].getInteger();
	CMacroCmdManager	*pMCM= CMacroCmdManager::getInstance();
	if(lineNb<0 || lineNb>=(sint)pMCM->NewKey->AllLines.size())
		result.setBool(false);
	else
	{
		CModalContainerEditCmd::CLine	&line= pMCM->NewKey->AllLines[lineNb];
		// get the action manager for this line
		nlassert(line.ComboAM<pMCM->ActionManagers.size());
		CActionsManager *pCurAM = pMCM->ActionManagers[line.ComboAM];
		nlassert(pCurAM);
		// can edit the action only if the action name is not "Force Display"
		result.setBool(!pCurAM->isActionDisplayForced(line.ActionName));
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("canEditKey", canEditKey)

// ***************************************************************************
static DECLARE_INTERFACE_USER_FCT(canDeleteKey)
{
	if(args.size()!=1)
	{
		nlwarning("<canDeleteKey> requires 1 arg");
		return false;
	}

	// cand delete?
	sint	lineNb= (sint)args[0].getInteger();
	CMacroCmdManager	*pMCM= CMacroCmdManager::getInstance();
	if(lineNb<0 || lineNb>=(sint)pMCM->NewKey->AllLines.size())
		result.setBool(false);
	else
	{
		CModalContainerEditCmd::CLine	&line= pMCM->NewKey->AllLines[lineNb];
		// get the action manager for this line
		nlassert(line.ComboAM<pMCM->ActionManagers.size());
		CActionsManager *pCurAM = pMCM->ActionManagers[line.ComboAM];
		nlassert(pCurAM);
		// cannot delete the action if the action name is "Force Display" AND NotAssigned (since already deleted)
		result.setBool( !(pCurAM->isActionDisplayForced(line.ActionName) && line.Combo.Key==KeyCount) );
	}

	return true;
}
REGISTER_INTERFACE_USER_FCT("canDeleteKey", canDeleteKey)

