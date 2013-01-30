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

#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"

#include "nel/misc/async_file_manager.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/line.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/variable.h"

NLMISC::TTicks			AStarTicks;
NLMISC::TTicks			PathTicks;
NLMISC::TTicks			ChainTicks;
NLMISC::TTicks			SurfTicks;
NLMISC::TTicks			ThisAStarTicks;
NLMISC::TTicks			ThisPathTicks;
NLMISC::TTicks			ThisChainTicks;
NLMISC::TTicks			ThisSurfTicks;

uint					PacsRetrieveVerbose = 0;

using namespace std;
using namespace NLMISC;

const float		InsureSurfaceThreshold = 0.5f;	// the threshold distance between 2 surfaces below which we insure the retrieved position to be inside the surface

H_AUTO_DECL ( NLPACS_Refresh_LR_Around )
H_AUTO_DECL ( NLPACS_Retrieve_Position )

#define	NLPACS_HAUTO_REFRESH_LR_AROUND	H_AUTO_USE ( NLPACS_Refresh_LR_Around )
#define	NLPACS_HAUTO_RETRIEVE_POSITION	H_AUTO_USE ( NLPACS_Retrieve_Position )

// CGlobalRetriever methods implementation

NLPACS::CGlobalRetriever::~CGlobalRetriever()
{
	// must be sure all current async loading is ended
	waitEndOfAsyncLoading();
}

//
void	NLPACS::CGlobalRetriever::init()
{
	_BBox.setCenter(CVector::Null);
	_BBox.setHalfSize(CVector::Null);

	_InstanceGrid.create(128, 160.0f);
}

void	NLPACS::CGlobalRetriever::initQuadGrid()
{
	_InstanceGrid.clear();
	_InstanceGrid.create(128, 160.0f);

	uint	i;
	for (i=0; i<_Instances.size(); ++i)
		_InstanceGrid.insert(_Instances[i].getBBox().getMin(), _Instances[i].getBBox().getMax(), i);
}

void	NLPACS::CGlobalRetriever::initRetrieveTable()
{
	uint	i;
	uint	size = 0;

	for (i=0; i<_Instances.size(); ++i)
	{
		if (_Instances[i].getInstanceId() != -1 && _Instances[i].getRetrieverId() != -1)
		{
			const CLocalRetriever	&retriever = getRetriever(_Instances[i].getRetrieverId());
			size =  std::max((uint)retriever.getSurfaces().size(), size);
		}
	}

	_RetrieveTable.resize(size);
	for (i=0; i<size; ++i)
		_RetrieveTable[i] = 0;
}

//

bool	NLPACS::CGlobalRetriever::selectInstances(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst, UGlobalPosition::TType type) const
{
	_InstanceGrid.select(bbox.getMin(), bbox.getMax());
	cst.CollisionInstances.clear();

	bool	allLoaded = true;

	NLPACS::CQuadGrid<uint32>::CIterator	it;
	for (it=_InstanceGrid.begin(); it!=_InstanceGrid.end(); ++it)
	{
		if ((type == UGlobalPosition::Landscape && _Instances[*it].getType() == CLocalRetriever::Interior) ||
			(type == UGlobalPosition::Interior && _Instances[*it].getType() == CLocalRetriever::Landscape))
			continue;

		if (_Instances[*it].getBBox().intersect(bbox))
		{
			if (!_RetrieverBank->isLoaded(_Instances[*it].getRetrieverId()))
				allLoaded = false;
			cst.CollisionInstances.push_back(*it);
		}
	}

	return allLoaded;
}

//

void	NLPACS::CGlobalRetriever::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	f.serialCont(_Instances);
	f.serial(_BBox);

	if (f.isReading())
		initAll(false);
}

//

void	NLPACS::CGlobalRetriever::check() const
{
	uint	i, j, k;

	for (i=0; i<_Instances.size(); ++i)
	{
		if (_Instances[i].getInstanceId() == -1)
		{
			nlwarning("Uninitialized instance %d", i);
			continue;
		}

		if (_Instances[i].getInstanceId() != (sint)i)
			nlwarning("InstanceId for instance %d is not correctly initialized", i);

		if (_Instances[i].getRetrieverId() == -1)
		{
			nlwarning("No retriever at instance %d", i);
			continue;
		}

		const CRetrieverInstance	&instance = _Instances[i];

		if (instance.getRetrieverId()<0 || instance.getRetrieverId()>=(sint)_RetrieverBank->getRetrievers().size())
		{
			nlwarning("Instance %d has wrong retriever reference", i);
			continue;
		}

		const CLocalRetriever		&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());

		for (j=0; j<retriever.getChains().size(); ++j)
		{
			const CChain	&chain = retriever.getChain(j);
			for (k=0; k<chain.getSubChains().size(); ++k)
			{
				if (chain.getSubChain(k) >= retriever.getOrderedChains().size())
				{
					nlwarning("retriever %d, chain %d: subchain %d reference is not valid", instance.getRetrieverId(), j, k);
					continue;
				}

				if (retriever.getOrderedChain(chain.getSubChain(k)).getParentId() != j)
				{
					nlwarning("retriever %d, ochain %d: reference on parent is not valid", instance.getRetrieverId(), chain.getSubChain(k));
					continue;
				}

				if (retriever.getOrderedChain(chain.getSubChain(k)).getIndexInParent() != k)
				{
					nlwarning("retriever %d, ochain %d: index on parent is not valid", instance.getRetrieverId(), chain.getSubChain(k));
					continue;
				}
			}

			if (chain.getLeft()<0 || chain.getLeft()>=(sint)retriever.getSurfaces().size())
			{
				nlwarning("retriever %d, chain %d: reference on left surface is not valid", instance.getRetrieverId(), j);
			}

			if (chain.getRight()>=(sint)retriever.getSurfaces().size() ||
				(chain.getRight()<=CChain::getDummyBorderChainId() && !CChain::isBorderChainId(chain.getRight())))
			{
				nlwarning("retriever %d, chain %d: reference on right surface is not valid", instance.getRetrieverId(), j);
			}

			if (CChain::isBorderChainId(chain.getRight()))
			{
				sint	link = chain.getBorderChainIndex();

				if (link<0 || link>=(sint)instance.getBorderChainLinks().size())
				{
					nlwarning("retriever %d, instance %d, chain %d: reference on right link is not valid", instance.getRetrieverId(), instance.getInstanceId(), j);
				}
				else
				{
					CRetrieverInstance::CLink	lnk = instance.getBorderChainLink(link);

					if (lnk.Instance != 0xFFFF || lnk.SurfaceId != 0xFFFF ||
						lnk.ChainId != 0xFFFF || lnk.BorderChainId != 0xFFFF)
					{
						if (lnk.Instance >= _Instances.size() ||
							_Instances[lnk.Instance].getRetrieverId()<0 ||
							_Instances[lnk.Instance].getRetrieverId()>(sint)_RetrieverBank->getRetrievers().size() ||
							lnk.SurfaceId >= getRetriever(_Instances[lnk.Instance].getRetrieverId()).getSurfaces().size() ||
							((lnk.ChainId >= getRetriever(_Instances[lnk.Instance].getRetrieverId()).getChains().size() ||
							lnk.BorderChainId >= getRetriever(_Instances[lnk.Instance].getRetrieverId()).getBorderChains().size()) && instance.getType() != CLocalRetriever::Interior ))
						{
							nlwarning("retriever %d, instance %d, link %d: reference on instance may be not valid [Inst=%d, Surf=%d, Chain=%d, BorderChain=%d]", instance.getRetrieverId(), instance.getInstanceId(), link, lnk.Instance, lnk.SurfaceId, lnk.ChainId, lnk.BorderChainId);
						}
					}
				}
			}
		}
	}
}

//

float	NLPACS::CGlobalRetriever::distanceToBorder(const UGlobalPosition &pos) const
{
	if (pos.InstanceId < 0 || pos.InstanceId > (sint)_Instances.size())
		return 0.0f;

	return getRetriever(_Instances[pos.InstanceId].getRetrieverId()).distanceToBorder(pos.LocalPosition);
}

void	NLPACS::CGlobalRetriever::getBorders(const UGlobalPosition &pos, std::vector<std::pair<NLMISC::CLine, uint8> > &edges)
{
	edges.clear();

	if (pos.InstanceId < 0)
		return;

	CVectorD		gpos = getDoubleGlobalPosition(pos);
	CAABBox			sbox;
	sbox.setCenter(gpos);
	sbox.setHalfSize(CVector(50.0f, 50.0f, 100.0f));

	getBorders(sbox, edges);
}

void	NLPACS::CGlobalRetriever::getBorders(const CAABBox &sbox, std::vector<std::pair<NLMISC::CLine, uint8> > &edges)
{
	edges.clear();

	selectInstances(sbox, _InternalCST);

	uint	inst;
	for (inst=0; inst<_InternalCST.CollisionInstances.size(); ++inst)
	{
		CRetrieverInstance	&instance = _Instances[_InternalCST.CollisionInstances[inst]];
		CLocalRetriever		&retriever = const_cast<CLocalRetriever &>(getRetriever(instance.getRetrieverId()));
		if (!retriever.isLoaded())
			continue;

		CChainQuad			&chainquad = retriever.getChainQuad();

		CAABBox				box;
		CVector				origin = instance.getOrigin();
		box.setCenter(sbox.getCenter()-origin);
		box.setHalfSize(sbox.getHalfSize());
		chainquad.selectEdges(box, _InternalCST);

		uint		ece;

		CVector		dz(0.0f, 0.0f, 0.5f);
		float		zp = (float)sbox.getCenter().z;
		for (ece=0; ece<_InternalCST.EdgeChainEntries.size(); ++ece)
		{
			CEdgeChainEntry		&entry = _InternalCST.EdgeChainEntries[ece];

			//
			const CChain	&fchain = retriever.getChain(retriever.getOrderedChain(entry.OChainId).getParentId());
			uint8			chainType = (fchain.getRight() >= 0 ? 1 : (fchain.isBorderChain() ? 2 : 0));

			//
			if (chainType == 1)
			{
				uint	left = fchain.getLeft();
				uint	right = fchain.getRight();

				const CRetrievableSurface	&lsurface = retriever.getSurface(left);
				const CRetrievableSurface	&rsurface = retriever.getSurface(right);

				bool	luw = (lsurface.getFlags() & (1 << CRetrievableSurface::IsUnderWaterBit)) != 0;
				bool	ruw = (rsurface.getFlags() & (1 << CRetrievableSurface::IsUnderWaterBit)) != 0;

				if (luw != ruw)
					chainType = 3;
			}

			if (retriever.getFullOrderedChains().size() > 0)
			{
				const COrderedChain3f	&ochain = retriever.getFullOrderedChain(entry.OChainId);

				uint	edge;
				for (edge=entry.EdgeStart; edge<entry.EdgeEnd; ++edge)
				{
					edges.push_back(make_pair(CLine(), chainType));
					edges.back().first.V0 = ochain[edge] + origin;
					edges.back().first.V1 = ochain[edge+1] + origin;
/*
					edges.push_back(make_pair(CLine(), chainType));
					edges.back().first.V0 = ochain[edge] + origin;
					edges.back().first.V1 = ochain[edge] + origin +dz;

					edges.push_back(make_pair(CLine(), chainType));
					edges.back().first.V0 = ochain[edge+1] + origin;
					edges.back().first.V1 = ochain[edge+1] + origin +dz;
*/
				}
			}
			else
			{
				const COrderedChain	&ochain = retriever.getOrderedChain(entry.OChainId);

				uint	edge;
				for (edge=entry.EdgeStart; edge<entry.EdgeEnd; ++edge)
				{
					edges.push_back(make_pair(CLine(), chainType));
					edges.back().first.V0 = ochain[edge].unpack3f() + origin;
					edges.back().first.V0.z = zp;
					edges.back().first.V1 = ochain[edge+1].unpack3f() + origin;
					edges.back().first.V1.z = zp;
				}
			}
		}
		// Bind edges for exterior mesh
		const CExteriorMesh &em = retriever.getExteriorMesh();
		const CExteriorMesh::CEdge *previousEdge = NULL;
		for(uint k = 0; k < em.getEdges().size(); ++k)
		{
			if (previousEdge)
			{
				edges.push_back(make_pair(CLine(), previousEdge->Link < 0 ? 4 : 5));
				edges.back().first.V0 = previousEdge->Start + origin;
				edges.back().first.V1 = em.getEdges()[k].Start + origin;
			}
			previousEdge = em.getEdges()[k].Link != -2 ? &em.getEdges()[k] : NULL;
		}
	}
}


//

void	NLPACS::CGlobalRetriever::makeLinks(uint n)
{
	CRetrieverInstance	&instance = _Instances[n];

	selectInstances(instance.getBBox(), _InternalCST);

	uint	i;
	for (i=0; i<_InternalCST.CollisionInstances.size(); ++i)
	{
		CRetrieverInstance	&neighbor = _Instances[_InternalCST.CollisionInstances[i]];

		if (neighbor.getInstanceId() == instance.getInstanceId())
			continue;

		try
		{
			instance.link(neighbor, _RetrieverBank->getRetrievers());
			neighbor.link(instance, _RetrieverBank->getRetrievers());
		}
		catch (const Exception &e)
		{
			nlwarning("in NLPACS::CGlobalRetriever::makeLinks()");
			nlwarning("caught an exception during linkage of %d and %d: %s", instance.getInstanceId(), neighbor.getInstanceId(), e.what());
		}
	}

	if (getRetriever(instance.getRetrieverId()).getType() == CLocalRetriever::Interior)
		instance.linkEdgeQuad(*this);
}

void	NLPACS::CGlobalRetriever::resetAllLinks()
{
	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		_Instances[n].unlink(_Instances);
}


void	NLPACS::CGlobalRetriever::makeAllLinks()
{
	resetAllLinks();

	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		makeLinks(n);
}

void	NLPACS::CGlobalRetriever::initAll(bool initInstances)
{
	if (initInstances)
	{
		uint	n;
		for (n=0; n<_Instances.size(); ++n)
			if (_Instances[n].getInstanceId() != -1 && _Instances[n].getRetrieverId() != -1)
				_Instances[n].init(_RetrieverBank->getRetriever(_Instances[n].getRetrieverId()));
	}

	initQuadGrid();
	initRetrieveTable();
}

//

const NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::makeInstance(uint32 retrieverId, uint8 orientation, const CVector &origin)
{
	uint	id;
	for (id=0; id<_Instances.size() && _Instances[id].getInstanceId()!=-1; ++id)
		;

	if (id == _Instances.size())
		_Instances.resize(id+1);

	CRetrieverInstance		&instance = _Instances[id];
	const CLocalRetriever	&retriever = getRetriever(retrieverId);

	if (_RetrieveTable.size() < retriever.getSurfaces().size())
		_RetrieveTable.resize(retriever.getSurfaces().size(), 0);

	instance.make(id, retrieverId, retriever, orientation, origin);

	CVector	hsize = instance.getBBox().getHalfSize();
	hsize.z = 0.0f;
	if (hsize != CVector::Null)
	{
		if (_BBox.getHalfSize() == CVector::Null)
		{
			_BBox = instance.getBBox();
		}
		else
		{
			_BBox.extend(instance.getBBox().getMin());
			_BBox.extend(instance.getBBox().getMax());
		}

		if (getRetriever(instance.getRetrieverId()).getType() == CLocalRetriever::Interior)
			instance.initEdgeQuad(*this);

		_InstanceGrid.insert(instance.getBBox().getMin(), instance.getBBox().getMax(), instance.getInstanceId());
	}

	return instance;
}

//

NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVector &estimated, float threshold) const
{
	return retrievePosition(CVectorD(estimated), (double)threshold, UGlobalPosition::Unspecified);
}

NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVectorD &estimated, double threshold) const
{
	return retrievePosition(estimated, threshold, UGlobalPosition::Unspecified);
}

NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVector &estimated) const
{
	return retrievePosition(estimated, 1.0e10f, UGlobalPosition::Unspecified);
}

NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVectorD &estimated) const
{
	return retrievePosition(estimated, 1.0e10, UGlobalPosition::Unspecified);
}

// Retrieves the position of an estimated point in the global retriever (double instead.)
NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVectorD &estimated, double /* threshold */, NLPACS::UGlobalPosition::TType retrieveSpec) const
{
	NLPACS_HAUTO_RETRIEVE_POSITION

	// the retrieved position
	CGlobalPosition				result = CGlobalPosition(-1, CLocalRetriever::CLocalPosition(-1, estimated));

	if (!_BBox.include(CVector((float)estimated.x, (float)estimated.y, (float)estimated.z)))
	{
		_ForbiddenInstances.clear();
		return result;
	}


	// get the best matching instances
	CAABBox	bbpos;
	bbpos.setCenter(estimated);
	bbpos.setHalfSize(CVector(0.5f, 0.5f, 0.5f));
	if (!selectInstances(bbpos, _InternalCST, retrieveSpec))
	{
		return result;
	}

	uint	i;

	_InternalCST.SortedSurfaces.clear();

	// for each instance, try to retrieve the position
	for (i=0; i<_InternalCST.CollisionInstances.size(); ++i)
	{
		uint32							id = _InternalCST.CollisionInstances[i];
		const CRetrieverInstance		&instance = _Instances[id];
		const CLocalRetriever			&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());

		uint	j;
		for (j=0; j<_ForbiddenInstances.size(); ++j)
			if (_ForbiddenInstances[j] == (sint32)id)
				break;

		if (j<_ForbiddenInstances.size() || !retriever.isLoaded())
			continue;

		instance.retrievePosition(estimated, retriever, _InternalCST);
	}

	_ForbiddenInstances.clear();

	if (!_InternalCST.SortedSurfaces.empty())
	{
		// if there are some selected surfaces, sort them
		std::sort(_InternalCST.SortedSurfaces.begin(), _InternalCST.SortedSurfaces.end(), CCollisionSurfaceTemp::CDistanceSurface());

		uint	selInstance;
		float	bestDist = 1.0e10f;
		for (selInstance=0; selInstance<_InternalCST.SortedSurfaces.size(); ++selInstance)
		{
			uint32						id = _InternalCST.SortedSurfaces[selInstance].Instance;
			const CRetrieverInstance	&instance = _Instances[id];

			if (instance.getType() == CLocalRetriever::Interior && _InternalCST.SortedSurfaces[selInstance].Distance < bestDist+6.0f)
				break;

			if (selInstance == 0)
				bestDist = _InternalCST.SortedSurfaces[0].Distance;
		}

		if (selInstance >= _InternalCST.SortedSurfaces.size())
			selInstance = 0;

		uint32							id = _InternalCST.SortedSurfaces[selInstance].Instance;
		const CRetrieverInstance		&instance = _Instances[id];
		const CLocalRetriever			&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());

		// get the UGlobalPosition of the estimation for this surface
		result.InstanceId = id;
		result.LocalPosition.Surface = _InternalCST.SortedSurfaces[selInstance].Surface;
		result.LocalPosition.Estimation = instance.getLocalPosition(estimated);

		CRetrieverInstance::snapVector(result.LocalPosition.Estimation);

		// if there are more than 1 one possible (and best matching) surface, insure the position within the surface (by moving the point)
//		if (_InternalCST.SortedSurfaces.size() >= 2 &&
//			_InternalCST.SortedSurfaces[1].Distance-_InternalCST.SortedSurfaces[0].Distance < InsureSurfaceThreshold)
		if (_InternalCST.SortedSurfaces[selInstance].FoundCloseEdge)
		{
			bool	moved;
			uint	numMove = 0;
			do
			{
				moved = retriever.insurePosition(result.LocalPosition);
				++numMove;
			}
			while (moved && numMove < 100);
			// the algo won't loop infinitely

			if (numMove > 1)
			{
				nldebug("PACS: insured position inside surface (%d,%d)-(%f,%f,%f), %d moves needed", result.InstanceId, result.LocalPosition.Surface, estimated.x, estimated.y, estimated.z, numMove-1);
			}

			if (moved)
			{
				nlwarning ("PACS: couldn't insure position (%.f,%.f) within the surface (surf=%d,inst=%d) after 100 retries", result.LocalPosition.Estimation.x, result.LocalPosition.Estimation.y, result.LocalPosition.Surface, result.InstanceId);
			}
		}

		// and after selecting the best surface (and some replacement) snap the point to the surface
		instance.snap(result.LocalPosition, retriever);


		if (PacsRetrieveVerbose)
			nlinfo("retrievePosition(%f,%f,%f) -> %d/%d/(%f,%f,%f) - %s/%s",
					estimated.x, estimated.y, estimated.z,
					result.InstanceId, result.LocalPosition.Surface,
					result.LocalPosition.Estimation.x, result.LocalPosition.Estimation.y, result.LocalPosition.Estimation.z,
					retriever.getIdentifier().c_str(),
					retriever.getType() == CLocalRetriever::Interior ? "Interior" : "Landscape");
	}
	else
	{
		if (PacsRetrieveVerbose)
			nlwarning("PACS: unable to retrieve correct position (%f,%f,%f)", estimated.x, estimated.y, estimated.z);
//		nlSleep(1);
	}

	return result;
}

//

// Retrieves the position of an estimated point in the global retriever using layer hint
NLPACS::UGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVectorD &estimated, uint h, sint &res) const
{
	// the retrieved position
	CGlobalPosition				result = CGlobalPosition(-1, CLocalRetriever::CLocalPosition(-1, estimated));

	if (!_BBox.include(CVector((float)estimated.x, (float)estimated.y, (float)estimated.z)))
	{
		_ForbiddenInstances.clear();
		res = Failed;
		return result;
	}


	// get the best matching instances
	CAABBox	bbpos;
	bbpos.setCenter(estimated);
	bbpos.setHalfSize(CVector(0.5f, 0.5f, 0.5f));
	bool	canGet = selectInstances(bbpos, _InternalCST);

	if (!canGet)
	{
		res = MissingLr;
		return result;
	}

	uint	i;

	_InternalCST.SortedSurfaces.clear();

	// for each instance, try to retrieve the position
	for (i=0; i<_InternalCST.CollisionInstances.size(); ++i)
	{
		uint32							id = _InternalCST.CollisionInstances[i];
		const CRetrieverInstance		&instance = _Instances[id];
		const CLocalRetriever			&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());

		uint	j;
		for (j=0; j<_ForbiddenInstances.size(); ++j)
			if (_ForbiddenInstances[j] == (sint32)id)
				break;

		if (j<_ForbiddenInstances.size() || !retriever.isLoaded())
			continue;

		instance.retrievePosition(estimated, retriever, _InternalCST, false);
	}

	_ForbiddenInstances.clear();

	if (!_InternalCST.SortedSurfaces.empty())
	{
		// if there are some selected surfaces, sort them
		std::sort(_InternalCST.SortedSurfaces.begin(), _InternalCST.SortedSurfaces.end(), CCollisionSurfaceTemp::CDistanceSurface());

		if (h >= _InternalCST.SortedSurfaces.size())
		{
			// found less surfaces than expected, abort
			res = Failed;
			return result;
		}

		uint32							id = _InternalCST.SortedSurfaces[h].Instance;
		const CRetrieverInstance		&instance = _Instances[id];
		const CLocalRetriever			&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());

		// get the UGlobalPosition of the estimation for this surface
		result.InstanceId = id;
		result.LocalPosition.Surface = _InternalCST.SortedSurfaces[h].Surface;
		result.LocalPosition.Estimation = instance.getLocalPosition(estimated);

		CRetrieverInstance::snapVector(result.LocalPosition.Estimation);

		// if there are more than 1 one possible (and best matching) surface, insure the position within the surface (by moving the point)
//		if (_InternalCST.SortedSurfaces.size() >= 2 &&
//			_InternalCST.SortedSurfaces[1].Distance-_InternalCST.SortedSurfaces[0].Distance < InsureSurfaceThreshold)
		if (_InternalCST.SortedSurfaces[h].FoundCloseEdge)
		{
			bool	moved;
			uint	numMove = 0;
			do
			{
				moved = retriever.insurePosition(result.LocalPosition);
				++numMove;
			}
			while (moved && numMove < 100);
			// the algo won't loop infinitely

			if (numMove > 1)
			{
				nldebug("PACS: insured position inside surface (%d,%d)-(%f,%f,%f), %d moves needed", result.InstanceId, result.LocalPosition.Surface, estimated.x, estimated.y, estimated.z, numMove-1);
			}

			if (moved)
			{
				nlwarning ("PACS: couldn't insure position (%.f,%.f) within the surface (surf=%d,inst=%d) after 100 retries", result.LocalPosition.Estimation.x, result.LocalPosition.Estimation.y, result.LocalPosition.Surface, result.InstanceId);
			}
		}

		// and after selecting the best surface (and some replacement) snap the point to the surface
		instance.snap(result.LocalPosition, retriever);
	}
	else
	{
		res = Failed;
//		nlwarning("PACS: unable to retrieve correct position (%f,%f,%f)", estimated.x, estimated.y, estimated.z);
//		nlSleep(1);
	}

	res = Success;
	return result;
}


//

sint32			NLPACS::CGlobalRetriever::getIdentifier(const string &id) const
{
	sint32	i;
	for (i=0; i<(sint32)(_RetrieverBank->getRetrievers().size()); ++i)
		if (getRetriever(i).getIdentifier() == id)
			return i;

	return -1;
}

const string	&NLPACS::CGlobalRetriever::getIdentifier(const NLPACS::UGlobalPosition &position) const
{
	static const string		nullString = string("");

	if (position.InstanceId == -1)
		return nullString;

	return getRetriever(_Instances[position.InstanceId].getRetrieverId()).getIdentifier();
}

sint32	NLPACS::CGlobalRetriever::getLocalRetrieverId(const NLPACS::UGlobalPosition &position) const
{
	if (position.InstanceId == -1)
		return -1;

	return _Instances[position.InstanceId].getRetrieverId();
}

//

bool			NLPACS::CGlobalRetriever::buildInstance(const string &id, const NLMISC::CVectorD &position, sint32 &instanceId)
{

	sint32	retrieverId = getIdentifier(id);

	instanceId = -1;

	// check retriever exists
	if (retrieverId < 0)
		return false;

	const CRetrieverInstance	&instance = makeInstance(retrieverId, 0, CVector(position));

	// check make instance success
	if (&instance == NULL || instance.getInstanceId() == -1 || instance.getRetrieverId() != retrieverId)
		return false;

	// links new instance to its neighbors
	makeLinks(instance.getInstanceId());

	instanceId = instance.getInstanceId();

	return true;
}

//

void		NLPACS::CGlobalRetriever::removeInstance(sint32 instanceId)
{
	if (instanceId < 0 || instanceId >= (sint32)_Instances.size() || _Instances[instanceId].getInstanceId() < 0)
	{
		nlwarning("CGlobalRetriever::removeInstance(): Can't unlink instance %d, doesn't exist", instanceId);
		return;
	}

	// get instance
	CRetrieverInstance	&instance = _Instances[instanceId];

	// unlink it from others
	instance.unlink(_Instances);


}

//

//
/*
void		NLPACS::CGlobalRetriever::snapToInteriorGround(UGlobalPosition &position) const
{
	const CRetrieverInstance	&instance = _Instances[position.InstanceId];
	if (instance.getType() != CLocalRetriever::Interior)
		return;

	const CLocalRetriever		&retriever = getRetriever(instance.getRetrieverId());
	instance.snapToInteriorGround(position.LocalPosition, retriever);
}
*/

//
CVector		NLPACS::CGlobalRetriever::getGlobalPosition(const UGlobalPosition &global) const
{
	if (global.InstanceId >= 0)
	{
		return _Instances[global.InstanceId].getGlobalPosition(global.LocalPosition.Estimation);
	}
	else
	{
		// it should be an error here
		return global.LocalPosition.Estimation;
	}
}

CVectorD	NLPACS::CGlobalRetriever::getDoubleGlobalPosition(const NLPACS::UGlobalPosition &global) const
{
	if (global.InstanceId >= 0)
	{
		return _Instances[global.InstanceId].getDoubleGlobalPosition(global.LocalPosition.Estimation);
	}
	else
	{
		// it should be an error here
		return CVectorD(global.LocalPosition.Estimation);
	}
}

//

void		NLPACS::CGlobalRetriever::findAStarPath(const NLPACS::UGlobalPosition &begin,
													const NLPACS::UGlobalPosition &end,
													vector<NLPACS::CRetrieverInstance::CAStarNodeAccess> &path,
													uint32 forbidFlags) const
{
	TTicks  astarStart;
	ThisAStarTicks = 0;
	astarStart = CTime::getPerformanceTime();

	// open and close lists
	// TODO: Use a smart allocator to avoid huge alloc/free and memory fragmentation
	// open is a priority queue (implemented as a stl multimap)
	multimap<float, CRetrieverInstance::CAStarNodeAccess>	open;
	// close is a simple stl vector
	vector<CRetrieverInstance::CAStarNodeAccess>			close;

	// inits start node and info
	CRetrieverInstance::CAStarNodeAccess					beginNode;
	beginNode.InstanceId = begin.InstanceId;
	beginNode.NodeId = (uint16)begin.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&beginInfo = getNode(beginNode);

	// inits end node and info.
	CRetrieverInstance::CAStarNodeAccess					endNode;
	endNode.InstanceId = end.InstanceId;
	endNode.NodeId = (uint16)end.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&endInfo = getNode(endNode);

	// set up first node...
	CRetrieverInstance::CAStarNodeAccess					node = beginNode;
	beginInfo.Parent.InstanceId = -1;
	beginInfo.Parent.NodeId = 0;
	beginInfo.Parent.ThroughChain = 0;
	beginInfo.Cost = 0;
	beginInfo.F = (endInfo.Position-beginInfo.Position).norm();

	// ... and inserts it in the open list.
	open.insert(make_pair(beginInfo.F, node));

	// TO DO: use a CVector2f instead
	CVector2f												endPosition = CVector2f(getGlobalPosition(end));

	uint	i;

	path.clear();

	for(;;)
	{
		if (open.empty())
		{
			// couldn't find a path
			return;
		}

		multimap<float, CRetrieverInstance::CAStarNodeAccess>::iterator	it;

		it = open.begin();
		node = it->second;
		open.erase(it);

		if (node == endNode)
		{
			// found a path
			CRetrieverInstance::CAStarNodeAccess			pathNode = node;
			uint											numNodes = 0;
			while (pathNode.InstanceId != -1)
			{
				++numNodes;
				CRetrieverInstance							&instance = _Instances[pathNode.InstanceId];
				CRetrieverInstance::CAStarNodeInfo			&pathInfo = instance._NodesInformation[pathNode.NodeId];
				pathNode = pathInfo.Parent;
			}

			path.resize(numNodes);
			pathNode = node;
			while (pathNode.InstanceId != -1)
			{
				path[--numNodes] = pathNode;
				CRetrieverInstance							&instance = _Instances[pathNode.InstanceId];
				CRetrieverInstance::CAStarNodeInfo			&pathInfo = instance._NodesInformation[pathNode.NodeId];
				pathNode = pathInfo.Parent;
			}

			ThisAStarTicks += (CTime::getPerformanceTime()-astarStart);

			nlinfo("found a path");
			for (i=0; i<path.size(); ++i)
			{
				CRetrieverInstance							&instance = _Instances[path[i].InstanceId];
				const CLocalRetriever						&retriever = _RetrieverBank->getRetriever(instance.getRetrieverId());
				nlinfo("pathNode %d = (Inst=%d, Node=%d, Through=%d)", i, path[i].InstanceId, path[i].NodeId, path[i].ThroughChain);
				if (path[i].ThroughChain != 0xffff)
				{
					const CChain								&chain = retriever.getChain(path[i].ThroughChain);
					nlinfo("   chain: left=%d right=%d", chain.getLeft(), chain.getRight());
					if (CChain::isBorderChainId(chain.getRight()))
					{
						CRetrieverInstance::CLink	lnk = instance.getBorderChainLink(CChain::convertBorderChainId(chain.getRight()));
						sint	instanceid = lnk.Instance;
						sint	id = lnk.SurfaceId;
						nlinfo("      right: instance=%d surf=%d", instanceid, id);
					}
				}
			}
			nlinfo("open.size()=%d", open.size());
			nlinfo("close.size()=%d", close.size());

			return;
		}

		// push successors of the current node
		CRetrieverInstance								&inst = _Instances[node.InstanceId];
		const CLocalRetriever							&retriever = _RetrieverBank->getRetriever(inst.getRetrieverId());
		const CRetrievableSurface						&surf = retriever.getSurface(node.NodeId);
		const vector<CRetrievableSurface::CSurfaceLink>	&chains = surf.getChains();

		CRetrieverInstance								*nextInstance;
		const CLocalRetriever							*nextRetriever;
		const CRetrievableSurface						*nextSurface;

		nlinfo("examine node (instance=%d,surf=%d,cost=%g)", node.InstanceId, node.NodeId, inst._NodesInformation[node.NodeId].Cost);

		for (i=0; i<chains.size(); ++i)
		{
			sint32	nextNodeId = chains[i].Surface;
			CRetrieverInstance::CAStarNodeAccess		nextNode;

			if (CChain::isBorderChainId(nextNodeId))
			{
				// if the chain points to another retriever

				// first get the edge on the retriever
				CRetrieverInstance::CLink	lnk = inst.getBorderChainLink(CChain::convertBorderChainId(nextNodeId));
				nextNode.InstanceId = lnk.Instance;

				if (nextNode.InstanceId < 0)
					continue;

				nextInstance = &_Instances[nextNode.InstanceId];
				nextRetriever = &(_RetrieverBank->getRetriever(nextInstance->getRetrieverId()));

				sint	nodeId = lnk.SurfaceId;
				nlassert(nodeId >= 0);
				nextNode.NodeId = (uint16)nodeId;
			}
			else if (nextNodeId >= 0)
			{
				// if the chain points to the same instance
				nextNode.InstanceId = node.InstanceId;
				nextNode.NodeId = (uint16) nextNodeId;
				nextInstance = &inst;
				nextRetriever = &retriever;
			}
			else
			{
				// if the chain cannot be crossed
				continue;
			}

			nextSurface = &(nextRetriever->getSurface(nextNode.NodeId));

			if (nextSurface->getFlags() & forbidFlags)
				continue;

			// compute new node value (heuristic and cost)

			CRetrieverInstance::CAStarNodeInfo	&nextInfo = nextInstance->_NodesInformation[nextNode.NodeId];
			float	stepCost = (nextInfo.Position-inst._NodesInformation[node.NodeId].Position).norm();
			float	nextCost = inst._NodesInformation[node.NodeId].Cost+stepCost;
			float	nextHeuristic = (nextInfo.Position-endPosition).norm();
			float	nextF = nextCost+nextHeuristic;

			vector<CRetrieverInstance::CAStarNodeAccess>::iterator			closeIt;
			for (closeIt=close.begin(); closeIt!=close.end() && *closeIt!=nextNode; ++closeIt)
				;

			if (closeIt != close.end() && nextInfo.F < nextF)
				continue;

			multimap<float, CRetrieverInstance::CAStarNodeAccess>::iterator	openIt;
			for (openIt=open.begin(); openIt!=open.end() && openIt->second!=nextNode; ++openIt)
				;

			if (openIt != open.end() && nextInfo.F < nextF)
				continue;

			if (openIt != open.end())
				open.erase(openIt);

			if (closeIt != close.end())
				close.erase(closeIt);

			nextInfo.Parent = node;
			nextInfo.Parent.ThroughChain = (uint16)(chains[i].Chain);
			nextInfo.Cost = nextCost;
			nextInfo.F = nextF;

			nlinfo("  adding node (instance=%d,surf=%d) f=%g, through=%d", nextNode.InstanceId, nextNode.NodeId, nextInfo.F, i);

			open.insert(make_pair(nextInfo.F, nextNode));
		}
		close.push_back(node);
	}
}



void	NLPACS::CGlobalRetriever::findPath(const NLPACS::UGlobalPosition &begin,
										   const NLPACS::UGlobalPosition &end,
										   NLPACS::CGlobalRetriever::CGlobalPath &path,
										   uint32 forbidFlags) const
{

	vector<CRetrieverInstance::CAStarNodeAccess>	astarPath;
	findAStarPath(begin, end, astarPath, forbidFlags);

	TTicks	surfStart;
	TTicks	chainStart;

	ThisChainTicks = 0;
	ThisSurfTicks = 0;
	ThisPathTicks = 0;

	path.clear();
	path.resize(astarPath.size());

	uint	i, j;
	for (i=0; i<astarPath.size(); ++i)
	{
		chainStart = CTime::getPerformanceTime();
		CLocalPath		&surf = path[i];
		surf.InstanceId = astarPath[i].InstanceId;
		const CLocalRetriever	&retriever = _RetrieverBank->getRetriever(_Instances[surf.InstanceId].getRetrieverId());

		// computes start point
		if (i == 0)
		{
			// if it is the first point, just copy the begin
			surf.Start.ULocalPosition::operator= (begin.LocalPosition);
		}
		else
		{
			// else, take the previous value and convert it in the current instance axis
			// TODO: avoid this if the instances are the same
			CVector	prev = _Instances[path[i-1].InstanceId].getGlobalPosition(path[i-1].End.Estimation);
			CVector	current = _Instances[surf.InstanceId].getLocalPosition(prev);
			surf.Start.Surface = astarPath[i].NodeId;
			surf.Start.Estimation = current;
		}

		// computes end point
		if (i == astarPath.size()-1)
		{
			surf.End.ULocalPosition::operator= (end.LocalPosition);
		}
		else
		{
			// get to the middle of the chain
			// first get the chain between the 2 surfaces
			const CChain			&chain = retriever.getChain(astarPath[i].ThroughChain);
			float					cumulLength = 0.0f, midLength=chain.getLength()*0.5f;
			for (j=0; j<chain.getSubChains().size() && cumulLength<=midLength; ++j)
				cumulLength += retriever.getOrderedChain(chain.getSubChain(j)).getLength();
			--j;
			const COrderedChain		&ochain = retriever.getOrderedChain(chain.getSubChain(j));
			surf.End.Surface = astarPath[i].NodeId;
			{
				if (ochain.getVertices().size() & 1)
				{
					surf.End.Estimation = ochain[(uint)ochain.getVertices().size()/2].unpack3f();
				}
				else
				{
					surf.End.Estimation = (ochain[(uint)ochain.getVertices().size()/2].unpack3f()+
										  ochain[(uint)ochain.getVertices().size()/2-1].unpack3f())*0.5f;
				}
			}
		}
		ThisChainTicks += (CTime::getPerformanceTime()-chainStart);

		surfStart = CTime::getPerformanceTime();
		retriever.findPath(surf.Start, surf.End, surf.Path, _InternalCST);
		ThisSurfTicks += (CTime::getPerformanceTime()-surfStart);
	}

	ThisPathTicks = ThisAStarTicks+ThisChainTicks+ThisSurfTicks;
	PathTicks += ThisPathTicks;
	SurfTicks += ThisSurfTicks;
	AStarTicks += ThisAStarTicks;
	ChainTicks += ThisChainTicks;
}


// ***************************************************************************

// ***************************************************************************
// ***************************************************************************
// Collisions part.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
const NLPACS::CRetrievableSurface	*NLPACS::CGlobalRetriever::getSurfaceById(const NLPACS::CSurfaceIdent &surfId) const
{
	if(surfId.RetrieverInstanceId>=0 && surfId.SurfaceId>=0)
	{
		sint32	locRetId= this->getInstance(surfId.RetrieverInstanceId).getRetrieverId();
		const CLocalRetriever		&retr = _RetrieverBank->getRetriever(locRetId);
		if (!retr.isLoaded() || surfId.SurfaceId >= (sint)retr.getSurfaces().size())
			return NULL;
		const CRetrievableSurface	&surf= retr.getSurface(surfId.SurfaceId);
		return &surf;
	}
	else
		return NULL;
}



// ***************************************************************************
void	NLPACS::CGlobalRetriever::findCollisionChains(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector &origin) const
{
//	H_AUTO(PACS_GR_findCollisionChains);

	sint	i,j;

	// 0. reset.
	//===========
	// reset possible chains.
//	H_BEFORE(PACS_GR_findCC_reset);
	cst.CollisionChains.clear();
	cst.resetEdgeCollideNodes();
//	H_AFTER(PACS_GR_findCC_reset);

	// 1. Find Instances which may hit this movement.
	//===========
//	H_BEFORE(PACS_GR_findCC_selectInstances);
	CAABBox		bboxMoveGlobal= bboxMove;
	bboxMoveGlobal.setCenter(bboxMoveGlobal.getCenter()+origin);
	selectInstances(bboxMoveGlobal, cst);
//	H_AFTER(PACS_GR_findCC_selectInstances);

	// 2. Fill CollisionChains.
	//===========
	// For each possible surface mesh, test collision.
	for(i=0 ; i<(sint)cst.CollisionInstances.size(); i++)
	{
//		H_BEFORE(PACS_GR_findCC_getAndComputeMove);
		// get retrieverInstance.
		sint32	curInstance= cst.CollisionInstances[i];
		const CRetrieverInstance	&retrieverInstance= getInstance(curInstance);

		// Retrieve the localRetriever of this instance.
		sint32	localRetrieverId= retrieverInstance.getRetrieverId();
		// If invalid one (hole), continue.
		if(localRetrieverId<0)
			continue;
		const CLocalRetriever		&localRetriever= _RetrieverBank->getRetriever(localRetrieverId);

		if (!localRetriever.isLoaded())
		{
			nlwarning("local retriever %d in %s not loaded, findCollisionChains in this retriever aborted", localRetrieverId, _RetrieverBank->getNamePrefix().c_str());
			continue;
		}

		// get delta between startPos.instance and curInstance.
		CVector		deltaOrigin;
		deltaOrigin= origin - retrieverInstance.getOrigin();

		// compute movement relative to this localRetriever.
		CAABBox		bboxMoveLocal= bboxMove;
		bboxMoveLocal.setCenter(bboxMoveLocal.getCenter()+deltaOrigin);

		// add possible collision chains with movement.
		//================
		sint		firstCollisionChain= (sint)cst.CollisionChains.size();
		CVector2f	transBase(-deltaOrigin.x, -deltaOrigin.y);
//		H_AFTER(PACS_GR_findCC_getAndComputeMove);

//		H_BEFORE(PACS_GR_findCC_testCollision);
		// Go! fill collision chains that this movement intersect.
		localRetriever.testCollision(cst, bboxMoveLocal, transBase);
		// if an interior, also test for external collisions
		if (retrieverInstance.getType() == CLocalRetriever::Interior)
			retrieverInstance.testExteriorCollision(cst, bboxMoveLocal, transBase, localRetriever);

		// how many collision chains added?  : nCollisionChain-firstCollisionChain.
		sint		nCollisionChain= (sint)cst.CollisionChains.size();
//		H_AFTER(PACS_GR_findCC_testCollision);


		// For all collision chains added, fill good SurfaceIdent info.
		//================
//		H_BEFORE(PACS_GR_findCC_fillSurfIdent);
		for(j=firstCollisionChain; j<nCollisionChain; j++)
		{
			CCollisionChain		&cc= cst.CollisionChains[j];

			// info are already filled for exterior chains.
			if (cc.ExteriorEdge)
				continue;

			// LeftSurface retrieverInstance is always curInstance.
			cc.LeftSurface.RetrieverInstanceId= curInstance;

			// If RightSurface is not an "edgeId" ie a pointer on a neighbor surface on another retrieverInstance.
			const	CChain		&originalChain= localRetriever.getChain(cc.ChainId);
			if( !originalChain.isBorderChainId(cc.RightSurface.SurfaceId) )
			{
				cc.RightSurface.RetrieverInstanceId= curInstance;
			}
			else
			{
				// we must find the surfaceIdent of the neighbor.

				CRetrieverInstance::CLink	link;
				// get the link to the next surface from the instance
				link = retrieverInstance.getBorderChainLink(CChain::convertBorderChainId(cc.RightSurface.SurfaceId));

				// get the neighbor instanceId.
				sint	neighborInstanceId= (sint16)link.Instance;
				// store in the current collisionChain Right.
				cc.RightSurface.RetrieverInstanceId= neighborInstanceId;

				// If no instance near us, this is a WALL.
				if(neighborInstanceId<0)
				{
					// mark as a Wall.
					cc.RightSurface.SurfaceId= -1;
				}
				else
				{
					// Get the good neighbor surfaceId.
					cc.RightSurface.SurfaceId= (sint16)link.SurfaceId;
				}
			}

			nlassert(cc.LeftSurface.RetrieverInstanceId < (sint)_Instances.size());
			nlassert(cc.RightSurface.RetrieverInstanceId < (sint)_Instances.size());
		}
//		H_AFTER(PACS_GR_findCC_fillSurfIdent);


		// For all collision chains added, look if they are a copy of preceding collsion chain (same Left/Right). Then delete them.
		//================
//		H_BEFORE(PACS_GR_findCC_removeDouble);
		for(j=firstCollisionChain; j<nCollisionChain; j++)
		{
			const CCollisionChain	&cj = cst.CollisionChains[j];

			if (cj.ExteriorEdge && cj.LeftSurface.RetrieverInstanceId!=-1)
				continue;

			// test for all collisionChain inserted before.
			for(sint k=0; k<firstCollisionChain; k++)
			{
				const CCollisionChain	&ck = cst.CollisionChains[k];

				if (cj.LeftSurface.RetrieverInstanceId != cj.RightSurface.RetrieverInstanceId &&
					cj.LeftSurface == ck.RightSurface && cj.RightSurface == ck.LeftSurface)
				{
					const CRetrieverInstance	&instj = getInstance(cj.LeftSurface.RetrieverInstanceId),
												&instk = getInstance(ck.LeftSurface.RetrieverInstanceId);
					const CLocalRetriever		&retrj = getRetriever(instj.getRetrieverId()),
												&retrk = getRetriever(instk.getRetrieverId());

					if (!retrj.isLoaded() || !retrk.isLoaded())
					{
						nlwarning("using not loaded retriever %d or %d in bank '%s', aborted", instj.getRetrieverId(), instk.getRetrieverId(), _RetrieverBank->getNamePrefix().c_str());
						continue;
					}

					nlassert(retrj.getChain(cj.ChainId).isBorderChain() && retrk.getChain(ck.ChainId).isBorderChain());

					if (instj.getBorderChainLink(retrj.getChain(cj.ChainId).getBorderChainIndex()).ChainId != ck.ChainId ||
						instk.getBorderChainLink(retrk.getChain(ck.ChainId).getBorderChainIndex()).ChainId != cj.ChainId)
					{
						continue;
					}

					// remove this jth entry.
					// by swapping with last entry. Only if not already last.
					if(j<nCollisionChain-1)
					{
						swap(cst.CollisionChains[j], cst.CollisionChains[nCollisionChain-1]);
						// NB: some holes remain in cst._EdgeCollideNodes, but do not matters since reseted at
						// each collision test.
					}

					// pop last entry.
					nCollisionChain--;
					cst.CollisionChains.resize(nCollisionChain);

					// next entry??
					j--;
					break;
				}
/*
				// if same surface Ident Left/Right==Left/Right or swapped Left/Right==Right/Left
				if( cst.CollisionChains[j].sameSurfacesThan(cst.CollisionChains[k]) )
				{
					// remove this jth entry.
					// by swapping with last entry. Only if not already last.
					if(j<nCollisionChain-1)
					{
						swap(cst.CollisionChains[j], cst.CollisionChains[nCollisionChain-1]);
						// NB: some holes remain in cst._EdgeCollideNodes, but do not matters since reseted at
						// each collision test.
					}

					// pop last entry.
					nCollisionChain--;
					cst.CollisionChains.resize(nCollisionChain);

					// next entry??
					j--;
					break;
				}
*/
			}

		}
//		H_AFTER(PACS_GR_findCC_removeDouble);
	}

}


// ***************************************************************************
void	NLPACS::CGlobalRetriever::testCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, float radius, const CVector2f bboxStart[4], TCollisionType colType) const
{
//	H_AUTO(PACS_GR_testCollisionWithCollisionChains);

	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	uint			nextCollisionSurfaceTested=0;
	sint			i;

	// reset result.
	cst.CollisionDescs.clear();
	// reset all collisionChain to not tested.
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];
		colChain.Tested= false;
	}

	vector<pair<sint32, bool> >	checkedExtEdges;


	/*
		To manage recovery, we must use such an algorithm, so we are sure to trace the way across all surfaces really
		collided, and discard any other (such as other floor or ceiling).
	*/
	for(;;)
	{
		// run all collisionChain.
		//========================
		for(i=0; i<(sint)cst.CollisionChains.size(); i++)
		{
			CCollisionChain		&colChain= cst.CollisionChains[i];

			/// TODO Tests Ben
			nlassert(colChain.LeftSurface.RetrieverInstanceId < (sint)_Instances.size());
			nlassert(colChain.RightSurface.RetrieverInstanceId < (sint)_Instances.size());

			// test only currentSurface/X. And don't test chains already tested before.
			if(colChain.hasSurface(currentSurface) && !colChain.Tested)
			{
				// we are testing this chain.
				colChain.Tested= true;

				// avoid checking twice a door
				if (colChain.ExteriorEdge && colChain.LeftSurface.RetrieverInstanceId != -1)
				{
					bool	enterInterior = (currentSurface.RetrieverInstanceId == colChain.RightSurface.RetrieverInstanceId);

					uint	j;
					sint32	cmp = (colChain.LeftSurface.RetrieverInstanceId<<16) + colChain.ChainId;
					for (j=0; j<checkedExtEdges.size() && (checkedExtEdges[j].first != cmp); ++j)
						;
					// if already crossed this edge, abort
					// this a door that is crossing a surface frontier
					if (j < checkedExtEdges.size())
					{
						if (checkedExtEdges[j].second != enterInterior)
							continue;
					}
					else
						checkedExtEdges.push_back(make_pair(cmp, enterInterior));
				}

				// test all edges of this chain, and get tmin
				//========================

				float		t=0.0, tMin=1;
				CVector2f	normal, normalMin(0.0f, 0.0f);
				// run list of edge.
				sint32		curEdge= colChain.FirstEdgeCollide;
				while(curEdge!=(sint32)0xFFFFFFFF)
				{
					// get the edge.
					CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

					// test collision with this edge.
					if(colType==CGlobalRetriever::Circle)
						t= colEdge.testCircleMove(startCol, deltaCol, radius, normal);
					else if(colType==CGlobalRetriever::BBox)
						t= colEdge.testBBoxMove(startCol, deltaCol, bboxStart, normal);

					// earlier collision??
					if(t<tMin)
					{
						tMin= t;
						normalMin= normal;
					}

					// next edge.
					curEdge= colEdge.Next;
				}


				// If collision with this chain, must insert it in the array of collision.
				//========================
				if(tMin<1)
				{
					CSurfaceIdent	collidedSurface= colChain.getOtherSurface(currentSurface);

					// if flag as an interior/landscape interface and leave interior surf, retrieve correct surface
					if (colChain.ExteriorEdge && currentSurface == colChain.LeftSurface)
					{
						// p= position until the bounding object collide the exterior edge
						CVector2f		p = startCol + deltaCol*tMin;
						// get the interior origin
						CVector			ori = getInstance(startSurface.RetrieverInstanceId).getOrigin();
						ori.z = 0.0f;

						// Estimate current Z
						UGlobalPosition	rp;
						rp.InstanceId = currentSurface.RetrieverInstanceId;
						rp.LocalPosition.Surface = currentSurface.SurfaceId;
						rp.LocalPosition.Estimation = p;
						// NB: getMeanHeight() should work here since we are still deep in the interior surface (edge collided with bounding volume)
						float	estimatedZ= getMeanHeight(rp);

						// retrieve the position, with the estimated Z
						CVectorD		zp = CVectorD(p.x, p.y, estimatedZ) + CVectorD(ori);
						// Do not allow the current interior instance
						_ForbiddenInstances.clear();
						_ForbiddenInstances.push_back(currentSurface.RetrieverInstanceId);
						UGlobalPosition	gp = retrievePosition(zp);

						collidedSurface.RetrieverInstanceId = gp.InstanceId;
						collidedSurface.SurfaceId = gp.LocalPosition.Surface;
					}

					/// TODO Tests Ben
					nlassert(collidedSurface.RetrieverInstanceId < (sint)_Instances.size());

					// insert or replace this collision in collisionDescs.
					// NB: yes this looks like a N algorithm (so N^2). But not so many collisions may arise, so don't bother.
					sint	indexInsert= (sint)cst.CollisionDescs.size();
					sint	colFound= -1;

					// start to search with nextCollisionSurfaceTested, because can't insert before.
					for(sint j= nextCollisionSurfaceTested; j<(sint)cst.CollisionDescs.size(); j++)
					{
						// we must keep time order.
						if(tMin < cst.CollisionDescs[j].ContactTime)
						{
							indexInsert= min(j, indexInsert);
						}
						// Does the collision with this surface already exist??
						if(cst.CollisionDescs[j].ContactSurface==collidedSurface)
						{
							colFound= j;
							// if we have found our old collision, stop, there is no need to search more.
							break;
						}
					}

					// Insert only if the surface was not already collided, or that new collision arise before old.
					if(colFound==-1 || indexInsert<=colFound)
					{
						CCollisionSurfaceDesc	newCol;
						newCol.ContactSurface= collidedSurface;
						newCol.ContactTime= tMin;
						newCol.ContactNormal.set(normalMin.x, normalMin.y, 0);

						// if, by chance, indexInsert==colFound, just replace old collision descriptor.
						if(colFound==indexInsert)
						{
							cst.CollisionDescs[indexInsert]= newCol;
						}
						else
						{
							// if any, erase old collision against this surface. NB: here, colFound>indexInsert.
							if(colFound!=-1)
								cst.CollisionDescs.erase(cst.CollisionDescs.begin() + colFound);

							// must insert the collision.
							cst.CollisionDescs.insert(cst.CollisionDescs.begin() + indexInsert, newCol);
						}
					}
				}
			}
		}

		// Find next surface to test.
		//========================
		// No more?? so this is the end.
		if(nextCollisionSurfaceTested>=cst.CollisionDescs.size())
			break;
		// else next one.
		else
		{
			// NB: with this algorithm, we are sure that no more collisions will arise before currentCollisionSurfaceTested.
			// so just continue with following surface.
			currentSurface= cst.CollisionDescs[nextCollisionSurfaceTested].ContactSurface;

			// Do we touch a wall??
			bool	isWall;
			if(currentSurface.SurfaceId<0)
				isWall= true;
			else
			{
				// test if it is a walkable wall.
				sint32	locRetId= this->getInstance(currentSurface.RetrieverInstanceId).getRetrieverId();

				if (!_RetrieverBank->getRetriever(locRetId).isLoaded())
				{
					nextCollisionSurfaceTested++;
					continue;
				}

				const CLocalRetriever		&retr = _RetrieverBank->getRetriever(locRetId);
				if (currentSurface.SurfaceId < (sint)retr.getSurfaces().size())
				{
					const CRetrievableSurface	&surf= _RetrieverBank->getRetriever(locRetId).getSurface(currentSurface.SurfaceId);
					isWall= !(surf.isFloor() || surf.isCeiling());
				}
				else
				{
					isWall = true;
				}
			}

			// If we touch a wall, this is the end of search.
			if(isWall)
			{
				// There can be no more collision after this one.
				cst.CollisionDescs.resize(nextCollisionSurfaceTested+1);
				break;
			}
			else
			{
				// Next time, we will test the following (NB: the array may grow during next pass, or reorder,
				// but only after nextCollisionSurfaceTested).
				nextCollisionSurfaceTested++;
			}
		}
	}

}


// ***************************************************************************
bool			NLPACS::CGlobalRetriever::verticalChain(const CCollisionChain &colChain) const
{
	// retrieve surfaces.
	const CRetrievableSurface	*left= getSurfaceById(colChain.LeftSurface);
	const CRetrievableSurface	*right= getSurfaceById(colChain.RightSurface);

	// test if left surface is a wall.
	bool						leftWall;
	if(!left)
		leftWall= true;
	else
		leftWall= !(left->isFloor() || left->isCeiling());

	// test if right surface is a wall.
	bool						rightWall;
	if(!right)
		rightWall= true;
	else
		rightWall= !(right->isFloor() || right->isCeiling());

	// true if both are a wall.
	return leftWall && rightWall;
}


// ***************************************************************************
NLPACS::CSurfaceIdent	NLPACS::CGlobalRetriever::testMovementWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &endCol,
		CSurfaceIdent startSurface, UGlobalPosition &restart) const
{
//	H_AUTO(PACS_GR_testMovementWithCollisionChains);

	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	sint			i;

	// reset result.
	cst.MoveDescs.clear();


	static vector<pair<sint32, bool> >	checkedExtEdges;

	/*
		To manage recovery, we must use such an algorithm, so we are sure to trace the way across all surfaces really
		collided, and discard any other (such as other floor or ceiling).

		This function is quite different from testCollisionWithCollisionChains() because she must detect all collisions
		with all edges of any chains (and not the minimum collision with a chain).
		This is done in 3 parts:
			- detect collisions with all edges.
			- sort.
			- leave only real collisions.
	*/
	// run all collisionChain.
	//========================
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];

		if (colChain.ExteriorEdge)
		{
			sint32	cmp = (colChain.LeftSurface.RetrieverInstanceId<<16) + colChain.ChainId;

			uint	j;
			for (j=0; j<checkedExtEdges.size() && (checkedExtEdges[j].first != cmp); ++j)
				;
			// if already crossed this edge, abort
			// this a door that is crossing a surface frontier
			if (j < checkedExtEdges.size())
				continue;
		}

		// test all edges of this chain, and insert if necessary.
		//========================
		CRational64		t;
		// run list of edge.
		sint32		curEdge= colChain.FirstEdgeCollide;
		while(curEdge!=(sint32)0xFFFFFFFF)
		{
			// get the edge.
			CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

			// test collision with this edge.
			CEdgeCollide::TPointMoveProblem		pmpb;
			t= colEdge.testPointMove(startCol, endCol, pmpb);
			// manage multiple problems of precision.
			if(t== -1)
			{
				static const string	errs[CEdgeCollide::PointMoveProblemCount]= {
					"ParallelEdges", "StartOnEdge", "StopOnEdge", "TraverseEndPoint", "EdgeNull"};
				// return a "Precision Problem" ident. movement is invalid.
				// BUT if startOnEdge, which should never arrive.
				if(pmpb==CEdgeCollide::StartOnEdge)
				{
					nlinfo("COL: Precision Problem: %s", errs[pmpb].c_str());
					checkedExtEdges.clear();
					return CSurfaceIdent(-1, -1);	// so in this case, block....
				}
				else if(pmpb==CEdgeCollide::EdgeNull)
				{
					/*
					// verify if it is an edge which separate 2 walls. in this case, ignore it. else, error.
					if(verticalChain(colChain))
					{
						t=1;	// no collision with this edge.
					}
					else
					{
						nlinfo("COL: Precision Problem: %s", errs[pmpb]);
						nlstop;		// this should not append.
						return CSurfaceIdent(-1, -1);
					}*/
					/* Actually, this is never a problem: we never get through this edge.
						Instead, we'll get through the neighbors edge.
						So just disable this edge.
					*/
					t= 1;
				}
				else
					return	CSurfaceIdent(-2, -2);
			}

			// collision??
			if(t<1)
			{
				// insert in list.
				cst.MoveDescs.push_back(CMoveSurfaceDesc(t, colChain.LeftSurface, colChain.RightSurface));
				cst.MoveDescs.back().ExteriorEdge = colChain.ExteriorEdge;
				cst.MoveDescs.back().ChainId = (uint16)colChain.ChainId;
				cst.MoveDescs.back().MovementSens= colEdge.Norm*(endCol-startCol)>=0;
			}

			// next edge.
			curEdge= colEdge.Next;
		}
	}


	// sort.
	//================
	// sort the collisions in ascending time order.
	sort(cst.MoveDescs.begin(), cst.MoveDescs.end());


	// Traverse the array of collisions.
	//========================
	for(i=0;i<(sint)cst.MoveDescs.size();i++)
	{
		CMoveSurfaceDesc	&msd = cst.MoveDescs[i];

		// Do we collide with this chain??
		if(msd.hasSurface(currentSurface))
		{
			// if flag as an interior/landscape interface and leave interior surf, retrieve correct surface
			if (msd.ExteriorEdge && msd.LeftSurface.RetrieverInstanceId != -1)
			{
				bool	enterInterior = (currentSurface.RetrieverInstanceId == msd.RightSurface.RetrieverInstanceId);

				// msd.MovementSens is true if we "geometrically" leave the interior.
				// If logic and geometric disagree, discard
				if(enterInterior == msd.MovementSens)
					continue;

				uint	j;
				sint32	cmp = (msd.LeftSurface.RetrieverInstanceId<<16) + msd.ChainId;
				for (j=0; j<checkedExtEdges.size() && (checkedExtEdges[j].first != cmp); ++j)
					;
				// if already crossed this edge, abort
				// this a door that is crossing a surface frontier
				if (j < checkedExtEdges.size())
				{
					if (checkedExtEdges[j].second != enterInterior)
						continue;
				}
				else
					checkedExtEdges.push_back(make_pair(cmp, enterInterior));

				// if leave interior, retrieve good position
				if (!enterInterior)
				{
					// p= position until the object center point collide the exterior edge
					float			ctime = (float)((double)(msd.ContactTime.Numerator)/(double)(msd.ContactTime.Denominator));
					CVector2f		p = startCol*(1.0f-ctime) + endCol*ctime;
					// get the interior origin
					CVector			ori = getInstance(startSurface.RetrieverInstanceId).getOrigin();
					ori.z = 0.0f;

					// Estimate current Z
					UGlobalPosition	rp;
					rp.InstanceId = currentSurface.RetrieverInstanceId;
					rp.LocalPosition.Surface = currentSurface.SurfaceId;
					rp.LocalPosition.Estimation = p;
					/* WE HAVE A PRECISION PROBLEM HERE (yoyo 12/04/2006)
						Since the point p has moved close to the exterior edge, because of precision, it may be actually
						OUT the interior surface!!
						thus getMeanHeight() will return 0!!
						Then the chosen landscape position can be completly false. eg:
							actual InteriorHeight: -84
							new possibles landscape surfaces heights: -84 and -16
							if we estimate by error InteriorHeight= 0, then we will
								have Best Landscape Surface == the one which has height=-16 !

						Hence we use a specific method that look a bit outisde the triangles
					*/
					float estimatedZ = getInteriorHeightAround(rp, 0.1f);

					// retrieve the position, with the estimated Z
					CVectorD		zp = CVectorD(p.x, p.y, estimatedZ) + CVectorD(ori);
					// Do not allow the current interior instance
					_ForbiddenInstances.clear();
					_ForbiddenInstances.push_back(currentSurface.RetrieverInstanceId);
					restart = retrievePosition(zp);

					return CSurfaceIdent(-3, -3);
				}
				else
				{
					currentSurface= msd.getOtherSurface(currentSurface);
				}
			}
			else
			{
				currentSurface= msd.getOtherSurface(currentSurface);
			}

			// Do we touch a wall?? should not happens, but important for security.
			bool	isWall;
			if(currentSurface.SurfaceId<0)
				isWall= true;
			else
			{
				// test if it is a walkable wall.
				sint32	locRetId= this->getInstance(currentSurface.RetrieverInstanceId).getRetrieverId();

				if (!_RetrieverBank->getRetriever(locRetId).isLoaded())
					continue;

				const CRetrievableSurface	&surf= _RetrieverBank->getRetriever(locRetId).getSurface(currentSurface.SurfaceId);
				isWall= !(surf.isFloor() || surf.isCeiling());
			}

			// If we touch a wall, this is the end of search.
			if(isWall)
			{
				// return a Wall ident. movement is invalid.
				checkedExtEdges.clear();
				return	CSurfaceIdent(-1, -1);
			}
		}
	}

	checkedExtEdges.clear();
	return currentSurface;
}



// ***************************************************************************
const	NLPACS::TCollisionSurfaceDescVector
	*NLPACS::CGlobalRetriever::testCylinderMove(const UGlobalPosition &startPos, const NLMISC::CVector &delta, float radius, CCollisionSurfaceTemp &cst) const
{
//	H_AUTO(PACS_GR_testCylinderMove);

	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// In a surface ?
	if (startPos.InstanceId==-1)
	{
		// Warning this primitive is not on a surface
		//nlassertonce (0);

		// Return NULL when lost
		return NULL;
	}
	// store this request in cst.
	cst.PrecStartSurface= startSurface;
	cst.PrecStartPos= startPos.LocalPosition.Estimation;
	cst.PrecDeltaPos= delta;
	cst.PrecValid= true;

	// 0.bis
	//===========
	// Abort if deltamove is 0,0,0.
	if (delta.isNull())
		return &cst.CollisionDescs;

	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// bounds the movement in a bbox.
	// compute start and end, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector		end= start+delta;
	// extend the bbox.
	bboxMove.setCenter(start-CVector(radius, radius, 0));
	bboxMove.extend(start+CVector(radius, radius, 0));
	bboxMove.extend(end-CVector(radius, radius, 0));
	bboxMove.extend(end+CVector(radius, radius, 0));


	// 3. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 4. test collisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	CVector2f	deltaCol(delta.x, delta.y);
	CVector2f	obbDummy[4];	// dummy OBB (not obb here so don't bother)
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, radius, obbDummy, CGlobalRetriever::Circle);

	// result.
	return &cst.CollisionDescs;
}


// ***************************************************************************
const	NLPACS::TCollisionSurfaceDescVector
	*NLPACS::CGlobalRetriever::testBBoxMove(const UGlobalPosition &startPos, const NLMISC::CVector &delta,
	const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const
{
//	H_AUTO(PACS_GR_testBBoxMove);

	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// In a surface ?
	if (startPos.InstanceId==-1)
	{
		// Warning this primitive is not on a surface
		//nlassertonce (0);

		// Return NULL when lost
		return NULL;
	}

	// store this request in cst.
	cst.PrecStartSurface= startSurface;
	cst.PrecStartPos= startPos.LocalPosition.Estimation;
	cst.PrecDeltaPos= delta;
	cst.PrecValid= true;

	// 0.bis
	//===========
	// Abort if deltamove is 0,0,0.
	if (delta.isNull())
		return &cst.CollisionDescs;

	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute OBB.
	//===========
	CVector2f	obbStart[4];
	// compute start, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector2f	obbCenter(start.x, start.y);
	CVector2f	locI2d(locI.x, locI.y);
	CVector2f	locJ2d(locJ.x, locJ.y);

	// build points in CCW.
	obbStart[0]= obbCenter - locI2d - locJ2d;
	obbStart[1]= obbCenter + locI2d - locJ2d;
	obbStart[2]= obbCenter + locI2d + locJ2d;
	obbStart[3]= obbCenter - locI2d + locJ2d;

	// 3. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// extend the bbox.
	bboxMove.setCenter(CVector(obbStart[0].x, obbStart[0].y, 0));
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0));
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0));
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0));
	bboxMove.extend(CVector(obbStart[0].x, obbStart[0].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0) + delta);



	// 4. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 5. test collisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	CVector2f	deltaCol(delta.x, delta.y);
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, 0, obbStart, CGlobalRetriever::BBox);

	// result.
	return &cst.CollisionDescs;
}



// ***************************************************************************
NLPACS::UGlobalPosition
	NLPACS::CGlobalRetriever::doMove(const NLPACS::UGlobalPosition &startPos, const NLMISC::CVector &delta, float t, NLPACS::CCollisionSurfaceTemp &cst, bool rebuildChains) const
{
//	H_AUTO(PACS_GR_doMove);

	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// clamp factor.
	clamp(t, 0.0f, 1.0f);

	// 0. reset.
	//===========
	// reset CollisionDescs.
	cst.CollisionDescs.clear();

	// In a surface ?
	if (startPos.InstanceId==-1)
	{
		// Warining: this primitive is not on a surface
		//nlassertonce (0);

		// Return startpos
		return startPos;
	}

	if(!rebuildChains)
	{
		// same move request than prec testMove() ??.
		if( cst.PrecStartSurface != startSurface ||
			cst.PrecStartPos!=startPos.LocalPosition.Estimation ||
			cst.PrecDeltaPos!=delta ||
			!cst.PrecValid)
		{
			// if not, just return start.
			//nlstop;
			//nlwarning ("BEN: you must fix this, it s happen!!!");
			return startPos;
		}
		// Since we are sure we have same movement than prec testMove(), no need to rebuild cst.CollisionChains.
	}
	else
	{
		// we don't have same movement than prec testMove(), we must rebuild cst.CollisionChains.
		// Prec settings no more valids.
		cst.PrecValid= false;
	}




	// 1. Choose a local basis (same than in testMove()).
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. test collisions with CollisionChains.
	//===========
	CVector		start= startPos.LocalPosition.Estimation;
	// compute end with real delta position.
	CVector		end= start + delta*t;

	// If asked, we must rebuild array of collision chains.
	if(rebuildChains)
	{
//		H_AUTO(PACS_GR_doMove_rebuildChains);

		// compute bboxmove.
		CAABBox		bboxMove;
		// must add some extent, to be sure to include snapped CLocalRetriever vertex (2.0f/256 should be sufficient).
		// Nb: this include the precision problem just below (move a little).
		float	radius= 4.0f/Vector2sAccuracy;
		bboxMove.setCenter(start-CVector(radius, radius, 0));
		bboxMove.extend(start+CVector(radius, radius, 0));
		bboxMove.extend(end-CVector(radius, radius, 0));
		bboxMove.extend(end+CVector(radius, radius, 0));

		// find possible collisions in bboxMove+origin. fill cst.CollisionChains.
		findCollisionChains(cst, bboxMove, origin);
	}


	// look where we arrive.
	CSurfaceIdent	endSurface;
	CVector			endRequest= end;
	const sint		maxPbPrec= 32;	// move away from 4 mm at max, in each 8 direction.
	sint			pbPrecNum= 0;

	// must snap the end position.
	CRetrieverInstance::snapVector(endRequest);
	end= endRequest;

	// verify start is already snapped
	{
		CVector		startTest= start;
		CRetrieverInstance::snapVector(startTest);
		nlassert( start == startTest );
	}


	// Normally, just one iteration is made in this loop (but if precision problem (stopOnEdge, startOnEdge....).
	for(;;)
	{
		// must snap the end position.
		CRetrieverInstance::snapVector(end);

		CVector2f	startCol(start.x, start.y);
		CVector2f	endCol(end.x, end.y);

		// If same 2D position, just return startPos (suppose no movement)
		if(endCol==startCol)
		{
			UGlobalPosition		res;
			res= startPos;
			// keep good z movement.
			res.LocalPosition.Estimation.z= end.z;
			return res;
		}

		// search destination problem.
		UGlobalPosition	restart;
		endSurface= testMovementWithCollisionChains(cst, startCol, endCol, startSurface, restart);

		// if no precision problem, Ok, we have found our destination surface (or anormal collide against a wall).
		if (endSurface.SurfaceId >= -1)
		{
			break;
		}
		// left an interior, retrieved position and ask to restart collision from retrieved position
		else if (endSurface.SurfaceId == -3)
		{
			start = getDoubleGlobalPosition(restart) - origin;
			startSurface.RetrieverInstanceId = restart.InstanceId;
			startSurface.SurfaceId = restart.LocalPosition.Surface;
			// should be snapped here
			CVector		startTest= start;
			CRetrieverInstance::snapVector(startTest);
			nlassert( start == startTest );
		}
		/* else we are in deep chit, for one on those reason:
			- traverse on point.
			- stop on a edge (dist==0).
			- start on a edge (dist==0).
			- run // on a edge (NB: dist==0 too).
		*/
		else if (endSurface.SurfaceId == -2)
		{
			// For simplicty, just try to move a little the end position
			if(pbPrecNum<maxPbPrec)
			{
				static struct	{sint x,y;}   dirs[8]= { {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
				sint	dir= pbPrecNum%8;
				sint	dist= pbPrecNum/8+1;
				CVector		dta;

				// compute small move.
				dta.x= dirs[dir].x * dist * 1.0f/SnapPrecision;
				dta.y= dirs[dir].y * dist * 1.0f/SnapPrecision;
				dta.z= 0;

				// add it to the original end pos requested.
				end= endRequest + dta;

				pbPrecNum++;
			}
			else
			{
				// do not move at all.
				endSurface= CSurfaceIdent(-1,-1);
				break;
			}
		}
	}

	// 3. return result.
	//===========
	// Problem?? do not move.
	if(endSurface.SurfaceId==-1)
		return startPos;
	else
	{
		// else must return good GlobalPosition.
		CGlobalPosition		res;

		res.InstanceId= endSurface.RetrieverInstanceId;
		res.LocalPosition.Surface= endSurface.SurfaceId;

		// compute newPos, localy to the endSurface.
		// get delta between startPos.instance and curInstance.
		// NB: for float precision, it is important to compute deltaOrigin, and after compute newPos in local.
		CVector		deltaOrigin;
		deltaOrigin= origin - getInstance(res.InstanceId).getOrigin();

		// Because Origin precision is 1 meter, and end precision is 1/1024 meter, we have no precision problem.
		// this is true because we cannot move more than, say 4*160 meters in one doMove().
		// So global position should not be bigger than 1024 * 1024/1024 meters.  => Hence 20 bits of precision is
		// required. We have 23 with floats.
		res.LocalPosition.Estimation= end + deltaOrigin;


		// result.
		return res;
	}

}


// ***************************************************************************
const NLPACS::TCollisionSurfaceDescVector	&NLPACS::CGlobalRetriever::testBBoxRot(const CGlobalPosition &startPos,
	const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const
{
//	H_AUTO(PACS_GR_testBBoxRot);

	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// should not doMove() after a testBBoxRot.
	cst.PrecValid= false;


	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute OBB.
	//===========
	CVector2f	obbStart[4];
	// compute start, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector2f	obbCenter(start.x, start.y);
	CVector2f	locI2d(locI.x, locI.y);
	CVector2f	locJ2d(locJ.x, locJ.y);

	// build points in CCW.
	obbStart[0]= obbCenter - locI2d - locJ2d;
	obbStart[1]= obbCenter + locI2d - locJ2d;
	obbStart[2]= obbCenter + locI2d + locJ2d;
	obbStart[3]= obbCenter - locI2d + locJ2d;

	// 3. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// extend the bbox.
	bboxMove.setCenter(CVector(obbStart[0].x, obbStart[0].y, 0));
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0));
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0));
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0));



	// 4. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 5. test Rotcollisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	testRotCollisionWithCollisionChains(cst, startCol, startSurface, obbStart);


	// result.
	return cst.CollisionDescs;
}


// ***************************************************************************
void	NLPACS::CGlobalRetriever::testRotCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &/* startCol */, CSurfaceIdent startSurface, const CVector2f bbox[4]) const
{
//	H_AUTO(PACS_GR_testRotCollisionWithCollisionChains);

	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	sint			i;

	// reset result.
	cst.RotDescs.clear();
	cst.CollisionDescs.clear();


	/*
		Test collisions with all collision chains. Then, to manage recovery, test the graph of surfaces.
	*/
	// run all collisionChain.
	//========================
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];


		// test all edges of this chain, and insert if necessary.
		//========================
		// run list of edge.
		sint32		curEdge= colChain.FirstEdgeCollide;
		while(curEdge!=(sint32)0xFFFFFFFF)
		{
			// get the edge.
			CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

			// test collision with this edge.
			if(colEdge.testBBoxCollide(bbox))
			{
				// yes we have a 2D collision with this chain.
				cst.RotDescs.push_back(CRotSurfaceDesc(colChain.LeftSurface, colChain.RightSurface));
				break;
			}

			// next edge.
			curEdge= colEdge.Next;
		}
	}


	// Traverse the array of collisions.
	//========================
	sint	indexCD=0;
	for(;;)
	{
		// What surfaces collided do we reach from this currentSurface??
		for(i=0;i<(sint)cst.RotDescs.size();i++)
		{
			// Do we collide with this chain?? chain not tested??
			if(cst.RotDescs[i].hasSurface(currentSurface) && !cst.RotDescs[i].Tested)
			{
				cst.RotDescs[i].Tested= true;

				// insert the collision with the other surface.
				CCollisionSurfaceDesc	col;
				col.ContactTime= 0;
				col.ContactNormal= CVector::Null;
				col.ContactSurface= cst.RotDescs[i].getOtherSurface(currentSurface);
				cst.CollisionDescs.push_back(col);
			}
		}

		// get the next currentSurface from surface collided (traverse the graph of collisions).
		if(indexCD<(sint)cst.CollisionDescs.size())
			currentSurface= cst.CollisionDescs[indexCD++].ContactSurface;
		else
			break;
	}

}

// ***************************************************************************

NLPACS::UGlobalRetriever *NLPACS::UGlobalRetriever::createGlobalRetriever (const char *globalRetriever, const NLPACS::URetrieverBank *retrieverBank)
{

	// Cast
//	nlassert (dynamic_cast<const NLPACS::CRetrieverBank*>(retrieverBank));
	const NLPACS::CRetrieverBank*	bank=static_cast<const NLPACS::CRetrieverBank*>(retrieverBank);

	CIFile	file;
	if (file.open(CPath::lookup(globalRetriever)))
	{
		CGlobalRetriever	*retriever = new CGlobalRetriever();

		// always set the retriever bank before serializing !!
		retriever->setRetrieverBank(bank);

		file.serial(*retriever);
		retriever->initAll(false);	// don't init instances as we serialized them

		return static_cast<UGlobalRetriever *>(retriever);
	}
	else
		return NULL;
}

// ***************************************************************************

void NLPACS::UGlobalRetriever::deleteGlobalRetriever (UGlobalRetriever *retriever)
{
	// Cast
	nlassert (dynamic_cast<NLPACS::CGlobalRetriever*>(retriever));
	NLPACS::CGlobalRetriever* r=static_cast<NLPACS::CGlobalRetriever*>(retriever);

	// Delete
	delete r;
}

// ***************************************************************************

float			NLPACS::CGlobalRetriever::getMeanHeight(const UGlobalPosition &pos) const
{
	// for wrong positions, leave it unchanged
	if ((pos.InstanceId==-1)||(pos.LocalPosition.Surface==-1))
		return pos.LocalPosition.Estimation.z;

	// get instance/localretriever.
	const CRetrieverInstance	&instance = getInstance(pos.InstanceId);
	const CLocalRetriever		&retriever= _RetrieverBank->getRetriever(instance.getRetrieverId());

	if (!retriever.isLoaded())
		return pos.LocalPosition.Estimation.z;

	// return height from local retriever
	return retriever.getHeight(pos.LocalPosition);
}

// ***************************************************************************
float			NLPACS::CGlobalRetriever::getInteriorHeightAround(const UGlobalPosition &pos, float outsideTolerance) const
{
	// for wrong positions, leave it unchanged
	if ((pos.InstanceId==-1)||(pos.LocalPosition.Surface==-1))
		return pos.LocalPosition.Estimation.z;

	// get instance/localretriever.
	const CRetrieverInstance	&instance = getInstance(pos.InstanceId);
	const CLocalRetriever		&retriever= _RetrieverBank->getRetriever(instance.getRetrieverId());

	if (!retriever.isLoaded())
		return pos.LocalPosition.Estimation.z;

	// return height from local retriever
	return retriever.getInteriorHeightAround(pos.LocalPosition, outsideTolerance);
}

// ***************************************************************************

bool NLPACS::CGlobalRetriever::testRaytrace (const CVectorD &/* v0 */, const CVectorD &/* v1 */)
{
	// TODO: implement raytrace
	return false;
}

// ***************************************************************************

void	NLPACS::CGlobalRetriever::refreshLrAround(const CVector &position, float radius)
{
	NLPACS_HAUTO_REFRESH_LR_AROUND

	// check if retriever bank is all loaded, and if yes don't refresh it
	if (_RetrieverBank->allLoaded())
		return;

	std::list<CLrLoader>::iterator ite = _LrLoaderList.begin();
	while (ite != _LrLoaderList.end())
	{
		// Finished loaded a lr, stream it into rbank
		if (ite->Finished && ite->Successful)
		{
			if (!ite->_Buffer.isReading())
				ite->_Buffer.invert();

			ite->_Buffer.resetBufPos();

			//		NLMEMORY::CheckHeap (true);

			const_cast<CRetrieverBank*>(_RetrieverBank)->loadRetriever(ite->LrId, ite->_Buffer);

			//		NLMEMORY::CheckHeap (true);

			ite->_Buffer.clear();

			//		NLMEMORY::CheckHeap (true);

			//nlinfo("Lr '%s' loading task complete", ite->LoadFile.c_str());

			// Remove this entry
			_LrLoaderList.erase (ite);

			break;
		}

		// Next lr
		ite++;
	}

	CAABBox	box;
	box.setCenter(position);
	box.setHalfSize(CVector(radius, radius, 1000.0f));

	selectInstances(box, _InternalCST);

	set<uint>	newlr, in, out;
	map<uint, CVector>	lrPosition;

	uint	i;
	for (i=0; i<_InternalCST.CollisionInstances.size(); ++i)
	{
		uint lrId = (uint)(_Instances[_InternalCST.CollisionInstances[i]].getRetrieverId());
		newlr.insert(lrId);
		lrPosition.insert (map<uint, CVector>::value_type(lrId, _Instances[_InternalCST.CollisionInstances[i]].getBBox().getCenter()));
	}

	const_cast<CRetrieverBank*>(_RetrieverBank)->diff(newlr, in, out);

	set<uint>::iterator	it;

	// unload all possible retrievers
	for (it=out.begin(); it!=out.end(); ++it)
	{
		const_cast<CRetrieverBank*>(_RetrieverBank)->unloadRetriever(*it);
		//nlinfo("Freed Lr '%s'", (_RetrieverBank->getNamePrefix() + "_" + toString(*it) + ".lr").c_str());
	}

	// if load task idle and more lr to load, setup load task
	set<uint>::iterator iteIn = in.begin();
	while (iteIn != in.end())
	{
		// Already exist ?
		ite = _LrLoaderList.begin();
		while (ite != _LrLoaderList.end())
		{
			if (ite->LrId == *iteIn)
				break;

			ite++;
		}

		// Not found ?
		if (ite == _LrLoaderList.end())
		{
			// Get the position fot this LR
			map<uint, CVector>::iterator iteLR = lrPosition.find(*iteIn);
			nlassert (iteLR != lrPosition.end());

			_LrLoaderList.push_back (CLrLoader (iteLR->second));
			CLrLoader &loader = _LrLoaderList.back();
			loader.Finished = false;
			loader.LrId = *iteIn;
			loader.LoadFile = _RetrieverBank->getNamePrefix() + "_" + toString(loader.LrId) + ".lr";

			CAsyncFileManager::getInstance().addLoadTask(&loader);

			//nlinfo("Lr '%s' added to load", loader.LoadFile.c_str());
		}

		// Next lr to load
		iteIn++;
	}
}

// ***************************************************************************
void	NLPACS::CGlobalRetriever::waitEndOfAsyncLoading()
{
	while (!_LrLoaderList.empty ())
	{
		std::list<CLrLoader>::iterator ite = _LrLoaderList.begin();
		while (ite != _LrLoaderList.end())
		{
			// Finished loaded a lr, stream it into rbank
			if (ite->Finished)
			{
				if (!ite->_Buffer.isReading())
					ite->_Buffer.invert();

				const_cast<CRetrieverBank*>(_RetrieverBank)->loadRetriever(ite->LrId, ite->_Buffer);

				ite->_Buffer.clear();

				// Remove this from the list
				_LrLoaderList.erase(ite);

				break;
			}

			//
			ite++;
		}

		if (!_LrLoaderList.empty())
			nlSleep(0);
	}

}

// ***************************************************************************
void	NLPACS::CGlobalRetriever::refreshLrAroundNow(const CVector &position, float radius)
{
	// check if retriever bank is all loaded, and if yes don't refresh it
	if (_RetrieverBank->allLoaded())
		return;

	// must wait all current have finished loading
	waitEndOfAsyncLoading();

	// Select new to load
	CAABBox	box;
	box.setCenter(position);
	box.setHalfSize(CVector(radius, radius, 1000.0f));

	selectInstances(box, _InternalCST);

	set<uint>	newlr, in, out;
	uint	i;
	for (i=0; i<_InternalCST.CollisionInstances.size(); ++i)
		newlr.insert((uint)(_Instances[_InternalCST.CollisionInstances[i]].getRetrieverId()));

	const_cast<CRetrieverBank*>(_RetrieverBank)->diff(newlr, in, out);

	set<uint>::iterator	it;

	// unload all possible retrievers
	for (it=out.begin(); it!=out.end(); ++it)
		const_cast<CRetrieverBank*>(_RetrieverBank)->unloadRetriever(*it);

	// unload all possible retrievers
	for (it=in.begin(); it!=in.end(); ++it)
	{
		string		fname = _RetrieverBank->getNamePrefix() + "_" + toString(*it) + ".lr";
		CIFile		f;
		if (!f.open(CPath::lookup(fname, false)))
		{
			nlwarning("Couldn't find file '%s' to load, retriever loading aborted", fname.c_str());
			continue;
		}

		const_cast<CRetrieverBank*>(_RetrieverBank)->loadRetriever(*it, f);
	}
}

void	NLPACS::CGlobalRetriever::CLrLoader::run()
{
	CIFile		f;

	// async
	f.setAsyncLoading(true);
	f.setCacheFileOnOpen(true);

	Successful = false;

	if (!f.open(CPath::lookup(LoadFile, false)))
	{
		nlwarning("Couldn't find file '%s' to load, retriever loading aborted", LoadFile.c_str());
		_Buffer.clear();
		Finished = true;
		return;
	}

	if (!_Buffer.isReading())
		_Buffer.invert();

	uint8	*buffer = _Buffer.bufferToFill(f.getFileSize());
	f.serialBuffer(buffer, f.getFileSize());

	Successful = true;
	Finished = true;
}

// ***************************************************************************
void	NLPACS::CGlobalRetriever::CLrLoader::getName (std::string &result) const
{
	result = "LoadLR(" + LoadFile + ")";
}


//
NLMISC_CATEGORISED_VARIABLE(nel, uint, PacsRetrieveVerbose, "Allow retrieve position to dump info");

// end of CGlobalRetriever methods implementation
