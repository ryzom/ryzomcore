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

#include "nel/3d/light.h"

using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

void CLight::setupDirectional (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& direction,
							   float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (DirectionalLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the direction
	setDirection (direction);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);

	// Dummy to avoid uninit data, and problems of cache
	setPosition(CVector::Null);
	setExponent(0.f);
	setCutoff(0.f);
}

// ***************************************************************************

void CLight::setupPointLight (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& position,
						const CVector& direction, float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (PointLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the position and direction
	setPosition (position);
	setDirection (direction);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);

	// Dummy to avoid uninit data, and problems of cache
	setExponent(0.f);
	setCutoff(0.f);
}

// ***************************************************************************

void CLight::setupSpotLight (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& position,
						const CVector& direction, float exponent, float cutoff, float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (SpotLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the position and direction
	setPosition (position);
	setDirection (direction);

	// Set spotlight parameters
	setExponent (exponent);
	setCutoff (cutoff);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);
}

// ***************************************************************************
void CLight::setupAttenuation (float farAttenuationBegin, float farAttenuationEnd)
{
	/* Yoyo: I changed this method because it did not work well for me
		The most important in a light is its farAttenuationEnd (anything beyond should not be lighted)
		The old compute had too smooth decrease, regarding this (at farAttenuationEnd, the light could be
		attenuated by a factor of 0.7 (instead of 0) and slowly decreased).
	*/

	// limit case
	if(farAttenuationEnd<=0)
	{
		_ConstantAttenuation= 1000000;
		_LinearAttenuation= 0;
		_QuadraticAttenuation= 0;
	}
	else
	{
		// The following factors are "it feels good for farAttenuationBegin=0/farAttenuationEnd=1" factors.
		// btw, at r=farAttenuationEnd=1, att= 1/11 ~= 0.
		const float	constant= 1.0f;
		const float	linear= 0.f;
		const float	quadratic= 10.0f;

		/*
			With GL/D3D   'att=1/(c+l*r+q*r2)'  formula, I think it is impossible to simulate correctly
			farAttenuationBegin (very big decrase if for instance farAttenuationBegin is near farAttenuationEnd),
			hence I simulate it very badly by multiplying the farAttenuationEnd by some factor
		*/
		float	factor= 1.f;
		if(farAttenuationBegin/farAttenuationEnd>0.5f)
			factor= 2.f;
		else if(farAttenuationBegin>0)
			factor= 1.f + 2*farAttenuationBegin/farAttenuationEnd;
		farAttenuationEnd*= factor;

		// scale according to farAttenuationEnd.
		_ConstantAttenuation= constant;
		_LinearAttenuation= linear/farAttenuationEnd;
		_QuadraticAttenuation= quadratic/sqr(farAttenuationEnd);
	}
}

// ***************************************************************************

void CLight::setupSpotExponent (float hotSpotAngle)
{
	float divid=(float)log (cos (hotSpotAngle));
	if (divid==0.f)
		divid=0.0001f;
	setExponent ((float)(log (0.9)/divid));
}

// ***************************************************************************

void CLight::setNoAttenuation ()
{
	_ConstantAttenuation=1.f;
	_QuadraticAttenuation=0.f;
	_LinearAttenuation=0.f;
}

// ***************************************************************************

} // NL3D
