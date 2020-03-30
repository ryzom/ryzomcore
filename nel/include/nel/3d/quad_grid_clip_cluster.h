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

#ifndef NL_QUAD_GRID_CLIP_CLUSTER_H
#define NL_QUAD_GRID_CLIP_CLUSTER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/clip_trav.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/fast_ptr_list.h"


namespace NL3D
{


class	CQuadGridClipCluster;


// ***************************************************************************
class CQuadGridClipClusterListDist
{
public:
	// An entry for each distance setup.
	std::vector<CFastPtrList<CTransformShape> >		Models;

public:
	// If 0 clipSons of all dist Setup, esle start from minDistSetup
	void		clipSons(uint minDistSetup);

	// insert a model in this listDist at the good place
	void		insertModel(uint distSetup, CTransformShape *model);

	// erase all models and relink to rootCluster
	void		resetSons(CClipTrav *clipTrav);

};


// ***************************************************************************
class CQuadGridClipClusterQTreeNode
{
public:
	CQuadGridClipCluster			*Owner;

	// 4 Sons
	CQuadGridClipClusterQTreeNode	*Sons[4];

	// List of objects inserted in this node
	CQuadGridClipClusterListDist	ListNode;

	// The BBox of this node.
	NLMISC::CAABBox					BBox;
	NLMISC::CAABBoxExt				BBoxExt;
	bool							Empty;

	// Am i the root?
	bool							RootNode;
	// Am i a leaf?
	bool							LeafNode;

	// The reference 2D BBox pivot to know how to insert models
	NLMISC::CAABBox					PivotBBox;

public:
	CQuadGridClipClusterQTreeNode();
	~CQuadGridClipClusterQTreeNode();

	// init me and sons
	void		init(CQuadGridClipCluster *owner, uint level, bool rootNode, const NLMISC::CAABBox &pivot);

	// clip the cluster or his sons
	void		clip(CClipTrav *clipTrav);

	// No cluster clip
	void		noFrustumClip(CClipTrav *clipTrav);

	// insert a model in this listDist at the good place
	void		insertModel(const NLMISC::CAABBox &worldBBox, uint distSetup, CTransformShape *model);

	// erase all models and relink to rootCluster
	void		resetSons(CClipTrav *clipTrav);

	// count numchildren and add sons
	void		profileNumChildren(uint distLevel, uint &result) const;
};


// ***************************************************************************
/**
 * A cluster of object for fast BBox clip.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CQuadGridClipCluster
{
public:
	// For insertion in the QuadGridClipManager
	CFastPtrListNode		ListNode;


public:
	/// Constructor
	CQuadGridClipCluster(uint numDist, float distMax, const NLMISC::CAABBox &pivot);
	~CQuadGridClipCluster();

	void		addModel(const NLMISC::CAABBox &worldBBox, CTransformShape *model);
	// NB: the BBox is not recomputed.
	void		removeModel(CTransformShape *model);

	void		clip(CClipTrav *clipTrav);

	// NB it is possible that profileNumChildren()==0 and isEmpty()==false!!
	bool					isEmpty() const {return _Root.Empty;}
	const NLMISC::CAABBox	&getBBox() const {return _Root.BBox;}
	sint					profileNumChildren(uint distLevel) const;

	void		resetSons(CClipTrav *clipTrav);

protected:
	friend class	CQuadGridClipClusterQTreeNode;

	// The max distance tested
	float									_DistMax;
	// The number of distance not infinite.
	uint									_NumDist;
	// _NumDist+1 (the infinite distance)
	uint									_NumDistTotal;

	// The Root of QuadTree.
	CQuadGridClipClusterQTreeNode			_Root;
};


} // NL3D


#endif // NL_QUAD_GRID_CLIP_CLUSTER_H

/* End of quad_grid_clip_cluster.h */
