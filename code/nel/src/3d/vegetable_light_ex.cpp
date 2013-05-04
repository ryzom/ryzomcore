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

#include "nel/3d/vegetable_light_ex.h"
#include "nel/3d/point_light_named.h"


namespace NL3D
{



void			CVegetableLightEx::computeCurrentColors()
{
	for(uint i=0;i<NumLights;i++)
	{
		// get the light.
		CPointLightNamed	*pl= PointLight[i];
		// get the attenuation
		uint	att= PointLightFactor[i];
		// modulate the color with it. Use the Unanimated one!!
		Color[i].modulateFromui(pl->getUnAnimatedDiffuse(), att);
	}
}


} // NL3D
