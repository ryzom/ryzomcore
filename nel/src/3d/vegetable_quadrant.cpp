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

#include "nel/3d/vegetable_quadrant.h"
#include "nel/misc/matrix.h"


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


/*
 * Constructor, just used to init quadrants.
 */
CVegetableQuadrant::CVegetableQuadrant()
{
	for(uint i=0;i<NL3D_VEGETABLE_NUM_QUADRANT;i++)
	{
		CMatrix	mat;
		mat.rotateZ(2*(float)Pi * i / NL3D_VEGETABLE_NUM_QUADRANT);
		Dirs[i]= mat.getJ();
	}
}


// Quadrants.
CVector		CVegetableQuadrant::Dirs[NL3D_VEGETABLE_NUM_QUADRANT];

// The variable to init the quadrants.
static	CVegetableQuadrant		InitVegetableQuadrant;


} // NL3D
