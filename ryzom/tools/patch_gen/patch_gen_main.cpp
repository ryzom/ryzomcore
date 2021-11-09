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

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/sstring.h"
//#include "game_share/handy_commands.h"

//-----------------------------------------------
//	MAIN
//-----------------------------------------------
int main(int argc,char** argv)
{
	NLMISC::createDebug();

	// if there are no command line args then display a friendly message and exit
	if (argc==1)
	{
		NLMISC::InfoLog->displayNL("SYNTAX: %s <command line>",argv[0]);
		NLMISC::InfoLog->displayNL("");
		NLMISC::InfoLog->displayNL("For a list of valid commands and their paramaters try: %s help",argv[0]);
		NLMISC::InfoLog->displayNL("");
		return 0;
	}

	// build the command line by concatnating input arguments (separating with spaces)
	NLMISC::CSString commandline;
	for (int i=1;i<argc;++i)
	{
		NLMISC::CSString s= argv[i];

		// check whether the argument needs to be quote encapsulated
		if (s.contains(' ') && s[0]!='\"')
			s= "\""+ s+ "\"";

		if (i>1)
			commandline+=' ';
		commandline+= s;
	}

	// have NeL treat the command
	NLMISC::ICommand::execute(commandline, *NLMISC::InfoLog);

	return 0;
}
