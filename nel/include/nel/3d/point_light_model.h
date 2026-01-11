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

#ifndef NL_POINT_LIGHT_MODEL_H
#define NL_POINT_LIGHT_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/transform.h"
#include "nel/3d/fast_ptr_list.h"


namespace NL3D {


class	CLightTrav;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		PointLightModelId=NLMISC::CClassId(0x7e842eba, 0x140b6c69);


// ***************************************************************************
/**
 * This model is a dynamic light. It handles a PointLight, where Pos is the worldPos updated by CScene
 *	at each render(). CPointLightModel are linked to the LightModelList in the LightTrav.
 *	It can handles SpotLight too, see PointLight.
 *
 *	Hrc: Lights herit CTransform so they can be put in hierarchy, even sticked to a skeleton. They can be hide,
 *	moved etc... (default CTransform).
 *	Clip: Lights are always in frustum, not renderable (default CTransform).
 *	Light: lightModels are not lightables (ie they can't be lighted). (default CTransform).
 *		traverseLight() is specialised.
 *
 *	PERFORMANCE WARNING: big lights (disabled attenuation and big attenuationEnd) slow down
 *	performances. (by experience, with a factor of 2).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightModel : public CTransform
{
public:
	/// Call at the beginning of the program, to register the model
	static	void	registerBasic();


public:

	/** The pointLight setup (color/attenuation). Do not use PointLight.Pos to setup the position.
	 *	Use the CTransform interface to set the position.
	 *	To enable SpotLight, use PointLight.setType(), and use PointLight.setSpotAngle() but don't use
	 *	PointLight.setSpotDirection to setup the direction. The direction of the spotLight is driven by
	 *	the J vector of the Transform WorldMatrix.
	 */
	CPointLight		PointLight;


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
	void			setInfluenceLightMap(bool enable) {_InfluenceLightMap= enable;}
	bool			getInfluenceLightMap() const {return _InfluenceLightMap;}


	/** The traverse() method is called to update the worldPosition of the light, resetLightedModels(), and
	 *	re-insert the light in the lightingManager.
	 */
	virtual void	traverseLight();


protected:
	/// Constructor
	CPointLightModel();
	/// Destructor
	virtual ~CPointLightModel();

	/// Implement the initModel method: link to the LightModelList.
	virtual void	initModel();


// *********************
private:
	friend class	CLightTrav;

	static CTransform	*creator() {return new CPointLightModel;}

	// Node for LightTrav
	CFastPtrListNode	_PointLightNode;

	/** tells if the pointLightModel is not hidden by user
	 *	actually, it is the result of hrc Visibility.
	 */
	bool	isHrcVisible() const
	{
		return _WorldVis;
	}

	/// see setDeltaPosToSkeletonWhenOutOfFrustum()
	CVector			_DeltaPosToSkeletonWhenOutOfFrustum;

	/** Same problem as _DeltaPosToSkeletonWhenOutOfFrustum, but this one is computed at each Visible frame.
	 *	And we interpolate between actual direction and backuped direction when the spot become visible (5 frames, hardcoded)
	 */
	CVector			_LastWorldSpotDirectionWhenOutOfFrustum;
	float			_TimeFromLastClippedSpotDirection;


	/// see setInfluenceLightMap()
	bool			_InfluenceLightMap;
};


} // NL3D


#endif // NL_POINT_LIGHT_MODEL_H

/* End of point_light_model.h */
