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

#ifndef NL_LIGHT_CONTRIBUTION_H
#define NL_LIGHT_CONTRIBUTION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/3d/point_light.h"


namespace NL3D
{


// ***************************************************************************
/** This is the maximum possible contribution of light. NB: actual max may be less
 *	because of setup in CLightingManager.
 */
#define	NL3D_MAX_LIGHT_CONTRIBUTION		6


// ***************************************************************************
/**
 * light contribution on a model. Owned by a CTransform
 *	computed by lighting manager. result CLight is computed at render.
 *
 * if FrozenStaticLightSetup, then the SunContribution won't never be recomputed, and
 *	the first NumFrozenStaticLight PointLight are considered always valid and their setup
 *	won't be recomputed too. It's means also that CTransform::resetLighting() do not affect those
 *	NumFrozenStaticLight.
 *
 *	Typically, FrozenStaticLightSetup is setup for models and lights THAT ARE IN SAME IG, and
 *	are deleted together. This last point is important because the first NumFrozenStaticLight PointLight
 *	pointers are never updated, so delete a light which is in this setup will cause memory failure.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLightContribution
{
public:
	/// This is the list of Light which influence us. The first NULL means end_of_list.
	CPointLight			*PointLight[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// An iterator on the list of model in the pointLight which owns our transform.
	CPointLight::ItTransformList	TransformIterator[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the factor of influence to apply to each point light.
	uint8				Factor[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the Attenuation factor of influence to apply to each point light. Used if the model
	uint8				AttFactor[NL3D_MAX_LIGHT_CONTRIBUTION];
	/// the Dynamic Local Ambient. equal to SunAmbient if no particular ambient light
	NLMISC::CRGBA		LocalAmbient;


	/// Tells if there is some frozen static light setup.
	bool				FrozenStaticLightSetup;
	/** if FrozenStaticLightSetup, tells the number of point light setup which are static.
	 *	NB: it is possible that FrozenStaticLightSetup==true, and NumFrozenStaticLight==0. it means
	 *	that the model is not touched by any static pointLight.
	 */
	uint8				NumFrozenStaticLight;
	// The contribution of the sun (directionnal light) on this model. This not apply to ambient part of the sun
	uint8				SunContribution;
	// True if this contribution use the MergedPointLight.
	bool				UseMergedPointLight;
	/** if FrozenStaticLightSetup, this is the frozen AmbientLight in ig.
	 *	can't be stored as RGBA, because the ambient color may change.
	 *	NULL means take full Sun ambient
	 */
	CPointLight			*FrozenAmbientLight;

	// This is the Merged Ambient which simulate lot of pointLights.
	CRGBA				MergedPointLight;

public:

	/// Constructor
	CLightContribution();

	/** Compute the current ambiant according to the FrozenAmbientLight, or LocalAmbient
	 *	NB: The MergerPointLight is not added (since not really an ambiant)
	 */
	CRGBA				computeCurrentAmbient(CRGBA sunAmbient) const
	{
		if(FrozenStaticLightSetup)
		{
			// Any FrozenAmbientLight provided??
			if(FrozenAmbientLight)
			{
				CRGBA	finalAmbient;
				// Take his current (maybe animated) ambient
				finalAmbient= FrozenAmbientLight->getAmbient();
				// Add with sun?
				if(FrozenAmbientLight->getAddAmbientWithSun())
				{
					finalAmbient.addRGBOnly(finalAmbient, sunAmbient);
				}
				return finalAmbient;
			}
			else
				// Take the sun ones.
				return sunAmbient;
		}
		else
		{
			// take localAmbient
			return LocalAmbient;
		}
	}

};


} // NL3D


#endif // NL_LIGHT_CONTRIBUTION_H

/* End of light_contribution.h */
