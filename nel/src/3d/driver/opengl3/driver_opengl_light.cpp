// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/light.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
uint	CDriverGL3::getMaxLight () const
{
	H_AUTO_OGL(CDriverGL3_getMaxLight)
	// return MaxLight=8.
	return MaxLight;
}


// ***************************************************************************
void	CDriverGL3::setLight (uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLight)
	// bkup real light, for lightmap dynamic lighting purpose
	if (num==0)
	{
		_UserLight0= light;
		// because the GL setup change, must dirt lightmap rendering
		_LightMapDynamicLightDirty= true;
	}

	setLightInternal(num, light);
}


CLight	CDriverGL3::getLight (uint8 num)
{
	return _UserLight[ num ];
}


// ***************************************************************************
void	CDriverGL3::setLightInternal(uint8 num, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLightInternal)
	// Check light count is good
	// nlassert(num < MaxLight);

	// Set the light
	if (num < MaxLight)
	{
		// GL light number
		GLenum lightNum = (GLenum)(GL_LIGHT0+num);

		// Get light mode
		CLight::TLightMode mode = light.getMode ();

		// Copy the mode
		_LightMode[num] = mode;
		_UserLight[num] = light;
		touchLightVP(num);

		// Set the position
		if ((mode == CLight::DirectionalLight) || (mode == CLight::SpotLight))
		{
			// Get the direction of the light
			_WorldLightDirection[num] = light.getDirection();
		}

		if (mode != CLight::DirectionalLight)
		{
			// Get the position of the light
			_WorldLightPos[num] = light.getPosition();
		}

		if (mode == CLight::SpotLight)
		{
			// Get the exponent of the spot
			float exponent = light.getExponent();

			// Get the cutoff of the spot
			float cutoff = 180.f * (light.getCutoff() / (float)NLMISC::Pi);

		}

	}
}

// ***************************************************************************
void	CDriverGL3::enableLight(uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLight)

	// enable the light in GL
	enableLightInternal(num, enable);

	// because the GL setup has changed, must dirt lightmap rendering
	_LightMapDynamicLightDirty = true;
}

bool	CDriverGL3::isLightEnabled (uint8 num)
{
	if (num < MaxLight)
		return _UserLightEnable[num];

	return false;
}


// ***************************************************************************
void	CDriverGL3::enableLightInternal(uint8 num, bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableLightInternal)
	// Check light count is good
	// nlassert(num < MaxLight);

	// Enable glLight
	if (num < MaxLight)
	{
		// _DriverGLStates.enableLight(num, enable); // FIXME GL3 VERTEX PROGRAM
		_UserLightEnable[num] = enable;
		touchLightVP(num);
	}
}


// ***************************************************************************

void	CDriverGL3::setAmbientColor (CRGBA color)
{
	H_AUTO_OGL(CDriverGL3_setAmbientColor)
	
}


// ***************************************************************************
void			CDriverGL3::setLightMapDynamicLight (bool enable, const CLight& light)
{
	H_AUTO_OGL(CDriverGL3_setLightMapDynamicLight)
	// just store, for future setup in lightmap material rendering
	_LightMapDynamicLightEnabled= enable;
	_LightMapDynamicLight= light;
	_LightMapDynamicLightDirty= true;
}


// ***************************************************************************
void			CDriverGL3::setupLightMapDynamicLighting(bool enable)
{
	H_AUTO_OGL(CDriverGL3_setupLightMapDynamicLighting)
	
	// start lightmap dynamic lighting
	if (enable)
	{
		// disable all lights but the 0th.
		for (uint i = 1; i < MaxLight; ++i)
			enableLightInternal(i, false);

		// if the dynamic light is really enabled
		if (_LightMapDynamicLightEnabled)
		{
			// then setup and enable
			setLightInternal(0, _LightMapDynamicLight);
			enableLightInternal(0, true);
		}
		// else just disable also the light 0
		else
		{
			enableLightInternal(0, false);
		}

		// ok it has been setup
		_LightMapDynamicLightDirty = false;
	}
	// restore old lighting
	else
	{
		// restore the light 0
		setLightInternal(0, _UserLight0);

		// restore all standard light enable states
		for (uint i = 0; i < MaxLight; ++i)
			enableLightInternal(i, _UserLightEnable[i]);
	}
}

} // NLDRIVERGL3
} // NL3D
