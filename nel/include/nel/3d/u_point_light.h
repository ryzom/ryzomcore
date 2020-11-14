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

#ifndef NL_U_POINT_LIGHT_H
#define NL_U_POINT_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "u_transform.h"


namespace NL3D
{


// ***************************************************************************
/**
 * Game interface for manipulating Dynamic Lights
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UPointLight : public UTransform
{
public:

	/// Set the ambient color of the light. Default to Black
	void			setAmbient (NLMISC::CRGBA ambient);
	/// Set the diffuse color of the light. Default to White
	void			setDiffuse (NLMISC::CRGBA diffuse);
	/// Set the specular color of the light. Default to White
	void			setSpecular (NLMISC::CRGBA specular);
	/// Set the diffuse and specular color of the light to the same value. don't modify _Ambient.
	void			setColor (NLMISC::CRGBA color);

	/// Get the ambient color of the light.
	NLMISC::CRGBA	getAmbient () const;
	/// Get the diffuse color of the light.
	NLMISC::CRGBA	getDiffuse () const;
	/// Get the specular color of the light.
	NLMISC::CRGBA	getSpecular () const;


	/** setup the attenuation of the light. if (0,0) attenuation is disabled.
	 *	clamp(attenuationBegin,0 , +oo) and clamp(attenuationEnd, attenuationBegin, +oo)
	 *	By default, attenuation is 10-30.
	 *	PERFORMANCE WARNING: big lights (disabled attenuation and big attenuationEnd) slow down
	 *	performances. (by experience, with a factor of 2).
	 */
	void			setupAttenuation(float attenuationBegin, float attenuationEnd);
	/// get the begin radius of the attenuation.
	float			getAttenuationBegin() const;
	/// get the end radius of the attenuation.
	float			getAttenuationEnd() const;


	/** Setup SpotLight. SpotLight is disabled by default. The direction of the spot is lead by the J vector of the
	 *	UPointLight WorldMatrix
	 */
	void			enableSpotlight(bool enable);
	/// Is Spotlight enabled?
	bool			isSpotlight() const;
	/** setup the spot AngleBegin and AngleEnd that define spot attenuation of the light. Useful only if SpotLight
	 *	NB: clamp(angleBegin, 0, PI); clamp(angleEnd, angleBegin, PI); Default is PI/4, PI/2
	 */
	void			setupSpotAngle(float spotAngleBegin, float spotAngleEnd);
	/// get the begin radius of the SpotAngles.
	float			getSpotAngleBegin() const;
	/// get the end radius of the SpotAngles.
	float			getSpotAngleEnd() const;




	/**	setup the deltaPosToSkeletonWhenOutOfFrustum
	 *	When a light is sticked to a skeleton, and if this skeleton is clipped, then the position of the light
	 *	can't be computed correctly without animating the skeleton. To allow good position of the light,
	 *	and to avoid recomputing the skeleton even if it is clipped, the light position is set to
	 *	skeletonMatrix * this "deltaPosToSkeletonWhenOutOfFrustum".
	 *
	 *	Default is (0, 0, 1.5).
	 *	You may change this according to the approximate size of the skeleton (dwarf or giant), and you must
	 *	take into account any mount (horse etc...). eg for a man on a elephant, a good value would be (0,0,5) :)
	 */
	void			setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos);
	/// see setDeltaPosToSkeletonWhenOutOfFrustum()
	const CVector	&getDeltaPosToSkeletonWhenOutOfFrustum() const;


	/** Special For Lightmap dynamic Lighting. if true, this light will influence lightmaped objects.
	 *	Lightmaped objects can be lighted by ONLY ONE (preference big) dynamic light.
	 *	If you setup multiple CPointLightModel with this flag, then it will randomly choose between one
	 *	of those visible lights.
	 *	NB: Lighting is made hardware per vertex.
	 */
	void			setInfluenceLightMap(bool enable);
	bool			getInfluenceLightMap() const;



	/// Proxy interface

	/// Constructors
	UPointLight() { _Object = NULL; }
	UPointLight(class CPointLightModel *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CPointLightModel *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CPointLightModel	*getObjectPtr() const {return (CPointLightModel*)_Object;}
};


} // NL3D


#endif // NL_U_POINT_LIGHT_H

/* End of u_point_light.h */
