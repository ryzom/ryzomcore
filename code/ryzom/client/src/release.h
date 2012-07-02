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



#ifndef CL_RELEASE_H
#define CL_RELEASE_H

#include "nel/misc/types_nl.h"



// called from farTP at reselection
void releaseMainLoopReselect();
// Release InGame Data
void releaseMainLoop(bool closeConnection);
// Release all.
void release();
// Release all.
void releaseOutGame();



#endif // CL_RELEASE_H

/* End of release.h */
