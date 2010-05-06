/** \file landscape.h
 * Landscape interface between the game and NeL
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef LANDSCAPE_H
#define LANDSCAPE_H

//
// Includes
//

#include <vector>

#include <nel/3d/u_landscape.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector.h>

//
// External definitions
//

namespace NL3D
{
	class UInstanceGroup;
}

namespace SBCLIENT {

class CTrajectory;


//
// External variables
//

extern std::vector<NL3D::UInstanceGroup*>	 InstanceGroups;
extern NLMISC::CVector						 SunDirection;


//
// External functions
//

void initLight();
void releaseLight();

void initLandscape();
void releaseLandscape();

void initAiming();
void releaseAiming();

void updateLandscape();

void loadAllZonesAround();

// Compute the collision with the landscape from the start position, using the given step,
// at maximum numSteps steps from the start. It uses full (!) physics computation, and should be
// identical on the server
NLMISC::CVector	getTarget(const NLMISC::CVector &start, const NLMISC::CVector &step, uint numSteps);

} /* namespace SBCLIENT */

#endif // LANDSCAPE_H

/* End of landscape.h */
