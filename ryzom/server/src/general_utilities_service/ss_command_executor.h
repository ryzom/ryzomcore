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


#ifndef SS_COMMAND_EXECUTOR_H
#define SS_COMMAND_EXECUTOR_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/sstring.h"


//-------------------------------------------------------------------------------------------------
// class CCommandExecutor
//-------------------------------------------------------------------------------------------------

class CCommandExecutor
{
private:
	// this is a singleton so make ctor private
	CCommandExecutor();

public:
	// singleton accessor
	static CCommandExecutor* getInstance();

public:
	// add a command to the command list
	void addCommand(NLMISC::CSString command);

	// display the list of pending commands
	void display() const;

	// execute the commands for the next tick
	void tickUpdate();

private:
	// private data
	typedef std::vector<NLMISC::CSString> TCommands;
	TCommands _Commands;
};


//-------------------------------------------------------------------------------------------------
#endif
