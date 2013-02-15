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

#include "nel/3d/quad_grid_clip_cluster.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/cluster.h"


using namespace NLMISC;
using namespace std;


namespace NL3D
{


H_AUTO_DECL( NL3D_QuadClip_ClusterClip );
H_AUTO_DECL( NL3D_QuadClip_SonsShowNoClip );


// ***************************************************************************
// 3 -> 400,200,100,50
#define NL3D_QUADGRID_CLIP_CLUSTER_DEPTH				2


// ***************************************************************************
#define NL3D_QCC_LEFT_DOWN		0
#define NL3D_QCC_RIGHT_DOWN		1
#define NL3D_QCC_LEFT_UP		2
#define NL3D_QCC_RIGHT_UP		3


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CQuadGridClipClusterListDist::clipSons(uint minDistSetup)
{
	for(uint i=minDistSetup; i<Models.size();i++)
	{
		CTransformShape	** pModel= Models[i].begin();
		uint	nSons= Models[i].size();
		for(;nSons>0;nSons--, pModel++)
		{
			(*pModel)->traverseClip();
		}
	}
}

// ***************************************************************************
void		CQuadGridClipClusterListDist::insertModel(uint distSetup, CTransformShape *model)
{
	Models[distSetup].insert(model, &model->_QuadClusterListNode);
}


// ***************************************************************************
void		CQuadGridClipClusterListDist::resetSons(CClipTrav *clipTrav)
{
	for(uint i=0; i<Models.size();i++)
	{
		// clean up model list
		CTransformShape	** pModel= Models[i].begin();
		uint	nSons= Models[i].size();
		for(;nSons>0;nSons--, pModel++)
		{
			// link the model to the rootCluster
			clipTrav->RootCluster->clipAddChild(*pModel);
		}
		// unlink all my sons from me
		Models[i].clear();
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CQuadGridClipClusterQTreeNode::CQuadGridClipClusterQTreeNode()
{
	Sons[0]=NULL;
	Sons[1]=NULL;
	Sons[2]=NULL;
	Sons[3]=NULL;
	Empty= true;
	Owner= NULL;
	RootNode= false;
	LeafNode= false;
}


// ***************************************************************************
void	CQuadGridClipClusterQTreeNode::init(CQuadGridClipCluster *owner, uint level, bool rootNode, const NLMISC::CAABBox &pivot)
{
	Owner= owner;
	RootNode= rootNode;
	PivotBBox= pivot;

	// If not a leaf, create sons
	if(level>0)
	{
		LeafNode= false;

		// split pivot for sons
		CAABBox	pivotSon;
		pivotSon.setSize(PivotBBox.getHalfSize());
		float	xMin= PivotBBox.getMin().x/2;
		float	yMin= PivotBBox.getMin().y/2;
		float	xMax= PivotBBox.getMax().x/2;
		float	yMax= PivotBBox.getMax().y/2;
		float	xCenter= PivotBBox.getCenter().x/2;
		float	yCenter= PivotBBox.getCenter().y/2;
		// LeftDown
		pivotSon.setCenter( CVector(xMin+xCenter,yMin+yCenter,0) );
		Sons[NL3D_QCC_LEFT_DOWN]= new CQuadGridClipClusterQTreeNode;
		Sons[NL3D_QCC_LEFT_DOWN]->init(owner, level-1, false, pivotSon);
		// RightDown
		pivotSon.setCenter( CVector(xMax+xCenter,yMin+yCenter,0) );
		Sons[NL3D_QCC_RIGHT_DOWN]= new CQuadGridClipClusterQTreeNode;
		Sons[NL3D_QCC_RIGHT_DOWN]->init(owner, level-1, false, pivotSon);
		// LeftUp
		pivotSon.setCenter( CVector(xMin+xCenter,yMax+yCenter,0) );
		Sons[NL3D_QCC_LEFT_UP]= new CQuadGridClipClusterQTreeNode;
		Sons[NL3D_QCC_LEFT_UP]->init(owner, level-1, false, pivotSon);
		// RithgUp
		pivotSon.setCenter( CVector(xMax+xCenter,yMax+yCenter,0) );
		Sons[NL3D_QCC_RIGHT_UP]= new CQuadGridClipClusterQTreeNode;
		Sons[NL3D_QCC_RIGHT_UP]->init(owner, level-1, false, pivotSon);
	}
	else
	{
		LeafNode= true;
	}

	// Create the distMax list only if root or leaf. No models in interleaved branches.
	if( LeafNode)
		ListNode.Models.resize(Owner->_NumDistTotal);
}


// ***************************************************************************
CQuadGridClipClusterQTreeNode::~CQuadGridClipClusterQTreeNode()
{
	// erase sons
	uint i;
	for(i=0; i<4;i++)
	{
		if(Sons[i])
			delete Sons[i];
		Sons[i]= NULL;
	}

	// check my list
	for(i=0; i<ListNode.Models.size();i++)
	{
		nlassert(ListNode.Models[i].empty());
	}
}

// ***************************************************************************
void		CQuadGridClipClusterQTreeNode::clip(CClipTrav *clipTrav)
{
	// if empty (test important for branch and leave clusters)
	if(Empty)
		return;

	H_BEFORE( NL3D_QuadClip_NodeClip );

	// Then clip against pyramid
	bool	unspecified= false;
	bool	visible= true;
	for(sint i=0;i<(sint)clipTrav->WorldPyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		if(!BBoxExt.clipBack(clipTrav->WorldPyramid[i]))
		{
			visible= false;
			break;
		}
		// else test is the bbox is partially or fully in the plane
		else if(!unspecified)
		{
			// if clipFront AND clipBack, it means partially.
			if(BBoxExt.clipFront(clipTrav->WorldPyramid[i]))
				unspecified= true;
		}
	}

	H_AFTER( NL3D_QuadClip_NodeClip );

	// if visible, parse sons
	if(visible)
	{
		// clip sons or cluster sons
		if(unspecified)
		{
			if( LeafNode)
			{
				// clip DistMax.
				CVector		c= BBoxExt.getCenter();
				float		dist= (c - clipTrav->CamPos).norm();
				dist-= BBoxExt.getRadius();
				sint	minDistSetup= (sint)floor(Owner->_NumDist*dist/Owner->_DistMax);
				// NB if too far, set _NumDist (ie will clip only the infinite objects ones)
				clamp(minDistSetup, 0, (sint)Owner->_NumDist);

				// clip the sons individually
				H_AUTO( NL3D_QuadClip_SonsClip );
				ListNode.clipSons(minDistSetup);
			}
			else
			{
				// clip cluster sons
				Sons[0]->clip(clipTrav);
				Sons[1]->clip(clipTrav);
				Sons[2]->clip(clipTrav);
				Sons[3]->clip(clipTrav);
			}
		}
		else
		{
			// udpdate the sons, but don't clip, because we know they are fully visible.
			clipTrav->ForceNoFrustumClip= true;

			// show all cluster sons or sons
			noFrustumClip(clipTrav);

			// reset flag
			clipTrav->ForceNoFrustumClip= false;
		}
	}
}


// ***************************************************************************
void		CQuadGridClipClusterQTreeNode::noFrustumClip(CClipTrav *clipTrav)
{
	// if empty (test important for branch and leave clusters)
	if(Empty)
		return;

	// clip the sons
	if( LeafNode)
	{
		// clip DistMax.
		CVector		c= BBoxExt.getCenter();
		float		dist= (c - clipTrav->CamPos).norm();
		dist-= BBoxExt.getRadius();
		sint minDistSetup= (sint)floor(Owner->_NumDist*dist/Owner->_DistMax);
		// NB if too far, set _NumDist (ie will clip only the infinite objects ones)
		clamp(minDistSetup, 0, (sint)Owner->_NumDist);

		// clip the sons
		H_AUTO_USE( NL3D_QuadClip_SonsShowNoClip );
		ListNode.clipSons(minDistSetup);
	}
	else
	{
		// forceShow of cluster sons
		Sons[0]->noFrustumClip(clipTrav);
		Sons[1]->noFrustumClip(clipTrav);
		Sons[2]->noFrustumClip(clipTrav);
		Sons[3]->noFrustumClip(clipTrav);
	}
}


// ***************************************************************************
void		CQuadGridClipClusterQTreeNode::insertModel(const NLMISC::CAABBox &worldBBox, uint distSetup, CTransformShape *model)
{
	// if leaf node, insert the model in the list
	if( LeafNode )
	{
		if(Empty)
		{
			Empty= false;
			BBox= worldBBox;
		}
		else
		{
			// extend the bbox with 2 corners of the incoming bbox (sufficient for an AABBox).
			BBox.extend( worldBBox.getCenter() + worldBBox.getHalfSize() );
			BBox.extend( worldBBox.getCenter() - worldBBox.getHalfSize() );
		}

		// insert in list
		ListNode.insertModel(distSetup, model);
	}
	// else, recurs insert in branch
	else
	{
		// choose what son according to pivot.
		CQuadGridClipClusterQTreeNode	*selectSon;
		if( worldBBox.getCenter().y<PivotBBox.getCenter().y )
		{
			if( worldBBox.getCenter().x<PivotBBox.getCenter().x )
				selectSon= Sons[NL3D_QCC_LEFT_DOWN];
			else
				selectSon= Sons[NL3D_QCC_RIGHT_DOWN];
		}
		else
		{
			if( worldBBox.getCenter().x<PivotBBox.getCenter().x )
				selectSon= Sons[NL3D_QCC_LEFT_UP];
			else
				selectSon= Sons[NL3D_QCC_RIGHT_UP];
		}

		// insert in this cluster
		selectSon->insertModel(worldBBox, distSetup, model);

		// extend my boox according to this son cluster.
		if(Empty)
		{
			Empty= false;
			BBox= selectSon->BBox;
		}
		else
		{
			// extend the bbox with 2 corners of the son bbox (sufficient for an AABBox).
			BBox.extend( selectSon->BBox.getCenter() + selectSon->BBox.getHalfSize() );
			BBox.extend( selectSon->BBox.getCenter() - selectSon->BBox.getHalfSize() );
		}
	}

	// update bboxExt
	BBoxExt= BBox;
}

// ***************************************************************************
void		CQuadGridClipClusterQTreeNode::resetSons(CClipTrav *clipTrav)
{
	ListNode.resetSons(clipTrav);
	if(Sons[0])
	{
		Sons[0]->resetSons(clipTrav);
		Sons[1]->resetSons(clipTrav);
		Sons[2]->resetSons(clipTrav);
		Sons[3]->resetSons(clipTrav);
	}
}


// ***************************************************************************
void		CQuadGridClipClusterQTreeNode::profileNumChildren(uint distLevel, uint &result) const
{
	if(distLevel<ListNode.Models.size())
		result+= ListNode.Models[distLevel].size();
	// Add cluster sons
	if(Sons[0])
	{
		Sons[0]->profileNumChildren(distLevel, result);
		Sons[1]->profileNumChildren(distLevel, result);
		Sons[2]->profileNumChildren(distLevel, result);
		Sons[3]->profileNumChildren(distLevel, result);
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CQuadGridClipCluster::CQuadGridClipCluster(uint numDist, float distMax, const NLMISC::CAABBox &pivot)
{
	_DistMax= distMax;
	_NumDist= numDist;
	_NumDistTotal= _NumDist+1;

	_Root.init(this, NL3D_QUADGRID_CLIP_CLUSTER_DEPTH, true, pivot);
}

// ***************************************************************************
CQuadGridClipCluster::~CQuadGridClipCluster()
{
}

// ***************************************************************************
void		CQuadGridClipCluster::addModel(const NLMISC::CAABBox &worldBBox, CTransformShape *model)
{
	// check not already inserted
	nlassert(!model->_QuadClusterListNode.isLinked());

	// Add the model to the good distance list
	float	modelDistMax= model->getDistMax();
	sint	distSetup;
	if(modelDistMax<0)
		distSetup= _NumDist;
	else
	{
		distSetup= (sint)floor(_NumDist*modelDistMax/_DistMax);
		// NB: if >=_DistMax, then distSetup==_NumDist, meaning infinite distance (never dist clipped)
		clamp(distSetup, 0, (sint)_NumDist);
	}

	// add / recurs to the quadtree
	_Root.insertModel(worldBBox, distSetup, model);
}


// ***************************************************************************
void		CQuadGridClipCluster::removeModel(CTransformShape *model)
{
	model->_QuadClusterListNode.unlink();
}


// ***************************************************************************
void		CQuadGridClipCluster::clip(CClipTrav *clipTrav)
{
	H_AUTO_USE( NL3D_QuadClip_ClusterClip );

	// clip the quadtree
	_Root.clip(clipTrav);
}

// ***************************************************************************
void		CQuadGridClipCluster::resetSons(CClipTrav *clipTrav)
{
	_Root.resetSons(clipTrav);
}


// ***************************************************************************
sint		CQuadGridClipCluster::profileNumChildren(uint distLevel) const
{
	uint	result= 0;
	_Root.profileNumChildren(distLevel, result);
	return result;
}


} // NL3D
