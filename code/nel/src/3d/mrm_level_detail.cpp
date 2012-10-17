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

#include "nel/3d/mrm_level_detail.h"


namespace NL3D
{


// ***************************************************************************
void			CMRMLevelDetail::compileDistanceSetup()
{
	// Compute _OODistDelta.
	OODistanceDelta= 1.0f / (DistanceCoarsest - DistanceFinest);
	/* Compute exponent pow, such that 0.5= dMiddle^pow;
		ie 0.5= e(ln dMiddle * pow)
	*/
	float	dMiddle= (DistanceCoarsest - DistanceMiddle) * OODistanceDelta;
	DistancePow= (float)(log(0.5) / log(dMiddle));
}


// ***************************************************************************
float			CMRMLevelDetail::getLevelDetailFromDist(float dist)
{
	if(dist <= DistanceFinest)
		return 1;
	if(dist >= DistanceCoarsest)
		return 0;

	float	d= (DistanceCoarsest - dist) * OODistanceDelta;
	return  (float)pow(d, DistancePow);
}


} // NL3D
