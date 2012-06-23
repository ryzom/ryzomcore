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

#ifndef NL_VEGETABLE_LIGHT_EX_H
#define NL_VEGETABLE_LIGHT_EX_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"


namespace NL3D
{


class	CPointLightNamed;


/**
 * Additional information to light Vegetables. 2 "precomputed" pointLights can
 *	additionally light the vegetables.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CVegetableLightEx
{
public:

	/// Must not change (hardcoded)
	enum	{MaxNumLight= 2};

	/// Number Of light currently really enabled.
	uint				NumLights;
	/// PointLights. Used at CVegetableManager::updateLighting() to get current colors of pointLights.
	CPointLightNamed	*PointLight[MaxNumLight];
	/// Direction of the light. the direction to the instance should be precomputed.
	CVector				Direction[MaxNumLight];
	/// Factor to be multiplied by color of the light. Actually it is the attenuation factor.
	uint				PointLightFactor[MaxNumLight];

	/// Current Color of the light.
	NLMISC::CRGBA		Color[MaxNumLight];

public:
	CVegetableLightEx()
	{
		NumLights= 0;
	}

	/// Fill Color array according to PointLight[] and PointLightFactor[].
	void				computeCurrentColors();
};


} // NL3D


#endif // NL_VEGETABLE_LIGHT_EX_H

/* End of vegetable_light_ex.h */
