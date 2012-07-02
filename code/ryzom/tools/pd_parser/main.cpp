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


#include "parser.h"
#include "templatizer.h"

using namespace std;
using namespace NLMISC;

extern CTemplatizer		Templatizer;

int main(int argc, char **argv)
{
	new CApplicationContext;

	string	inputfile;

	uint	arg;
	for (arg=1; (sint)arg<argc; ++arg)
	{
		if (strcmp(argv[arg], "-hauto") == 0)
		{
			GenerateHAuto = true;
		}
		else if (strcmp(argv[arg], "-dbgmsg") == 0)
		{
			VerboseMode = true;
			GenerateDebugMessages = true;
		}
		else if (strcmp(argv[arg], "-onlylogs") == 0)
		{
			GenerateOnlyLogs = true;
		}
		else
		{
			inputfile = argv[arg];
		}
	}

	if (inputfile == "")
	{
		nlwarning("Error: missing input file(s)");
		exit(EXIT_FAILURE);
	}

/*
	if (argc == 3)
	{
		string			templateFile = argv[2];

		NLMISC::CIFile	f;
		if (!f.open(templateFile))
		{
			nlwarning("Error: failed to open template input file '%s'", templateFile.c_str());
			exit(EXIT_FAILURE);
		}
		else
		{
			uint	size = f.getFileSize();
			char	*buffer = new char[size+1];
			f.serialBuffer((uint8*)buffer, size);
			buffer[size] = '\0';
			f.close();

			if (!Templatizer.build(buffer))
			{
				nlwarning("Error: failed to parse template input file '%s'", templateFile.c_str());
				//exit(EXIT_FAILURE);
			}
		}
	}
	else
	{
		Templatizer.build("root{}");
	}
*/

	Templatizer.build("root{}");

	CParseNode	*main = parse(inputfile);

	GenerateCpp = true;
	//VerboseMode = true;

	if (main)
	{
		main->execute();
		//Templatizer.eval();
	}

	printf("Code generation done");

	return 0;

/*
	if (argc != 2)
	{
		nlwarning("Error: missing input file(s)");
		exit(EXIT_FAILURE);
	}

	if (!Templatizer.build("root{}"))
	{
		exit(EXIT_FAILURE);
	}


	CParseNode	*main = parse(argv[1]);

	GenerateCpp = true;
	VerboseMode = true;

	if (main)
	{
		main->execute();
	}

	return 0;
*/
}
