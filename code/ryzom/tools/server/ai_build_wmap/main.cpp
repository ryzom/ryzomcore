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



#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/config_file.h"

#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

extern string				OutputPath;
extern vector<string>		PacsPrimPath;
extern vector<string>		LookupPath;
extern vector<string>		LookupNoRecursePath;

sint	main(sint argc, char **argv)
{
	createDebug();

	CConfigFile			cf;
	CConfigFile::CVar	*var;

	cf.load(string("ai_build_wmap.cfg"));

	//
	var = cf.getVarPtr("Paths");
	if (var != NULL)
	{
		for (uint i=0; i < var->size(); ++i)
			LookupPath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("NoRecursePaths");
	if (var != NULL)
	{
		for (uint i=0; i < var->size(); ++i)
			LookupNoRecursePath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("PacsPrimPaths");
	if (var != NULL)
	{
		for (uint i=0; i < var->size(); ++i)
			PacsPrimPath.push_back(var->asString(i));
	}

	var = cf.getVarPtr("OutputPath");
	if (var != NULL)
	{
		OutputPath = CPath::standardizePath(var->asString());
	}


	vector<string>	commands;
	var = cf.getVarPtr("Commands");
	if (var != NULL)
	{
		for (uint i=0; i < var->size(); ++i)
			commands.push_back(var->asString(i));
	}

	string	cmd;
	for (sint i=1; i < argc; ++i)
	{
		if (string(argv[i]) == string("-"))
		{
			if (cmd != "")
			{
				commands.push_back(cmd);
				cmd = "";
			}
		}
		else
		{
			if (cmd != "")
				cmd += ' ';
			cmd += argv[i];
		}
	}

	if (cmd != "")
		commands.push_back(cmd);

	NLMISC::createDebug ();
	nlinfo("Running commands:");

	for (uint i=0; i<commands.size(); ++i)
		ICommand::execute(commands[i], *InfoLog);

	return 0;
}
