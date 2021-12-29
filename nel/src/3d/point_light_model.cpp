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

#include "nel/3d/point_light_model.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/root_model.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/scene.h"


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
void	CPointLightModel::registerBasic()
{
	CScene::registerModel( PointLightModelId, TransformId, CPointLightModel::creator);
}

// ***************************************************************************
CPointLightModel::CPointLightModel()
{
	_DeltaPosToSkeletonWhenOutOfFrustum.set(0, 0, 1.5f);
	_TimeFromLastClippedSpotDirection= 0;
	_InfluenceLightMap= false;
}


// ***************************************************************************
CPointLightModel::~CPointLightModel()
{
}


// ***************************************************************************
void	CPointLightModel::initModel()
{
	CTransform::initModel();

	// link me to the root of light.
	getOwnerScene()->getLightTrav().addPointLightModel(this);

}


// ***************************************************************************
void			CPointLightModel::setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos)
{
	_DeltaPosToSkeletonWhenOutOfFrustum= deltaPos;
}


// ***************************************************************************
const CVector	&CPointLightModel::getDeltaPosToSkeletonWhenOutOfFrustum() const
{
	return _DeltaPosToSkeletonWhenOutOfFrustum;
}


// ***************************************************************************
void	CPointLightModel::traverseLight()
{
	CLightTrav			&lightTrav= getOwnerScene()->getLightTrav();

	// Note: any dynamic light is supposed to always move each frame, so they are re-inserted in the
	// quadGrid each frame.


	// reset all models lighted by this light.
	// Then models are marked dirty and their light setup is reseted
	PointLight.resetLightedModels();


	// if the light is visible (ie not hiden)
	if( isHrcVisible() )
	{
		// If the light is not hidden by any skeleton.
		if( isClipVisible() )
		{
			// recompute the worldPosition of the light.
			PointLight.setPosition( getWorldMatrix().getPos() );

			// recompute the worldSpotDirection of the light.
			if(PointLight.getType() == CPointLight::SpotLight)
			{
				// Interpolate over time. (hardcoded)
				_TimeFromLastClippedSpotDirection-= 0.05f;
				if(_TimeFromLastClippedSpotDirection <= 0)
				{
					PointLight.setupSpotDirection(getWorldMatrix().getJ());
				}
				else
				{
					CVector		actualSpotDir= getWorldMatrix().getJ();
					// Interpolate
					float	t= _TimeFromLastClippedSpotDirection;
					CVector		interpSpotDir= actualSpotDir*(1-t) + _LastWorldSpotDirectionWhenOutOfFrustum * t;
					// set the interpolated one.
					PointLight.setupSpotDirection(interpSpotDir);
				}
			}
		}
		else
		{
			// We are hidden because a skeleton has hide us, or created during anim detail traversal (or else don't know why).
			// NB: a point light can't be in a cluster since it is not a transformShape...
			if (!_AncestorSkeletonModel) return;
			const CMatrix &skMatrix= _AncestorSkeletonModel->getWorldMatrix();

			PointLight.setPosition( skMatrix * _DeltaPosToSkeletonWhenOutOfFrustum );

			// recompute the worldSpotDirection of the light.
			if(PointLight.getType() == CPointLight::SpotLight)
			{
				// If last frame, this pointLight was visible (Time is not 1)
				if(_TimeFromLastClippedSpotDirection != 1)
				{
					// Take the current World spot direction
					_LastWorldSpotDirectionWhenOutOfFrustum= PointLight.getSpotDirection();
					// reset time.
					_TimeFromLastClippedSpotDirection= 1;
				}

				// Don't need to modify PointLight spot direction since already setuped (when model was visible)
			}
		}

		// now, insert this light in the quadGrid. NB: in CLightTrav::traverse(), the quadGrid is cleared before here.
		// This light will touch (resetLighting()) any model it may influence.
		lightTrav.LightingManager.addDynamicLight(&PointLight);


		// if this light is the one that should influence the lightmapped objects
		if(_InfluenceLightMap)
		{
			// setup a light with attenuation, for more accurate vertex lighting
			CLight	vertexLight;

			// setup position/type/direction/color
			if(PointLight.getType()!=CPointLight::SpotLight)
				vertexLight.setupPointLight(CRGBA::Black, PointLight.getDiffuse(), PointLight.getSpecular(),
					PointLight.getPosition(), CVector::K);
			else
				vertexLight.setupSpotLight(CRGBA::Black, PointLight.getDiffuse(), PointLight.getSpecular(),
					PointLight.getPosition(), PointLight.getSpotDirection(), 1, float(Pi)/2);

			// setup attenuation
			vertexLight.setupAttenuation(PointLight.getAttenuationBegin(), PointLight.getAttenuationEnd());

			// setup hotspot for spotLight
			if(PointLight.getType()==CPointLight::SpotLight)
			{
				// hotspot == where attenuation should be 0.9
				vertexLight.setupSpotExponent(PointLight.getSpotAngleBegin()*0.9f + PointLight.getSpotAngleEnd()*0.1f);
			}

			// setup the driver
			getOwnerScene()->getDriver()->setLightMapDynamicLight(true, vertexLight);
		}
	}

}



} // NL3D
