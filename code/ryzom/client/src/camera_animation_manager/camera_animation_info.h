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

#ifndef RY_CAMERAANIMATIONINFO_H
#define RY_CAMERAANIMATIONINFO_H


#include "nel/misc/vector.h"


/************************************************************************/
/* Class that contains information about the camera
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
struct TCameraAnimationInfo
{
	TCameraAnimationInfo(const NLMISC::CVector& camPos, 
						const NLMISC::CVector& camLookAt, 
						float elapsedTimeSinceStartStep)
	{
		CamPos = camPos;
		CamLookAt = camLookAt;
		ElapsedTimeSinceStartStep = elapsedTimeSinceStartStep;
	}

	TCameraAnimationInfo() {}

	NLMISC::CVector CamPos;						/// Camera position
	NLMISC::CVector CamLookAt;					/// Camera look at position

	float ElapsedTimeSinceStartStep;			/// Elapsed time in second since the beginning of this step
};



#endif /* RY_CAMERAANIMATIONINFO_H */
