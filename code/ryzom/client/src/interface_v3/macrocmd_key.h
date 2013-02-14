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



#ifndef RZ_MACROCMD_KEY_H
#define RZ_MACROCMD_KEY_H

#include "../actions_client.h"

namespace NLGUI
{
	class CGroupContainer;
}

// ***************************************************************************
#define WIN_EDITCMD_TITLE_NEW_KEY			"uiNewKey"
#define WIN_EDITCMD_TITLE_EDIT_KEY			"uiEditKey"
#define WIN_EDITCMD_TITLE_NEW_CMD			"uiNewCmd"
#define WIN_EDITCMD_TITLE_EDIT_CMD			"uiEditCmd"

// ***************************************************************************
class CModalContainerEditCmd
{
public:

	CModalContainerEditCmd();
	void create (const std::string &name, bool bDefKey, bool allowAllActions);
	void setTitle (const std::string &uistr); // UI String (uiEditCmd) to be localized

	void deactivate();
	void activate();
	void activateFrom (const std::string &cmdName, const std::string &cmdParams, sint nRef);

	void onChangeCategory(); // Called by the AH
	void onChangeAction();
	void checkCurrentCommandValidity();

public:
	NLGUI::CGroupContainer *Win;
	std::string WinName;
	bool CanDefineKey;

	// Used by all_keys container to handle deletion and the like
	struct CLine
	{
		uint8			ComboAM;	// action manager of the line
		CCombo			Combo;		// combo of the line (KeyCount if not associated)
		CAction::CName	ActionName;
	};
	std::vector<CLine> AllLines;

	CActionsManager *CurAM;				// Current Action Manager When Editing
	CLine	CurrentEditCmdLine;			// The current combo/action line edited
	std::vector<std::string> CurrentEditCmdCategories;

	sint32 CurrentEditCmdNb; // -1 -> create

	std::string DbComboSelCat;
	std::string DbComboSelAct;
	std::string DbComboSel1P;
	std::string DbComboSel2P;
	std::string DbComboDisp1P;
	std::string DbComboDisp2P;

private:
	// allow all actions? not only macroisable one?
	bool	_AllowAllActions;

	bool getCatIndexAndAM (sint32 nInCatIndex, sint32 &nOutCatIndex, CActionsManager*&nOutAM);
	bool getBaseActionIndex(const CCategory &cat, sint32 nInActIndex, sint32 &nOutActIndex);
	void invalidCurrentCommand();
	void validCurrentCommand();
	bool isParamValid (sint32 nParamIndex);
};

// ***************************************************************************
// Interface Part
// ***************************************************************************

#define TEMPLATE_EDITCMD	"teditcmd"

#define WIN_EDITCMD_TITLE						":header_opened:title"

#define	WIN_EDITCMD_COMBO_CATEGORY				":content:combo_cat"
#define WIN_EDITCMD_COMBO_ACTION				":content:combo_action"
#define WIN_EDITCMD_COMBO_FIRST_PARAM_LIST		":content:first_param_list"
#define WIN_EDITCMD_COMBO_FIRST_PARAM_EDITBOX	":content:first_param_editbox:eb"
#define WIN_EDITCMD_COMBO_SECOND_PARAM_LIST		":content:second_param_list"
#define WIN_EDITCMD_COMBO_SECOND_PARAM_EDITBOX	":content:second_param_editbox:eb"

#define VIEW_EDITCMD_FIRST_PARAM_NAME			":content:first_param_name"
#define VIEW_EDITCMD_SECOND_PARAM_NAME			":content:second_param_name"

#define CTRL_EDITCMD_BUTTON_OK					":content:ok_cancel:ok"
#define CTRL_EDITCMD_KEY_GROUP					":content:keygrp"
#define CTRL_EDITCMD_BUTTON_KEY					":content:keygrp:butkey"
#define VIEW_EDITCMD_TEXT_KEY					":content:keygrp:txtkey"
#define VIEW_EDITCMD_TEXT_KEY_DEFAULT			"uiEditCmdNone"

#define DB_EDITCMD_PREFIX						"UI:TEMP"
#define DB_EDITCMD_COMBO_SELECT_CATEGORY		"COMBO_SELECT_CATEGORY"
#define DB_EDITCMD_COMBO_SELECT_ACTION			"COMBO_SELECT_ACTION"
#define DB_EDITCMD_COMBO_SELECT_FIRST_PARAM		"COMBO_SELECT_FIRST_PARAM"
#define DB_EDITCMD_COMBO_SELECT_SECOND_PARAM	"COMBO_SELECT_SECOND_PARAM"
// display (-1,none)(0,list)(1,editbox)
#define DB_EDITCMD_COMBO_DISPLAY_FIRST_PARAM	"DISPLAY_FIRST_PARAM"
#define DB_EDITCMD_COMBO_DISPLAY_SECOND_PARAM	"DISPLAY_SECOND_PARAM"


#endif // RZ_MACROCMD_KEY_H
