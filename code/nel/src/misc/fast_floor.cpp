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

#include "stdmisc.h"

#include "nel/misc/fast_floor.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

int		OptFastFloorCWStack[OptFastFloorCWStackSize];
int		*OptFastFloorCWStackEnd = OptFastFloorCWStack + OptFastFloorCWStackSize;
int		*OptFastFloorCWStackPtr = OptFastFloorCWStack;

#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM) && defined(NL_USE_FASTFLOOR)

double	OptFastFloorMagicConst = pow(2.0,52) + pow(2.0,51);
float	OptFastFloorMagicConst24 = (float)pow(2.0,23);

#endif

} // NLMISC
