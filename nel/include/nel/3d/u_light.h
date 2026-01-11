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

#ifndef NL_U_LIGHT_H
#define NL_U_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"


namespace NL3D
{

// ***************************************************************************
/**
 * ULight implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class ULight
{
public:
	// Three modes for the light
	enum TLightMode
	{
		// The light is directional.
		DirectionalLight,

		// The light is a point.
		PointLight,

		// The light is a spotlight with a cone.
		SpotLight
	};

	/**
	 *	This is the static function create a stand alone light.
	 */
	static	ULight	*createLight ();

	virtual ~ULight() {}

	/// \name Quick setup.
	//@{

	/// Quick setup a directional light
	virtual void setupDirectional (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& direction,
							float constant=1, float linear=0, float quadratic=0) = 0;

	/// Quick setup a point light
	virtual void setupPointLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float constant=1, float linear=0, float quadratic=0) = 0;

	/// Quick setup a spotlight
	virtual void setupSpotLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float exponent, float cutoff, float constant=1, float linear=0, float quadratic=0) = 0;

	/**
	  * Setup attenuation with begin and end attenuation distance.
	  *
	  * \param farAttenuationBegin is the distance of the begin of the attenuation (attenuation == 0.9f)
	  * \param farAttenuationEnd is the distance of the end of the attenuation (attenuation == 0.1f)
	  */
	virtual void setupAttenuation (float farAttenuationBegin, float farAttenuationEnd) = 0;

	/**
	  * Set no attenuation.
	  *
	  * The light will not use attenuation.
	  *
	  */
	virtual void setNoAttenuation () = 0;

	/**
	  * Setup spot exponent with angle of the hotspot.
	  *
	  * \param hotSpotAngle is the angle in radian between the axis of the spot and the vector from light
	  * where attenuation is == 0.9.
	  */
	virtual void setupSpotExponent (float hotSpotAngle) = 0;

	//@}


	/// \name Set methods.
	//@{

	/**
	  * Set the light mode.
	  */
	virtual void setMode (ULight::TLightMode mode) = 0;

	/**
	  * Set the ambiant color of the light.
	  */
	virtual void setAmbiant (const NLMISC::CRGBA& ambiant) = 0;

	/**
	  * Set the diffuse color of the light.
	  */
	virtual void setDiffuse (const NLMISC::CRGBA& diffuse) = 0;

	/**
	  * Set the specular color of the light.
	  */
	virtual void setSpecular (const NLMISC::CRGBA& specular) = 0;

	/**
	  * Set the position of the light. Used only for SpotLight and PointLight.
	  */
	virtual void setPosition (const NLMISC::CVector& position) = 0;

	/**
	  * Set the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	virtual void setDirection (const NLMISC::CVector& direction) = 0;

	/**
	  * Set the Intensity distribution of the light. Should be between [0, 1]. Used only for SpotLight.
	  */
	virtual void setExponent (float exponent) = 0;

	/**
	  * Set the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	virtual void setCutoff (float cutoff) = 0;

	/**
	  * Set constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual void setConstantAttenuation (float constant) = 0;

	/**
	  * Set linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual void setLinearAttenuation (float linear) = 0;

	/**
	  * Set quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual void setQuadraticAttenuation (float quadratic) = 0;

	//@}

	/// \name Get methods.
	//@{

	/**
	  * Get the light mode.
	  */
	virtual ULight::TLightMode getMode () const = 0;

	/**
	  * Get the ambiant color of the light.
	  */
	virtual NLMISC::CRGBA getAmbiant () const = 0;

	/**
	  * Get the diffuse color of the light.
	  */
	virtual NLMISC::CRGBA getDiffuse () const = 0;

	/**
	  * Get the specular color of the light.
	  */
	virtual NLMISC::CRGBA getSpecular () const = 0;

	/**
	  * Get the position of the light. Used only for SpotLight and PointLight.
	  */
	virtual NLMISC::CVector getPosition () const = 0;

	/**
	  * Get the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	virtual NLMISC::CVector getDirection () const = 0;

	/**
	  * Get the exponent of the light. Used only for SpotLight.
	  */
	virtual float getExponent () const = 0;

	/**
	  * Get the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	virtual float getCutoff () const = 0;

	/**
	  * Get constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual float getConstantAttenuation () const = 0;

	/**
	  * Get linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual float getLinearAttenuation () const = 0;

	/**
	  * Get quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	virtual float getQuadraticAttenuation () const = 0;

	//@}
};


} // NL3D


#endif // NL_U_LIGHT_H

/* End of u_light.h */
