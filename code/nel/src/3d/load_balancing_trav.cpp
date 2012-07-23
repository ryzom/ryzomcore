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

#include "nel/3d/load_balancing_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/transform.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
#define	NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER	50
#define	NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO		1.1f

namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLoadBalancingGroup::CLoadBalancingGroup()
{
	_PrecPolygonBalancingMode= CLoadBalancingGroup::PolygonBalancingOff;
	_NbFaceWanted= 20000;
	_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
	_DefaultGroup= false;

	_NbFacePass0= 0;
	_FaceRatio= 1;
}


// ***************************************************************************
void			CLoadBalancingGroup::computeRatioAndSmooth(TPolygonBalancingMode polMode)
{
	// If Default group, disable load balancing
	if(_DefaultGroup)
		polMode= PolygonBalancingOff;

	// Compute ratio
	switch(polMode)
	{
	case PolygonBalancingOff:
		_FaceRatio= 1;
		break;
	case PolygonBalancingOn	:
		if(_NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / _NbFacePass0;
		else
			_FaceRatio= 1;
		break;
	case PolygonBalancingClamp:
		if(_NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / _NbFacePass0;
		else
			_FaceRatio= 1;
		clamp(_FaceRatio, 0, 1);
		break;
		default: break;
	};

	// smooth the value.
	// if change of PolygonBalancingMode, reset the _ValueSmoother.
	if(polMode!=_PrecPolygonBalancingMode)
	{
		_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
		_PrecPolygonBalancingMode= polMode;
	}
	// if not PolygonBalancingOff, smooth the ratio.
	if(polMode!=PolygonBalancingOff)
	{
		// FIX: If the _FaceRatio is not a float (NaN or +-oo), don't add it!!
		if(isValidDouble(_FaceRatio))
			_ValueSmoother.addValue(_FaceRatio);
		float	fSmooth= _ValueSmoother.getSmoothValue();

		// If after smoothing, the number of faces is still too big, reduce smooth effect! (frustrum clip effect)
		if(fSmooth*_NbFacePass0 > _NbFaceWanted*NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO)
		{
			// reset the smoother
			_ValueSmoother.reset();
			// reduce smooth effect
			fSmooth= _FaceRatio*NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO;
			_ValueSmoother.addValue(fSmooth);
		}

		// take the smoothed value.
		_FaceRatio= fSmooth;
	}


}



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLoadBalancingTrav::CLoadBalancingTrav()
{
	PolygonBalancingMode= CLoadBalancingGroup::PolygonBalancingOff;

	// Add the default group and make it default
	_GroupMap["Default"].Name= "Default";
	_GroupMap["Default"]._DefaultGroup= true;

	// set the DefaultGroup ptr.
	_DefaultGroup= &_GroupMap["Default"];

	// prepare some space
	_VisibleList.resize(1024);
	_CurrentNumVisibleModels= 0;
}


// ***************************************************************************
void				CLoadBalancingTrav::clearVisibleList()
{
	_CurrentNumVisibleModels= 0;
}


// ***************************************************************************
void				CLoadBalancingTrav::traverse()
{
	H_AUTO( NL3D_TravLoadBalancing );

	CTravCameraScene::update();

	// Reset each group.
	//================
	ItGroupMap	it= _GroupMap.begin();
	for(;it!=_GroupMap.end();it++)
	{
		// reset _NbFacePass0.
		it->second._NbFacePass0= 0;
	}


	// Traverse the graph 2 times.

	// 1st pass, count NBFaces drawed.
	//================
	_LoadPass= 0;
	// count _NbFacePass0.
	traverseVisibilityList();


	// Reset _SumNbFacePass0
	_SumNbFacePass0= 0;
	// For each group
	it= _GroupMap.begin();
	for(;it!=_GroupMap.end();it++)
	{
		// compute ratio and smooth
		it->second.computeRatioAndSmooth(PolygonBalancingMode);
		// update _SumNbFacePass0
		_SumNbFacePass0+= it->second.getNbFaceAsked();
	}


	// 2nd pass, compute Faces that will be drawed.
	//================
	_LoadPass= 1;
	traverseVisibilityList();

}


// ***************************************************************************
void				CLoadBalancingTrav::traverseVisibilityList()
{
	// Traverse all nodes of the visibility list.
	for(uint i=0; i<_CurrentNumVisibleModels; i++)
	{
		CTransform	*model= _VisibleList[i];
		model->traverseLoadBalancing();
	}
}


// ***************************************************************************
float				CLoadBalancingTrav::getNbFaceAsked () const
{
	return _SumNbFacePass0;
}


// ***************************************************************************
CLoadBalancingGroup	*CLoadBalancingTrav::getOrCreateGroup(const std::string &group)
{
	// find
	ItGroupMap	it;
	it= _GroupMap.find(group);
	// if not exist, create.
	if(it==_GroupMap.end())
	{
		// create and set name.
		it= _GroupMap.insert(make_pair(group, CLoadBalancingGroup())).first;
		it->second.Name= group;
	}

	return &(it->second);
}

// ***************************************************************************
void				CLoadBalancingTrav::setGroupNbFaceWanted(const std::string &group, uint nFaces)
{
	// get/create if needed, and assign.
	getOrCreateGroup(group)->setNbFaceWanted(nFaces);
}

// ***************************************************************************
uint				CLoadBalancingTrav::getGroupNbFaceWanted(const std::string &group)
{
	// get/create if needed, and get.
	return getOrCreateGroup(group)->getNbFaceWanted();
}

// ***************************************************************************
float				CLoadBalancingTrav::getGroupNbFaceAsked (const std::string &group) const
{
	TGroupMap::const_iterator	it;
	it= _GroupMap.find(group);
	if(it==_GroupMap.end())
		return 0;
	else
		return it->second.getNbFaceAsked();
}


// ***************************************************************************
void				CLoadBalancingTrav::reserveVisibleList(uint numModels)
{
	// enlarge only.
	if(numModels>_VisibleList.size())
		_VisibleList.resize(numModels);
}


} // NL3D
