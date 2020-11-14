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

#include "nel/3d/lighting_manager.h"
#include "nel/3d/point_light.h"
#include "nel/3d/transform.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/logic_info.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/algo.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
/* LightQuadGrid setup. This is the same setup for StaticLightedModelQuadGrid setup.
	NB: with this setup, a light will lies into 4*4=16 squares of a quadGrid at max.
*/
#define	NL3D_LIGHT_QUAD_GRID_SIZE				128
#define	NL3D_LIGHT_QUAD_GRID_ELTSIZE			10.f
#define	NL3D_LIGHT_QUAD_GRID_RADIUS_LIMIT		20.f
// Factor for a level to the next: size/=factor, eltSize*=factor, and radiusLimit*=factor.
#define	NL3D_LIGHT_QUAD_GRID_FACTOR				4


// this is a big radius for light that don't have attenuation
#define	NL3D_DEFAULT_NOATT_LIGHT_RADIUS					1000.f
// this is used when the model is out of attBegin/attEnd, to modulate the influence
#define	NL3D_DEFAULT_OUT_OF_ATT_LIGHT_INF_FACTOR		0.1f
// Defualt LightTransitionThreshold
#define	NL3D_DEFAULT_LIGHT_TRANSITION_THRESHOLD			0.1f


// ***************************************************************************
CLightingManager::CLightingManager(bool bSmallScene)
{
	// Init the lightQuadGrids and StaticLightedModelQuadGrid
	// finer level.
	uint	qgSize= NL3D_LIGHT_QUAD_GRID_SIZE;
	float	eltSize= NL3D_LIGHT_QUAD_GRID_ELTSIZE;
	float	radiusLimit= NL3D_LIGHT_QUAD_GRID_RADIUS_LIMIT;

	if (bSmallScene)
	{
		qgSize = 4;
	}

	// for all levels
	for(uint i=0;i<NL3D_QUADGRID_LIGHT_NUM_LEVEL;i++)
	{
		// init _LightQuadGrid and _StaticLightedModelQuadGrid
		_LightQuadGrid[i].create(qgSize, eltSize);
		_StaticLightedModelQuadGrid[i].create(qgSize, eltSize);
		_LightQuadGridRadiusLimit[i]= radiusLimit;

		// coarser quadGrid level
		qgSize/= NL3D_LIGHT_QUAD_GRID_FACTOR;
		qgSize= max(qgSize, 1U);
		eltSize*= NL3D_LIGHT_QUAD_GRID_FACTOR;
		radiusLimit*= NL3D_LIGHT_QUAD_GRID_FACTOR;
	}


	// default paramters
	_NoAttLightRadius= NL3D_DEFAULT_NOATT_LIGHT_RADIUS;
	_OutOfAttLightInfFactor= NL3D_DEFAULT_OUT_OF_ATT_LIGHT_INF_FACTOR;
	_LightTransitionThreshold= NL3D_DEFAULT_LIGHT_TRANSITION_THRESHOLD;


	// Default number of pointLight on an object.
	setMaxLightContribution(3);
}


// ***************************************************************************
void		CLightingManager::setMaxLightContribution(uint nlights)
{
	_MaxLightContribution= min(nlights, (uint)NL3D_MAX_LIGHT_CONTRIBUTION);
}


// ***************************************************************************
void		CLightingManager::setNoAttLightRadius(float noAttLightRadius)
{
	nlassert(noAttLightRadius>0);
	_NoAttLightRadius= noAttLightRadius;
}


// ***************************************************************************
void		CLightingManager::setOutOfAttLightInfFactor(float outOfAttLightInfFactor)
{
	outOfAttLightInfFactor= max(0.f, outOfAttLightInfFactor);
	_OutOfAttLightInfFactor= outOfAttLightInfFactor;
}


// ***************************************************************************
void		CLightingManager::setLightTransitionThreshold(float lightTransitionThreshold)
{
	clamp(lightTransitionThreshold, 0.f, 1.f);
	_LightTransitionThreshold= lightTransitionThreshold;
}



// ***************************************************************************
void		CLightingManager::clearDynamicLights()
{
	// for all levels
	for(uint i=0;i<NL3D_QUADGRID_LIGHT_NUM_LEVEL;i++)
	{
		// clear all the lights in the quadGrid.
		_LightQuadGrid[i].clear();
	}

	// clear the list.
	_DynamicLightList.clear();
}

// ***************************************************************************
void		CLightingManager::addDynamicLight(CPointLight *light)
{

	// Insert the light in the quadGrid.
	//----------
	CPointLightInfo		plInfo;
	plInfo.Light= light;

	// build the bounding sphere for this light, with the AttenuationEnd of the light
	float	radius=light->getAttenuationEnd();
	// if attenuation is disabled (ie getAttenuationEnd() return 0), then have a dummy radius
	if(radius==0)
		radius= _NoAttLightRadius;
	plInfo.Sphere.Center= light->getPosition();
	plInfo.Sphere.Radius= radius;

	// build a bbox, so it includes the sphere
	CVector	bbmin= light->getPosition();
	bbmin-= CVector(radius, radius, radius);
	CVector	bbmax= light->getPosition();
	bbmax+= CVector(radius, radius, radius);

	// choose the correct quadgrid according to the radius of the light.
	uint qgId;
	for(qgId= 0; qgId<NL3D_QUADGRID_LIGHT_NUM_LEVEL-1; qgId++)
	{
		// if radius is inferior to the requested radius for this quadGrid, ok!
		if(radius<_LightQuadGridRadiusLimit[qgId])
			break;
	}
	// insert this light in the correct quadgrid.
	_LightQuadGrid[qgId].insert(bbmin, bbmax, plInfo);


	// touch the objects around the lights.
	//----------

	// Select the static lightedModels to update around this light.
	// Use the same level of _StaticLightedModelQuadGrid than me to not select too many squares in the quadGrid
	_StaticLightedModelQuadGrid[qgId].select(bbmin, bbmax);
	// For all those models.
	CQuadGrid<CTransform*>::CIterator	itModel= _StaticLightedModelQuadGrid[qgId].begin();
	while(itModel != _StaticLightedModelQuadGrid[qgId].end() )
	{
		CTransform	*model= *itModel;
		const CVector &modelPos= model->getWorldMatrix().getPos();

		// test first if this model is in the area of the light.
		if( plInfo.Sphere.include(modelPos) )
		{
			// yes, then this model must recompute his lighting, because a dynamic light touch him.
			model->resetLighting();
		}

		// next.
		itModel++;
	}


	// insert the light in the list.
	//----------
	_DynamicLightList.push_back(light);

}

// ***************************************************************************
CLightingManager::CQGItLightedModel	CLightingManager::eraseStaticLightedModel(CQGItLightedModel ite)
{
	// Erase the iterator for all levels
	for(uint i=0;i<NL3D_QUADGRID_LIGHT_NUM_LEVEL;i++)
	{
		// NB: it is possible here that the iterator is NULL (ie end()).
		_StaticLightedModelQuadGrid[i].erase(ite.QgItes[i]);
	}

	// return end(), ie NULL iterators
	return CQGItLightedModel();
}

// ***************************************************************************
CLightingManager::CQGItLightedModel	CLightingManager::insertStaticLightedModel(CTransform *model)
{
	CQGItLightedModel	ite;
	const CVector &worldPos= model->getWorldMatrix().getPos();

	// Insert the models in all levels, because addDynamicLight() may choose the best suited level to select
	for(uint i=0;i<NL3D_QUADGRID_LIGHT_NUM_LEVEL;i++)
	{
		ite.QgItes[i]= _StaticLightedModelQuadGrid[i].insert(worldPos, worldPos, model);
	}

	// return the iterator
	return ite;
}


// ***************************************************************************
struct	CSortLight
{
	CPointLight		*PointLight;
	float			Influence;

};

// ***************************************************************************
void		CLightingManager::computeModelLightContributions(NLMISC::CRGBA sunAmbient, CTransform *model, CLightContribution &lightContrib,
	ILogicInfo *logicInfo)
{
	sint	i;

	// This is the list of light which touch this model.
	static std::vector<CPointLightInfluence>	lightList;
	// static, for malloc perf.
	lightList.clear();

	// the position of the model.
	CVector modelPos;
	float	modelRadius;

	// depends on model
	model->getLightHotSpotInWorld(modelPos, modelRadius);

	// First pass, fill the list of light which touch this model.
	//=========
	// get the dynamic lights around the model
	getDynamicPointLightList(modelPos, lightList);

	// if not already precomputed, append staticLights to this list.
	if( !lightContrib.FrozenStaticLightSetup )
	{
		// If no logicInfo provided
		if(!logicInfo)
		{
			// Default: suppose full SunLight and no PointLights.
			lightContrib.SunContribution= 255;
			// Take full SunAmbient.
			lightContrib.LocalAmbient= sunAmbient;
			// do not append any pointLight to the setup
		}
		else
		{
			// NB: SunContribution is computed by logicInfo
			logicInfo->getStaticLightSetup(sunAmbient, lightList, lightContrib.SunContribution, lightContrib.LocalAmbient);
		}
	}

	// Second pass, in the lightList, choose the best suited light to lit this model
	//=========

	// for each light, modulate the factor of influence
	for(i=0; i<(sint)lightList.size();i++)
	{
		CPointLight	*pl= lightList[i].PointLight;

		// get the distance from the light to the model
		float	dist= (pl->getPosition() - modelPos).norm();
		float	distMinusRadius= dist - modelRadius;

		// modulate the factor by the distance and the attenuation distance.
		float	 inf;
		float	attBegin= pl->getAttenuationBegin();
		float	attEnd= pl->getAttenuationEnd();
		// if no attenuation
		if( attEnd==0 )
		{
			// influence is awlays 1.
			inf= 1;

			// If SpotLight, must modulate with SpotAttenuation.
			if(pl->getType() == CPointLight::SpotLight)
				inf*= pl->computeLinearAttenuation(modelPos, dist, modelRadius);
		}
		else
		{
			// if correct attenuation radius
			if(distMinusRadius<attBegin)
			{
				// NB: we are sure that attBegin>0, because dist>=0.
				// if < attBegin, inf should be ==1, but must select the nearest lights; for better
				// understanding of the scene
				inf= 1 + _OutOfAttLightInfFactor * (attBegin - distMinusRadius);	// inf E [1, +oo[
				// NB: this formula favour big lights (ie light with big attBegin).

				// If SpotLight, must modulate with SpotAttenuation.
				if(pl->getType() == CPointLight::SpotLight)
					inf*= pl->computeLinearAttenuation(modelPos, dist, modelRadius);
			}
			else if(distMinusRadius<attEnd)
			{
				// we are sure attEnd-attBegin>0 because of the test
				// compute influence of the light: attenuation and SpotAttenuation
				inf= pl->computeLinearAttenuation(modelPos, dist, modelRadius);
			}
			else
			{
				// if >= attEnd, inf should be ==0, but must select the nearest lights; for better
				// understanding of the scene
				inf= _OutOfAttLightInfFactor * (attEnd - distMinusRadius);		// inf E ]-oo, 0]
			}
		}

		// modulate the influence with this factor
		lightList[i].BkupInfluence= lightList[i].Influence;
		lightList[i].Influence*= inf;

		// Bkup distance to model.
		lightList[i].DistanceToModel= dist;
	}

	// sort the light by influence
	sort(lightList.begin(), lightList.end());

	// prepare Light Merging.
	static std::vector<float>	lightToMergeWeight;
	lightToMergeWeight.clear();
	lightToMergeWeight.resize(lightList.size(), 1);


	// and choose only max light.
	uint	startId= 0;
	uint	ligthSrcId= 0;
	// skip already setuped light (statically)
	if(lightContrib.FrozenStaticLightSetup)
		startId= lightContrib.NumFrozenStaticLight;

	// If there is still place for unFrozen static lights or dynamic lights.
	if(startId < _MaxLightContribution)
	{
		// setup the transition.
		float	deltaMinInfluence= _LightTransitionThreshold;
		float	minInfluence= 0;
		sint	doMerge= 0;
		sint	firstLightToMergeFull= _MaxLightContribution-startId;
		// If there is more light than we can accept in not merged mode, Must merge
		if((sint)lightList.size() > firstLightToMergeFull)
		{
			doMerge= 1;
			// the minInfluence is the influence of the first light not taken.
			minInfluence= lightList[firstLightToMergeFull].Influence;
			// but must still be >=0.
			minInfluence= max(minInfluence, 0.f);
		}
		// Any light under this minInfluence+deltaMinInfluence will be smoothly darken.
		float	minInfluenceStart = minInfluence + deltaMinInfluence;
		float	OOdeltaMinInfluence= 1.0f / deltaMinInfluence;
		/* NB: this is not an error if we have only 3 light for example (assuming _MaxLightContribution=3),
			and still have minInfluenceStart>0.
			It's to have a continuity in the case of for example we have 3 lights in lightLists at frame T0,
			resulting in "doMerge=0"  and at frame T1 we have 4 lights, the farthest having an influence of 0
			or nearly 0.
		*/

		// fill maxLight at max.
		for(i=startId;i<(sint)_MaxLightContribution; i++)
		{
			// if not so many pointLights found, end!!
			if(ligthSrcId>=lightList.size())
				break;
			else
			{
				CPointLight	*pl= lightList[ligthSrcId].PointLight;
				float		inf= lightList[ligthSrcId].Influence;
				float		bkupInf= lightList[ligthSrcId].BkupInfluence;
				float		distToModel= lightList[ligthSrcId].DistanceToModel;
				// else fill it.
				lightContrib.PointLight[i]= pl;

				// Compute the Final factor of influence of the light.
				if(inf >= minInfluenceStart)
				{
					// For Static LightSetup BiLinear to work correctly, modulate with BkupInfluence
					// don't worry about the precision of floor, because of *255.
					lightContrib.Factor[i]= (uint8)NLMISC::OptFastFloor(bkupInf*255);
					// Indicate that this light don't need to be merged at all!
					lightToMergeWeight[ligthSrcId]= 0;
				}
				else
				{
					float	f= (inf-minInfluence) * OOdeltaMinInfluence;
					// For Static LightSetup BiLinear to work correctly, modulate with BkupInfluence
					// don't worry about the precision of floor, because of *255.
					sint	fi= NLMISC::OptFastFloor( bkupInf*f*255 );
					clamp(fi, 0, 255);
					lightContrib.Factor[i]= fi;
					// The rest of the light contribution is to be merged.
					lightToMergeWeight[ligthSrcId]= 1-f;
				}

				// Compute the Final Att factor for models using Global Attenuation. NB: modulate with Factor
				// don't worry about the precision of floor, because of *255.
				// NB: compute att on the center of the model => modelRadius==0
				sint	attFactor= NLMISC::OptFastFloor( lightContrib.Factor[i] * pl->computeLinearAttenuation(modelPos, distToModel) );
				lightContrib.AttFactor[i]= (uint8)attFactor;

				// must append this lightedModel to the list in the light.
				lightContrib.TransformIterator[i]= pl->appendLightedModel(model);

				// next light.
				ligthSrcId++;
			}
		}

		// Compute LightToMerge.
		if(doMerge)
		{
			CRGBAF		mergedAmbient(0,0,0);
			uint		j;

			// For all lights in the lightList, merge Diffuse and Ambient term to mergedAmbient.
			for(j=0;j<lightToMergeWeight.size();j++)
			{
				if(lightToMergeWeight[j] && lightList[j].Influence>0)
				{
					CPointLight	*pl= lightList[j].PointLight;

					// Get the original influence (ie for static PLs, biLinear influence)
					float		bkupInf= lightList[j].BkupInfluence;
					// Get the attenuation of the pointLight to the model
					float		distToModel= lightList[j].DistanceToModel;
					float		attInf= pl->computeLinearAttenuation(modelPos, distToModel);
					// Attenuate the color of the light by biLinear and distance/spot attenuation.
					float		lightInf= attInf * bkupInf;
					// Modulate also by the percentage to merge
					lightInf*= lightToMergeWeight[j];
					// Add the full ambient term of the light, attenuated by light attenuation and WeightMerge
					mergedAmbient.R+= pl->getAmbient().R * lightInf;
					mergedAmbient.G+= pl->getAmbient().G * lightInf;
					mergedAmbient.B+= pl->getAmbient().B * lightInf;
					// Add 0.25f of the diffuse term of the light, attenuated by light attenuation and WeightMerge
					// mul by 0.25f, because this is the averaged contribution of the diffuse part of a
					// light to a sphere (do the maths...).
					float	f= lightInf*0.25f;
					mergedAmbient.R+= pl->getDiffuse().R * f;
					mergedAmbient.G+= pl->getDiffuse().G * f;
					mergedAmbient.B+= pl->getDiffuse().B * f;
				}
			}


			// Setup the merged Light
			CRGBA	amb;
			sint	v;
			// Because of floating point error, it appears that sometime result may be slightly below 0.
			// => clamp necessary
			v= NLMISC::OptFastFloor(mergedAmbient.R);	fastClamp8(v);	amb.R= v;
			v= NLMISC::OptFastFloor(mergedAmbient.G);	fastClamp8(v);	amb.G= v;
			v= NLMISC::OptFastFloor(mergedAmbient.B);	fastClamp8(v);	amb.B= v;
			amb.A = 255;
			lightContrib.MergedPointLight= amb;

			// Indicate we use the merged pointLight => the model must recompute lighting each frame
			lightContrib.UseMergedPointLight= true;
		}
		else
		{
			// If the model is freezeHRC(), need to test each frame only if MergedPointLight is used.
			lightContrib.UseMergedPointLight= false;
		}
	}
	else
	{
		// point to end the list
		i= startId;
	}

	// End the list.
	if(i<NL3D_MAX_LIGHT_CONTRIBUTION)
	{
		lightContrib.PointLight[i]= NULL;
	}

}


// ***************************************************************************
void		CLightingManager::getDynamicPointLightList(const CVector &worldPos, std::vector<CPointLightInfluence>	&lightList)
{
	// For all quadGrids.
	for(uint qgId=0; qgId<NL3D_QUADGRID_LIGHT_NUM_LEVEL; qgId++)
	{
		CQuadGrid<CPointLightInfo>	&quadGrid= _LightQuadGrid[qgId];

		// select the lights around this position in the quadGrids.
		quadGrid.select(worldPos, worldPos);

		// for all possible found lights
		CQuadGrid<CPointLightInfo>::CIterator	itLight;
		for(itLight= quadGrid.begin(); itLight!=quadGrid.end(); itLight++)
		{
			// verify it includes the entity
			if( (*itLight).Sphere.include(worldPos) )
			{
				// ok, insert in list.
				CPointLightInfluence	pli;
				pli.PointLight= (*itLight).Light;
				// No special Influence degradation scheme for Dynamic lighting
				pli.Influence= 1;
				lightList.push_back( pli );
			}
		}
	}
}



} // NL3D
