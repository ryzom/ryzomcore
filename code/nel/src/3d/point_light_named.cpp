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

#include "nel/3d/point_light_named.h"


namespace NL3D {


// ***************************************************************************
CPointLightNamed::CPointLightNamed()
{
	// copy setup from current
	_DefaultAmbient= getAmbient();
	_DefaultDiffuse= getDiffuse();
	_DefaultSpecular= getSpecular();
	_UnAnimatedDiffuse= getDiffuse();
}


// ***************************************************************************
void			CPointLightNamed::setLightFactor(NLMISC::CRGBA nFactor)
{
	setLightFactor(nFactor, nFactor);
}


// ***************************************************************************
void			CPointLightNamed::setLightFactor(NLMISC::CRGBA nAnimatedFactor, NLMISC::CRGBA nUnAnimatedFactor)
{
	CRGBA	col;
	// setup current ambient.
	col.modulateFromColor(_DefaultAmbient, nAnimatedFactor);
	setAmbient(col);
	// setup current diffuse.
	col.modulateFromColor(_DefaultDiffuse, nAnimatedFactor);
	setDiffuse(col);
	// setup current specular.
	col.modulateFromColor(_DefaultSpecular, nAnimatedFactor);
	setSpecular(col);

	// special UnAnimatedDiffuse
	col.modulateFromColor(_DefaultDiffuse, nUnAnimatedFactor);
	_UnAnimatedDiffuse= col;
}


// ***************************************************************************
void			CPointLightNamed::serial(NLMISC::IStream &f)
{
	sint ver = f.serialVersion(1);

	// Serialize parent.
	CPointLight::serial(f);

	// Serialize my data
	f.serial(AnimatedLight);
	f.serial(_DefaultAmbient);
	f.serial(_DefaultDiffuse);
	f.serial(_DefaultSpecular);

	if (ver>=1)
		f.serial(LightGroup);

	// read: and copy default _UnAnimatedDiffuse
	if(f.isReading())
	{
		_UnAnimatedDiffuse= getDiffuse();
	}
}

} // NL3D
