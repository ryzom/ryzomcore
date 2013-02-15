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

#include "nel/3d/quad_grid_clip_manager.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/clip_trav.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/cluster.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
void	CQuadGridClipManager::registerBasic()
{
	CScene::registerModel(QuadGridClipManagerId, TransformId, CQuadGridClipManager::creator);
}


// ***************************************************************************
CQuadGridClipManager::CQuadGridClipManager()
{
	_ClusterSize= 0;
	_X= _Y= 0;
	_Width= _Height= 0;
}
// ***************************************************************************
CQuadGridClipManager::~CQuadGridClipManager()
{
	reset();
}


// ***************************************************************************
void				CQuadGridClipManager::init(float clusterSize, uint numDist, float maxDist, float radiusMax )
{
	// reset first.
	reset();

	// copy params.
	nlassert(clusterSize>0);
	_ClusterSize= clusterSize;
	_MaxDist= maxDist;
	_NumDist= numDist;
	_RadiusMax= radiusMax;
}

// ***************************************************************************
void				CQuadGridClipManager::reset()
{
	// delete the clusters.
	if(getOwnerScene())
	{
		sint	oldX0, oldX1, oldY0, oldY1;

		oldX0= _X;
		oldX1= _X+_Width;
		oldY0= _Y;
		oldY1= _Y+_Height;

		for(sint y=oldY0; y<oldY1; y++)
		{
			for(sint x=oldX0; x<oldX1; x++)
			{
				deleteCaseModels(&getOwnerScene()->getClipTrav(), x,y);
			}
		}

		// clear the grid.
		_QuadGridClusterCases.clear();
	}

	// reset others params.
	_ClusterSize= 0;
	_X= _Y= 0;
	_Width= _Height= 0;
}

// ***************************************************************************
void				CQuadGridClipManager::updateClustersFromCamera(const CVector &camPos)
{
	H_AUTO( NL3D_QuadClip_updateClusters );

	CClipTrav *pClipTrav= &getOwnerScene()->getClipTrav();

	sint	newX0, newX1, newY0, newY1;
	sint	oldX0, oldX1, oldY0, oldY1;

	oldX0= _X;
	oldX1= _X+_Width;
	oldY0= _Y;
	oldY1= _Y+_Height;

	// compute the new square of clusters to build.
	newX0= (sint)floor( (camPos.x - _RadiusMax) / _ClusterSize);
	newX1= (sint)ceil( (camPos.x + _RadiusMax) / _ClusterSize);
	newY0= (sint)floor( (camPos.y - _RadiusMax) / _ClusterSize);
	newY1= (sint)ceil( (camPos.y + _RadiusMax) / _ClusterSize);

	// keep an histeresis of one cluster: do not delete "young" clusters created before.
	if(newX0>= oldX0+1)		// NB: if newX0==oldX0+1, then newX0= _X => no change.
		newX0--;
	if(newY0>= oldY0+1)		// same reasoning.
		newY0--;
	if(newX1<= oldX1-1)		// NB: if newX1==oldX1-1, then newX1= oldX1 => no change.
		newX1++;
	if(newY1<= oldY1-1)		// same reasoning.
		newY1++;

	// Have we got to update the array.
	if(newX0!=oldX0 || newX1!=oldX1 || newY0!=oldY0 || newY1!=oldY1)
	{
		sint	x,y;

		// delete olds models.
		// simple: test all cases
		for(y=oldY0; y<oldY1; y++)
		{
			for(x=oldX0; x<oldX1; x++)
			{
				// if out the new square?
				if(x<newX0 || x>=newX1 || y<newY0 || y>=newY1)
					deleteCaseModels(pClipTrav, x,y);
			}
		}

		// build new array
		// satic for alloc optimisation.
		static	vector<CQuadGridClipCluster*>	newQuadGridClusterCases;
		sint	newWidth= newX1-newX0;
		sint	newHeight= newY1-newY0;
		newQuadGridClusterCases.resize(newWidth * newHeight);
		// simple: test all cases
		for(y=newY0; y<newY1; y++)
		{
			for(x=newX0; x<newX1; x++)
			{
				CQuadGridClipCluster	*&newCase= newQuadGridClusterCases[ (y-newY0)*newWidth + (x-newX0) ];

				// if out the old square?
				if(x<oldX0 || x>=oldX1 || y<oldY0 || y>=oldY1)
				{
					// build the 2D bbox where the models should fit.
					CAABBox		pivotBBox;
					pivotBBox.setMinMax(
						CVector(x*_ClusterSize,y*_ClusterSize,0),
						CVector((x+1)*_ClusterSize,(y+1)*_ClusterSize,0) );
					// build new case. Clusters are empty.
					newCaseModels(newCase, pivotBBox);
				}
				else
				{
					// copy from old.
					CQuadGridClipCluster	*oldCase= _QuadGridClusterCases[ (y-_Y)*_Width + (x-_X) ];

					newCase= oldCase;
				}
			}
		}

		// just copy from new.
		_QuadGridClusterCases= newQuadGridClusterCases;
		_X= newX0;
		_Y= newY0;
		_Width= newWidth;
		_Height= newHeight;
	}
}


// ***************************************************************************
bool				CQuadGridClipManager::linkModel(CTransformShape *pTfmShp)
{
	H_AUTO( NL3D_QuadClip_linkModel );

	// use the position to get the cluster to use.
	CAABBox box;
	pTfmShp->getAABBox (box);

	// Transform the box in the world
	const CMatrix &wm = pTfmShp->getWorldMatrix();
	// compute center in world.
	CVector cLocal  = box.getCenter();
	CVector cWorld  = wm * cLocal;
	// prepare bbox.
	CAABBox worldBBox;
	worldBBox.setCenter(cWorld);
	CVector hs = box.getHalfSize();

	// For 8 corners.
	for(uint i=0;i<8;i++)
	{
		CVector		corner;
		// compute the corner of the bbox.
		corner= cLocal;
		if(i&1)		corner.x+=hs.x;
		else		corner.x-=hs.x;
		if((i/2)&1)	corner.y+=hs.y;
		else		corner.y-=hs.y;
		if((i/4)&1)	corner.z+=hs.z;
		else		corner.z-=hs.z;
		// Transform the corner in world.
		corner = wm * corner;
		// Extend the bbox with it.
		worldBBox.extend(corner);
	}


	// Position in the grid.
	sint	x,y;
	x= (sint)floor( cWorld.x / _ClusterSize);
	y= (sint)floor( cWorld.y / _ClusterSize);

	// verify if IN the current grid of clusters created.
	if( x>=_X && x<_X+_Width && y>=_Y && y<_Y+_Height )
	{
		// add the model and extend the bbox of this cluster.
		CQuadGridClipCluster	*cluster= _QuadGridClusterCases[ (y-_Y)*_Width + (x-_X) ];

		// if this cluster is empty, add it now to the list of not empty (do the test before add)
		if( cluster->isEmpty() )
		{
			_NotEmptyQuadGridClipClusters.insert(cluster, &cluster->ListNode);
		}

		// add the model => no more empty
		cluster->addModel(worldBBox, pTfmShp);

		return true;
	}
	else
	{
		return false;
	}

}


// ***************************************************************************
void				CQuadGridClipManager::deleteCaseModels(CClipTrav *pClipTrav, sint x, sint y)
{
	H_AUTO( NL3D_QuadClip_deleteCaseModels );

	nlassert(x>=_X && x<_X+_Width && y>=_Y && y<_Y+_Height);

	CQuadGridClipCluster	*cluster= _QuadGridClusterCases[ (y-_Y)*_Width + (x-_X) ];

	// first, unlink all sons from cluster, and link thems to RootCluster.
	cluster->resetSons(pClipTrav);

	// delete the cluster. NB: auto-unlinked from _NotEmptyQuadGridClipClusters
	delete cluster;

	// NB: do not delete array for alloc/free optimisation.
}


// ***************************************************************************
void				CQuadGridClipManager::newCaseModels(CQuadGridClipCluster *&cluster, const NLMISC::CAABBox &pivotBbox)
{
	H_AUTO( NL3D_QuadClip_newCaseModels );

	// create clusters.
	cluster= new CQuadGridClipCluster(_NumDist, _MaxDist, pivotBbox);
}


// ***************************************************************************
void				CQuadGridClipManager::traverseClip()
{
	CClipTrav *pClipTrav= &getOwnerScene()->getClipTrav();

	// Run All NotEmpty Clusters,
	CQuadGridClipCluster	**it;
	it= _NotEmptyQuadGridClipClusters.begin();
	uint	numClusters= _NotEmptyQuadGridClipClusters.size();
	for(;numClusters>0;numClusters--,it++)
	{
		(*it)->clip(pClipTrav);
	}
}


// ***************************************************************************
void				CQuadGridClipManager::profile() const
{
	nlinfo(" ***** CQuadGridClipManager stats");
	nlinfo(" There is %d clusters", _Width*_Height );
	sint	numEmptyClusters= 0;
	float	minAreaRatio= 1000;
	float	maxAreaRatio= 0;
	float	meanAreaRatio= 0;

	// test all cases
	for(sint y=0;y<_Height;y++)
	{
		for(sint x=0;x<_Width;x++)
		{
			const CQuadGridClipCluster	*cluster= _QuadGridClusterCases[y*_Width+x];
			if( cluster->isEmpty() )
				numEmptyClusters++;
			else
			{
				CAABBox	bb= cluster->getBBox();
				float	area= (bb.getSize().x*bb.getSize().y) / (_ClusterSize*_ClusterSize);
				meanAreaRatio+= area;
				minAreaRatio= min( minAreaRatio, area);
				maxAreaRatio= max( maxAreaRatio, area);
			}
		}
	}

	// display
	if( numEmptyClusters==_Width*_Height )
	{
		nlinfo( " The ClipManager is completely Empty!!!!");
	}
	else
	{
		sint	numClusters= _Width*_Height-numEmptyClusters;
		nlinfo( "    This Level has %d clusters not empty over %d", numClusters, _Width*_Height);
		meanAreaRatio/= numClusters;
		nlinfo("     . minAreaRatio= %f", minAreaRatio);
		nlinfo("     . maxAreaRatio= %f", maxAreaRatio);
		nlinfo("     . meanAreaRatio= %f", meanAreaRatio);

		// Do Stats per distance setup
		for(uint lvl=0;lvl<_NumDist+1;lvl++)
		{
			nlinfo("    * Stats for Distance up to %f m", lvl<_NumDist?(lvl+1)*_MaxDist/_NumDist:-1.0f);
			sint	minNumChildren= 100000000;
			sint	maxNumChildren= 0;
			float	meanNumChildren= 0;
			sint	totalNumChildren= 0;

			// test all cases
			for(sint y=0;y<_Height;y++)
			{
				for(sint x=0;x<_Width;x++)
				{
					const CQuadGridClipCluster	*cluster= _QuadGridClusterCases[y*_Width+x];
					if( cluster->isEmpty() )
						numEmptyClusters++;
					else
					{
						// count number of sons
						sint	numSons;
						numSons= cluster->profileNumChildren(lvl);
						meanNumChildren+= numSons;
						totalNumChildren+= numSons;
						minNumChildren= min( minNumChildren, numSons);
						maxNumChildren= max( maxNumChildren, numSons);
					}
				}
			}

			// display
			meanNumChildren/= numClusters;
			nlinfo("     . minNumChildren= %d", minNumChildren);
			nlinfo("     . maxNumChildren= %d", maxNumChildren);
			nlinfo("     . meanNumChildren= %f", meanNumChildren);
			nlinfo("     . totalNumChildren= %d", totalNumChildren);
		}
	}
}


} // NL3D
