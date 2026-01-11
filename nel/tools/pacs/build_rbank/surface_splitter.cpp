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

#include "surface_splitter.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;

// -----------------------
//

// Constructor
CSurfaceSplitter::CSurfaceSplitter()
{
}


//
void	CSurfaceSplitter::build(CLocalRetriever &lr)
{
	nlinfo("--- Build SurfaceSplitter...");
	nlinfo("------------------------------------------");
	uint	i;

	initEdgeGrid();

	_NumChains = 0;
	_NumSurfaces = 0;
	_NumTips = 0;

	for (i=0; i<lr.getChains().size(); ++i)
		buildChain(lr, i);

	for (i=0; i<lr.getSurfaces().size(); ++i)
		buildSurface(lr, i);

	nlinfo("converted %d chains & %d surfaces", lr.getChains().size(), lr.getSurfaces().size());
	_NumSurfaces = (uint)lr.getSurfaces().size();
	_NumChains = (uint)lr.getChains().size();

	//splitChains();

	dump();

	nlinfo("------------------------------------------");
}

//
void	CSurfaceSplitter::buildChain(CLocalRetriever &lr, uint chain)
{
	const NLPACS::CChain	&pacsChain = lr.getChain(chain);

	CSurfaceId				left = pacsChain.getLeft() >= 0 ? CSurfaceId(0, (uint16)pacsChain.getLeft()) : CSurfaceId();
	CSurfaceId				right = pacsChain.getRight() >= 0 ? CSurfaceId(0, (uint16)pacsChain.getRight()) : CSurfaceId();
	vector<CVector2s64>		vertices;

	uint	i;
	// walk through all subchains
	for (i=0; i<pacsChain.getSubChains().size(); ++i)
	{
		uint	ochain = pacsChain.getSubChain(i);
		const NLPACS::COrderedChain	&pacsOChain = lr.getOrderedChain(ochain);

		sint	j;
		// walk through subchain, forward or backward
		if (pacsOChain.isForward())
		{
			for (j=0; j<(sint)pacsOChain.getVertices().size()-1; ++j)
				vertices.push_back(CVector2s64(pacsOChain[j]));
		}
		else
		{
			for (j=(sint)pacsOChain.getVertices().size()-1; j>0; --j)
				vertices.push_back(CVector2s64(pacsOChain[j]));
		}

		// add final chain point
		if (i == pacsChain.getSubChains().size()-1)
			vertices.push_back(CVector2s64(pacsOChain[j]));
	}

	addChain(left,right, vertices);
}

//
void	CSurfaceSplitter::buildSurface(CLocalRetriever &lr, uint surface)
{
	const NLPACS::CRetrievableSurface	&pacsSurface = lr.getSurface(surface);
	CSurface							newSurface;

	newSurface.Id = CSurfaceId(0, surface);

	uint	i;
	// walk through all loops
	for (i=0; i<pacsSurface.getLoops().size(); ++i)
	{
		const NLPACS::CRetrievableSurface::TLoop	&pacsLoop = pacsSurface.getLoop(i);
		newSurface.Loops.push_back(CSurfaceSplitter::CLoop());
		newSurface.Loops.back().Surface = newSurface.Id;

		uint	j;
		// walk through loop
		for (j=0; j<pacsLoop.size(); ++j)
		{
			uint	chainIndex = pacsLoop[j];
			uint	chain = pacsSurface.getChain(chainIndex).Chain;

			newSurface.Loops.back().Chains.push_back(CChainId(chain));
		}
	}

	_Surfaces.insert(make_pair(newSurface.Id, newSurface));
}



//
void	CSurfaceSplitter::initEdgeGrid()
{
	_Edges.create(256, 2.0f);
}


//
void	CSurfaceSplitter::splitChains()
{
	nlinfo("Split chains...");

	uint	numInters = 0;
	uint	i;

	for (i=0; i<_NumChains; ++i)
	{
		TChainMap::iterator	it = _Chains.find(CChainId(i));
		if (it != _Chains.end())
			splitChain(it, numInters);
	}

	nlinfo("%d intersections found", numInters);
}

//
void	CSurfaceSplitter::splitChain(TChainMap::iterator it, uint &numInters)
{
	CChain	&chain = (*it).second;

	if (chain.DontSplit)
		return;

	uint	edge;
	// for each edge of the chain, test for other edges collision
	for (edge=0; edge<chain.Vertices.size()-1; ++edge)
	{
		CVector	p0 = chain.Vertices[edge].asVector();
		CVector	p1 = chain.Vertices[edge+1].asVector();
		CVector	pmin, pmax;
		pmin.minof(p0, p1);
		pmax.maxof(p0, p1);

		_Edges.select(pmin, pmax);

		CFixed64	closerDist(10.0);
		bool		collisionFound = false;
		CEdgeId		collidingEdge;
		CVector2s64	collision;

		TEdgeGrid::CIterator	it;
		for (it=_Edges.begin(); it!=_Edges.end(); ++it)
		{
			CEdgeId		iedge = *it;

			//
			if (chain.Id == iedge.Chain && (edge == iedge.Edge || edge+1 == iedge.Edge || edge-1 == iedge.Edge))
				continue;

			CChain		*ichain = getChain(iedge.Chain);

			if (ichain == NULL)
			{
				nlwarning("Couldn't find referenced chain %d", iedge.Chain.Id);
				continue;
			}

			if (ichain->DontSplit)
				continue;

			CVector2s64	inters;
			CFixed64	ndist;

			if (intersect(chain.Vertices[edge], chain.Vertices[edge+1], ichain->Vertices[iedge.Edge], ichain->Vertices[iedge.Edge+1], inters, ndist))
			{
				if (inters != chain.Vertices[edge+1] || edge != chain.Vertices.size()-2)
				{
					++numInters;
					nlinfo("Intersection: %d:%d[%.3f,%.3f-%.3f,%.3f]-%d:%d[%.3f,%.3f-%.3f,%.3f] : [%.3f,%.3f]", chain.Id.Id, edge, p0.x, p0.y, p1.x, p1.y, iedge.Chain.Id, iedge.Edge, ichain->Vertices[iedge.Edge].asVector().x, ichain->Vertices[iedge.Edge].asVector().y, ichain->Vertices[iedge.Edge+1].asVector().x, ichain->Vertices[iedge.Edge+1].asVector().y, inters.asVector().x, inters.asVector().y);

					if (closerDist > ndist)
					{
						collisionFound = true;
						collidingEdge = iedge;
						collision = inters;
					}
				}
				else
				{
					nlinfo("Intersection: %d:%d[%.3f,%.3f-%.3f,%.3f]-%d:%d[%.3f,%.3f-%.3f,%.3f] : [%.3f,%.3f] -- SKIPPED", chain.Id.Id, edge, p0.x, p0.y, p1.x, p1.y, iedge.Chain.Id, iedge.Edge, ichain->Vertices[iedge.Edge].asVector().x, ichain->Vertices[iedge.Edge].asVector().y, ichain->Vertices[iedge.Edge+1].asVector().x, ichain->Vertices[iedge.Edge+1].asVector().y, inters.asVector().x, inters.asVector().y);
				}
			}
		}

		// split chain
		if (collisionFound)
		{
			if (chain.Id == collidingEdge.Chain)
			{
				// self colliding chain

				// check order
				//nlassert(edge >= 0); // always true for unsigned
				nlassert(edge < collidingEdge.Edge);
				nlassert(collidingEdge.Edge < chain.Vertices.size()-1);

				// must split the chain in 3 parts
				uint				e;
				vector<CVector2s64>	begin;
				vector<CVector2s64>	middle;
				vector<CVector2s64>	end;
				vector<CChainId>	v;
				CChainId			beginId;
				CChainId			middleId;
				CChainId			endId;

				for (e=0; e<=edge; ++e)
					begin.push_back(chain.Vertices[e]);

				begin.push_back(collision);

				if (collision != chain.Vertices[e])
					middle.push_back(collision);

				for (; e<=collidingEdge.Edge; ++e)
					middle.push_back(chain.Vertices[e]);

				middle.push_back(collision);

				if (collision != chain.Vertices[e])
					end.push_back(collision);

				for (; e<chain.Vertices.size(); ++e)
					end.push_back(chain.Vertices[e]);

				beginId = addChain(chain.Left, chain.Right, begin, true);
				middleId = addChain(chain.Left, chain.Right, middle);
				endId = addChain(chain.Left, chain.Right, end);

				v.push_back(beginId);
				v.push_back(middleId);
				v.push_back(endId);

				replaceChain(chain.Id, v);
			}
			else
			{
				//nlassert(edge >= 0); // always true for unsigned
				nlassert(edge < chain.Vertices.size()-1);

				// split the chain
				uint				e;
				vector<CVector2s64>	begin;
				vector<CVector2s64>	end;
				vector<CChainId>	v;
				CChainId			beginId;
				CChainId			endId;

				// split the first chain
				for (e=0; e<=edge; ++e)
					begin.push_back(chain.Vertices[e]);

				begin.push_back(collision);

				if (collision != chain.Vertices[e])
					end.push_back(collision);

				for (; e<chain.Vertices.size(); ++e)
					end.push_back(chain.Vertices[e]);

				beginId = addChain(chain.Left, chain.Right, begin, true);
				endId = addChain(chain.Left, chain.Right, end);

				v.push_back(beginId);
				v.push_back(endId);

				replaceChain(chain.Id, v);

				// reset for second chain
				begin.clear();
				end.clear();
				v.clear();

				// split the second chain
				CChain	*collide = getChain(collidingEdge.Chain);

				for (e=0; e<=collidingEdge.Edge; ++e)
					begin.push_back(collide->Vertices[e]);

				begin.push_back(collision);

				if (collision != collide->Vertices[e])
					end.push_back(collision);

				for (; e<collide->Vertices.size(); ++e)
					end.push_back(collide->Vertices[e]);

				beginId = addChain(collide->Left, collide->Right, begin);
				endId = addChain(collide->Left, collide->Right, end);

				v.push_back(beginId);
				v.push_back(endId);

				replaceChain(collide->Id, v);
			}

			return;
		}
	}
}

//
bool	CSurfaceSplitter::intersect(const CVector2s64 &v0, const CVector2s64 &v1,
									const CVector2s64 &c0, const CVector2s64 &c1,
									CVector2s64 &intersect,
									CFixed64 &ndist)
{
	if (v0 == c0 || v0 == c1)
		return false;

	if (v1 == c0 || v1 == c1)
	{
		intersect = v1;
		ndist = CFixed64(1.0);
		return true;
	}

	CVector2s64		nnc(c0.y-c1.y, c1.x-c0.x),
					nnv(v0.y-v1.y, v1.x-v0.x);

	CFixed64		dv = (v1-v0)*nnc;

	// vectors are colinears ?
	if ((sint64)dv == 0)
		return false;

	CFixed64		nv = (c0-v0)*nnc;

	if ((sint64)dv < 0)
		dv=-dv, nv=-nv;
	// intersection outside main edge ? (or at first point ?)
	if ((sint64)nv<=0 || nv>dv)
		return false;

	CFixed64		dc = (c1-c0)*(c1-c0);
	// second edge null ?
	if ((sint64)dc == 0)
		return false;

	CFixed64		lv = nv/dv;

	if ((sint64)lv == 0)
		return false;

	CFixed64		nc = (v0-c0 + (v1-v0)*lv)*(c1-c0);
	// intersection outside colliding edge ?
	if ((sint64)nc<0 || nc>dc)
		return false;

	// treat each singular case
	if (nv == dv)
		intersect = v1;
	else if ((sint64)nc == 0)
		intersect = c0;
	else if (nc == dc)
		intersect = c1;
	else
		// compute intersecting point
		intersect = v0 + (v1-v0)*lv;

	ndist = lv;

	return true;
}



//
CSurfaceSplitter::CChainId	CSurfaceSplitter::addChain(const CSurfaceId &left, const CSurfaceId &right, const vector<CVector2s64> &points, bool dontSplit)
{
	pair<TChainMap::iterator, bool>	res = _Chains.insert(make_pair(CChainId(_NumChains++), CChain()));

	CChain	&chain = (*(res.first)).second;

	chain.Id = (*(res.first)).first;
	chain.Left = left;
	chain.Right = right;

	uint	i;
	for (i=0; i<points.size()-1; ++i)
	{
		if (points[i] == points[i+1])
		{
			nlwarning("--- !! points are together !! ---");
			nlstop;
		}
	}

	chain.Vertices = points;
	chain.DontSplit = dontSplit;

	uint	edge;
	for (edge=0; edge<chain.Vertices.size()-1; ++edge)
	{
		CVector	p0 = chain.Vertices[edge].asVector();
		CVector	p1 = chain.Vertices[edge+1].asVector();

		p0.z = -100.0f;
		p1.z = +100.0f;

		CVector	pmin, pmax;
		pmin.minof(p0, p1);
		pmax.maxof(p0, p1);

		chain.Iterators.push_back(_Edges.insert(pmin, pmax, CEdgeId(chain.Id, edge)));
	}

	return chain.Id;
}

//
void	CSurfaceSplitter::removeChain(CChainId chainId)
{
	// get chain it
	TChainMap::iterator	it = _Chains.find(chainId);

	if (it == _Chains.end())
		return;

	CChain	&chain = (*it).second;

	CSurface	*surf;
	
	if ((surf = getSurface(chain.Left)))
		surf->removeChain(chainId);

	if ((surf = getSurface(chain.Right)))
		surf->removeChain(chainId);
}

//
void	CSurfaceSplitter::replaceChain(CChainId chainId, const vector<CChainId> &chains)
{
	// get chain it
	TChainMap::iterator	it = _Chains.find(chainId);

	if ((it == _Chains.end()))
		return;

	CChain	&chain = (*it).second;

	CSurface	*surf;

	nlinfo("-- Replace --");
	dumpChain(chainId);
	nlinfo("-- By --");
	uint	c;
	for (c=0; c<chains.size(); ++c)
		dumpChain(chains[c]);
	nlinfo("-- End Replace --");
	
	if ((surf = getSurface(chain.Left)))
	{
		uint	loop;
		for (loop=0; loop<surf->Loops.size(); ++loop)
		{
			CLoop	&ploop = surf->Loops[loop];
			vector<CChainId>::iterator	it;
			for (it=ploop.Chains.begin(); it!=ploop.Chains.end(); ++it)
			{
				if (*it == chainId)
				{
					it = ploop.Chains.erase(it);
					sint	i;
					for (i=(sint)chains.size()-1; i>=0; --i)
					{
						it = ploop.Chains.insert(it, chains[i]);
					}
				}
			}
		}
	}

	if ((surf = getSurface(chain.Right)))
	{
		uint	loop;
		for (loop=0; loop<surf->Loops.size(); ++loop)
		{
			CLoop	&ploop = surf->Loops[loop];
			vector<CChainId>::iterator	it;
			for (it=ploop.Chains.begin(); it!=ploop.Chains.end(); ++it)
			{
				if (*it == chainId)
				{
					it = ploop.Chains.erase(it);
					uint	i;
					for (i=0; i<chains.size(); ++i)
					{
						it = ploop.Chains.insert(it, chains[i]);
					}
				}
			}
		}
	}

	uint	i;
	for (i=0; i<chain.Iterators.size(); ++i)
		_Edges.erase(chain.Iterators[i]);

	_Chains.erase(it);
}
