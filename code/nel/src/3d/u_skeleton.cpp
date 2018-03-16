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

#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/play_list_user.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene.h"
#include "nel/3d/shape_bank.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Skeleton )

#define	NL3D_HAUTO_UI_SKELETON						H_AUTO_USE( NL3D_UI_Skeleton )

// ***************************************************************************

uint		USkeleton::getNumBoneComputed() const
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();

	return object->getNumBoneComputed();
}

// ***************************************************************************

void		USkeleton::setInterpolationDistance(float dist)
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	object->setInterpolationDistance(dist);
}

// ***************************************************************************

float		USkeleton::getInterpolationDistance() const
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	return object->getInterpolationDistance();
}

// ***************************************************************************

void		USkeleton::setShapeDistMax(float distMax)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(object && object->Shape)
	{
		object->Shape->setDistMax(distMax);
	}
}

// ***************************************************************************

float		USkeleton::getShapeDistMax() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(object && object->Shape)
	{
		return object->Shape->getDistMax();
	}
	else
		return -1;
}

// ***************************************************************************

bool		USkeleton::bindSkin(UInstance mi)
{
	NL3D_HAUTO_UI_SKELETON;

	if(mi.empty())
	{
		nlerror("USkeleton::bindSkin(): mi is NULL");
		return false;
	}
	CTransform			*trans= dynamic_cast<CTransform*>(mi.getObjectPtr());
	CMeshBaseInstance	*meshi= dynamic_cast<CMeshBaseInstance*>(trans);
	if(meshi==NULL)
	{
		nlerror("USkeleton::bindSkin(): mi is not a MeshInstance or MeshMRMInstance");
		return false;
	}
	CSkeletonModel	*object = getObjectPtr();
	return object->bindSkin(meshi);
}

// ***************************************************************************

void		USkeleton::stickObject(UTransform mi, uint boneId)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(mi.empty())
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= dynamic_cast<CTransform*>(mi.getObjectPtr());
	object->stickObject(trans, boneId);
}

// ***************************************************************************

void		USkeleton::stickObjectEx(UTransform mi, uint boneId, bool forceCLod)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(mi.empty())
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= dynamic_cast<CTransform*>(mi.getObjectPtr());
	object->stickObjectEx(trans, boneId, forceCLod);
}

// ***************************************************************************

void		USkeleton::detachSkeletonSon(UTransform mi)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(mi.empty())
		nlerror("USkeleton::detachSkeletonSon(): mi is NULL");
	CTransform		*trans= dynamic_cast<CTransform*>(mi.getObjectPtr());
	object->detachSkeletonSon(trans);
}

// ***************************************************************************

uint		USkeleton::getNumBones() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return (uint)object->Bones.size();
}

// ***************************************************************************

UBone		USkeleton::getBone(uint boneId) const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(boneId>=object->Bones.size())
		nlerror("getBone(): bad boneId");
	return UBone (&(object->Bones[boneId]));
}

// ***************************************************************************

sint		USkeleton::getBoneIdByName(const std::string &boneName) const
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	return object->getBoneIdByName(boneName);
}

// ***************************************************************************

bool		USkeleton::isBoneComputed(uint boneId) const
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	return object->isBoneComputed(boneId);
}

// ***************************************************************************

bool USkeleton::forceComputeBone(uint boneId)
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	return object->forceComputeBone(boneId);
}

// ***************************************************************************

void		USkeleton::setLodCharacterShape(sint shapeId)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setLodCharacterShape(shapeId);
}

// ***************************************************************************

sint		USkeleton::getLodCharacterShape() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getLodCharacterShape();
}

// ***************************************************************************

void		USkeleton::enableLOD(bool isEnable)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->enableLOD(isEnable);
}

// ***************************************************************************

void		USkeleton::setLodCharacterAnimId(uint animId)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setLodCharacterAnimId(animId);
}

// ***************************************************************************

uint		USkeleton::getLodCharacterAnimId() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getLodCharacterAnimId();
}

// ***************************************************************************

void		USkeleton::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setLodCharacterAnimTime(time);
}

// ***************************************************************************

TGlobalAnimationTime	USkeleton::getLodCharacterAnimTime() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getLodCharacterAnimTime();
}

// ***************************************************************************

bool		USkeleton::isDisplayedAsLodCharacter() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->isDisplayedAsLodCharacter();
}

// ***************************************************************************

void		USkeleton::setLodCharacterDistance(float dist)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setLodCharacterDistance(dist);
}

// ***************************************************************************

float		USkeleton::getLodCharacterDistance() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getLodCharacterDistance();
}

// ***************************************************************************

void		USkeleton::setLodCharacterWrapMode(bool wrapMode)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setLodCharacterWrapMode(wrapMode);
}

// ***************************************************************************

bool		USkeleton::getLodCharacterWrapMode() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getLodCharacterWrapMode();
}

// ***************************************************************************

void		USkeleton::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
}

// ***************************************************************************

bool		USkeleton::computeRenderedBBox(NLMISC::CAABBox &bbox, bool computeInWorld)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->computeRenderedBBox(bbox, computeInWorld);
}

// ***************************************************************************
bool		USkeleton::computeRenderedBBoxWithBoneSphere(NLMISC::CAABBox &bbox, bool computeInWorld)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->computeRenderedBBoxWithBoneSphere(bbox, computeInWorld);
}

// ***************************************************************************

bool		USkeleton::computeCurrentBBox(NLMISC::CAABBox &bbox, UPlayList *playList, double playTime, bool forceCompute /* = false */, bool computeInWorld)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();

	if(playList!=NULL)
	{
		CPlayListUser	*plUser= static_cast<CPlayListUser*>(playList);
		plUser->evalPlayList(playTime);
	}

	return object->computeCurrentBBox(bbox, forceCompute, computeInWorld);
}

// ***************************************************************************

void		USkeleton::computeLodTexture()
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	object->computeLodTexture();
}

// ***************************************************************************

void		USkeleton::setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl)
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	object->setBoneAnimCtrl(boneId, ctrl);
}

// ***************************************************************************

IAnimCtrl	*USkeleton::getBoneAnimCtrl(uint boneId) const
{
	NL3D_HAUTO_UI_SKELETON;

	CSkeletonModel	*object = getObjectPtr();
	return object->getBoneAnimCtrl(boneId);
}

// ***************************************************************************
void					USkeleton::setSSSWOPos(const NLMISC::CVector &pos)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setSSSWOPos(pos);
}

// ***************************************************************************
const NLMISC::CVector	&USkeleton::getSSSWOPos() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getSSSWOPos();
}

// ***************************************************************************
void					USkeleton::setSSSWODir(const NLMISC::CVector &dir)
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	object->setSSSWODir(dir);
}

// ***************************************************************************
const NLMISC::CVector	&USkeleton::getSSSWODir() const
{
	NL3D_HAUTO_UI_SKELETON;
	CSkeletonModel	*object = getObjectPtr();
	return object->getSSSWODir();
}

// ***************************************************************************
const std::string		&USkeleton::getShapeName() const
{
	NL3D_HAUTO_UI_SKELETON;

	static std::string emptyStr;

	CSkeletonModel	*object = getObjectPtr();
	if(!object)
		return emptyStr;

	// get the shape bank
	CScene *scene= object->getOwnerScene();
	CShapeBank	*sb= scene->getShapeBank();
	if(!sb)
		return emptyStr;

	// get the shape name
	const std::string *str= sb->getShapeNameFromShapePtr(object->Shape);
	if(str)
		return *str;
	else
		return emptyStr;

}

// ***************************************************************************
void USkeleton::getStickedObjects(std::vector<UTransform> &sticks)
{
	NL3D_HAUTO_UI_SKELETON;

	sticks.clear();

	CSkeletonModel	*sm= getObjectPtr();
	if(!sm)
		return;

	const std::set<CTransform*>	&stickSet= sm->getStickedObjects();
	std::set<CTransform*>::const_iterator	it= stickSet.begin();
	sticks.reserve(stickSet.size());
	for(;it!=stickSet.end();it++)
	{
		sticks.push_back(*it);
	}
}

// ***************************************************************************
void USkeleton::cast(UTransform object)
{
	attach(dynamic_cast<CSkeletonModel*>(object.getObjectPtr()));
}

// ***************************************************************************
void			USkeleton::setLodEmit(NLMISC::CRGBA emit)
{
	getObjectPtr()->setLodEmit(emit);
}

// ***************************************************************************
NLMISC::CRGBA	USkeleton::getLodEmit() const
{
	return getObjectPtr()->getLodEmit();
}


} // NL3D
