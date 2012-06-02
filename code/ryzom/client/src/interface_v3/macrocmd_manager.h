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



#ifndef RZ_MACROCMD_MANAGER_H
#define RZ_MACROCMD_MANAGER_H

#include "nel/misc/stream.h"
#include "../actions.h"

// ***************************************************************************
class CActionsManager;
class CModalContainerEditCmd;

// ***************************************************************************
class CMacroCmd
{
public:
	class CCommand // An action handler
	{
	public:
		std::string		Name;
		std::string		Params;

		// ------------------------------
		CCommand() { }

		CCommand(const char *name)
		{
			Name = name;
		}

		CCommand(const char *name, const char *params)
		{
			Name = name;
			Params = params;
		}

		void serial (NLMISC::IStream &f);
	};

	// All commands
	std::vector<CCommand> Commands;

	CCombo Combo;

	// Display bitmap form
	uint8 BitmapBack;	// 0xFF = Pas de Back
	uint8 BitmapIcon;	// 0xFF = Pas de Ico
	uint8 BitmapOver;	// 0xFF = Pas de Over
	std::string DispText;	// Simple text (A-Z,0-9)

	// Display strings form
	std::string Name;

	// The macro id
	sint32 ID;

	// ------------------------------
public:
	CMacroCmd();

	void addCommand (const std::string &sName, const std::string &sParam, sint32 nPos=-1);
	void delCommand (uint cmdNb);
	void moveUpCommand (uint cmdNb);
	void moveDownCommand (uint cmdNb);

	void writeTo (xmlNodePtr node) const;
	bool readFrom (xmlNodePtr node);
};

// ***************************************************************************
class CMacroCmdManager
{
public:

	static CMacroCmdManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CMacroCmdManager;
		return _Instance;
	}

	// release singleton
	static void releaseInstance();

	void initInGame();
	void uninitInGame();

	const std::vector<CMacroCmd> &getMacros() { return _Macros; }
	void addMacro (const CMacroCmd &m, sint32 nPos=-1);
	void delMacro(sint32 nMacNb);
	void removeAllMacros();

	void refreshMacroCombo();

	sint32 getTexIdBack(uint8 nb) { return (nb>=_OptBackId.size()) ? _OptBackId[_OptBackId.size()-1] : _OptBackId[nb]; }
	sint32 getTexIdIcon(uint8 nb) { return (nb>=_OptIconId.size()) ? _OptIconId[_OptIconId.size()-1] : _OptIconId[nb]; }
	sint32 getTexIdOver(uint8 nb) { return (nb>=_OptOverId.size()) ? _OptOverId[_OptOverId.size()-1] : _OptOverId[nb]; }

	void execute (uint nMacroNb);
	void executeID (sint32 nMacroID);
	void updateMacroExecution ();
	void receiveActionEnd(uint8 actionId);

	uint32 getActionId() { return _ActionId; }
	void incActionId() { _ActionId++; }

	const CMacroCmd	*getMacroFromMacroID(sint32 id);

	// Call This method when keys changed (delete, edit, etc...)
	void	refreshAllKeyDisplays();

public:
	std::vector<CActionsManager*> ActionManagers;

	CMacroCmd CurrentEditMacro;
	sint32 CurrentEditMacroNb; // -1 -> create;

	CModalContainerEditCmd *EditCmd;
	CModalContainerEditCmd *NewKey;

protected:

	std::vector<CMacroCmd> _Macros;

	std::vector<sint32>	_OptBackId; // All the back bitmap possible
	std::vector<sint32>	_OptIconId; // All the icon bitmap possible
	std::vector<sint32>	_OptOverId; // All the over bitmap possible

	sint32 _CurExecMac; // Current macro in execution
	sint32 _CurExecCmd;
	sint32 _CurExecCmdWait;
	sint32 _CurExecCmdWaitId;
	bool _CurExecCmdWaitEnded;

	uint8 _ActionId;

	sint32 _MacroIDGenerator;
private:

	static CMacroCmdManager *_Instance;

	CMacroCmdManager();
	~CMacroCmdManager();
	void delActionManagerEntries();
	void addActionManagerEntries();

};

// ***************************************************************************
// Interface Part
// ***************************************************************************

#define AH_MACRO_EXEC							"macro_exec"

#define WIN_MACRO								"ui:interface:macros"
#define WIN_MACRO_CONTENT						"ui:interface:macros:content:macros:list"
#define	TEMPLATE_MACRO_ELT						"element_macro"
#define	TEMPLATE_MACRO_ELT_TEXT					"macro_name"
#define	TEMPLATE_MACRO_ELT_ICON					"icon"
#define	TEMPLATE_MACRO_ELT_KEYTEXT				"macro_key"


#define WIN_NEWMACRO							"ui:interface:new_macro"
#define	VIEW_NEWMACRO_TITLE						"ui:interface:new_macro:header_opened:title"
#define	NEWMACRO_TITLE_NEW						"uiNewMacro"
#define	NEWMACRO_TITLE_EDIT						"uiEditMacro"
#define CTRL_NEWMACRO_ICON						"ui:interface:new_macro:content:icon"
#define GROUP_NEWMACRO_EDIT_NAME				"ui:interface:new_macro:content:edit_name"
#define GROUP_NEWMACRO_COMMANDS					"ui:interface:new_macro:content:cmds"
#define	VIEW_NEWMACRO_KEY						"ui:interface:new_macro:content:macro_key"
#define TEMPLATE_NEWMACRO_COMMAND				"macro_command"
#define TEMPLATE_NEWMACRO_COMMAND_TEXT			"macro_name"

#define WIN_MACROICONCREATION					"ui:interface:macro_icon_creation"
#define CTRL_MACROICONCREATION_ICON				"ui:interface:macro_icon_creation:icon"
#define GROUP_MACROICONCREATION_BACK			"ui:interface:macro_icon_creation:IconBackground"
#define GROUP_MACROICONCREATION_ICON			"ui:interface:macro_icon_creation:IconForeground"
#define GROUP_MACROICONCREATION_OVER			"ui:interface:macro_icon_creation:IconPattern"
#define CTRL_MACROICONCREATION_BUTTON			":but"
#define CTRL_MACROICONCREATION_EDITTEXT			"ui:interface:macro_icon_creation:edit_name"

// ***************************************************************************


#endif // RZ_MACROCMD_MANAGER_H
