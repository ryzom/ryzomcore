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



#ifndef CL_COMMANDS_H
#define CL_COMMANDS_H

#include "nel/misc/types_nl.h"


/// Function to release all things allocated for commands.
void releaseCommands();

/**
  * User command class. This NeL command is binded to an action handler with the commands.xml file.
  */
class CUserCommand : public NLMISC::ICommand
{
public:
	// Mode
	class CMode
	{
	public:
		uint					KeywordsCount;
		std::string				Action;
		std::vector<std::string>		Keywords;
	};

	CUserCommand (const std::string &commandName, const ucstring &help, const ucstring &argsHelp);

	void addMode (const std::string &action, uint numArg, bool infiniteAgr, const std::vector<std::string> &keywords);

	// From ICommand
	virtual bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human);

	// Create a command
	static void createCommand (const char *name, const char *action, const char *params);

	// release memory
	static void release();

	// The command map
	static std::map<std::string, CUserCommand*> CommandMap;

	// Strings
	CMode									InfiniteMode;
	std::map<uint, CMode>					FixedArgModes;
	std::string	CommandName;
	std::string	Params;
};

#endif // CL_COMMANDS_H

/* End of console.h */
