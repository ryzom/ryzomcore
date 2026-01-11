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

#include "nel/3d/cluster.h"
#include "nel/3d/portal.h"
#include "nel/misc/stream.h"
#include "nel/misc/string_mapper.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"
//#include "mesh_instance.h"
#include "nel/3d/scene_group.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// 0.5 cm of precision
#define CLUSTERPRECISION 0.005


// ***************************************************************************
CCluster::CCluster ()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	FatherVisible = VisibleFromFather = false;
	FatherAudible = AudibleFromFather = false;
	Father = NULL;
	Group = NULL;

	// map a no fx string
	_EnvironmentFxId = CStringMapper::map("no fx");
	// map a no soundgroup string
	_SoundGroupId = CStringMapper::map("");

	// I am a transform cluster
	CTransform::setIsCluster(true);

	// Default: not traversed
	_Visited= false;
	_CameraIn= false;
}


// ***************************************************************************
CCluster::~CCluster()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	unlinkFromClusterTree();
}


void CCluster::setSoundGroup(const std::string &soundGroup)
{
	_SoundGroupId = CStringMapper::map(soundGroup);
}
void CCluster::setSoundGroup(const NLMISC::TStringId &soundGroupId)
{
	_SoundGroupId = soundGroupId;
}
const std::string &CCluster::getSoundGroup()
{
	return CStringMapper::unmap(_SoundGroupId);
}

NLMISC::TStringId CCluster::getSoundGroupId()
{
	return _SoundGroupId;
}
void CCluster::setEnvironmentFx(const std::string &environmentFx)
{
	_EnvironmentFxId = CStringMapper::map(environmentFx);
}
void CCluster::setEnvironmentFx(const NLMISC::TStringId &environmentFxId)
{
	_EnvironmentFxId = environmentFxId;
}
const std::string	&CCluster::getEnvironmentFx()
{
	return CStringMapper::unmap(_EnvironmentFxId);
}

NLMISC::TStringId CCluster::getEnvironmentFxId()
{
	return _EnvironmentFxId;
}



// ***************************************************************************
void CCluster::unlinkFromParent()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// unlink from father sons list
	if (Father)
	{
		 Father->Children.erase(remove(Father->Children.begin(), Father->Children.end(), this), Father->Children.end());
		 Father = NULL;
	}
}

// ***************************************************************************
void CCluster::unlinkSons()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// tells all sons that they have no more father
	for(uint k = 0; k < Children.size(); ++k)
	{
		if (Children[k]->Father == this)
		{
			Children[k]->Father = NULL;
		}
	}
	NLMISC::contReset(Children);
}



// ***************************************************************************
void CCluster::unlinkFromClusterTree()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	unlinkFromParent();
	unlinkSons();
}



// ***************************************************************************
void CCluster::registerBasic ()
{
	CScene::registerModel (ClusterId, 0, CCluster::creator);
}

// ***************************************************************************
bool CCluster::makeVolume (const CVector& p1, const CVector& p2, const CVector& p3)
{
	uint i;
	// Check if the plane is not close to a plane that already define the cluster
	for (i = 0; i < _LocalVolume.size(); ++i)
	{
		float f1 = fabsf (_LocalVolume[i]*p1);
		float f2 = fabsf (_LocalVolume[i]*p2);
		float f3 = fabsf (_LocalVolume[i]*p3);
		if ((f1 < CLUSTERPRECISION) && (f2 < CLUSTERPRECISION) && (f3 < CLUSTERPRECISION))
			return true;
	}
	// Check if we want to add a triangle not completely in the predefined volume
	for (i = 0; i < _LocalVolume.size(); ++i)
	{
		float f1 = _LocalVolume[i]*p1;
		float f2 = _LocalVolume[i]*p2;
		float f3 = _LocalVolume[i]*p3;
		if ((f1 > CLUSTERPRECISION) && (f2 > CLUSTERPRECISION) && (f3 > CLUSTERPRECISION))
			return false;
	}
	// Build volume
	CPlane p;
	p.make (p1, p2, p3);
	p.normalize();
	_LocalVolume.push_back (p);
	// Build BBox
	if (_LocalVolume.size() == 1)
		_LocalBBox.setCenter(p1);
	else
		_LocalBBox.extend(p1);
	_LocalBBox.extend(p2);
	_LocalBBox.extend(p3);
	_Volume = _LocalVolume;
	_BBox = _LocalBBox;
	return true;
}

// ***************************************************************************
bool CCluster::isIn (const CVector& p)
{
	for (uint i = 0; i < _Volume.size(); ++i)
		if (_Volume[i]*p > CLUSTERPRECISION)
			return false;
	return true;
}


// ***************************************************************************
bool CCluster::isIn (const CAABBox& b)
{
	for (uint i = 0; i < _Volume.size(); ++i)
	{
		if (!b.clipBack (_Volume[i]))
			return false;
	}
	return true;
}

// ***************************************************************************
bool CCluster::isIn (const NLMISC::CVector& center, float size)
{
	for (uint i = 0; i < _Volume.size(); ++i)
		if (_Volume[i]*center > size)
			return false;
	return true;
}

// ***************************************************************************
bool CCluster::clipSegment (NLMISC::CVector &p0, NLMISC::CVector &p1)
{
	for (uint i = 0; i < _Volume.size(); ++i)
	{
		if (!_Volume[i].clipSegmentBack(p0, p1))
			return false;
	}
	return true;
}

// ***************************************************************************
void CCluster::resetPortalLinks ()
{
	_Portals.clear();
}

// ***************************************************************************
void CCluster::link (CPortal* portal)
{
	_Portals.push_back (portal);
}

// ***************************************************************************
void CCluster::unlink (CPortal* portal)
{
	uint32 pos;
	for (pos = 0; pos < _Portals.size(); ++pos)
	{
		if (_Portals[pos] == portal)
			break;
	}
	if (pos < _Portals.size())
		_Portals.erase (_Portals.begin()+pos);
}

// ***************************************************************************
void CCluster::serial (NLMISC::IStream&f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	sint version = f.serialVersion (3);

	if (version >= 1)
		f.serial (Name);

	f.serialCont (_LocalVolume);
	f.serial (_LocalBBox);
	f.serial (FatherVisible);
	f.serial (VisibleFromFather);
	if (f.isReading())
	{
		_Volume = _LocalVolume;
		_BBox = _LocalBBox;
	}

	if (version >= 2)
	{
		if (f.isReading())
		{
			std::string soundGroup;
			std::string envFxName;

			f.serial(soundGroup);
			_SoundGroupId = CStringMapper::map(soundGroup);

			f.serial(envFxName);
			if (envFxName.empty())
				envFxName = "no fx";
			_EnvironmentFxId = CStringMapper::map(envFxName);
		}
		else
		{
			// write the sound group
			std::string soundGroup = CStringMapper::unmap(_SoundGroupId);
			f.serial(soundGroup);
			// write the env fx name
			std::string envFxName = CStringMapper::unmap(_EnvironmentFxId);
			if (envFxName == "no fx")
				envFxName.clear();
			f.serial(envFxName);
		}

//		nldebug("Cluster %s, sound group [%s]", Name.c_str(), CStringMapper::unmap(_SoundGroupId).c_str());
	}

	if (version >= 3)
	{
		f.serial(AudibleFromFather);
		f.serial(FatherAudible);
	}
	else
	{
		// copy the visual property
		AudibleFromFather = VisibleFromFather;
		FatherAudible = FatherVisible;
	}
}

// ***************************************************************************
void CCluster::setWorldMatrix (const CMatrix &WM)
{
	uint32 i;
	CMatrix invWM = WM;
	invWM.invert();

	// Transform the volume
	for (i = 0; i < _LocalVolume.size(); ++i)
		_Volume[i] = _LocalVolume[i] * invWM;

	_BBox = NLMISC::CAABBox::transformAABBox(WM, _LocalBBox);

	// Transform the bounding box
	/*CVector p[8];
	p[0].x = _LocalBBox.getMin().x;
	p[0].y = _LocalBBox.getMin().y;
	p[0].z = _LocalBBox.getMin().z;

	p[1].x = _LocalBBox.getMax().x;
	p[1].y = _LocalBBox.getMin().y;
	p[1].z = _LocalBBox.getMin().z;

	p[2].x = _LocalBBox.getMin().x;
	p[2].y = _LocalBBox.getMax().y;
	p[2].z = _LocalBBox.getMin().z;

	p[3].x = _LocalBBox.getMax().x;
	p[3].y = _LocalBBox.getMax().y;
	p[3].z = _LocalBBox.getMin().z;

	p[4].x = _LocalBBox.getMin().x;
	p[4].y = _LocalBBox.getMin().y;
	p[4].z = _LocalBBox.getMax().z;

	p[5].x = _LocalBBox.getMax().x;
	p[5].y = _LocalBBox.getMin().y;
	p[5].z = _LocalBBox.getMax().z;

	p[6].x = _LocalBBox.getMin().x;
	p[6].y = _LocalBBox.getMax().y;
	p[6].z = _LocalBBox.getMax().z;

	p[7].x = _LocalBBox.getMax().x;
	p[7].y = _LocalBBox.getMax().y;
	p[7].z = _LocalBBox.getMax().z;

	for (i = 0; i < 8; ++i)
		p[i] = WM.mulPoint(p[i]);

	CAABBox boxTemp;

	boxTemp.setCenter(p[0]);
	for (i = 1; i < 8; ++i)
		boxTemp.extend(p[i]);
	_BBox = boxTemp;*/
}

// ***************************************************************************
void CCluster::traverseHrc ()
{
	CTransform::traverseHrc ();

	setWorldMatrix (_WorldMatrix);

	for (uint32 i = 0; i < getNbPortals(); ++i)
	{
		CPortal *pPortal = getPortal(i);
		pPortal->setWorldMatrix (_WorldMatrix);
	}

	// Re affect the cluster to the accelerator if not the root
	if (!isRoot())
	{
		Group->_ClipTrav->unregisterCluster(this);
		Group->_ClipTrav->registerCluster (this);
	}
}

// ***************************************************************************
bool CCluster::clip ()
{
	return true;
}


// ***************************************************************************
void CCluster::traverseClip ()
{
	// This is the root call called by the SceneRoot
	recursTraverseClip(NULL);
}


// ***************************************************************************
void CCluster::recursTraverseClip(CTransform *caller)
{
	if (_Visited)
		return;
	_Visited = true;

	// The cluster is visible because we are in it
	// So clip the models attached (with MOT links) to the cluster
	uint	num= clipGetNumChildren();
	uint32	i;
	for(i=0;i<num;i++)
		clipGetChild(i)->traverseClip();

	// Debug visible clusters
	CClipTrav &clipTrav = getOwnerScene()->getClipTrav();
	if (clipTrav.getClusterVisibilityTracking())
	{
		clipTrav.addVisibleCluster(this);
	}

	// And look through portals
	for (i = 0; i < getNbPortals(); ++i)
	{
		CPortal*pPortal = getPortal (i);
		vector<CPlane> WorldPyrTemp = clipTrav.WorldPyramid;
		bool backfaceclipped = false;
		CCluster *pOtherSideCluster;
		if (pPortal->getCluster(0) == this)
			pOtherSideCluster = pPortal->getCluster (1);
		else
			pOtherSideCluster = pPortal->getCluster (0);

		if (Father != NULL)
		if (caller == Father) // If the caller is the father
		if (VisibleFromFather)
			// Backface clipping
			if( !pPortal->isInFront( clipTrav.CamPos ))
				backfaceclipped = true;

		if (!backfaceclipped && pOtherSideCluster)
		{
			/* If the otherSide cluster is fully visible because the camera is IN, then don't need to clip.
				This is important to landscape test, to ensure that pyramid are strictly equal from 2 paths which
				come from the 2 clusters where the camera start
			*/
			if (pOtherSideCluster->isCameraIn() || pPortal->clipPyramid (clipTrav.CamPos, clipTrav.WorldPyramid))
			{
				pOtherSideCluster->recursTraverseClip(this);
			}
		}

		clipTrav.WorldPyramid = WorldPyrTemp;
	}

	// Link up in hierarchy
	if ((FatherVisible)&&(Father != NULL))
	{
		Father->recursTraverseClip(this);
	}

	// Link down in hierarchy
	for (i = 0; i < Children.size(); ++i)
	if (Children[i]->VisibleFromFather)
	{
		Children[i]->recursTraverseClip(this);
	}

	_Visited = false;
}


// ***************************************************************************
void CCluster::applyMatrix(const NLMISC::CMatrix &m)
{
	uint32 i;
	CMatrix invM = m;
	invM.invert();
	nlassert(_Volume.size() == _LocalVolume.size());

	// Transform the volume
	for (i = 0; i < _LocalVolume.size(); ++i)
	{
		_Volume[i] = _Volume[i] * invM;
		_LocalVolume[i] = _LocalVolume[i] * invM;
	}

	// Transform the bounding boxes
	_BBox = NLMISC::CAABBox::transformAABBox(m, _BBox);
	_LocalBBox = NLMISC::CAABBox::transformAABBox(m, _LocalBBox);
}


// ***************************************************************************
void CCluster::cameraRayClip(const CVector &start, const CVector &end, std::vector<CCluster*> &clusterVisited)
{
	uint	i;
	if (_Visited)
		return;
	_Visited = true;

	// The cluster is visible because we are in it. add it to the list of cluster (if not already inserted)
	for(i=0;i<clusterVisited.size();i++)
	{
		if(clusterVisited[i]==this)
			break;
	}
	if(i==clusterVisited.size())
		clusterVisited.push_back(this);

	// look through portals
	for (i = 0; i < getNbPortals(); ++i)
	{
		CPortal*pPortal = getPortal (i);
		CCluster *pOtherSideCluster;
		if (pPortal->getCluster(0) == this)
			pOtherSideCluster = pPortal->getCluster (1);
		else
			pOtherSideCluster = pPortal->getCluster (0);

		/*
		bool backfaceclipped = false;
		if (Father != NULL)
		if (caller == Father) // If the caller is the father
		if (VisibleFromFather)
			// Backface clipping
			if( !pPortal->isInFront( clipTrav.CamPos ))
				backfaceclipped = true;
		if (!backfaceclipped && pOtherSideCluster)*/

		if (pOtherSideCluster)
		{
			if (pPortal->clipRay (start, end))
			{
				pOtherSideCluster->cameraRayClip(start, end, clusterVisited);
			}
		}
	}

	/* Link up in hierarchy. Test the Inverse Flag, cause the path is inverted here!!!
		ie: if I allow the camera to go out, it MUST can re-enter (ie if I am VisibleFromFather)
	*/
	if ((VisibleFromFather)&&(Father != NULL))
	{
		Father->cameraRayClip(start, end, clusterVisited);
	}

	// Link down in hierarchy
	for (i = 0; i < Children.size(); ++i)
	{
		// same remark. test FatherVisible, not VisibleFromFather
		if (Children[i]->FatherVisible)
		{
			Children[i]->cameraRayClip(start, end, clusterVisited);
		}
	}

	_Visited = false;
}


} // NL3D
