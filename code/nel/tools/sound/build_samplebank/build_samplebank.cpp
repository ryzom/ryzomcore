// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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
	if (nNbArg < 3)
	{
		nlinfo("ERROR : Wrong number of arguments\n");
		nlinfo("USAGE : build_samplebank  <source_samplebank> <build_samplebank> <samplebank_name>\n");
		return EXIT_FAILURE;
	}
	string samplebanksDir = string(ppArgs[1]);
	if (!CFile::isDirectory(samplebanksDir))
	{
		nlerrornoex("Directory source_samplebank '%s' does not exist", samplebanksDir.c_str());
		return EXIT_FAILURE;
	}
	string buildSampleBanksDir = string(ppArgs[2]);
	if (!CFile::isDirectory(buildSampleBanksDir))
	{
		nlerrornoex("Directory build_samplebank '%s' does not exist", buildSampleBanksDir.c_str());
		return EXIT_FAILURE;
	}
	string sampleBankName = string(ppArgs[3]);
	
	// build the sample bank
	UAudioMixer::buildSampleBank(samplebanksDir, buildSampleBanksDir, sampleBankName);
	
	// and that's all folks
	return EXIT_SUCCESS;
}

/* end of file */
