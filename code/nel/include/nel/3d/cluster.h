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

#ifndef NL_CLUSTER_H
#define NL_CLUSTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/plane.h"
#include "nel/misc/aabbox.h"

#include "nel/3d/transform.h"

#include <vector>

namespace NLMISC
{
class CRGBA;
class IStream;
struct EStream;
}

namespace NL3D {


class CPortal;
class CInstanceGroup;

const NLMISC::CClassId	ClusterId=NLMISC::CClassId(0x13f37e46, 0x3e880780);

/**
 * CCluster
 *
 * The objects are in the MOT clip traversal
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CCluster : public CTransform
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/


public:

	static void registerBasic ();

public:

	CCluster();

	~CCluster();

	/**
	 * Make the cluster volume from a plane made with 3 points
	 * All the faces passed must be connected with at least one of the others
	 * return false if the plane we want to add make the volume concave
	 */
	bool makeVolume (const NLMISC::CVector& p1, const NLMISC::CVector& p2, const NLMISC::CVector& p3);


	/// Apply the given matrix to this cluster local / world volume & bbox
	void applyMatrix(const NLMISC::CMatrix &m);

	/**
	 * return true if the vertex is in the cluster volume
	 */
	bool isIn (const NLMISC::CVector& p);

	/**
	 * return true if the Bounding box intersect the cluster
	 */
	bool isIn (const NLMISC::CAABBox& b);

	/**
	 * return true if the Sphere intersect the cluster
	 */
	bool isIn (const NLMISC::CVector& center, float size);

	/**
	 * return true if the segment intersect the cluster
	 *	start / end are the input and ouptut clipped segment
	 */
	bool clipSegment (NLMISC::CVector &p0, NLMISC::CVector &p1);


	NLMISC::CAABBox& getBBox() { return _BBox; }

	/// Linking

	// reset all links to portals.
	void resetPortalLinks ();

	void link (CPortal* portal);
	void unlink (CPortal* portal);

	uint32 getNbPortals() {return (uint32)_Portals.size();}
	CPortal* getPortal(uint32 pos) {return _Portals[pos];}

	/// Serial

	void serial (NLMISC::IStream& f);

	void setWorldMatrix (const NLMISC::CMatrix &WM);

	bool isRoot() { return _LocalVolume.size() == 0; }

	//\name Sound related.
	//@{
	void				setSoundGroup(const std::string &soundGroup);
	void				setSoundGroup(const NLMISC::TStringId &soundGroupId);
	const std::string	&getSoundGroup();
	NLMISC::TStringId	getSoundGroupId();
	void				setEnvironmentFx(const std::string &environmentFx);
	void				setEnvironmentFx(const NLMISC::TStringId &environmentFxId);
	const std::string	&getEnvironmentFx();
	NLMISC::TStringId	getEnvironmentFxId();
	//@}


	/// \name CTransform Specialisation
	// @{
	virtual void	traverseHrc();
	virtual void	traverseClip();
	virtual	bool	clip();
	// For ClipTrav only.
	void			setCameraIn(bool state) {_CameraIn= state;}
	bool			isCameraIn() const {return _CameraIn;}
	// @}

	// clusters to clusters calls
	void	recursTraverseClip(CTransform *caller);

	// Cluster Graph parsing with a ray (for CScene::findCameraClusterSystemFromRay())
	void	cameraRayClip(const CVector &start, const CVector &end, std::vector<CCluster*> &clusterVisited);

private:

	static CTransform *creator () {return new CCluster;}

public:
	// Moving cluster
	CQuadGrid<CCluster*>::CIterator AccelIt;

	// Additionnal properties
	std::string				Name;
	bool					FatherVisible;
	bool					VisibleFromFather;
	bool					FatherAudible;
	bool					AudibleFromFather;
	CCluster				*Father;
	std::vector<CCluster*>	Children;
	CInstanceGroup			*Group;

private:

	/// Portals list
	std::vector<CPortal*>	_Portals;

	/// Local bounding box and volume
	/// -----------------------------
	/// The bounding box of the volume
	NLMISC::CAABBox _LocalBBox;
	/// All planes are oriented to the exterior of the volume
	std::vector<NLMISC::CPlane> _LocalVolume;

	/// World bounding box and volume (valid after hrc trav)
	/// -----------------------------
	NLMISC::CAABBox _BBox;
	/// All planes are oriented to the exterior of the volume
	std::vector<NLMISC::CPlane> _Volume;

	/// Sound group name id
	NLMISC::TStringId	_SoundGroupId;
	/// Environement Fx name Id (using CStringMapper)
	NLMISC::TStringId	_EnvironmentFxId;

private:
	// This unlink this cluster from its sons and its parents
	void	unlinkFromClusterTree();
	//		unlink this cluster from its parent
	void	unlinkFromParent();
	//		unlink this cluster sons
	void	unlinkSons();

	/// ***** Clip Traversal
	bool	_Visited;
	// true if in clip pass the camera is in. valid only during clip pass...
	bool	_CameraIn;

	/// Friends classes
	friend class CInstanceGroup;
};


} // NL3D


#endif // NL_CLUSTER_H

/* End of cluster.h */
