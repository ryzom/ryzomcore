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

#ifndef NL_VEGETABLE_QUADRANT_H
#define NL_VEGETABLE_QUADRANT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/3d/vegetable_def.h"


namespace NL3D
{


using NLMISC::CVector;


// ***************************************************************************
/**
 * Static Quadrant direction for Vegetable ZSort rdrPass.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableQuadrant
{
public:

	/// Constructor
	CVegetableQuadrant();

	// Directions of quadrants
	static CVector		Dirs[NL3D_VEGETABLE_NUM_QUADRANT];
};


} // NL3D


#endif // NL_VEGETABLE_QUADRANT_H

/* End of vegetable_quadrant.h */
