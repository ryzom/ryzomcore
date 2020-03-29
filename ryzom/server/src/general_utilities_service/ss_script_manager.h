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


#ifndef SS_SCRIPT_MANAGER_H
#define SS_SCRIPT_MANAGER_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"

// local
#include "ss_script_manager.h"
#include "ss_command_executor.h"


//-------------------------------------------------------------------------------------------------
// class CScriptManager
//-------------------------------------------------------------------------------------------------

class CScriptManager
{
private:
	// this is a singleton so make ctor private
	CScriptManager();

public:
	// singleton accessor
	static CScriptManager* getInstance();

public:
	// clear the set of loaded scripts from RAM
	void clear();

	// read the named script file
	bool readScriptFile(const std::string& fileName);

	// display the contents of the script manager
	void display() const;

	// add a line to a named script
	void addScriptLine(const std::string& scriptName,const std::string& line);

	// run the named script (if it exists)
	void runScript(const std::string& scriptName,uint32 delay=0) const;

private:
	// private data
	typedef std::vector<NLMISC::CSString> TScriptLines;
	typedef std::map<NLMISC::CSString,TScriptLines> TScripts;

	TScripts _Scripts;
};


//-------------------------------------------------------------------------------------------------
#endif
