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

#include "nel/3d/u_transform.h"
#include "nel/3d/transform.h"
#include "nel/3d/instance_group_user.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/scene.h"

H_AUTO_DECL( NL3D_Transform_Set_Cluster_System )

#define	NL3D_HAUTO_SET_CLUSTER_SYSTEM		H_AUTO_USE( NL3D_Transform_Set_Cluster_System )


using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
void UTransform::setClusterSystem (UInstanceGroup *pIG)
{
	NL3D_HAUTO_SET_CLUSTER_SYSTEM

	CTransform	*object = getObjectPtr();
	if (object->getForceClipRoot())
	{
		nlwarning("Transform has been flagged to be glued to the root, and thus can't be clusterized. See UTransform::setForceClipRoot(bool).");
		return;
	}
	if ((pIG == NULL) || (pIG == (UInstanceGroup*)-1))
	{
		if (pIG == NULL)
			object->setClusterSystem (NULL);
		else
			object->setClusterSystem ((CInstanceGroup*)-1);
	}
	else
		object->setClusterSystem (&((CInstanceGroupUser*)pIG)->getInternalIG());
}

// ***************************************************************************
UInstanceGroup *UTransform::getClusterSystem () const
{
	CTransform	*object = getObjectPtr();
	CInstanceGroup	*ig= object->getClusterSystem();
	if(ig==((CInstanceGroup*)-1))
		return ((UInstanceGroup*)-1);
	else if(ig==NULL)
		return NULL;
	else
		return ig->getUserInterface();
}

// ***************************************************************************
void			UTransform::getLastParentClusters(std::vector<CCluster*> &clusters) const
{
	CTransform	*object = getObjectPtr();
	CScene *scene = object->getOwnerScene();
	// look in the list of parent of the transform object and extract the CCluster parents
	if (scene == NULL)
		return;

	CClipTrav	&clipTrav= scene->getClipTrav();

	uint	num= object->clipGetNumParents();
	for(uint i=0;i<num;i++)
	{
		CCluster *pcluster = dynamic_cast<CCluster*>(object->clipGetParent(i));
		if (pcluster != NULL)
			clusters.push_back(pcluster);
	}

	// If the object is link to a QuadCluster, add the RootCluster to the list
	CTransformShape	*trShp= dynamic_cast<CTransformShape*>( object );
	if( trShp && trShp->isLinkToQuadCluster() )
		clusters.push_back(clipTrav.RootCluster);
}


// ***************************************************************************
void			UTransform::freezeHRC()
{
	CTransform	*object = getObjectPtr();
	object->freezeHRC();
}

// ***************************************************************************
void			UTransform::unfreezeHRC()
{
	CTransform	*object = getObjectPtr();
	while (object)
	{
		object->unfreezeHRC();
		object = object->hrcGetParent();
	}
}


// ***************************************************************************
void			UTransform::setLoadBalancingGroup(const std::string &group)
{
	CTransform	*object = getObjectPtr();
	object->setLoadBalancingGroup(group);
}
// ***************************************************************************
const std::string	&UTransform::getLoadBalancingGroup() const
{
	CTransform	*object = getObjectPtr();
	return object->getLoadBalancingGroup();
}

// ***************************************************************************
void			UTransform::setMeanColor(NLMISC::CRGBA color)
{
	CTransform	*object = getObjectPtr();
	object->setMeanColor(color);
}
// ***************************************************************************
NLMISC::CRGBA	UTransform::getMeanColor() const
{
	CTransform	*object = getObjectPtr();
	return object->getMeanColor();
}

// ***************************************************************************
const CMatrix	&UTransform::getLastWorldMatrixComputed() const
{
	CTransform	*object = getObjectPtr();
	return object->getWorldMatrix();
}

// ***************************************************************************
void			UTransform::enableCastShadowMap(bool state)
{
	CTransform	*object = getObjectPtr();
	object->enableCastShadowMap(state);
}

// ***************************************************************************
bool			UTransform::canCastShadowMap() const
{
	CTransform	*object = getObjectPtr();
	return object->canCastShadowMap();
}

// ***************************************************************************
void			UTransform::enableReceiveShadowMap(bool state)
{
	CTransform	*object = getObjectPtr();
	object->enableReceiveShadowMap(state);
}

// ***************************************************************************
bool			UTransform::canReceiveShadowMap() const
{
	CTransform	*object = getObjectPtr();
	return object->canReceiveShadowMap();
}

// ***************************************************************************
void			UTransform::parent(UTransform newFather)
{
	CTransform	*object = getObjectPtr();
	if (object->getForceClipRoot())
	{
		nlwarning("Transform has been flagged to be glued to the root, can't change parent. See UTransform::setForceClipRoot(bool).");
		return;
	}
	if(!newFather.empty())
	{
		// link me to other.
		CTransform	*other= newFather.getObjectPtr();
		if(other->getOwnerScene()!=object->getOwnerScene())
			nlerror("Try to parent 2 object from 2 differnet scenes!!");
		other->hrcLinkSon( object );
	}
	else
	{
		// link me to Root.
		object->getOwnerScene()->getRoot()->hrcLinkSon( object );
	}
}

// ***************************************************************************
void UTransform::hide()
{
	CTransform	*object = getObjectPtr();
	object->hide();
}

// ***************************************************************************

void UTransform::show()
{
	CTransform	*object = getObjectPtr();
	object->show();
}

// ***************************************************************************

void UTransform::setUserClipping(bool enable)
{
	CTransform	*object = getObjectPtr();
	object->setUserClipping(enable);
}

// ***************************************************************************

bool UTransform::getUserClipping() const
{
	CTransform	*object = getObjectPtr();
	return object->getUserClipping();
}

// ***************************************************************************

void UTransform::heritVisibility()
{
	CTransform	*object = getObjectPtr();
	object->heritVisibility();
}

// ***************************************************************************

UTransform::TVisibility UTransform::getVisibility()
{
	CTransform	*object = getObjectPtr();
	return (UTransform::TVisibility)(uint32)object->getVisibility();
}

// ***************************************************************************

void UTransform::setOrderingLayer(uint layer)
{
	CTransform	*object = getObjectPtr();
	object->setOrderingLayer(layer);
}

// ***************************************************************************

uint UTransform::getOrderingLayer() const
{
	CTransform	*object = getObjectPtr();
	return object->getOrderingLayer();
}

// ***************************************************************************

void UTransform::setUserLightable(bool enable)
{
	CTransform	*object = getObjectPtr();
	object->setUserLightable(enable);
}

// ***************************************************************************

bool UTransform::getUserLightable() const
{
	CTransform	*object = getObjectPtr();
	return  object->getUserLightable();
}

// ***************************************************************************

void UTransform::setLogicInfo(ILogicInfo *logicInfo)
{
	CTransform	*object = getObjectPtr();
	object->setLogicInfo(logicInfo);
}

// ***************************************************************************

bool UTransform::getLastWorldVisState() const
{
	CTransform	*object = getObjectPtr();
	return object->isHrcVisible();
}

// ***************************************************************************

bool UTransform::getLastClippedState() const
{
	CTransform	*object = getObjectPtr();
	return object->isClipVisible();
}

// ***************************************************************************

void UTransform::setTransparency(bool v)
{
	CTransform	*object = getObjectPtr();
	object->setTransparency(v);
}

// ***************************************************************************

void UTransform::setOpacity(bool v)
{
	CTransform	*object = getObjectPtr();
	object->setOpacity(v);
}

// ***************************************************************************
void UTransform::setBypassLODOpacityFlag(bool bypass)
{
	CTransform	*object = getObjectPtr();
	object->setBypassLODOpacityFlag(bypass);
}

// ***************************************************************************

uint32 UTransform::isOpaque()
{
	CTransform	*object = getObjectPtr();
	return object->isOpaque();
}

// ***************************************************************************

uint32 UTransform::isTransparent()
{
	CTransform	*object = getObjectPtr();
	return object->isTransparent();
}

// ***************************************************************************

void UTransform::setForceClipRoot(bool forceClipRoot)
{
	CTransform	*object = getObjectPtr();
	object->setForceClipRoot(forceClipRoot);
}

// ***************************************************************************

bool UTransform::getForceClipRoot() const
{
	CTransform	*object = getObjectPtr();
	return object->getForceClipRoot();
}

// ***************************************************************************

void UTransform::setTransparencyPriority(uint8 priority)
{
	CTransform	*object = getObjectPtr();
	object->setTransparencyPriority(priority);
}

// ***************************************************************************
void UTransform::setShadowMapDirectionZThreshold(float zthre)
{
	CTransform	*object = getObjectPtr();
	object->setShadowMapDirectionZThreshold(zthre);
}

// ***************************************************************************
float UTransform::getShadowMapDirectionZThreshold() const
{
	CTransform	*object = getObjectPtr();
	return object->getShadowMapDirectionZThreshold();
}

// ***************************************************************************
void UTransform::setShadowMapMaxDepth(float depth)
{
	CTransform	*object = getObjectPtr();
	object->setShadowMapMaxDepth(depth);
}

// ***************************************************************************
float	UTransform::getShadowMapMaxDepth() const
{
	CTransform	*object = getObjectPtr();
	return object->getShadowMapMaxDepth();
}

// ***************************************************************************
bool	UTransform::supportFastIntersect() const
{
	CTransform	*object = getObjectPtr();
	return object->supportFastIntersect();
}

// ***************************************************************************
bool	UTransform::fastIntersect(const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D)
{
	CTransform	*object = getObjectPtr();
	return object->fastIntersect(p0, dir, dist2D, distZ, computeDist2D);
}


} // NL3D
