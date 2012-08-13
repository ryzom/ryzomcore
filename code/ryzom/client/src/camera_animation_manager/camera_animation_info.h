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
/* Class that contains new information about the camera
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
struct TCameraAnimationOutputInfo
{
	TCameraAnimationOutputInfo() {}

	NLMISC::CVector CamPos;						/// Camera position
	NLMISC::CVector CamLookAtDir;				/// Camera look at direction
};

/************************************************************************/
/* Class that contains current information about the camera
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
struct TCameraAnimationInputInfo
{
	TCameraAnimationInputInfo(const NLMISC::CVector& currCamPos, const NLMISC::CVector& currCamLookAtDir, 
								const NLMISC::CVector& startCamPos, const NLMISC::CVector& startCamLookAtDir, 
								const NLMISC::CVector& animStartCamPos, const NLMISC::CVector& animStartCamLookAtDir, 
								float elapsedTimeSinceStartStep)
	{
		CamPos = currCamPos;
		CamLookAtDir = currCamLookAtDir;

		StartCamPos = startCamPos;
		StartCamLookAtDir = startCamLookAtDir;

		AnimStartCamPos = animStartCamPos;
		AnimStartCamLookAtDir = animStartCamLookAtDir;

		ElapsedTimeSinceStartStep = elapsedTimeSinceStartStep;
	}

	TCameraAnimationInputInfo(const TCameraAnimationOutputInfo& output, const TCameraAnimationInputInfo& input)
	{
		CamPos = output.CamPos;
		CamLookAtDir = output.CamLookAtDir;

		StartCamPos = input.StartCamPos;
		StartCamLookAtDir = input.StartCamLookAtDir;

		AnimStartCamPos = input.AnimStartCamPos;
		AnimStartCamLookAtDir = input.AnimStartCamLookAtDir;

		ElapsedTimeSinceStartStep = input.ElapsedTimeSinceStartStep;
	}

	TCameraAnimationOutputInfo toOutput() const
	{
		TCameraAnimationOutputInfo output;
		output.CamPos = CamPos;
		output.CamLookAtDir = CamLookAtDir;
		return output;
	}

	NLMISC::CVector CamPos;						/// Current camera position
	NLMISC::CVector CamLookAtDir;				/// Current camera look at direction

	NLMISC::CVector StartCamPos;				/// Start camera position
	NLMISC::CVector StartCamLookAtDir;			/// Start camera look at direction

	NLMISC::CVector AnimStartCamPos;			/// Camera position at animation start
	NLMISC::CVector AnimStartCamLookAtDir;		/// Camera look at direction an animation start

	float ElapsedTimeSinceStartStep;			/// Elapsed time in second since the beginning of this step
};





#endif /* RY_CAMERAANIMATIONINFO_H */
