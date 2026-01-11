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

#ifndef _GRID_TRAVERSAL_H
#define _GRID_TRAVERSAL_H

#include "types_nl.h"

namespace NLMISC
{

class CVector2f;

/** Utility class for incremental grid traversal
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
class CGridTraversal
{
public:
	/** begin to traverse a grid along the given segments
	  * \return first location in the grid
	  */
	static void startTraverse(const NLMISC::CVector2f &start, sint &nextX, sint &nextY);
	/** continue grid traversal from the given position
	  * If there are remaingind cells to traverse then true is returned, x & y are updated
	  */
	static bool traverse(const NLMISC::CVector2f &start, const NLMISC::CVector2f &dir, sint &x, sint &y);
};


} // NLMISC

#endif
