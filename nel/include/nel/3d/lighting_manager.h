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

#ifndef NL_LIGHTING_MANAGER_H
#define NL_LIGHTING_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/quad_grid.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/point_light_influence.h"
#include <vector>


namespace NL3D {


class	CPointLight;
class	CLightContribution;
class	CTransform;
class	ILogicInfo;


// ***************************************************************************
// Yes, it's sound like a quadTree. same number of level for Light and Lighted models.
#define	NL3D_QUADGRID_LIGHT_NUM_LEVEL	4


// ***************************************************************************
/**
 * Owned by CLightingTrav. This class compute modelContributions.
 *	It gets which dynamic light may influence a model, ask to ILogicInfo the contribution of staticLights
 *	then decides which light are best suitable to influence the model.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLightingManager
{
public:
	/// An iterator on a model inserted in the ObjectQuadGrid. An id used for eraseStaticLightedModel()
	struct	CQGItLightedModel
	{
	private:
		friend class	CLightingManager;
		CQuadGrid<CTransform*>::CIterator	QgItes[NL3D_QUADGRID_LIGHT_NUM_LEVEL];
	};

public:

	/// Constructor (bSmallScene is used to setup size of the grids)
	CLightingManager(bool bSmallScene);


	/// \name Parameters
	// @{
	/** set the max number of point light that can influence a model. NB: clamped by NL3D_MAX_LIGHT_CONTRIBUTION
	 *	Default is 3.
	 *	NB: the sun contribution is not taken into account
	 */
	void		setMaxLightContribution(uint nlights);
	uint		getMaxLightContribution() const {return _MaxLightContribution;}

	/** Advanced. When a light has no attenuation, it's still inserted in a quadgrid with some radius and won't
	 *	influence models beyond. You can setup this radius with this method. Default is 1000m.
	 *	NB: nlassert(noAttLightRadius>0);
	 */
	void		setNoAttLightRadius(float noAttLightRadius);
	float		getNoAttLightRadius() const {return _NoAttLightRadius;}

	/** Advanced. When a model is out of [AttBegin, AttEnd] of a light, the computed influence of the light
	 *	used to choose "best lights" is not constant, and is a function of distance multiplied by a factor you can
	 *	setup here. Default is 0.1f and is good for lights with att like (50, 100) (arbitrary).
	 */
	void		setOutOfAttLightInfFactor(float outOfAttLightInfFactor);
	float		getOutOfAttLightInfFactor() const {return _OutOfAttLightInfFactor;}

	/** Advanced. When a model is influenced by more light than allowed, or when it reach the limits
	 *	of the light (attenuationEnd), the light can be darkened according to some threshold.
	 *	The resultLightColor begin to fade when distModelToLight== attEnd- threshold*(attEnd-attBegin).
	 *	when distModelToLight== 0, resultLightColor==Black.
	 *	By default, this value is 0.1f. Setting higher values will smooth transition but will
	 *	generally darken the global effects of lights.
	 *	NB: clamp(value, 0, 1);
	 */
	void		setLightTransitionThreshold(float lightTransitionThreshold);
	float		getLightTransitionThreshold() const  {return _LightTransitionThreshold;}

	// @}


	/// \name Dynamic Lights localisation.
	// @{
	/// clear for the pass all the lights.
	void		clearDynamicLights();
	/** temp add a dynamic light to the manager. light is added to the _LightQuadGrid.
	 *	This method calls CTransform::resetLighting() for all models around the light.
	 *
	 *	Additionaly light are added to a list (a vector of pointer), see getDynamicLightList()
	 */
	void		addDynamicLight(CPointLight *light);
	/** retrieve (for this pass only) list of all pointLights visible in scene
	 */
	const std::vector<CPointLight*>	&getAllDynamicLightList() const {return _DynamicLightList;}
	// @}


	/// \name Static Lighted Objects Localisation. Used for lights to touch nearest static models.
	// @{
	/** erase a lighted object to the _StaticLightedModelQuadGrid. must do it at deletion of the model or when it
	 *	leaves freeHRC state.
	 *	NB: default CQGItLightedModel (ie NULL) can be passed in.
	 *	\return quadgrid.end(), ie NULL iterator.
	 */
	CQGItLightedModel	eraseStaticLightedModel(CQGItLightedModel ite);
	/** insert a lighted object to the _StaticLightedModelQuadGrid. must not be inserted before
	 *	must do it only for static objects, ie when freeHRC state is validated (see CTransform::update())
	 *	NB: only lightable models with no AncestorSkeletonModel should be inserted
	 */
	CQGItLightedModel	insertStaticLightedModel(CTransform *model);
	// @}


	/** get the description of nearsest lights viewed for this model.
	 *	Dynamic lights are parsed to get list of lights, and ILogicInfo->getStaticLightDesc() is parsed too.
	 *	Then, maxLights are returned, which may be a blended result of max influence light.
	 *
	 *	NB: model->_LightContribution is filled with real contributions of lights.
	 *	NB: model is append to the _LightedModelList of each contributed light
	 *	NB: this list is valid until model->isNeedUpdateLighting() is true.
	 */
	void		computeModelLightContributions(NLMISC::CRGBA sunAmbient, CTransform *model, CLightContribution &lightContrib,
		ILogicInfo *logicInfo= NULL);


// ***********
private:

	/// get the list of dynamic light viewed from a position. append to lightList
	void		getDynamicPointLightList(const CVector &worldPos, std::vector<CPointLightInfluence>	&lightList);


private:

	struct	CPointLightInfo
	{
		CPointLight				*Light;
		NLMISC::CBSphere		Sphere;
	};


	// There is 4 levels of quadGrid for either Light and StaticLightedModels.
	// there is 4 _StaticLightedModelQuadGrid for addDynamicLight() with big light to not select too many squares.
	CQuadGrid<CTransform*>		_StaticLightedModelQuadGrid[NL3D_QUADGRID_LIGHT_NUM_LEVEL];
	CQuadGrid<CPointLightInfo>	_LightQuadGrid[NL3D_QUADGRID_LIGHT_NUM_LEVEL];
	// This is the radius a light can't override to fit in a quadGrid
	float						_LightQuadGridRadiusLimit[NL3D_QUADGRID_LIGHT_NUM_LEVEL];
	// List of dynamic lights for a render pass
	std::vector<CPointLight*>	_DynamicLightList;

	/// \name Parameters.
	// @{
	uint						_MaxLightContribution;
	float						_NoAttLightRadius;
	float						_OutOfAttLightInfFactor;
	float						_LightTransitionThreshold;
	// @}

};


} // NL3D


#endif // NL_LIGHTING_MANAGER_H

/* End of lighting_manager.h */
