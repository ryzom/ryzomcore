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

#ifndef NL_LIGHT_H
#define NL_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"


namespace NL3D
{


/**
 * Light class to work with driver.
 *
 * Warning: nothing is initialized by default.
 *
 * To use this light you must initialize the MODE (setMode), the COLORS and the 3 ATTENUATION FACTORS.
 * If the mode is spotlight or pointlight, you must initialize the POSITION.
 * If the mode is spotlight or directionallight, you must initialize the DIRECTION.
 * If the mode is spotlight, you must intialize the EXPONENT and the CUTOFF.
 *
 * To do so, you can use one of the three methods: setupDirectional, setupPointLight, setupSpotLight or
 * use the specifics methods.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CLight
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

	/// \name Quick setup.
	//@{

	/// Quick setup a directional light
	void setupDirectional (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& direction,
							float constant=1, float linear=0, float quadratic=0);

	/// Quick setup a point light
	void setupPointLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float constant=1, float linear=0, float quadratic=0);

	/// Quick setup a spotlight
	void setupSpotLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position,
						const NLMISC::CVector& direction, float exponent, float cutoff, float constant=1, float linear=0, float quadratic=0);

	/**
	  * Setup attenuation with begin and end attenuation distance.
	  *
	  * \param farAttenuationBegin is the distance of the begin of the attenuation (attenuation == 0.9f)
	  * \param farAttenuationEnd is the distance of the end of the attenuation (attenuation == 0.1f)
	  */
	void setupAttenuation (float farAttenuationBegin, float farAttenuationEnd);

	/**
	  * Set no attenuation.
	  *
	  * The light will not use attenuation.
	  *
	  */
	void setNoAttenuation ();

	/**
	  * Setup spot exponent with angle of the hotspot.
	  *
	  * \param hotSpotAngle is the angle in radian between the axis of the spot and the vector from light
	  * where attenuation is == 0.9.
	  */
	void setupSpotExponent (float hotSpotAngle);

	//@}


	/// \name Set methods.
	//@{

	/**
	  * Set the light mode.
	  */
	void setMode (TLightMode mode)
	{
		_Mode=mode;
	}

	/**
	  * Set the ambiant color of the light.
	  */
	void setAmbiant (const NLMISC::CRGBA& ambiant)
	{
		_Ambiant=ambiant;
	}

	/**
	  * Set the diffuse color of the light.
	  */
	void setDiffuse (const NLMISC::CRGBA& diffuse)
	{
		_Diffuse=diffuse;
	}

	/**
	  * Set the specular color of the light.
	  */
	void setSpecular (const NLMISC::CRGBA& specular)
	{
		_Specular=specular;
	}

	/**
	  * Set the position of the light. Used only for SpotLight and PointLight.
	  */
	void setPosition (const NLMISC::CVector& position)
	{
		_Position=position;
	}

	/**
	  * Set the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	void setDirection (const NLMISC::CVector& direction)
	{
		_Direction=direction;
	}

	/**
	  * Set the Intensity distribution of the light. Should be between [0, 1]. Used only for SpotLight.
	  */
	void setExponent (float exponent)
	{
		_Exponent=exponent;
	}

	/**
	  * Set the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	void setCutoff (float cutoff)
	{
		_Cutoff=cutoff;
	}

	/**
	  * Set constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setConstantAttenuation (float constant)
	{
		_ConstantAttenuation=constant;
	}

	/**
	  * Set linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setLinearAttenuation (float linear)
	{
		_LinearAttenuation=linear;
	}

	/**
	  * Set quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setQuadraticAttenuation (float quadratic)
	{
		_QuadraticAttenuation=quadratic;
	}

	//@}

	/// \name Get methods.
	//@{

	/**
	  * Get the light mode.
	  */
	TLightMode getMode () const
	{
		return _Mode;
	}

	/**
	  * Get the ambiant color of the light.
	  */
	NLMISC::CRGBA getAmbiant () const
	{
		return _Ambiant;
	}

	/**
	  * Get the diffuse color of the light.
	  */
	NLMISC::CRGBA getDiffuse () const
	{
		return _Diffuse;
	}

	/**
	  * Get the specular color of the light.
	  */
	NLMISC::CRGBA getSpecular () const
	{
		return _Specular;
	}

	/**
	  * Get the position of the light. Used only for SpotLight and PointLight.
	  */
	NLMISC::CVector getPosition () const
	{
		return _Position;
	}

	/**
	  * Get the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	NLMISC::CVector getDirection () const
	{
		return _Direction;
	}

	/**
	  * Get the exponent of the light. Used only for SpotLight.
	  */
	float getExponent () const
	{
		return _Exponent;
	}

	/**
	  * Get the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	float getCutoff () const
	{
		return _Cutoff;
	}

	/**
	  * Get constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getConstantAttenuation () const
	{
		return _ConstantAttenuation;
	}

	/**
	  * Get linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getLinearAttenuation () const
	{
		return _LinearAttenuation;
	}

	/**
	  * Get quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION +
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getQuadraticAttenuation () const
	{
		return _QuadraticAttenuation;
	}

	//@}
private:
	// The light mode
	TLightMode			_Mode;

	// The light color.
	NLMISC::CRGBA		_Ambiant;
	NLMISC::CRGBA		_Diffuse;
	NLMISC::CRGBA		_Specular;

	// The position.
	NLMISC::CVector		_Position;
	NLMISC::CVector		_Direction;

	// The spotlight parameters.
	float				_Exponent;
	float				_Cutoff;

	// Attenuation
	float				_ConstantAttenuation;
	float				_LinearAttenuation;
	float				_QuadraticAttenuation;
};


} // NL3D


#endif // NL_LIGHT_H

/* End of light.h */
