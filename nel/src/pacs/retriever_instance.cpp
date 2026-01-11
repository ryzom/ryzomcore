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

#include "stdpacs.h"

#include "nel/pacs/retriever_instance.h"
#include "nel/pacs/global_retriever.h"

using namespace std;
using namespace NLMISC;



NLPACS::CRetrieverInstance::CRetrieverInstance()
{
	reset();
}


void	NLPACS::CRetrieverInstance::resetLinks()
{
	uint	i;
	// WARNING !!
	// this is a HARD reset !
	// only the instance i reset, no care about neighbors !!
	for (i=0; i<_Neighbors.size(); ++i)
		_Neighbors[i] = -1;
	_BorderChainLinks.clear();

	_ExteriorEdgeQuad.removeLinks();
}

void	NLPACS::CRetrieverInstance::resetLinks(uint32 id)
{
	vector<sint32>::iterator	rit;
	for (rit=_Neighbors.begin(); rit!=_Neighbors.end(); )
		if (*rit == (sint32)id)
			rit = _Neighbors.erase(rit);
		else
			++rit;

	uint	i;
	for (i=0; i<_BorderChainLinks.size(); ++i)
		if (_BorderChainLinks[i].Instance == (uint16)id)
			_BorderChainLinks[i].reset();

	_ExteriorEdgeQuad.removeLinks(id);
}

void	NLPACS::CRetrieverInstance::reset()
{
	// WARNING !!
	// this is a HARD reset !
	// only the instance i reset, no care about neighbors !!
	_NodesInformation.clear();
	_InstanceId = -1;
	_RetrieverId = -1;
	_Orientation = 0;
	_Origin = CVector::Null;
	_Type = CLocalRetriever::Landscape;
	_BorderChainLinks.clear();
	_ExteriorEdgeQuad.clear();

	resetLinks();
}

void	NLPACS::CRetrieverInstance::init(const CLocalRetriever &retriever)
{
	if (!retriever.isLoaded())
		return;
/*
	_NodesInformation.resize(retriever.getSurfaces().size());
	uint	i;

	// Resets _NodesInformation for later pathfinding graph annotation.
	for (i=0; i<_NodesInformation.size(); ++i)
	{
		CVector	pos = getGlobalPosition(retriever.getSurfaces()[i].getCenter());
		_NodesInformation[i].Position = CVector2f(pos.x, pos.y);
	}
*/

	_Type = retriever.getType();
	_BorderChainLinks.resize(retriever.getBorderChains().size());
}

void	NLPACS::CRetrieverInstance::make(sint32 instanceId, sint32 retrieverId, const CLocalRetriever &retriever,
										 uint8 orientation, const CVector &origin)
{
	if (_InstanceId != -1 || _RetrieverId != -1)
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::make");
		nlwarning("_InstanceId=%d _RetrieverId=%d _Orientation=%d", _InstanceId, _RetrieverId, _Orientation);
		nlwarning("instanceId=%d retrieverId=%d orientation=%d", instanceId, retrieverId, orientation);
		nlerror("Retriever instance %d has already been set", _InstanceId);
	}

	_InstanceId = instanceId;
	_RetrieverId = retrieverId;
	_Orientation = (orientation%4);
	_Origin = origin;

	_BBox = retriever.getBBox();
	_BBox.setCenter(_BBox.getCenter()+_Origin);

	if (_Orientation == 1 || _Orientation == 3)
	{
		CVector	hs = _BBox.getHalfSize();
		std::swap(hs.x, hs.y);
		_BBox.setHalfSize(hs);
	}

	init(retriever);
}

//
void	NLPACS::CRetrieverInstance::initEdgeQuad(NLPACS::CGlobalRetriever &gr)
{
	const CLocalRetriever	&lr = gr.getRetriever(_RetrieverId);

	if (lr.getType() != CLocalRetriever::Interior)
	{
		nlerror("Attempt to init the edgequad of instance %d whereas local retriever %d is not interior", _InstanceId, _RetrieverId);
	}

	// build the edge quad
	_ExteriorEdgeQuad.build(lr.getExteriorMesh(), gr, gr.getInternalCST(),_InstanceId);
}

void	NLPACS::CRetrieverInstance::linkEdgeQuad(NLPACS::CGlobalRetriever &gr)
{
	const CLocalRetriever				&lr = gr.getRetriever(_RetrieverId);
	const CExteriorMesh					&em = lr.getExteriorMesh();

	const vector<CExteriorEdgeEntry>	&ee = _ExteriorEdgeQuad.getEdgeEntries();

	// here we fill (partially) the _BorderChainLinks table
	uint	i;
	for (i=0; i<ee.size(); ++i)
	{
		const CExteriorMesh::CEdge	&edge = em.getEdge(ee[i].EdgeId);
		if (edge.Link != -1)
		{
			const CExteriorMesh::CLink		&link = em.getLink(edge.Link);

			if (link.SurfaceId != 0xFFFF && link.ChainId != 0xFFFF && link.BorderChainId != 0xFFFF)
			{
				CRetrieverInstance::CLink	&borderLink = _BorderChainLinks[link.BorderChainId];
				borderLink.ChainId = 0xFFFF;		// no opposite chain
				borderLink.BorderChainId = 0xFFFF;	// idem
				if ((borderLink.Instance == 0xFFFF && borderLink.SurfaceId == 0xFFFF) ||
					(borderLink.Instance == (uint16)(ee[i].Exterior.RetrieverInstanceId) &&
					 borderLink.SurfaceId == (uint16)(ee[i].Exterior.SurfaceId) ))
				{
					borderLink.Instance = (uint16)(ee[i].Exterior.RetrieverInstanceId);
					borderLink.SurfaceId = (uint16)(ee[i].Exterior.SurfaceId);
				}
				else
				{
					nlwarning("Instance %d, borderLink %d: link already set to inst=%d, surf=%d, try to set to inst=%d, surf=%d",
								_InstanceId, link.BorderChainId, borderLink.Instance, borderLink.SurfaceId,
								(uint16)(ee[i].Exterior.RetrieverInstanceId), (uint16)(ee[i].Exterior.SurfaceId));
				}
			}
		}
	}
}

/* Links the current retriever instance to another instance
 * on the given edge.
 */
void	NLPACS::CRetrieverInstance::link(CRetrieverInstance &neighbor,
										 const vector<CLocalRetriever> &retrievers)
{
	uint	i, j;
	for (i=0; i<_Neighbors.size(); ++i)
		if (_Neighbors[i] == neighbor._InstanceId)
			return;

	const CLocalRetriever						&retriever = retrievers[_RetrieverId];
	const CLocalRetriever						&nRetriever = retrievers[neighbor._RetrieverId];

	const vector<CChain>						&chains = retriever.getChains(),
												&nChains = nRetriever.getChains();
	const vector<uint16>						&borderChains = retriever.getBorderChains(),
												&nBorderChains = nRetriever.getBorderChains();

	vector< pair<CVector,CVector> >				chainTips,
												nChainTips;

	_BorderChainLinks.resize(borderChains.size());
	neighbor._BorderChainLinks.resize(nBorderChains.size());

/*
	for (i=0; i<borderChains.size(); ++i)
		chainTips.push_back(make_pair(retriever.getTip(chains[borderChains[i]].getStartTip()).Point,
									  retriever.getTip(chains[borderChains[i]].getStopTip()).Point));
*/
	for (i=0; i<borderChains.size(); ++i)
		chainTips.push_back(make_pair(retriever.getStartVector(borderChains[i]),
									  retriever.getStopVector(borderChains[i])));

	CVector	translation = neighbor._Origin - _Origin;
/*
	for (i=0; i<nBorderChains.size(); ++i)
		nChainTips.push_back(make_pair(nRetriever.getTip(nChains[nBorderChains[i]].getStartTip()).Point+translation,
									   nRetriever.getTip(nChains[nBorderChains[i]].getStopTip()).Point+translation));
*/
	for (i=0; i<nBorderChains.size(); ++i)
		nChainTips.push_back(make_pair(nRetriever.getStartVector(nBorderChains[i])+translation,
									   nRetriever.getStopVector(nBorderChains[i])+translation));

	for (i=0; i<borderChains.size(); ++i)
	{
		// if the chain is already linked, just step
		if (_BorderChainLinks[i].Instance != 0xFFFF || _BorderChainLinks[i].BorderChainId != 0xFFFF ||
			_BorderChainLinks[i].ChainId != 0xFFFF || _BorderChainLinks[i].SurfaceId != 0xFFFF)
			continue;

		float	bestDist = 1.0f;
		sint	best = -1;

		for (j=0; j<nBorderChains.size(); ++j)
		{
			if (neighbor._BorderChainLinks[j].Instance != 0xFFFF || neighbor._BorderChainLinks[j].BorderChainId != 0xFFFF ||
				neighbor._BorderChainLinks[j].ChainId != 0xFFFF || neighbor._BorderChainLinks[j].SurfaceId != 0xFFFF)
				continue;

			float	d = (chainTips[i].first-nChainTips[j].second).norm()+(chainTips[i].second-nChainTips[j].first).norm();
			if (d < bestDist)
			{
				bestDist = d;
				best = j;
			}
		}

		// if no best match, just don't link
		if (bestDist > 1.0e-1f || best == -1)
			continue;

		_BorderChainLinks[i].Instance = (uint16)neighbor._InstanceId;
		_BorderChainLinks[i].BorderChainId = (uint16)best;
		_BorderChainLinks[i].ChainId = nBorderChains[_BorderChainLinks[i].BorderChainId];
		_BorderChainLinks[i].SurfaceId = (uint16)nChains[_BorderChainLinks[i].ChainId].getLeft();

		neighbor._BorderChainLinks[best].Instance = (uint16)_InstanceId;
		neighbor._BorderChainLinks[best].BorderChainId = (uint16)i;
		neighbor._BorderChainLinks[best].ChainId = borderChains[neighbor._BorderChainLinks[best].BorderChainId];
		neighbor._BorderChainLinks[best].SurfaceId = (uint16)chains[neighbor._BorderChainLinks[best].ChainId].getLeft();
	}

	_Neighbors.push_back(neighbor._InstanceId);
	neighbor._Neighbors.push_back(_InstanceId);
}


void	NLPACS::CRetrieverInstance::unlink(vector<CRetrieverInstance> &instances)
{
	uint	i;

	for (i=0; i<_Neighbors.size(); ++i)
		instances[_Neighbors[i]].resetLinks(_InstanceId);

	resetLinks();
}




void	NLPACS::CRetrieverInstance::retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst, bool sortByDistance) const
//NLPACS::CLocalRetriever::CLocalPosition	NLPACS::CRetrieverInstance::retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst) const
{
/*
	CVector							localEstimated;
	CLocalRetriever::CLocalPosition	retrieved;

	// get local coordinates
	localEstimated = getLocalPosition(estimated);
	// Yoyo: must snap vector.
	CRetrieverInstance::snapVector(localEstimated);

	// fills _RetrieveTable by retrievingPosition.
	retriever.retrievePosition(localEstimated, cst);

	uint	i, surf;
	sint	bestSurf = -1;
	sint	lastSurf = -1;
	float	bestDistance = 1.0e10f;
	float	bestHeight;
	bool	lfound;

	// for each surface in the retriever
	for (i=0; i<cst.PossibleSurfaces.size(); ++i)
	{
		surf = cst.PossibleSurfaces[i];
		cst.SurfaceLUT[surf].first = false;
		// if the surface contains the estimated position.
		if (cst.SurfaceLUT[surf].second != 0)
		{
			// at least remembers the last seen surface...
			cst.SurfaceLUT[surf].second = 0;
			float			meanHeight;
			const CQuadLeaf	*leaf;
			ULocalPosition	lp;
			lfound = false;

			switch (_Type)
			{
			case CLocalRetriever::Landscape:
				// for landscape
				// search in the surface's quad tree for the actual height
				leaf = retriever.getSurfaces()[surf].getQuadTree().getLeaf(localEstimated);
				// if there is no acceptable leaf, just give up
				if (leaf == NULL)
					continue;
				meanHeight = leaf->getMaxHeight();
				lfound = true;
				break;
			case CLocalRetriever::Interior:
				// for interior
				// get the exact position
				lp.Surface = surf;
				lp.Estimation = localEstimated;
				meanHeight = localEstimated.z;
				retriever.snapToInteriorGround(lp, lfound);
				if (lfound)
					meanHeight = lp.Estimation.z;
				break;
			default:
				// hu?
				continue;
			}

			// if it is closer to the estimation than the previous remembered...
			float	distance = (float)fabs(localEstimated.z-meanHeight);
			if (distance < bestDistance && lfound)
			{
				bestDistance = distance;
				bestHeight = meanHeight;
				bestSurf = surf;
			}
		}
	}

	if (bestSurf != -1)
	{
		// if there is a best surface, returns it
		retrieved.Surface = bestSurf;
		retrieved.Estimation = CVector(localEstimated.x, localEstimated.y, bestHeight);
	}
	else
	{
		// else return the last remembered...
		retrieved.Surface = lastSurf;
		retrieved.Estimation = localEstimated;
	}

	return retrieved;
*/
	retrievePosition(CVectorD(estimated), retriever, cst, sortByDistance);
}

void	NLPACS::CRetrieverInstance::retrievePosition(const NLMISC::CVectorD &estimated, const CLocalRetriever &retriever, CCollisionSurfaceTemp &cst, bool sortByDistance) const
{
	CVector							localEstimated;

//	nldebug("PACS: retrievePosition in instance %d (retriever %d)", _InstanceId, _RetrieverId);

	// get local coordinates
	localEstimated = getLocalPosition(estimated);
	// Yoyo: must snap vector.
	CRetrieverInstance::snapVector(localEstimated);

	// fills _RetrieveTable by retrievingPosition.
	retriever.retrievePosition(localEstimated, cst);

	uint	i, surf;
/*	sint	bestSurf = -1;
	sint	lastSurf = -1;
	float	bestDistance = 1.0e10f;
	float	bestHeight;*/
	bool	found = false;

	switch (_Type)
	{
	case CLocalRetriever::Landscape:
		// for landscape
		for (i=0; i<cst.PossibleSurfaces.size(); ++i)
		{
			surf = cst.PossibleSurfaces[i];

//			nldebug("PACS: surface %d: count %d", surf, cst.SurfaceLUT[surf].Counter);

			// if the surface contains the estimated position.
			if (cst.SurfaceLUT[surf].Counter == 2)
			{
				float			meanHeight;
/*
				const CQuadLeaf	*leaf;

				// search in the surface's quad tree for the actual height
				leaf = retriever.getSurfaces()[surf].getQuadTree().getLeaf(localEstimated);
				// if there is no acceptable leaf, just give up
				if (leaf != NULL)
				{
					meanHeight = leaf->getMaxHeight();
					//meanHeight = retriever.getSurfaces()[surf].getQuadTree().getInterpZ(localEstimated);

					// if it is closer to the estimation than the previous remembered...
					found = true;
					float	distance = (float)fabs(localEstimated.z-meanHeight);
					cst.SortedSurfaces.push_back(CCollisionSurfaceTemp::CDistanceSurface(distance, (uint16)surf, (uint16)_InstanceId, cst.SurfaceLUT[surf].FoundCloseEdge));
				}
*/

				meanHeight = retriever.getSurface(surf).getQuantHeight()*2.0f + 1.0f;

				// if it is closer to the estimation than the previous remembered...
				found = true;
				float	distance = sortByDistance ? (float)fabs(localEstimated.z-meanHeight) : meanHeight;
				cst.SortedSurfaces.push_back(CCollisionSurfaceTemp::CDistanceSurface(distance, (uint16)surf, (uint16)_InstanceId, cst.SurfaceLUT[surf].FoundCloseEdge));

			}
			else if (cst.SurfaceLUT[surf].Counter != 0)
			{
				nlwarning("PACS: unexpected surface (%d) count (%d) at instance %d (pos=(%f,%f,%f))", surf, cst.SurfaceLUT[surf].Counter, _InstanceId, estimated.x, estimated.y, estimated.z);
			}

			cst.SurfaceLUT[surf].reset();
		}

		break;

	case CLocalRetriever::Interior:
		// for interior
		for (i=0; i<cst.PossibleSurfaces.size(); ++i)
		{
			surf = cst.PossibleSurfaces[i];
			// if the surface contains the estimated position.
			if (cst.SurfaceLUT[surf].Counter == 2)
			{
				ULocalPosition	lp;
				bool			lfound = false;

				// get the exact position
				lp.Surface = surf;
				lp.Estimation = localEstimated;
				retriever.snapToInteriorGround(lp, lfound);
				if (lfound)
				{
					// if it is closer to the estimation than the previous remembered...
					found = true;
					float	distance = sortByDistance ? (float)fabs(localEstimated.z-lp.Estimation.z) : lp.Estimation.z;
					cst.SortedSurfaces.push_back(CCollisionSurfaceTemp::CDistanceSurface(distance, (uint16)surf, (uint16)_InstanceId, cst.SurfaceLUT[surf].FoundCloseEdge));
				}
			}
			else if (cst.SurfaceLUT[surf].Counter != 0)
			{
				nlwarning("PACS: unexpected surface (%d) count (%d) at instance %d (pos=(%f,%f,%f))", surf, cst.SurfaceLUT[surf].Counter, _InstanceId, estimated.x, estimated.y, estimated.z);
			}

			cst.SurfaceLUT[surf].reset();
		}
		break;

	default:
		nlerror("Unknown instance type %d !!", _Type);
		break;
	}

	cst.OutCounter = 0;
}



void	NLPACS::CRetrieverInstance::snapToInteriorGround(NLPACS::ULocalPosition &position,
														 const NLPACS::CLocalRetriever &retriever) const
{
	bool	lfound;
	retriever.snapToInteriorGround(position, lfound);
}

void	NLPACS::CRetrieverInstance::snap(NLPACS::ULocalPosition &position, const NLPACS::CLocalRetriever &retriever) const
{
	if (_Type == CLocalRetriever::Landscape)
	{
		// search in the surface's quad tree for the actual height
//		position.Estimation.z = retriever.getSurfaces()[position.Surface].getQuadTree().getInterpZ(position.Estimation);

		position.Estimation.z = retriever.getHeight(position);

/*
		const CQuadLeaf	*leaf = retriever.getSurfaces()[position.Surface].getQuadTree().getLeaf(position.Estimation);
		// if there is no acceptable leaf, just give up
		if (leaf != NULL)
		{
			position.Estimation.z = leaf->getMaxHeight();
		}
		else
		{
			nlwarning("PACS: couldn't snap position (%f,%f,%f) on surface %d instance %d", position.Estimation.x, position.Estimation.y, position.Estimation.z, position.Surface, _InstanceId);
		}
*/
	}
	else if (_Type == CLocalRetriever::Interior)
	{
		bool	lfound;
		retriever.snapToInteriorGround(position, lfound);
	}
	else
	{
		nlwarning("PACS: unknown instance (%d) type %d", _InstanceId, _Type);
	}
}





CVector	NLPACS::CRetrieverInstance::getLocalPosition(const CVector &globalPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
	case 0:
		return CVector(+globalPosition.x-_Origin.x, +globalPosition.y-_Origin.y, globalPosition.z-_Origin.z);
		break;
	case 1:
		return CVector(+globalPosition.y-_Origin.y, -globalPosition.x+_Origin.x, globalPosition.z-_Origin.z);
		break;
	case 2:
		return CVector(-globalPosition.x+_Origin.x, -globalPosition.y+_Origin.y, globalPosition.z-_Origin.z);
		break;
	case 3:
		return CVector(-globalPosition.y+_Origin.y, +globalPosition.x-_Origin.x, globalPosition.z-_Origin.z);
		break;
	}
}

CVector	NLPACS::CRetrieverInstance::getLocalPosition(const CVectorD &globalPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
	case 0:
		return CVector((float)(+globalPosition.x-_Origin.x), (float)(+globalPosition.y-_Origin.y), (float)(globalPosition.z-_Origin.z));
		break;
	case 1:
		return CVector((float)(+globalPosition.y-_Origin.y), (float)(-globalPosition.x+_Origin.x), (float)(globalPosition.z-_Origin.z));
		break;
	case 2:
		return CVector((float)(-globalPosition.x+_Origin.x), (float)(-globalPosition.y+_Origin.y), (float)(globalPosition.z-_Origin.z));
		break;
	case 3:
		return CVector((float)(-globalPosition.y+_Origin.y), (float)(+globalPosition.x-_Origin.x), (float)(globalPosition.z-_Origin.z));
		break;
	}
}

CVector	NLPACS::CRetrieverInstance::getGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
	case 0:
		return CVector(+localPosition.x+_Origin.x, +localPosition.y+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 1:
		return CVector(-localPosition.y+_Origin.x, +localPosition.x+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 2:
		return CVector(-localPosition.x+_Origin.x, -localPosition.y+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 3:
		return CVector(+localPosition.y+_Origin.x, -localPosition.x+_Origin.y, localPosition.z+_Origin.z );
		break;
	}
}

CVectorD	NLPACS::CRetrieverInstance::getDoubleGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
	case 0:
		return CVectorD(+(double)localPosition.x+(double)_Origin.x, +(double)localPosition.y+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 1:
		return CVectorD(-(double)localPosition.y+(double)_Origin.x, +(double)localPosition.x+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 2:
		return CVectorD(-(double)localPosition.x+(double)_Origin.x, -(double)localPosition.y+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 3:
		return CVectorD(+(double)localPosition.y+(double)_Origin.x, -(double)localPosition.x+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	}
}



// ***************************************************************************
void	NLPACS::CRetrieverInstance::testExteriorCollision(NLPACS::CCollisionSurfaceTemp &cst, const CAABBox &bboxMove, const CVector2f &transBase, const NLPACS::CLocalRetriever &retriever) const
{
	sint	i;

	// 0. select ordered chains in the chainquad.
	//=====================================
	sint	nEei= _ExteriorEdgeQuad.selectEdges(bboxMove, cst);
	// NB: cst.OChainLUT is assured to be full of 0xFFFF after this call (if was right before).


	// 1. regroup them in chains. build cst.CollisionChains
	//=====================================
	// NB: use cst.OChainLUT to look if a Chain has been inserted before.
	uint16	*edgeLUT= cst.OChainLUT;

	// bkup where we begin to add chains.
	uint	firstChainAdded= (uint)cst.CollisionChains.size();

	// For all exterioredge entry.
	for(i=0;i<nEei;i++)
	{
		// get the edge entry and the edge
		uint16						eei = cst.ExteriorEdgeIndexes[i];
		const CExteriorEdgeEntry	&eee = _ExteriorEdgeQuad.getEdgeEntry(eei);

		// WELL ACTUALLY DO BOTHER ABOUT DOORS !!
/*
		// don't bother about doors
		if (eee.Interior.RetrieverInstanceId != -1)
			continue;
*/

		// add/retrieve the id in cst.CollisionChains.
		//=================================
		uint				ccId;
		// if never added.
		if(edgeLUT[eei]==0xFFFF)
		{
			// add a new CCollisionChain.
			ccId= (uint)cst.CollisionChains.size();
			cst.CollisionChains.push_back(CCollisionChain());
			// Fill it with default.
			cst.CollisionChains[ccId].Tested= false;
			cst.CollisionChains[ccId].ExteriorEdge = true;
			cst.CollisionChains[ccId].FirstEdgeCollide= 0xFFFFFFFF;
			cst.CollisionChains[ccId].ChainId= eei;
			// Fill Left right info.
			cst.CollisionChains[ccId].LeftSurface = eee.Interior;
			cst.CollisionChains[ccId].RightSurface = eee.Exterior;

			// store this Id in the LUT of chains.
			edgeLUT[eei]= uint16(ccId);
		}
		else
		{
			// get the id of this collision chain.
			ccId= edgeLUT[eei];

			// ACTUALLY, THIS SHOULD NEVER HAPPEN
			// since ext edge are only 1 segment
		}

		// add edge collide to the list.
		//=================================
		CCollisionChain			&colChain= cst.CollisionChains[ccId];

		CVector2f	p0 = CVector2f(retriever._ExteriorMesh.getEdge(eee.EdgeId).Start);
		CVector2f	p1 = CVector2f(retriever._ExteriorMesh.getEdge(eee.EdgeId+1).Start);

		// alloc a new edgeCollide.
		uint32	ecnId= cst.allocEdgeCollideNode();
		CEdgeCollideNode	&ecn= cst.getEdgeCollideNode(ecnId);

		// append to the front of the list.
		ecn.Next= colChain.FirstEdgeCollide;
		colChain.FirstEdgeCollide= ecnId;

		// build this edge.
		p0+= transBase;
		p1+= transBase;
		ecn.make(p0, p1);
	}



	// 2. Reset LUT to 0xFFFF.
	//=====================================

	// for all collisions chains inserted (starting from firstChainAdded), reset LUT.
	for(i=firstChainAdded; i<(sint)cst.CollisionChains.size(); i++)
	{
		uint	ccId= cst.CollisionChains[i].ChainId;
		edgeLUT[ccId]= 0xFFFF;
		cst.CollisionChains[i].ChainId = _ExteriorEdgeQuad.getEdgeEntry(ccId).EdgeId;
	}

}




void	NLPACS::CRetrieverInstance::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	Version 1:
		- added type and _EdgeQuad
	*/
	sint	ver= f.serialVersion(1);

	f.serial(_InstanceId, _RetrieverId, _Orientation, _Origin);
	f.serialCont(_Neighbors);
	f.serialCont(_BorderChainLinks);
	f.serial(_BBox);

	// serialises the number of nodes
	uint16	totalNodes = uint16(_NodesInformation.size());
	f.serial(totalNodes);
	if (f.isReading())
	{
		// if the stream is reading, reinits the temps table...
		_NodesInformation.resize(totalNodes);
	}

	if (ver >= 1)
	{
		f.serialEnum(_Type);
		f.serial(_ExteriorEdgeQuad);

		// a fix for old versions (with wrong _Type value)
		if (_Type != CLocalRetriever::Interior)	_Type = CLocalRetriever::Landscape;
	}
}



void	NLPACS::CRetrieverInstance::resetBorderChainLinks(const vector<uint> &links)
{
	uint	i;
	for (i=0; i<links.size(); ++i)
	{
		if (links[i] >= _BorderChainLinks.size())
			_BorderChainLinks.resize(links[i]+1);

		_BorderChainLinks[links[i]].reset();
	}
}
