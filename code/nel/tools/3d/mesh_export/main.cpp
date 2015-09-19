// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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

#include <nel/misc/types_nl.h>
#include "../mesh_utils/mesh_utils.h"

#include <nel/misc/cmd_args.h>
#include <nel/misc/path.h>

int printHelp(const NLMISC::CCmdArgs &args)
{
	printf("NeL Mesh Export\n");
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	NLMISC::CCmdArgs args;
	args.setArgs(argc, (const char **)argv);

	if (args.getArgs().size() == 1
		|| args.haveArg('h')
		|| args.haveLongArg("help"))
		return printHelp(args);

	const NLMISC::CSString &filePath = args.getArgs().back();
	if (!NLMISC::CFile::fileExists(filePath))
	{
		printHelp(args);
		nlerror("File '%s' does not exist", filePath);
		return EXIT_FAILURE;
	}

	CMeshUtilsSettings settings;
	settings.SourceFilePath = filePath;
	
	settings.DestinationDirectory = args.getArg('d');
	if (settings.DestinationDirectory.empty())
		settings.DestinationDirectory = args.getLongArg("dst");
	if (settings.DestinationDirectory.empty())
		settings.DestinationDirectory = filePath + "_export";
	settings.DestinationDirectory += "/";

	return exportScene(settings);
}

/* end of file */
