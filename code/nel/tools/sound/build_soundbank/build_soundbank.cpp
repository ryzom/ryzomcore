/**
 * Build Soundbank
 * $Id: build_soundbank.cpp 2320 2010-03-06 23:04:11Z kaetemi $
 * \file build_soundbank.cpp
 * \brief Build Soundbank
 * \date 2010-03-06 21:43GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2010  by authors
 * 
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License,
 * or (at your option) any later version.
 * 
 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/log.h>
#include <nel/misc/path.h>
#include <nel/sound/u_audio_mixer.h>

// Project includes
// ...

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace {

} /* anonymous namespace */

int main(int nNbArg, char **ppArgs)
{
	// create debug stuff
	createDebug();

	// verify all params
	if (nNbArg < 4)
	{
		nlinfo("ERROR : Wrong number of arguments\n");
		nlinfo("USAGE : build_soundbank  <leveldesign> <dfn> <build_packed_sheets>\n");
		return EXIT_FAILURE;
	}
	string leveldesignDir = string(ppArgs[1]);
	if (!CFile::isDirectory(leveldesignDir))
	{
		nlerrornoex("Directory leveldesign '%s' does not exist", leveldesignDir.c_str());
		return EXIT_FAILURE;
	}
	string dfnDir = string(ppArgs[2]);
	if (!CFile::isDirectory(dfnDir))
	{
		nlerrornoex("Directory dfn '%s' does not exist", dfnDir.c_str());
		return EXIT_FAILURE;
	}
	string exportDir = string(ppArgs[3]);
	if (!CFile::isDirectory(exportDir))
	{
		nlerrornoex("Directory build_packed_sheets '%s' does not exist", exportDir.c_str());
		return EXIT_FAILURE;
	}
	
	// add search paths
	CPath::addSearchPath(leveldesignDir, true, false);
	CPath::addSearchPath(dfnDir, true, false);

	// build the sound bank
	UAudioMixer::buildSoundBank(exportDir);
	
	// and that's all folks
	return EXIT_SUCCESS;
}

/* end of file */
