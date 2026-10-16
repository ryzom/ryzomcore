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

#ifndef NL_LIGHT_USER_H
#define NL_LIGHT_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_light.h"
#include "nel/3d/light.h"


namespace NL3D
{

class	CDriverUser;


// ***************************************************************************
/**
 * ULight implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLightUser : public ULight
{
protected:
	CLight			_Light;
	friend class	CDriverUser;

public:
	virtual ~CLightUser() NL_OVERRIDE {}

	/// \name Quick setup.
	//@{

	/// Quick setup a directional light
	void setupDirectional (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& direction,
							float constant=1, float linear=0, float quadratic=0) NL_OVERRIDE
	{
		_Light.setupDirectional (ambiant, diffuse, specular, direction, constant, linear, quadratic);
	}

	/// Quick setup a point light
	void setupPointLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float constant=1, float linear=0, float quadratic=0) NL_OVERRIDE
	{
		_Light.setupPointLight (ambiant, diffuse, specular, position, direction, constant, linear, quadratic);
	}

	/// Quick setup a spotlight
	void setupSpotLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float exponent, float cutoff, float constant=1, float linear=0, float quadratic=0) NL_OVERRIDE
	{
		_Light.setupSpotLight (ambiant, diffuse, specular, position, direction, exponent, cutoff, constant, linear, quadratic);
	}

	/**
	  * Setup attenuation with begin and end attenuation distance.
	  *
	  * \param farAttenuationBegin is the distance of the begin of the attenuation (attenuation == 0.9f)
	  * \param farAttenuationEnd is the distance of the end of the attenuation (attenuation == 0.1f)
	  */
	void setupAttenuation (float farAttenuationBegin, float farAttenuationEnd) NL_OVERRIDE
	{
		_Light.setupAttenuation (farAttenuationBegin, farAttenuationEnd);
	}

	/**
	  * Set no attenuation.
	  *
	  * The light will not use attenuation.
	  *
	  */
	void setNoAttenuation () NL_OVERRIDE
	{
		_Light.setNoAttenuation ();
	}

	/**
	  * Setup spot exponent with angle of the hotspot.
	  *
	  * \param hotSpotAngle is the angle in radian between the axis of the spot and the vector from light
	  * where attenuation is == 0.9.
	  */
	void setupSpotExponent (float hotSpotAngle) NL_OVERRIDE
	{
		_Light.setupSpotExponent (hotSpotAngle);
	}

	//@}


	/// \name Set methods.
	//@{

	/**
	  * Set the light mode.
	  */
	void setMode (ULight::TLightMode mode) NL_OVERRIDE
	{
		_Light.setMode ((CLight::TLightMode)(uint32)mode);
	}

	/**
	  * Set the ambiant color of the light.
	  */
	void setAmbiant (const NLMISC::CRGBA& ambiant) NL_OVERRIDE
	{
		_Light.setAmbiant (ambiant);
	}

	/**
	  * Set the diffuse color of the light.
	  */
	void setDiffuse (const NLMISC::CRGBA& diffuse) NL_OVERRIDE
	{
		_Light.setDiffuse (diffuse);
	}

	/**
	  * Set the specular color of the light.
	  */
	void setSpecular (const NLMISC::CRGBA& specular) NL_OVERRIDE
	{
		_Light.setSpecular (specular);
	}

	/**
	  * Set the position of the light. Used only for SpotLight and PointLight.
	  */
	void setPosition (const NLMISC::CVector& position) NL_OVERRIDE
	{
		_Light.setPosition (position);
	}

	/**
	  * Set the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	void setDirection (const NLMISC::CVector& direction) NL_OVERRIDE
	{
		_Light.setDirection (direction);
	}

	/**
	  * Set the Intensity distribution of the light. Should be between [0, 1]. Used only for SpotLight.
	  */
	void setExponent (float exponent) NL_OVERRIDE
	{
		_Light.setExponent (exponent);
	}

	/**
	  * Set the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	void setCutoff (float cutoff) NL_OVERRIDE
	{
		_Light.setCutoff (cutoff);
	}

	/**
	  * Set constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setConstantAttenuation (float constant) NL_OVERRIDE
	{
		_Light.setConstantAttenuation (constant);
	}

	/**
	  * Set linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setLinearAttenuation (float linear) NL_OVERRIDE
	{
		_Light.setLinearAttenuation (linear);
	}

	/**
	  * Set quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setQuadraticAttenuation (float quadratic) NL_OVERRIDE
	{
		_Light.setQuadraticAttenuation (quadratic);
	}

	//@}

	/// \name Get methods.
	//@{

	/**
	  * Get the light mode.
	  */
	ULight::TLightMode getMode () const NL_OVERRIDE
	{
		return (ULight::TLightMode)(uint32)_Light.getMode ();
	}

	/**
	  * Get the ambiant color of the light.
	  */
	NLMISC::CRGBA getAmbiant () const NL_OVERRIDE
	{
		return _Light.getAmbiant ();
	}

	/**
	  * Get the diffuse color of the light.
	  */
	NLMISC::CRGBA getDiffuse () const NL_OVERRIDE
	{
		return _Light.getDiffuse ();
	}

	/**
	  * Get the specular color of the light.
	  */
	NLMISC::CRGBA getSpecular () const NL_OVERRIDE
	{
		return _Light.getSpecular ();
	}

	/**
	  * Get the position of the light. Used only for SpotLight and PointLight.
	  */
	NLMISC::CVector getPosition () const NL_OVERRIDE
	{
		return _Light.getPosition ();
	}

	/**
	  * Get the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	NLMISC::CVector getDirection () const NL_OVERRIDE
	{
		return _Light.getDirection ();
	}

	/**
	  * Get the exponent of the light. Used only for SpotLight.
	  */
	float getExponent () const NL_OVERRIDE
	{
		return _Light.getExponent ();
	}

	/**
	  * Get the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	float getCutoff () const NL_OVERRIDE
	{
		return _Light.getCutoff ();
	}

	/**
	  * Get constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getConstantAttenuation () const NL_OVERRIDE
	{
		return _Light.getConstantAttenuation ();
	}

	/**
	  * Get linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getLinearAttenuation () const NL_OVERRIDE
	{
		return _Light.getLinearAttenuation ();
	}

	/**
	  * Get quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getQuadraticAttenuation () const NL_OVERRIDE
	{
		return _Light.getQuadraticAttenuation ();
	}

	//@}
};


} // NL3D


#endif // NL_LIGHT_USER_H

/* End of light_user.h */
