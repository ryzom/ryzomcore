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

#include "nel/3d/u_instance.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_base.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/mesh_multi_lod_instance.h"
#include "nel/3d/seg_remanence.h"
#include "nel/misc/debug.h"
#include "nel/3d/scene.h"
#include "nel/3d/shape_bank.h"


using	namespace NLMISC;

namespace NL3D
{


// ***************************************************************************

void UInstance::getShapeAABBox(NLMISC::CAABBox &bbox) const
{
	CTransformShape	*object = getObjectPtr();
	object->getAABBox(bbox);
}

// ***************************************************************************
void UInstance::setBlendShapeFactor (const std::string &blendShapeName, float factor, bool /* dynamic */)
{
	CTransformShape	*object = getObjectPtr();
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(object);

	if (mi)
	{
		mi->setBlendShapeFactor (blendShapeName, factor);
	}
}

// ***************************************************************************
void		UInstance::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	CTransformShape	*object = getObjectPtr();
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(object);

	// Just for CMeshBaseInstance.
	if (mi)
	{
		mi->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}


// ***************************************************************************
void		UInstance::setShapeDistMax(float distMax)
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->Shape)
	{
		object->Shape->setDistMax(distMax);
	}
}

// ***************************************************************************
float		UInstance::getShapeDistMax() const
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->Shape)
	{
		return object->Shape->getDistMax();
	}
	else
		return -1;
}


// ***************************************************************************
void		UInstance::selectTextureSet(uint id)
{
	CTransformShape	*object = getObjectPtr();
	CMeshBaseInstance *mbi  = dynamic_cast<CMeshBaseInstance *>(object);
	if (mbi)
		mbi->selectTextureSet(id);
}


// ***************************************************************************
void		UInstance::enableAsyncTextureMode(bool enable)
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		mbi->enableAsyncTextureMode(enable) ;
	}
}
// ***************************************************************************
bool		UInstance::getAsyncTextureMode() const
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		return mbi->getAsyncTextureMode() ;
	}
	else
		return false;
}
// ***************************************************************************
void		UInstance::startAsyncTextureLoading()
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		mbi->startAsyncTextureLoading(getPos());
	}
}
// ***************************************************************************
bool		UInstance::isAsyncTextureReady()
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		return mbi->isAsyncTextureReady();
	}
	else
		return true;
}
// ***************************************************************************
void		UInstance::setAsyncTextureDistance(float dist)
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		mbi->setAsyncTextureDistance(dist);
	}
}
// ***************************************************************************
float		UInstance::getAsyncTextureDistance() const
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		return mbi->getAsyncTextureDistance();
	}
	else
		return 0.f;
}
// ***************************************************************************
void		UInstance::setAsyncTextureDirty(bool flag)
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		mbi->setAsyncTextureDirty(flag);
	}
}
// ***************************************************************************
bool		UInstance::isAsyncTextureDirty() const
{
	CTransformShape	*object = getObjectPtr();
	if(object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		return mbi->isAsyncTextureDirty();
	}
	else
		return false;
}

// ***************************************************************************
void UInstance::setCoarseMeshDist(float dist)
{
	CTransformShape	*object = getObjectPtr();
	CMeshMultiLodInstance *mmli = dynamic_cast<CMeshMultiLodInstance *>(object);
	if (mmli) mmli->setCoarseMeshDist(dist);
}

// ***************************************************************************
float UInstance::getCoarseMeshDist() const
{
	CTransformShape	*object = getObjectPtr();
	CMeshMultiLodInstance *mmli = dynamic_cast<CMeshMultiLodInstance *>(object);
	return mmli ? mmli->getCoarseMeshDist() : -1.f;
}

// ***************************************************************************
void UInstance::setSliceTime(float duration)
{
	CTransformShape	*object = getObjectPtr();
	CSegRemanence *sr = dynamic_cast<CSegRemanence *>(object);
	if (!sr) return;
	sr->setSliceTime(duration);
}

// ***************************************************************************
float UInstance::getSliceTime() const
{
	CTransformShape	*object = getObjectPtr();
	CSegRemanence *sr = dynamic_cast<CSegRemanence *>(object);
	if (!sr) return 0.f;
	return sr->getSliceTime();
}

// ***************************************************************************
bool UInstance::supportMaterialRendering(UDriver &drv, bool forceBaseCaps)
{
	CTransformShape	*object = getObjectPtr();
	IDriver *driver = static_cast<CDriverUser*>(&drv)->getDriver ();
	const uint count = object->getNumMaterial ();
	uint i;
	for (i=0; i<count; i++)
	{
		if (!object->getMaterial (i)->isSupportedByDriver(*driver, forceBaseCaps))
			return false;
	}
	return true;
}

// ***************************************************************************

uint UInstance::getNumMaterials() const
{
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Object);
	if(mi)
		return (uint)mi->Materials.size();
	else
		return 0;
}

// ***************************************************************************

UInstanceMaterial UInstance::getMaterial(uint materialId)
{
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Object);
	nlassertex (mi, ("Should be a CMeshBaseInstance object. Call getNumMaterials() first."));

	// create user mats.
	return UInstanceMaterial (mi, &mi->Materials[materialId], &mi->AsyncTextures[materialId]);
}

// ***************************************************************************

bool UInstance::canStartStop()
{
	CTransformShape	*object = getObjectPtr();
	return object->canStartStop();
}

// ***************************************************************************

void UInstance::start()
{
	CTransformShape	*object = getObjectPtr();
	object->start();
}

// ***************************************************************************

void UInstance::stop()
{
	CTransformShape	*object = getObjectPtr();
	object->stop();
}

// ***************************************************************************

bool UInstance::isStarted() const
{
	CTransformShape	*object = getObjectPtr();
	return object->isStarted();
}

// ***************************************************************************

float UInstance::getDistMax() const
{
	CTransformShape	*object = getObjectPtr();
	return object->getDistMax();
}

// ***************************************************************************

void UInstance::setDistMax(float distMax)
{
	CTransformShape	*object = getObjectPtr();
	object->setDistMax(distMax);
}

// ***************************************************************************
UShape		UInstance::getShape() const
{

	CTransformShape	*object = getObjectPtr();
	if(!object)
		return UShape();

	// get the shape name
	return UShape(object->Shape);
}

// ***************************************************************************
const std::string &UInstance::getShapeName() const
{

	static std::string emptyStr;

	CTransformShape	*object = getObjectPtr();
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
void	UInstance::cast(UTransform object)
{
	attach(dynamic_cast<CTransformShape*>(object.getObjectPtr()));
}

// ***************************************************************************
bool	UInstance::getDefaultPos (CVector &pos) const
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		CMeshBase *mb= (CMeshBase*)(IShape*)(mbi->Shape);
		pos= mb->getDefaultPos()->getDefaultValue();
		return true;
	}
	return false;
}

// ***************************************************************************
bool	UInstance::getDefaultRotQuat (CQuat &rotQuat) const
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		CMeshBase *mb= (CMeshBase*)(IShape*)(mbi->Shape);
		rotQuat= mb->getDefaultRotQuat()->getDefaultValue();
		return true;
	}
	return false;
}

// ***************************************************************************
bool	UInstance::getDefaultScale (CVector &scale) const
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->isMeshBaseInstance())
	{
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		CMeshBase *mb= (CMeshBase*)(IShape*)(mbi->Shape);
		scale= mb->getDefaultScale()->getDefaultValue();
		return true;
	}
	return false;
}

// ***************************************************************************
void	UInstance::setRelativeScale (const CVector &rs)
{
	CTransformShape	*object = getObjectPtr();
	if(object && object->isMeshBaseInstance())
	{
		// get the default scale
		CMeshBaseInstance *mbi  = static_cast<CMeshBaseInstance *>(object);
		CMeshBase *mb= (CMeshBase*)(IShape*)(mbi->Shape);
		CVector scale= mb->getDefaultScale()->getDefaultValue();
		// scale it by the relative one
		scale.x*= rs.x;
		scale.y*= rs.y;
		scale.z*= rs.z;
		object->setScale(scale);
	}
}


} // NL3D
