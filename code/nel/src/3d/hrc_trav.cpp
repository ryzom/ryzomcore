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

#include "std3d.h"

#include "nel/3d/hrc_trav.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
void				CHrcTrav::traverse()
{
	H_AUTO( NL3D_TravHRC );

	_MovingObjects.clear();

	// Traverse the graph.
	if(Scene->getRoot())
		Scene->getRoot()->traverseHrc();

	// Inc the date.
	// NB: Now, models update is done before ALL traversals.
	// Hence, we must inc the value before scene rendering. This is equivalent to start with 1, and inc at end of traverse().
	CurrentDate++;
}


}
