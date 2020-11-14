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

#include "nel/3d/light_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/root_model.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/point_light_model.h"
#include "nel/3d/scene.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace	NL3D
{

// ***************************************************************************
CLightTrav::CLightTrav(bool bSmallScene) : LightingManager(bSmallScene)
{
	_LightedList.resize(1024);
	_CurrentNumVisibleModels= 0;

	LightingSystemEnabled= false;
}

// ***************************************************************************
void		CLightTrav::clearLightedList()
{
	_CurrentNumVisibleModels= 0;
}


// ***************************************************************************
void		CLightTrav::addPointLightModel(CPointLightModel *pl)
{
	_DynamicLightList.insert(pl, &pl->_PointLightNode);
}


// ***************************************************************************
void		CLightTrav::traverse()
{
	H_AUTO( NL3D_TravLight );

	uint i;


	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;

	/* for each visible lightable transform, reset them only if they have MergedPointLight.
		NB: dynamic objetcs don't need it because already done in traverseHRC()
		(but don't worry, reset state is flagged, so resetLighting() no op...)
		This is important only for static object (freezeHRC()).
		Why? because we are not sure the MergedPointLight does not represent moving DynamicPointLights.
		Actually, it surely does. Because most of the static light setup return<=2 lights, and MaxLightContribution
		is typically==3. So any additional light may surely be a dynamic one.
		NB: this may not be useful since dynamicLights resetLighting() of all models in range. But this is important
		when the dynamic light leave the model quiclky! (because don't dirt the model).
		NB: this is also useful only if there is no dynamic light but the ones merged in MergedPointLight.
		Because dynamic always reset their old attached models (see below). This still can arise if for example
		_MaxLightContribution=2 and there is a FrozenStaticLightSetup of 2 lights....
	*/
	for(i=0; i<_CurrentNumVisibleModels; i++ )
	{
		// if the model has a MergedPointLight, reset him (NB: already done for dynamics models)
		if(_LightedList[i]->useMergedPointLight())
		{
			_LightedList[i]->resetLighting();
		}
	}


	// By default, lightmaped objects are not lit by any light
	CLight	noLight;
	noLight.setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, CVector::K);
	Scene->getDriver()->setLightMapDynamicLight(false, noLight);

	// clear the quadGrid of dynamicLights
	LightingManager.clearDynamicLights();

	// for each lightModel, process her: recompute position, resetLightedModels(), and append to the quadGrid.
	CPointLightModel	**pLight= _DynamicLightList.begin();
	uint	numPls= _DynamicLightList.size();
	for(;numPls>0;numPls--, pLight++)
	{
		(*pLight)->traverseLight();
	}

	// for each visible lightable transform
	for(i=0; i<_CurrentNumVisibleModels; i++ )
	{
		// traverse(), to recompute light contribution (if needed).
		_LightedList[i]->traverseLight();
	}

}


// ***************************************************************************
void		CLightTrav::reserveLightedList(uint numModels)
{
	// enlarge only.
	if(numModels>_LightedList.size())
		_LightedList.resize(numModels);
}


}
