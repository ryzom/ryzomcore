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

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include <stdio.h>

using namespace NLMISC;

int main(int argc, char* argv[])
{
	if (argc <3)
	{
		printf ("lock [filein] [fleout]\n\t");
	}
	else
	{
		// Random
		srand ((int)CTime::getLocalTime ());

		// Open the filein
		while (1)
		{
			// Rename the file
			if (rename(argv[1], argv[2]) == 0)
			{
				return 1;
			}
			else
			{
				// Sleep..
				printf ("Locked. Waiting for lock to be released\n");
				uint sleepTime = (rand () * 5000) / RAND_MAX;
				nlSleep (sleepTime);
			}
		}

		// Open the fileout
		FILE *fileOut = fopen (argv[2], "w");
	}

	return 0;
}
