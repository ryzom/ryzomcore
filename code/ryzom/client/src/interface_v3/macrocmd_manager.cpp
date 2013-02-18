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
#include "macrocmd_manager.h"
#include "macrocmd_key.h"

#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "dbctrl_sheet.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_list.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/group_container.h"
#include "group_modal_get_key.h"
#include "task_bar_manager.h"
#include "sphrase_manager.h"

using namespace std;
using namespace NLMISC;

// ------------------------------------------------------------------------------------------------
CMacroCmdManager *CMacroCmdManager::_Instance = NULL;
// ------------------------------------------------------------------------------------------------
// CMacroCmd
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CMacroCmd::CMacroCmd()
{
	BitmapBack = 0xFF;
	BitmapIcon = 0xFF;
	BitmapOver = 0xFF;
	Combo.Key = KeyCount;
	Combo.KeyButtons = noKeyButton;
	ID = -1;
}

// ------------------------------------------------------------------------------------------------
void CMacroCmd::writeTo (xmlNodePtr node) const
{
	// New node
	xmlNodePtr macroNode = xmlNewChild ( node, NULL, (const xmlChar*)"macro", NULL );

	// Props
	xmlSetProp (macroNode, (const xmlChar*)"name", (const xmlChar*)ucstring(Name).toUtf8().c_str());
	xmlSetProp (macroNode, (const xmlChar*)"id",   (const xmlChar*)toString(ID).c_str());
	xmlSetProp (macroNode, (const xmlChar*)"back", (const xmlChar*)toString(BitmapBack).c_str());
	xmlSetProp (macroNode, (const xmlChar*)"icon", (const xmlChar*)toString(BitmapIcon).c_str());
	xmlSetProp (macroNode, (const xmlChar*)"over", (const xmlChar*)toString(BitmapOver).c_str());
	xmlSetProp (macroNode, (const xmlChar*)"text", (const xmlChar*)DispText.c_str());

	for (uint i = 0; i < Commands.size(); ++i)
	{
		xmlNodePtr cmdNode = xmlNewChild ( macroNode, NULL, (const xmlChar*)"command", NULL );
		xmlSetProp (cmdNode, (const xmlChar*)"name", (const xmlChar*)Commands[i].Name.c_str());
		xmlSetProp (cmdNode, (const xmlChar*)"params", (const xmlChar*)Commands[i].Params.c_str());
	}
}

// ------------------------------------------------------------------------------------------------
bool CMacroCmd::readFrom (xmlNodePtr node)
{
	CXMLAutoPtr ptrName;

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"name" );
	if (ptrName)
	{
		ucstring ucName;
		ucName.fromUtf8((const char*)ptrName);
		Name = ucName.toString();
	}

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"id" );
	if (ptrName) fromString((const char*)ptrName, ID);

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"back" );
	if (ptrName) fromString((const char*)ptrName, BitmapBack);

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"icon" );
	if (ptrName) fromString((const char*)ptrName, BitmapIcon);

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"over" );
	if (ptrName) fromString((const char*)ptrName, BitmapOver);

	ptrName = (char*) xmlGetProp( node, (xmlChar*)"text" );
	if (ptrName) DispText = (const char*)ptrName;

	node = node->children;
	while (node)
	{
		if ( stricmp((char*)node->name,"command") == 0 )
		{
			CCommand c;

			ptrName = (char*) xmlGetProp( node, (xmlChar*)"name" );
			if (ptrName) c.Name = (const char*)ptrName;

			ptrName = (char*) xmlGetProp( node, (xmlChar*)"params" );
			if (ptrName) c.Params = (const char*)ptrName;

			Commands.push_back(c);
		}

		node = node->next;
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
void CMacroCmd::addCommand(const string &sName, const string &sParam, sint32 nPos)
{
	CCommand c(sName.c_str(), sParam.c_str());
	if (nPos == -1)
	{
		Commands.push_back(c);
	}
	else
	{
		Commands.insert(Commands.begin()+nPos, c);
	}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmd::delCommand (uint cmdNb)
{
	Commands.erase(Commands.begin()+cmdNb);
}

// ------------------------------------------------------------------------------------------------
void CMacroCmd::moveUpCommand (uint cmdNb)
{
	if (cmdNb == 0) return;
	CCommand c = Commands[cmdNb];
	Commands[cmdNb] = Commands[cmdNb-1];
	Commands[cmdNb-1] = c;
}

// ------------------------------------------------------------------------------------------------
void CMacroCmd::moveDownCommand (uint cmdNb)
{
	if (cmdNb == (Commands.size()-1)) return;
	CCommand c = Commands[cmdNb];
	Commands[cmdNb] = Commands[cmdNb+1];
	Commands[cmdNb+1] = c;
}


// ------------------------------------------------------------------------------------------------
// CMacroCmdManager
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CMacroCmdManager::CMacroCmdManager()
{
	// Init globals
	ActionManagers.clear();
	ActionManagers.resize(2);
	ActionManagers[0] = &Actions;
	ActionManagers[1] = &EditActions;
	_CurExecMac = -1;
	_CurExecCmd = -1;
	_CurExecCmdWait = -1;
	_ActionId = 0;
	EditCmd = NULL;
	NewKey = NULL;
	_MacroIDGenerator = 0;
}

CMacroCmdManager::~CMacroCmdManager()
{
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::initInGame()
{
	// Get all custom icon bitmaps
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewRenderer &rVR = *CViewRenderer::getInstance();
	CInterfaceOptions *pIO = CWidgetManager::getInstance()->getOptions("macro_custom_icon");
	if (pIO != NULL)
	{
		string sTmp;
		uint i, nOpt;

		const string prefix[3] = { "bg_", "fg_", "ov_" };
		vector<sint32> *wheretostock[3] = { &_OptBackId, &_OptIconId, &_OptOverId };

		for (nOpt = 0; nOpt < 3; nOpt++)
		{
			i = 0;
			do
			{
				sTmp = pIO->getValStr(prefix[nOpt]+NLMISC::toString(i));
				if (!sTmp.empty())
				{
					sint32 nTexId = rVR.getTextureIdFromName(sTmp);
					wheretostock[nOpt]->push_back(nTexId);
				}
				++i;
			}
			while (!sTmp.empty());
		}
	}

	// Do not begin at 0
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_NUMBER")->setValue64(0);

	// Create the NewKey container. Can edit key, and can assign all actions (not only macroisable ones)
	NewKey = new CModalContainerEditCmd;
	NewKey->create("newkey", true, true);

	// Create the EditCmd container. Cannot edit key, and can assign only macroisable actions
	EditCmd = new CModalContainerEditCmd;
	EditCmd->create("editcmd", false, false);

	// Put the macro id generator after the last macro ID
	for (uint i = 0; i < _Macros.size(); ++i)
	{
		if (_Macros[i].ID >= _MacroIDGenerator)
		{
			_MacroIDGenerator = _Macros[i].ID + 1;
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::uninitInGame()
{
	if (NewKey)
	{
		delete NewKey;
		NewKey = NULL;
	}
	if (EditCmd)
	{
		delete EditCmd;
		EditCmd = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::removeAllMacros()
{
	_Macros.clear();
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::addMacro (const CMacroCmd &m, sint32 nPos)
{
	delActionManagerEntries();

	CMacroCmd mCopy = m;

	if (mCopy.ID == -1)
	{
		mCopy.ID = _MacroIDGenerator;
		_MacroIDGenerator++;
	}

	if (nPos == -1)
	{
		_Macros.push_back(mCopy);
	}
	else
	{
		_Macros.insert(_Macros.begin()+nPos, mCopy);
	}
	addActionManagerEntries();
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::delMacro(sint32 nMacNb)
{
	delActionManagerEntries();
	_Macros.erase(_Macros.begin()+nMacNb);
	addActionManagerEntries();
}

// ------------------------------------------------------------------------------------------------
// Refresh key association that can be changed in another place than in macro container
void CMacroCmdManager::refreshMacroCombo()
{
	CActionsManager *pAM = ActionManagers[0];

	for (uint i=0; i < _Macros.size(); ++i)
	{
		const CActionsManager::TActionComboMap &rACmap = pAM->getActionComboMap();
		CActionsManager::TActionComboMap::const_iterator it = rACmap.find(CAction::CName(AH_MACRO_EXEC,toString(i).c_str()));
		if (it != rACmap.end())
			_Macros[i].Combo = it->second;
		else
			_Macros[i].Combo.Key = KeyCount;
	}
}

// ------------------------------------------------------------------------------------------------
// Remove all action manager entries
void CMacroCmdManager::delActionManagerEntries()
{
	refreshMacroCombo();

	CActionsManager *pAM = ActionManagers[0];

	// Remove all combos (seach them in the action manager)
	for (uint i=0; i < _Macros.size(); ++i)
	{
		const CActionsManager::TActionComboMap &rACmap = pAM->getActionComboMap();
		CActionsManager::TActionComboMap::const_iterator it = rACmap.find(CAction::CName(AH_MACRO_EXEC,toString(i).c_str()));
		if (it != rACmap.end())
		{
			pAM->removeCombo(it->second);
		}
	}

	// Remove category
	pAM->removeCategory("MacroCat");
}

// ------------------------------------------------------------------------------------------------
// Build the parameters on the action manager
void CMacroCmdManager::addActionManagerEntries()
{
	CActionsManager *pAM = ActionManagers[0];

	// Create new category
	CCategory cat;

	cat.LocalizedName = "uiMacroCat";
	cat.Name = "MacroCat";

	CBaseAction a;
	a.Name = AH_MACRO_EXEC;
	a.LocalizedName = "uiMacroExec";
	a.Repeat = false;
	a.Contexts = "game";

	CBaseAction::CParameter p;
	p.Type = CBaseAction::CParameter::Constant;
	p.Name = "";
	p.LocalizedName = "uiMacroName";

	uint i;
	for (i=0; i < _Macros.size(); ++i)
	{
		CBaseAction::CParameter::CValue v;
		v.LocalizedValue = _Macros[i].Name;
		v.Value = toString(i);
		v.Contexts = "game";
		p.Values.push_back(v);
	}
	a.Parameters.push_back(p);
	cat.BaseActions.push_back(a);
	cat.Macroisable= false;	// not macroisable (do not allow recurs)
	pAM->addCategory(cat);

	// Add the combos
	for (i=0; i < _Macros.size(); ++i)
	{
		if (_Macros[i].Combo.Key != KeyCount)
		{
			pAM->addCombo(CAction::CName(AH_MACRO_EXEC,toString(i).c_str()), _Macros[i].Combo);
		}
	}
}

// ------------------------------------------------------------------------------------------------
// execute a macro
void CMacroCmdManager::execute(uint nMacronb)
{
	if (_CurExecMac != -1) return; // Cannot execute a macro while last macro is not finished
	_CurExecMac = nMacronb;
	_CurExecCmd = 0;
	_CurExecCmdWait = -1;
	updateMacroExecution();
}

// ------------------------------------------------------------------------------------------------
// execute a macro from an ID of macro
void CMacroCmdManager::executeID (sint32 nMacroID)
{
	// Look for the macro
	for (uint i = 0; i < _Macros.size(); ++i)
	{
		if (_Macros[i].ID == nMacroID)
		{
			execute(i);
			return;
		}
	}
	nlwarning("Macro (ID:%d) not found", nMacroID);
}

// ------------------------------------------------------------------------------------------------
const CMacroCmd	*CMacroCmdManager::getMacroFromMacroID(sint32 nMacroID)
{
	// Look for the macro
	for (uint i = 0; i < _Macros.size(); ++i)
	{
		if (_Macros[i].ID == nMacroID)
		{
			return &_Macros[i];
		}
	}
	nlwarning("Macro (ID:%d) not found", nMacroID);
	return NULL;
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::updateMacroExecution ()
{
	if (_CurExecMac != -1)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CMacroCmd &rMC = _Macros[_CurExecMac];

		for(;;)
		{
			// Check if current command is waiting for finish
			if (_CurExecCmdWait != -1)
			{
				// Is the command ended ?
				if (_CurExecCmdWaitEnded)
				{
					_CurExecCmdWaitEnded = false;
					// Yes ! ok lets process next command
					_CurExecCmdWait = -1;
					_CurExecCmd++;
					if (_CurExecCmd == (sint32)rMC.Commands.size())
					{
						_CurExecMac = -1;
						break;
					}
				}
				else
				{
					// The command has not ended : Do not continue executing anything !
					break;
				}
			}

			CMacroCmd::CCommand &rC = rMC.Commands[_CurExecCmd];

			// If this is a 'waitforserver' action command that we have to wait for
			bool bWaitForServer = false;
			for (uint j = 0; j < ActionManagers.size(); ++j)
			{
				CAction::CName c(rC.Name.c_str(), rC.Params.c_str());
				const CBaseAction *pBA = ActionManagers[j]->getBaseAction(c);
				if (pBA != NULL)
				{
					if (pBA->WaitForServer)
						bWaitForServer = true;
					break;
				}
			}

			// Here we have to execute the current command
			CAHManager::getInstance()->runActionHandler(rC.Name, NULL, rC.Params);

			// Flush interface links (else bug with Macro "Select ShortCutBar/Run Shortcut"
			IngameDbMngr.flushObserverCalls();
			NLGUI::CDBManager::getInstance()->flushObserverCalls();

			if (bWaitForServer)
			{
				_CurExecCmdWait = _CurExecCmd;
				_CurExecCmdWaitId = _ActionId;
				_CurExecCmdWaitEnded = false;
				break;
			}

			// Next Command
			_CurExecCmd++;
			if (_CurExecCmd == (sint32)rMC.Commands.size())
			{
				_CurExecMac = -1;
				break;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::receiveActionEnd(uint8 actionId)
{
	if (_CurExecCmd != -1)
		if (_CurExecCmdWait != -1)
			if (_CurExecCmdWaitId == actionId)
			{
				_CurExecCmdWaitEnded = true;
			}
}

// ------------------------------------------------------------------------------------------------
void CMacroCmdManager::refreshAllKeyDisplays()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// Refresh Key Window
	CAHManager::getInstance()->runActionHandler("keys_open", NULL);
	// Refresh gestion_windows container
	CAHManager::getInstance()->runActionHandler("gestion_windows_update_key_binding", NULL);
}

// ------------------------------------------------------------------------------------------------
class CHandlerMacroExec : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		uint macro;
		fromString(Params, macro);
		pMCM->execute (macro);
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacroExec, AH_MACRO_EXEC);

// ------------------------------------------------------------------------------------------------
class CHandlerMacroRecActEnd : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		pMCM->receiveActionEnd((uint8)NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_NUMBER")->getValue64());
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacroRecActEnd, "macro_receive_action_end");


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// MACRO INTERFACE MACRO INTERFACE MACRO INTERFACE MACRO INTERFACE MACRO
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
// macro_icon_creation CONTAINER
// ***************************************************************************

// ***************************************************************************
class	CHandlerSetMacroBack : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sTP = getParam(Params, "target");
		if (sTP.empty()) return;
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sTP));
		sint32 nb;
		fromString(getParam(Params, "value"), nb);
		if (pCS != NULL) pCS->setMacroBack((uint8)nb);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetMacroBack, "set_macro_back");

// ***************************************************************************
class	CHandlerSetMacroIcon : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sTP = getParam(Params, "target");
		if (sTP.empty()) return;
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sTP));
		sint32 nb;
		fromString(getParam(Params, "value"), nb);
		if (pCS != NULL) pCS->setMacroIcon((uint8)nb);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetMacroIcon, "set_macro_icon");

// ***************************************************************************
class	CHandlerSetMacroOver : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sTP = getParam(Params, "target");
		if (sTP.empty()) return;
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sTP));
		sint32 nb;
		fromString(getParam(Params, "value"), nb);
		if (pCS != NULL) pCS->setMacroOver((uint8)nb);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetMacroOver, "set_macro_over");

// ***************************************************************************
class	CHandlerEBUpdateMacroText: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(pCaller);
		if (pEB == NULL) return;

		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(Params));
		if (pCS == NULL) return;
		pCS->setMacroText(pEB->getInputStringAsStdString());
	}
};
REGISTER_ACTION_HANDLER( CHandlerEBUpdateMacroText, "eb_update_macro_text");

// ***************************************************************************
// Called when we click on the ok button from the macro_icon_creation container
class	CHandlerMacroIconCreation : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_MACROICONCREATION_ICON));
		if (pCS != NULL) pCS->writeToMacro(pMCM->CurrentEditMacro);

		pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_NEWMACRO_ICON));
		if (pCS != NULL) pCS->readFromMacro(pMCM->CurrentEditMacro);

		CWidgetManager::getInstance()->disableModalWindow();
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacroIconCreation, "macro_icon_creation_ok");

// ***************************************************************************
// Called when the modal macro icon creation opens
class	CHandlerMacroIconCreationOpen : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		CAHManager::getInstance()->runActionHandler("reset_pushed", NULL, string("dblink=")+GROUP_MACROICONCREATION_BACK);
		CAHManager::getInstance()->runActionHandler("reset_pushed", NULL, string("dblink=")+GROUP_MACROICONCREATION_ICON);
		CAHManager::getInstance()->runActionHandler("reset_pushed", NULL, string("dblink=")+GROUP_MACROICONCREATION_OVER);

		uint8 back = pMCM->CurrentEditMacro.BitmapBack;
		if (back != 0xff)
		{
			string sButton = string(GROUP_MACROICONCREATION_BACK) + CTRL_MACROICONCREATION_BUTTON + toString(back+1);
			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(sButton));
			if (pCB != NULL) pCB->setPushed(true);
		}

		uint8 icon = pMCM->CurrentEditMacro.BitmapIcon;
		if (icon != 0xff)
		{
			string sButton = string(GROUP_MACROICONCREATION_ICON) + CTRL_MACROICONCREATION_BUTTON + toString(icon+1);
			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(sButton));
			if (pCB != NULL) pCB->setPushed(true);
		}

		uint8 over = pMCM->CurrentEditMacro.BitmapOver;
		if (over != 0xff)
		{
			string sButton = string(GROUP_MACROICONCREATION_OVER) + CTRL_MACROICONCREATION_BUTTON + toString(over+1);
			CCtrlBaseButton *pCB = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(sButton));
			if (pCB != NULL) pCB->setPushed(true);
		}

		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(CTRL_MACROICONCREATION_EDITTEXT));
		if (pEB != NULL)
		{
			pEB->setInputStringAsStdString(pMCM->CurrentEditMacro.DispText);
			CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_MACROICONCREATION_ICON));
			if (pCS != NULL)
				pCS->setMacroText(pEB->getInputStringAsStdString());
		}

		CAHManager::getInstance()->runActionHandler("set_macro_back", NULL, string("target=")+CTRL_MACROICONCREATION_ICON+"|value="+toString(back));
		CAHManager::getInstance()->runActionHandler("set_macro_icon", NULL, string("target=")+CTRL_MACROICONCREATION_ICON+"|value="+toString(icon));
		CAHManager::getInstance()->runActionHandler("set_macro_over", NULL, string("target=")+CTRL_MACROICONCREATION_ICON+"|value="+toString(over));
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacroIconCreationOpen, "macro_icon_creation_open");

// ***************************************************************************
// new_macro CONTAINER
// ***************************************************************************

// ***************************************************************************
sint32 getCmdNbFromId(string id) // copy the string ! (do not change for a const string &)
{
	if (id.rfind(":c") == string::npos) return -1;
	id = id.substr(id.rfind(":c")+2,id.size());
	sint32 ret;
	fromString(id, ret);
	return ret;
}

// ***************************************************************************
// Called from context menu when we right click on a command of the new_macro container
class	CHandlerNewMacroCmdMoveUp: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		sint nCmdNb = getCmdNbFromId(pCaller->getId());
		pMCM->CurrentEditMacro.moveUpCommand(nCmdNb);
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager::getInstance()->EditCmd->deactivate();
		CAHManager::getInstance()->runActionHandler("new_macro_open",NULL);
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroCmdMoveUp, "new_macro_cmd_move_up");

// ***************************************************************************
// Called from context menu when we right click on a command of the new_macro container
class	CHandlerNewMacroCmdMoveDown: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		sint nCmdNb = getCmdNbFromId(pCaller->getId());
		pMCM->CurrentEditMacro.moveDownCommand(nCmdNb);
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager::getInstance()->EditCmd->deactivate();
		CAHManager::getInstance()->runActionHandler("new_macro_open",NULL);
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroCmdMoveDown, "new_macro_cmd_move_down");

// ***************************************************************************
// Called from context menu when we right click on a command of the new_macro container
class	CHandlerNewMacroCmdEdit: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		sint nCmdNb = getCmdNbFromId(pCaller->getId());
		pMCM->EditCmd->activateFrom (pMCM->CurrentEditMacro.Commands[nCmdNb].Name,
									pMCM->CurrentEditMacro.Commands[nCmdNb].Params,
									nCmdNb);
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroCmdEdit, "new_macro_cmd_edit");

// ***************************************************************************
// Called from context menu when we right click on a command of the new_macro container
class	CHandlerNewMacroCmdDelete: public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		sint nCmdNb = getCmdNbFromId(pCaller->getId());
		pMCM->CurrentEditMacro.delCommand(nCmdNb);
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager::getInstance()->EditCmd->deactivate();
		CAHManager::getInstance()->runActionHandler("new_macro_open",NULL);
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroCmdDelete, "new_macro_cmd_delete");

// ***************************************************************************
void addCommandLine (CGroupList *pParent, uint cmdNb, const ucstring &cmdName)
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();

	vector< pair<string, string> > vParams;
	vParams.push_back(pair<string,string>("id", "c"+toString(cmdNb)));
	CInterfaceGroup *pNewCmd = CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_NEWMACRO_COMMAND, pParent->getId(), vParams);
	if (pNewCmd == NULL) return;

	CViewText *pVT = dynamic_cast<CViewText*>(pNewCmd->getView(TEMPLATE_NEWMACRO_COMMAND_TEXT));
	if (pVT != NULL) pVT->setText(cmdName);

	pNewCmd->setParent (pParent);
	pParent->addChild (pNewCmd);
}

// ***************************************************************************
// Called when we push the new command button on the new_macro container
class	CHandlerNewMacroNewCmd: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("new_macro_enter_name",NULL);
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		pMCM->EditCmd->activate();
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroNewCmd, "new_macro_new_cmd");

// ***************************************************************************
// Called when we enter the name of the current macro
class	CHandlerNewMacroEnterName : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (pCaller == NULL)
			pCaller = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:new_macro:content:edit_name"));
		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(pCaller);
		if (pEB == NULL) return;

		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		pMCM->CurrentEditMacro.Name = pEB->getInputStringAsStdString();
		if ((pMCM->CurrentEditMacro.Name.size() >= 2) &&
			(pMCM->CurrentEditMacro.Name[0] == 'u') && (pMCM->CurrentEditMacro.Name[1] == 'i'))
			pMCM->CurrentEditMacro.Name[0] = 'U';
		pEB->setInputString(pMCM->CurrentEditMacro.Name);
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroEnterName, "new_macro_enter_name");

// ***************************************************************************
// Called when the new_macro container opens
class	CHandlerNewMacroOpen : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Init 'new_macro' container from the global current macro (gCurrentEditMacro)
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		// Icon
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_NEWMACRO_ICON));
		if (pCS != NULL) pCS->readFromMacro(pMCM->CurrentEditMacro);
		// Name
		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(GROUP_NEWMACRO_EDIT_NAME));
		if (pEB != NULL) pEB->setInputString(pMCM->CurrentEditMacro.Name);
		// Commands
		CGroupList *pList = dynamic_cast<CGroupList*>(CWidgetManager::getInstance()->getElementFromId(GROUP_NEWMACRO_COMMANDS));
		if (pList == NULL) return;
		// Key Shortcut
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_NEWMACRO_KEY));
		if (pVT != NULL)
		{
			if (pMCM->CurrentEditMacro.Combo.Key == KeyCount)
				pVT->setText(CI18N::get(VIEW_EDITCMD_TEXT_KEY_DEFAULT));
			else
				pVT->setText(pMCM->CurrentEditMacro.Combo.toUCString());
		}

		pList->clearGroups();
		pList->setDynamicDisplaySize(true);

		for (uint i = 0; i < pMCM->CurrentEditMacro.Commands.size(); ++i)
		{
			ucstring commandName;
			for (uint j = 0; j < pMCM->ActionManagers.size(); ++j)
			{
				CAction::CName c(pMCM->CurrentEditMacro.Commands[i].Name.c_str(), pMCM->CurrentEditMacro.Commands[i].Params.c_str());
				if (pMCM->ActionManagers[j]->getBaseAction(c) != NULL)
				{
					commandName = pMCM->ActionManagers[j]->getBaseAction(c)->getActionLocalizedText(c);
					break;
				}
			}

			addCommandLine(pList, i, commandName);
		}
		pMCM->EditCmd->CurAM = pMCM->ActionManagers[0];
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroOpen, "new_macro_open");

// ***************************************************************************
// Called when we click on ok button of the new_macro container
class	CHandlerNewMacroOk : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		CSPhraseManager	*pPM = CSPhraseManager::getInstance();

		// Validate name
		CAHManager::getInstance()->runActionHandler("new_macro_enter_name",NULL);

		// Check if macro has more than one command
		if (pMCM->CurrentEditMacro.Commands.size() == 0) return;

		// Add a macro
		if (pMCM->CurrentEditMacroNb != -1)
		{
			// retrieve the Id of the edited macro, to keep the same id in addMacro()
			pMCM->CurrentEditMacro.ID= pMCM->getMacros()[pMCM->CurrentEditMacroNb].ID;
			pMCM->delMacro(pMCM->CurrentEditMacroNb);
			pMCM->addMacro(pMCM->CurrentEditMacro,pMCM->CurrentEditMacroNb);
			// update the Memory Manager
			pPM->updateMacroShortcuts(pMCM->CurrentEditMacro.ID);
			// reset id of EditMacro
			pMCM->CurrentEditMacro.ID= -1;
		}
		else
		{
			pMCM->addMacro(pMCM->CurrentEditMacro);
		}

		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_NEWMACRO));
		if (pGC != NULL) pGC->setActive (false);

		CAHManager::getInstance()->runActionHandler("macros_open",NULL);

		// Refresh key containers
		pMCM->refreshAllKeyDisplays();
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewMacroOk, "new_macro_ok");

// ***************************************************************************
// macros CONTAINER
// ***************************************************************************

// ***************************************************************************
sint32 getMacroFromId(string id) // Do not change for a const string &
{
	string::size_type npos = id.rfind(":m");
	if (npos == string::npos) return -1;
	id = id.substr(npos+2,id.size());
	sint32 ret;
	fromString(id, ret);
	return ret;
}

// ***************************************************************************
void addMacroLine (CGroupList *pParent, uint macNb, const CMacroCmd &macro)
{
	CInterfaceManager	*pIM = CInterfaceManager::getInstance();

	vector< pair<string, string> > vParams;
	vParams.push_back(pair<string,string>("id", "m"+toString(macNb)));
	CInterfaceGroup *pNewMacro = CWidgetManager::getInstance()->getParser()->createGroupInstance(TEMPLATE_MACRO_ELT, pParent->getId(), vParams);
	if (pNewMacro == NULL) return;

	CViewText *pVT = dynamic_cast<CViewText*>(pNewMacro->getView(TEMPLATE_MACRO_ELT_TEXT));
	if (pVT != NULL) pVT->setText(macro.Name);

	CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(pNewMacro->getCtrl(TEMPLATE_MACRO_ELT_ICON));
	if (pCS != NULL) pCS->readFromMacro(macro);

	pVT = dynamic_cast<CViewText*>(pNewMacro->getView(TEMPLATE_MACRO_ELT_KEYTEXT));
	if (pVT != NULL)
	{
		if (macro.Combo.Key != KeyCount)
			pVT->setText(macro.Combo.toUCString());
		else
			pVT->setText(CI18N::get(VIEW_EDITCMD_TEXT_KEY_DEFAULT));
	}

	pNewMacro->setParent (pParent);
	pParent->addChild (pNewMacro);
}
// ***************************************************************************
// Called when the macros container opens
class	CHandlerMacrosOpen : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Init 'macros' container from the macro manager
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();

		CGroupList *pList = dynamic_cast<CGroupList*>(CWidgetManager::getInstance()->getElementFromId(WIN_MACRO_CONTENT));
		if (pList == NULL) return;

		pList->clearGroups();
		pList->setDynamicDisplaySize(true);

		// Refresh the shortcut key that can be changed in the keys container
		pMCM->refreshMacroCombo();

		// Add all macros template
		const vector<CMacroCmd> &vM = pMCM->getMacros();
		for (uint i = 0; i < vM.size(); ++i)
		{
			addMacroLine(pList, i, vM[i]);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosOpen, "macros_open");

// ***************************************************************************
// Called when we click the new macro button on the macros container
class	CHandlerMacrosNewMacro : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		// Reinit the new_macro container and variables
		CMacroCmd mc;
		mc.Name = "NewMacro";
		mc.BitmapBack = 0;
		mc.BitmapIcon = 0;
		mc.BitmapOver = 0;
		pMCM->CurrentEditMacro = mc;

		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_NEWMACRO));
		if (pGC != NULL)
		{
			pGC->setTitle(NEWMACRO_TITLE_NEW);
			pGC->setActive (false);
			pGC->setActive (true);
		}

		pMCM->CurrentEditMacroNb = -1;
		pMCM->EditCmd->deactivate();
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosNewMacro, "macros_new_macro");

// ***************************************************************************
// Called from context menu on a macro
class	CHandlerMacrosExec : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		sint nMacNb = getMacroFromId(pCaller->getId());

		CAHManager::getInstance()->runActionHandler(AH_MACRO_EXEC,pCaller,toString(nMacNb));
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosExec, "macros_exec");

// ***************************************************************************
// Called from context menu on a macro
class	CHandlerMacrosEdit : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		sint nMacNb = getMacroFromId(pCaller->getId());
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();
		pMCM->CurrentEditMacro = pMCM->getMacros()[nMacNb];
		pMCM->CurrentEditMacroNb = nMacNb;
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_NEWMACRO));
		if (pGC != NULL)
		{
			pGC->setTitle(NEWMACRO_TITLE_EDIT);
			pGC->setActive (false);
			pGC->setActive (true);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosEdit, "macros_edit");

// ***************************************************************************
// Called from context menu on a macro
class	CHandlerMacrosDel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		// build params string		
		sint nMacNb = getMacroFromId(pCaller->getId());
		string Params("MacroId=" + toString(nMacNb));

		// Ask if ok before calling action handler to delete macro
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQDeleteMacro"), "macros_do_del", Params);
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosDel, "macros_del");

// ***************************************************************************
// Called after the player confirmed he wants to delete the macro
class	CHandlerMacrosDoDel : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /*pCaller*/, const string & Params)
	{
		CSPhraseManager	*pPM = CSPhraseManager::getInstance();		
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CMacroCmdManager *pMCM = CMacroCmdManager::getInstance();		

		// get params
		sint nMacNb;
		fromString(getParam(Params, "MacroId"), nMacNb);
		// update the TB Manager
		pPM->deleteMacroShortcuts(pMCM->getMacros()[nMacNb].ID);
		// then delete
		pMCM->delMacro(nMacNb);
		CAHManager::getInstance()->runActionHandler("macros_open",NULL);

		// Refresh key containers
		pMCM->refreshAllKeyDisplays();
	}
};
REGISTER_ACTION_HANDLER( CHandlerMacrosDoDel, "macros_do_del");

