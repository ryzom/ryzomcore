// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2012  by authors
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"

using namespace NLMISC;

int main (int argc, char **argv)
{
	for (uint i = 0; i < 8; ++i)
	{
		CTime::CTimerInfo timerInfo;
		CTime::probeTimerInfo(timerInfo);
	}

	printf ("\nPress <return> to exit\n");
	getchar ();

	return EXIT_SUCCESS;
}
