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

#include "nel/3d/skeleton_shape.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/scene.h"
#include "nel/misc/bsphere.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
void	CSkeletonShape::CLod::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(Distance);
	f.serialCont(ActiveBones);
}


// ***************************************************************************
CSkeletonShape::CSkeletonShape()
{
	// By default for now....
	// Temp. Have a huge BBox, so clip badly.
	_BBox.setCenter(CVector(0,0,1.5));
	_BBox.setSize(CVector(3,3,3));
}


// ***************************************************************************
sint32			CSkeletonShape::getBoneIdByName(const std::string &name) const
{
	std::map<std::string, uint32>::const_iterator	it= _BoneMap.find(name);
	if(it==_BoneMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
void			CSkeletonShape::build(const std::vector<CBoneBase> &bones)
{
	uint	i;

	// copy bones.
	_Bones= bones;

	// for all bones
	for(i=0;i<_Bones.size();i++)
	{
		// build the map.
		_BoneMap[_Bones[i].Name]= i;
		// validate distances.
		_Bones[i].LodDisableDistance= max(0.f, _Bones[i].LodDisableDistance);

		// get fahter dist.
		sint32	fatherId= _Bones[i].FatherId;
		// if father exist and is not "always enabled"
		if(fatherId>=0 && _Bones[fatherId].LodDisableDistance!=0)
		{
			float	fatherDist= _Bones[fatherId].LodDisableDistance;
			// I must disable me at least before my father (never after).
			if(_Bones[i].LodDisableDistance==0)
				_Bones[i].LodDisableDistance= fatherDist;
			else
				_Bones[i].LodDisableDistance= min(_Bones[i].LodDisableDistance, fatherDist);
		}
	}

	// build Lod Information.
	//==============
	_Lods.clear();

	// build all distances used.
	set<float>	distSet;
	for(i=0;i<_Bones.size();i++)
	{
		float	dist= _Bones[i].LodDisableDistance;
		// if lod enabled for this bone, add a new distance, or do nothing
		if(dist>0)
			distSet.insert(dist);
	}

	// create a lod for each distance used + 1 (the "dist==0" distance).
	_Lods.resize(distSet.size() + 1);
	// create the default lod: all bones activated.
	_Lods[0].Distance=0;
	_Lods[0].ActiveBones.resize(_Bones.size(), 0xFF);

	// For each lods not 0th.
	set<float>::iterator	it= distSet.begin();
	for(uint j=1; j<_Lods.size(); j++, it++)
	{
		float	lodDist= *it;
		// set the distance of activation
		_Lods[j].Distance= lodDist;
		// resize and default to all enabled.
		_Lods[j].ActiveBones.resize(_Bones.size(), 0xFF);

		// Search what lod are to be disabled at this distance.
		for(i=0;i<_Bones.size();i++)
		{
			float	dist= _Bones[i].LodDisableDistance;
			// if the dist of the lod is greater (or equal) to the disableDist of the bone,
			// and if the bone is not "always enabled", disable the bone
			if(lodDist>=dist && dist!=0 )
				_Lods[j].ActiveBones[i]= 0;
		}

	}

}


// ***************************************************************************
void			CSkeletonShape::retrieve(std::vector<CBoneBase> &bones) const
{
	bones= _Bones;
}


// ***************************************************************************
CTransformShape		*CSkeletonShape::createInstance(CScene &scene)
{
	// Create a CSkeletonModel, an instance of a mesh.
	//===============================================
	CSkeletonModel		*sm= (CSkeletonModel*)scene.createModel(NL3D::SkeletonModelId);
	sm->Shape= this;

	// setup bones.
	//=================
	sm->Bones.reserve(_Bones.size());
	for(sint i=0;i<(sint)_Bones.size();i++)
	{
		// Append a new bone.
		sm->Bones.push_back( CBone(&_Bones[i]) );

		// Must set the Animatable father of the bone (the skeleton model!).
		sm->Bones[i].setFather(sm, CSkeletonModel::OwnerBit);
	}

	// Must create and init skeleton bone usage to 0.
	sm->initBoneUsages();

	// For skinning: setup skeleton in Skin LoadBalancing group
	sm->setLoadBalancingGroup("Skin");

	return sm;
}

// ***************************************************************************
void			CSkeletonShape::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- _Lods.
	*/
	sint	ver= f.serialVersion(1);

	f.serialCont(_Bones);
	f.serialCont(_BoneMap);

	if(ver>=1)
		f.serialCont(_Lods);
	else
	{
		// create a skeleton shape with bones activated all the time
		_Lods.resize(1);
		// create the default lod: all bones activated.
		_Lods[0].Distance=0;
		_Lods[0].ActiveBones.resize(_Bones.size(), 0xFF);
	}
}

// ***************************************************************************

float CSkeletonShape::getNumTriangles (float distance)
{
	// No polygons
	return 0;
}


// ***************************************************************************
bool	CSkeletonShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}

	return true;
}


// ***************************************************************************
void		CSkeletonShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	bbox= _BBox;
}


// ***************************************************************************
uint		CSkeletonShape::getLodForDistance(float dist) const
{
	uint	start=0;
	uint	end= (uint)_Lods.size();
	// find lower_bound by dichotomy
	while(end-1>start)
	{
		uint	pivot= (end+start)/2;
		// return the lower_bound, ie return first start with _Lods[pivot].Distance<=dist
		if(_Lods[pivot].Distance <= dist)
			start= pivot;
		else
			end= pivot;
	}

	return start;
}


} // NL3D
