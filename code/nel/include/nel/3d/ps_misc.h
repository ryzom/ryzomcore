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



#ifndef PS_MISC_H
#define PS_MISC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/3d/particle_system_process.h"

namespace NL3D   {

/** Find a quantity to add to a float f such as  f + deltaT * numStep, be >= than a given float (endValue). numStep is an integer,.
  * It gives the number of steps used. If the number of steps is equal or greater than numStep, it is clamped to numStep
  * \param f the start value.
  * \return The number of steps.
  */
inline uint ScaleFloatGE(float f, float deltaT, float clampValue, uint numStep)
{
	NL_PS_FUNC(ScaleFloatGE)
	if (f >= clampValue) return 0;
	float endValue = f + numStep * deltaT;
	if (endValue < clampValue) return numStep;
	const uint numAfterInitialDate = 	(uint) ((endValue - clampValue) / deltaT);
	return numStep - numAfterInitialDate;
}


/** Private function used to fill a buffer with the same value by using the given subdivision.
  * NumStep and startValue are modified to give the steps that are above or equal to clampValue.
  * \param value		The value to fill the buffer with.
  * \param clampValue	A float such as startValue + n * deltaT < clampValue, where n is the number of value to fill.
  * \param startValue	The start value, used to get the number of steps, see clampValue.
  * \param deltaT		The step between values
  * \param maxNumStep	The max number of steps that can be filled. It is modified to
  * \param destPos		The destination, that will be filled with the given value
  * \param stride		Number of byte between each value to be copied
  */
inline NLMISC::CVector *FillBufUsingSubdiv(const	NLMISC::CVector &value,
									  float					clampValue,
									  float					&startValue,
									  float					deltaT,
									  uint					&maxNumStep,
									  NLMISC::CVector		*destPos,
									  uint32				stride
									  )
{
	NL_PS_FUNC(FillBufUsingSubdiv)
	uint numToFill = ScaleFloatGE(startValue, deltaT, clampValue, maxNumStep);
	nlassert(numToFill <= maxNumStep);
	startValue += numToFill * deltaT;
	maxNumStep  -= numToFill;
	while (numToFill--)
	{
		*destPos = value;
		destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
	}

	return destPos;
}

} // NL3D

#endif
