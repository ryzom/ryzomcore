// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <sstream>

#include "nel/misc/plane.h"

#include "nel/pacs/local_retriever.h"
#include "nel/pacs/collision_desc.h"
#include "nel/pacs/retriever_instance.h"

#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;

/// The max distance allowed to merge tips.
const float	NLPACS::CLocalRetriever::_TipThreshold = 0.1f;
const float	NLPACS::CLocalRetriever::_EdgeTipThreshold = 0.1f;

/// The threshold distance to insure a position belongs to a surface
const float	InsurePositionThreshold = 2.0e-2f;

//static float	hybrid2dNorm(const CVector &v)
//{
//	return (float)(sqrt(sqr(v.x)+sqr(v.y))+fabs(v.z)*0.1);
//}





NLPACS::CLocalRetriever::CLocalRetriever()
{
	_Type = Landscape;
	_Loaded = false;
	LoadCheckFlag = false;
}


void	NLPACS::CLocalRetriever::clear()
{
	contReset(_OrderedChains);
	contReset(_FullOrderedChains);
	contReset(_Chains);
	contReset(_Surfaces);
	contReset(__Tips);
	contReset(_BorderChains);
	uint	i;
	for (i=0; i<NumMaxCreatureModels; ++i)
		contReset(_Topologies[i]);
	_ChainQuad.clear();
	_ExteriorMesh.clear();
	contReset(_InteriorVertices);
	contReset(_InteriorFaces);
	_FaceGrid.clear();
	_Id.resize(0);
	_Loaded = false;
	LoadCheckFlag = false;
}


const CVector	&NLPACS::CLocalRetriever::getStartVector(uint32 chain) const
{
	const COrderedChain3f	&ochain = _FullOrderedChains[_Chains[chain].getSubChains().front()];
	return (ochain.isForward()) ? ochain.getVertices().front() : ochain.getVertices().back();
}

const CVector	&NLPACS::CLocalRetriever::getStopVector(uint32 chain) const
{
	const COrderedChain3f	&ochain = _FullOrderedChains[_Chains[chain].getSubChains().back()];
	return (ochain.isForward()) ? ochain.getVertices().back() : ochain.getVertices().front();
}




const CVector	&NLPACS::CLocalRetriever::getStartVector(uint32 chain, sint32 surface) const
{
	bool					onLeft = _Chains[chain].getLeft() == surface;
	const COrderedChain3f	&ochain = onLeft ? _FullOrderedChains[_Chains[chain].getSubChains().front()] :
											   _FullOrderedChains[_Chains[chain].getSubChains().back()];

	if (ochain.isForward() == onLeft)
		return ochain.getVertices().front();
	else
		return ochain.getVertices().back();
}

const CVector	&NLPACS::CLocalRetriever::getStopVector(uint32 chain, sint32 surface) const
{
	bool					onLeft = _Chains[chain].getLeft() == surface;
	const COrderedChain3f	&ochain = onLeft ? _FullOrderedChains[_Chains[chain].getSubChains().back()] :
											   _FullOrderedChains[_Chains[chain].getSubChains().front()];

	if (ochain.isForward() == onLeft)
		return ochain.getVertices().back();
	else
		return ochain.getVertices().front();
}



/*
uint16			NLPACS::CLocalRetriever::getStartTip(uint32 chain, sint32 surface) const
{
	return (_Chains[chain].getLeft() == surface) ? _Chains[chain].getStartTip() : _Chains[chain].getStopTip();
}

uint16			NLPACS::CLocalRetriever::getStopTip(uint32 chain, sint32 surface) const
{
	return (_Chains[chain].getLeft() == surface) ? _Chains[chain].getStopTip() : _Chains[chain].getStartTip();
}





void			NLPACS::CLocalRetriever::setStartTip(uint32 chain, sint32 surface, uint16 startTip)
{
	if (_Chains[chain].getLeft() == surface)
		_Chains[chain]._StartTip = startTip;
	else
		_Chains[chain]._StopTip = startTip;
}

void			NLPACS::CLocalRetriever::setStopTip(uint32 chain, sint32 surface, uint16 stopTip)
{
	if (_Chains[chain].getLeft() == surface)
		_Chains[chain]._StopTip = stopTip;
	else
		_Chains[chain]._StartTip = stopTip;
}
*/



uint32			NLPACS::CLocalRetriever::getPreviousChain(uint32 chain, sint32 surface) const
{
	uint								loop;
	uint								loopIndex;

	if (_Chains[chain].getLeft() == surface)
	{
		loop = _Chains[chain]._LeftLoop;
		loopIndex = _Chains[chain]._LeftLoopIndex;
	}
	else
	{
		loop = _Chains[chain]._RightLoop;
		loopIndex = _Chains[chain]._RightLoopIndex;
	}

	const CRetrievableSurface			&surf = _Surfaces[surface];
	const CRetrievableSurface::TLoop	&sLoop = surf._Loops[loop];
	return surf._Chains[sLoop[(loopIndex+sLoop.size()-1)%sLoop.size()]].Chain;
}

uint32			NLPACS::CLocalRetriever::getNextChain(uint32 chain, sint32 surface) const
{
	uint								loop;
	uint								loopIndex;

	if (_Chains[chain].getLeft() == surface)
	{
		loop = _Chains[chain]._LeftLoop;
		loopIndex = _Chains[chain]._LeftLoopIndex;
	}
	else
	{
		loop = _Chains[chain]._RightLoop;
		loopIndex = _Chains[chain]._RightLoopIndex;
	}

	const CRetrievableSurface			&surf = _Surfaces[surface];
	const CRetrievableSurface::TLoop	&sLoop = surf._Loops[loop];
	return surf._Chains[sLoop[(loopIndex+1)%sLoop.size()]].Chain;
}




void	NLPACS::CLocalRetriever::unify()
{
/*
	uint	i, j;

	for (i=0; i<_Chains.size(); ++i)
		_Chains[i].unify(_OrderedChains);

	for (i=0; i<_Tips.size(); ++i)
	{
		NLPACS::CLocalRetriever::CTip	&tip = _Tips[i];
		CVector2s ptip = tip.Point;

		for (j=0; j<tip.Chains.size(); ++j)
		{
			if (tip.Chains[j].Start)
			{
				if (_Chains[tip.Chains[j].Chain].getStartVector(_OrderedChains) != ptip)
					nlwarning("chain %d is not stuck to tip %d", tip.Chains[j].Chain, i);
				_Chains[tip.Chains[j].Chain].setStartVector(ptip, _OrderedChains);
			}
			else
			{
				if (_Chains[tip.Chains[j].Chain].getStopVector(_OrderedChains) != ptip)
					nlwarning("chain %d is not stuck to tip %d", tip.Chains[j].Chain, i);
				_Chains[tip.Chains[j].Chain].setStopVector(ptip, _OrderedChains);
			}
		}
	}

	_FullOrderedChains.resize(_OrderedChains.size());
	for (i=0; i<_OrderedChains.size(); ++i)
		_FullOrderedChains[i].unpack(_OrderedChains[i]);
*/
}







void	NLPACS::CLocalRetriever::dumpSurface(uint surf, const CVector &vect) const
{
	const CRetrievableSurface	&surface = _Surfaces[surf];

	nlinfo("dump surf %d", surf);
	nlinfo("%d chains, %d loops", surface._Chains.size(), surface._Loops.size());

	uint	i, j, k;

	for (i=0; i<surface._Chains.size(); ++i)
	{
		uint			chainId = surface._Chains[i].Chain;
		const CChain	&chain = _Chains[chainId];
		nlinfo("-- chain %d[%d]: %d sub left=%d right=%d start=%d stop=%d", i, chainId, chain.getSubChains().size(), chain.getLeft(), chain.getRight(), chain.getStartTip(), chain.getStopTip());

		for (j=0; j<chain.getSubChains().size(); ++j)
		{
			const COrderedChain3f	&ochain = _FullOrderedChains[chain.getSubChain(j)];
			const COrderedChain		&ochains = _OrderedChains[chain.getSubChain(j)];
			nlinfo("     subchain %d[%d]: fwd=%d parent=%d idx=%d", j, chain.getSubChain(j), ochain.isForward(), ochain.getParentId(), ochain.getIndexInParent());
			for (k=0; k<ochain.getVertices().size(); ++k)
				nlinfo("       v[%d]=(%.3f,%.3f,%.3f) (%d,%d)", k, ochain.getVertices()[k].x+vect.x, ochain.getVertices()[k].y+vect.y, ochain.getVertices()[k].z+vect.z, ochains.getVertices()[k].x, ochains.getVertices()[k].y);
		}

	}

	for (i=0; i<surface._Loops.size(); ++i)
	{
		const CRetrievableSurface::TLoop	&loop = surface._Loops[i];
		nlinfo("-- loop %d: %d chains length=%.2f", i, loop.size(), loop.Length);
		char	wbuffer[256];
		stringstream ss;
		sprintf(wbuffer, "    chains:");
		ss << wbuffer;
		for (j=0; j<loop.size(); ++j)
		{
			sprintf(wbuffer, " %d[%d]", loop[j], surface._Chains[loop[j]].Chain);
			ss << wbuffer;
		}
		nlinfo("%s", ss.str().c_str());
	}
}


float	NLPACS::CLocalRetriever::distanceToBorder(const ULocalPosition &pos) const
{
	if (!isLoaded())
		return 0.0f;

	const CRetrievableSurface	&surf = _Surfaces[pos.Surface];
	uint						i, j;
	float						minDist = 1.0e10f, dist;

	for (i=0; i<surf._Chains.size(); ++i)
	{
		const CChain	&chain = _Chains[surf._Chains[i].Chain];
		for (j=0; j<chain.getSubChains().size(); ++j)
		{
			dist = _OrderedChains[chain.getSubChain(j)].distance(pos.Estimation);
			if (dist < minDist)
			{
				minDist = dist;
			}
		}
	}

	return minDist;
}





sint32	NLPACS::CLocalRetriever::addSurface(uint8 normalq, uint8 orientationq,
											uint8 mat, uint8 charact, uint8 level,
											bool isUnderWater, float waterHeight,
											bool clusterHint,
											const CVector &center,
											const NLPACS::CSurfaceQuadTree &quad,
											sint8 quantHeight)
{
	// creates a new surface...
	sint32	newId = (sint32)_Surfaces.size();
	_Surfaces.resize(newId+1);
	CRetrievableSurface	&surf = _Surfaces.back();

	// ... and fills it
	surf._NormalQuanta = normalq;
	surf._OrientationQuanta = orientationq;
	surf._Material = mat;
	surf._Character = charact;
	surf._Level = level;
	surf._Quad = quad;
	surf._Center = center;
	surf._QuantHeight = quantHeight;

	// WARNING!! MODIFY THESE IF QUANTAS VALUES CHANGE !!
	surf._IsFloor = (surf._NormalQuanta <= 1);
	surf._IsCeiling = (surf._NormalQuanta >= 3);

	surf._Flags = 0;
	surf._Flags |= (surf._IsFloor) ? (1<<CRetrievableSurface::IsFloorBit) : 0;
	surf._Flags |= (surf._IsCeiling) ? (1<<CRetrievableSurface::IsCeilingBit) : 0;
	surf._Flags |= (!surf._IsFloor && !surf._IsCeiling) ? (1<<CRetrievableSurface::IsSlantBit) : 0;
	surf._Flags |= clusterHint ? (1<<CRetrievableSurface::ClusterHintBit) : 0;

	surf._Flags |= (isUnderWater) ? (1<<CRetrievableSurface::IsUnderWaterBit) : 0;
	surf._WaterHeight = waterHeight;

	surf._Flags |= ((0xffffffff<<(CRetrievableSurface::NormalQuantasStartBit)) & CRetrievableSurface::NormalQuantasBitMask);

	return newId;
}

sint32	NLPACS::CLocalRetriever::addChain(const vector<CVector> &verts,
										  sint32 left, sint32 right)
{
	vector<CVector>	vertices = verts;
	uint		i;

	if (vertices.size() < 2)
	{
		nlwarning("in NLPACS::CLocalRetriever::addChain()");
		nlwarning("The chain has less than 2 vertices");
		return -1;
	}

	// Remove doubled vertices due to CVector2s snapping
	vector<CVector2s>	converts;

	for (i=0; i<vertices.size(); ++i)
		converts.push_back(CVector2s(vertices[i]));

	vector<CVector2s>::iterator	next2s = converts.begin(), it2s, prev2s;
	prev2s = next2s; ++next2s;
	it2s = next2s; ++next2s;

	vector<CVector>::iterator	it3f = vertices.begin();
	CVector						prev3f = *it3f;
	++it3f;


	for (; it2s != converts.end() && next2s != converts.end(); )
	{
		// if the next point is equal to the previous
		if (*it2s == *prev2s || *it2s == *next2s)
		{
			// then remove the next point
			it2s = converts.erase(it2s);
			it3f = vertices.erase(it3f);

			prev2s = it2s;
			--prev2s;
			next2s = it2s;
			++next2s;
		}
		else
		{
			// else remember the next point, and step to the next...
			++prev2s;
			++it2s;
			++next2s;
			++it3f;
			prev3f = *it3f;
		}
	}

	if (vertices.size() < 2)
	{
		nlwarning("in NLPACS::CLocalRetriever::addChain()");
		nlwarning("The chain was snapped to a single point");
		return -1;
	}

	sint32		newId = (sint32)_Chains.size();
	_Chains.resize(newId+1);
	CChain		&chain = _Chains.back();

	if (left>(sint)_Surfaces.size())
		nlerror ("left surface id MUST be id<%d (id=%d)", _Surfaces.size(), left);
	if (right>(sint)_Surfaces.size())
		nlerror ("right surface id MUST be id<%d (id=%d)", _Surfaces.size(), right);

	// checks if we can build the chain.
	if (newId > 65535)
		nlerror("in NLPACS::CLocalRetriever::addChain(): reached the maximum number of chains");

	CRetrievableSurface	*leftSurface = (left>=0) ? &(_Surfaces[left]) : NULL;
	CRetrievableSurface	*rightSurface = (right>=0) ? &(_Surfaces[right]) : NULL;

	// adds the chain and the link to the surface links vector.
	if (leftSurface != NULL)
		leftSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, right));
	if (rightSurface != NULL)
		rightSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, left));

	chain._StartTip = 0xffff;
	chain._StopTip = 0xffff;

	// make the chain and its subchains.
	vector<uint>	empty;
	chain.make(vertices, left, right, _OrderedChains, (uint16)newId, _FullOrderedChains, empty);

	return newId;
}




void	NLPACS::CLocalRetriever::computeLoopsAndTips()
{
	// for each surface,
	// examine each chain tip to match another tip inside the surface tips
	// if there is no matching tip, then creates a new one

	uint	i, j;

	for (i=0; i<_Surfaces.size(); ++i)
	{
		CRetrievableSurface	&surface = _Surfaces[i];

		vector<bool>		chainFlags;
		chainFlags.resize(surface._Chains.size());
		for (j=0; j<chainFlags.size(); ++j)
			chainFlags[j] = false;

		uint	totalAdded = 0;

		for(;;)
		{
			for (j=0; j<chainFlags.size() && chainFlags[j]; ++j)
				;

			if (j == chainFlags.size())
				break;

			uint32						loopId = (uint32)surface._Loops.size();
			surface._Loops.push_back(CRetrievableSurface::TLoop());
			CRetrievableSurface::TLoop	&loop = surface._Loops.back();

			CVector	loopStart = getStartVector(surface._Chains[j].Chain, i);
			CVector	currentEnd = getStopVector(surface._Chains[j].Chain, i);
			_Chains[surface._Chains[j].Chain].setLoopIndexes(i, loopId, (uint)loop.size());
			loop.push_back(uint16(j));
			chainFlags[j] = true;

			float	loopCloseDistance;

			for(;;)
			{
//				loopCloseDistance = hybrid2dNorm(loopStart-currentEnd);
				loopCloseDistance = (loopStart-currentEnd).norm();

				// choose the best matching start vector
				sint	bestChain = -1;
				float	best = 1.0e10f;
				CVector	thisStart;
				for (j=0; j<chainFlags.size(); ++j)
				{
					if (chainFlags[j])
						continue;
					thisStart = getStartVector(surface._Chains[j].Chain, i);
//					float	d = hybrid2dNorm(thisStart-currentEnd);
					float	d = (thisStart-currentEnd).norm();
					if (d < best)
					{
						best = d;
						bestChain = j;
					}
				}

				if ((bestChain == -1 || best > 4.0e-2f)&& loopCloseDistance > 4.0e-2f)
				{
					nlwarning("in NLPACS::CLocalRetriever::computeTips()");

					dumpSurface(i);

					for (j=0; j<surface._Chains.size(); ++j)
					{
						CVector	start = getStartVector(surface._Chains[j].Chain, i);
						CVector	end = getStopVector(surface._Chains[j].Chain, i);
						nlinfo("surf=%d chain=%d", i, surface._Chains[j].Chain);
						nlinfo("start=(%f,%f,%f)", start.x, start.y, start.z);
						nlinfo("end=(%f,%f,%f)", end.x, end.y, end.z);
					}

					nlwarning("bestChain=%d best=%f", bestChain, best);
					nlwarning("loopCloseDistance=%f", loopCloseDistance);
					nlerror("Couldn't close loop on surface=%d", i);
				}
				else if ((best > 1.0e0f && loopCloseDistance < 3.0e-2f) ||
						 loopCloseDistance < 1.0e-3f)
				{
					break;
				}

				currentEnd = getStopVector(surface._Chains[bestChain].Chain, i);
				_Chains[surface._Chains[bestChain].Chain].setLoopIndexes(i, loopId, (uint)loop.size());
				loop.push_back(uint16(bestChain));
				chainFlags[bestChain] = true;
				++totalAdded;
			}
		}
	}
/*
	dumpSurface(9);
	dumpSurface(10);

	for (i=0; i<_Chains.size(); ++i)
	{
		if (i == 127)
			nlinfo("");

		uint	whichTip;
		// for both tips (start and stop)
		for (whichTip=0; whichTip<=1; ++whichTip)
		{
			// get the tip id
			uint	thisTip = (whichTip) ? _Chains[i].getStopTip() : _Chains[i].getStartTip();

			if (thisTip != 0xffff && thisTip >= _Tips.size())
			{
				nlwarning("in NLPACS::CLocalRetriever::computeLoopsAndTips()");
				nlerror("checked a tip that doesn't exist on chain %d (tipId=%d)", i, thisTip);
			}

			// if it is unaffected yet creates an new tip and affect it to the common chains
			if (thisTip == 0xffff)
			{
				uint	turn;
				uint	tipId = _Tips.size();

				if (tipId == 62)
					nlinfo("");

				_Tips.resize(tipId+1);
				CTip	&tip = _Tips[tipId];
				tip.Point = (whichTip) ? getStopVector(i) : getStartVector(i);

				for (turn=0; turn<=1; ++turn)
				{
					uint	chain = i;

					//
					if (whichTip)
						_Chains[chain]._StopTip = tipId;
					else
						_Chains[chain]._StartTip = tipId;

					sint32	surf = (!turn && !whichTip || turn && whichTip) ? _Chains[chain].getLeft() : _Chains[chain].getRight();

					while (surf >= 0)
					{

						CChain	&nextChain = (turn) ? _Chains[chain = getNextChain(chain, surf)] : _Chains[chain = getPreviousChain(chain, surf)];
						bool	isForward = (nextChain.getLeft() == surf);	// tells if the left surf is the current surf
						bool	selectTip = isForward && !turn || !isForward && turn;
						uint16	&tipRef = selectTip ? nextChain._StopTip : nextChain._StartTip;
						surf = (isForward) ? nextChain.getRight() : nextChain.getLeft();

						if (tipRef != 0xffff && tipRef != tipId)
						{
							nlwarning("in NLPACS::CLocalRetriever::computeLoopsAndTips()");
							nlerror("Trying to setup a already created tip (tipId=%d, previous=%d)", tipId, tipRef);
						}
						else if (tipRef != 0xffff)
						{
							break;
						}

						tipRef = tipId;
					}
				}
			}
		}
	}

	for (i=0; i<_Chains.size(); ++i)
	{
		uint	startTip = _Chains[i].getStartTip(),
				stopTip = _Chains[i].getStopTip();


//		if (_Chains[i].getEdge() >= 0 && startTip == stopTip)
//		{
//			nlwarning("NLPACS::CLocalRetriever::computeLoopsAndTips(): chain %d on edge %d has same StartTip and StopTip", i, _Chains[i].getEdge(), startTip, stopTip);
//		}


		_Tips[startTip].Chains.push_back(CTip::CChainTip(i, true));
		_Tips[stopTip].Chains.push_back(CTip::CChainTip(i, false));
	}
*/
	for (i=0; i<_Surfaces.size(); ++i)
	{
		for (j=0; j<_Surfaces[i]._Loops.size(); ++j)
		{
			_Surfaces[i]._Loops[j].Length = 0.0f;
			uint	k;

			for (k=0; k<_Surfaces[i]._Loops[j].size(); ++k)
				_Surfaces[i]._Loops[j].Length += _Chains[_Surfaces[i]._Chains[_Surfaces[i]._Loops[j][k]].Chain].getLength();
		}
	}
}


//
void	NLPACS::CLocalRetriever::buildSurfacePolygons(uint32 surface, list<CPolygon> &polygons) const
{
	const CRetrievableSurface	&surf = _Surfaces[surface];

	uint	i, j, k, l;

	for (i=0; i<surf._Loops.size(); ++i)
	{
		polygons.push_back(CPolygon());
		CPolygon	&poly = polygons.back();

		for (j=0; j<surf._Loops[i].size(); ++j)
		{
			const CChain	&chain = _Chains[surf._Loops[i][j]];
			bool			chainforward = ((uint32)chain._Left == surface);

			if (chainforward)
			{
				for (k=0; k<chain._SubChains.size(); ++k)
				{
					const COrderedChain		&ochain = _OrderedChains[chain._SubChains[k]];
					bool					ochainforward = ochain.isForward();

					if (ochainforward)
					{
						for (l=0; l<ochain.getVertices().size()-1; ++l)
							poly.Vertices.push_back(ochain[l].unpack3f());
					}
					else
					{
						for (l=(uint)ochain.getVertices().size()-1; l>0; --l)
							poly.Vertices.push_back(ochain[l].unpack3f());
					}
				}
			}
			else
			{
				for (k=(uint)chain._SubChains.size(); (sint)k>0; --k)
				{
					const COrderedChain		&ochain = _OrderedChains[chain._SubChains[k]];
					bool					ochainforward = ochain.isForward();

					if (ochainforward)
					{
						for (l=(uint)ochain.getVertices().size()-1; (sint)l>0; --l)
							poly.Vertices.push_back(ochain[l].unpack3f());
					}
					else
					{
						for (l=0; l<ochain.getVertices().size()-1; ++l)
							poly.Vertices.push_back(ochain[l].unpack3f());
					}
				}
			}
		}
	}
}

//
void	NLPACS::CLocalRetriever::build3dSurfacePolygons(uint32 surface, list<CPolygon> &polygons) const
{
	const CRetrievableSurface	&surf = _Surfaces[surface];

	uint	i, j, k, l;

	for (i=0; i<surf._Loops.size(); ++i)
	{
		polygons.push_back(CPolygon());
		CPolygon	&poly = polygons.back();

		for (j=0; j<surf._Loops[i].size(); ++j)
		{
			const CRetrievableSurface::TLoop	&loop = surf._Loops[i];
			const CChain						&chain = _Chains[surf._Chains[loop[j]].Chain];
			bool								chainforward = ((uint32)chain._Left == surface);

			if (chainforward)
			{
				for (k=0; k<chain._SubChains.size(); ++k)
				{
					const COrderedChain3f	&ochain = _FullOrderedChains[chain._SubChains[k]];
					bool					ochainforward = ochain.isForward();

					if (ochainforward)
					{
						for (l=0; l<ochain.getVertices().size()-1; ++l)
							poly.Vertices.push_back(ochain[l]);
					}
					else
					{
						for (l=(uint)ochain.getVertices().size()-1; l>0; --l)
							poly.Vertices.push_back(ochain[l]);
					}
				}
			}
			else
			{
				for (k=(uint)chain._SubChains.size()-1; (sint)k>=0; --k)
				{
					const COrderedChain3f	&ochain = _FullOrderedChains[chain._SubChains[k]];
					bool					ochainforward = ochain.isForward();

					if (ochainforward)
					{
						for (l=(uint)ochain.getVertices().size()-1; (sint)l>0; --l)
							poly.Vertices.push_back(ochain[l]);
					}
					else
					{
						for (l=0; l<ochain.getVertices().size()-1; ++l)
							poly.Vertices.push_back(ochain[l]);
					}
				}
			}
		}
	}
}


// not implemented...
void	NLPACS::CLocalRetriever::sortTips()
{
}




void	NLPACS::CLocalRetriever::findBorderChains()
{
	uint	chain;

	// for each chain, if it belongs to an edge of the
	// local retriever, then adds it to the _BorderChains.
	for (chain=0; chain<_Chains.size(); ++chain)
		if (_Chains[chain].isBorderChain())
		{
			sint32	index = (sint32)_BorderChains.size();
			_BorderChains.push_back(uint16(chain));
			_Chains[chain].setBorderChainIndex(index);
		}
}

void	NLPACS::CLocalRetriever::updateChainIds()
{
	uint	surf, link;

	for (surf=0; surf<_Surfaces.size(); ++surf)
	{
		CRetrievableSurface	&surface = _Surfaces[surf];

		for (link=0; link<surface._Chains.size(); ++link)
		{
			sint32	chain = surface._Chains[link].Chain;

			if (_Chains[chain]._Left == (sint32)surf)
				surface._Chains[link].Surface = _Chains[chain]._Right;
			else if (_Chains[chain]._Right == (sint32)surf)
				surface._Chains[link].Surface = _Chains[chain]._Left;
			else
			{
				nlwarning("in NLPACS::CLocalRetriever::updateEdgesOnSurfaces()");
				nlerror("Can't find back point to surface %d on chain %d", surf, chain);
			}
		}
	}
}

void	NLPACS::CLocalRetriever::computeTopologies()
{
	//nlinfo("compute topologies");

	// Find topologies out...
	uint		character;
	for (character=0; character<NumCreatureModels; ++character)
	{
		// for each type of creature, flood fill surfaces...
		sint32	surface;
		uint	topology = 0;

		for (surface=0; surface<(sint)_Surfaces.size(); ++surface)
		{
			if (_Surfaces[surface]._Topologies[character] == -1 &&
				_Surfaces[surface]._Character == character)
			{
				vector<sint32>	surfacesStack;
				surfacesStack.push_back(surface);

				while (!surfacesStack.empty())
				{
					CRetrievableSurface	&current = _Surfaces[surfacesStack.back()];
					surfacesStack.pop_back();
					current._Topologies[character] = topology;

					uint	i;
					for (i=0; i<current._Chains.size(); ++i)
					{
						CChain	&chain = _Chains[current._Chains[i].Chain];
						sint32	link = (chain.getLeft() == surface) ? chain.getRight() : chain.getLeft();
						if (link>=0 && link<(sint)_Surfaces.size() &&
							_Surfaces[link]._Topologies[character] == -1 &&
							_Surfaces[link]._Character >= character)
						{
							surfacesStack.push_back(link);
							_Surfaces[link]._Topologies[character] = topology;
						}
					}
				}

				++topology;
			}
		}

		_Topologies[character].resize(topology);
		//nlinfo("generated %d topologies for character %d", topology, character);
	}

	uint		surface;
	for (surface=0; surface<_Surfaces.size(); ++surface)
	{
		CRetrievableSurface	&current = _Surfaces[surface];

		for (character=0; character<NumCreatureModels; ++character)
			if (current._Topologies[character] >= 0)
				_Topologies[character][current._Topologies[character]].push_back(surface);
	}
}

void	NLPACS::CLocalRetriever::translate(const NLMISC::CVector &translation)
{
	uint	i;
	for (i=0; i<_OrderedChains.size(); ++i)
		_OrderedChains[i].translate(translation);
	for (i=0; i<_Surfaces.size(); ++i)
		_Surfaces[i].translate(translation);
/*
	for (i=0; i<_Tips.size(); ++i)
		_Tips[i].translate(translation);
*/
}

void	NLPACS::CLocalRetriever::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version (with collision info).
	Version 1:
		- interior vertices and faces, for interior ground snapping
	Version 2:
		- face grid added.
	Version 3:
		- identifier added.
	Version 4:
		- topologies no more in stream (obsolete)
	*/
	sint	ver= f.serialVersion(4);

	if (ver < 4)
		throw EOlderStream();

	uint	i;
	f.serialCont(_Chains);
	f.serialCont(_OrderedChains);
	f.serialCont(_FullOrderedChains);
	f.serialCont(_Surfaces);
	f.serialCont(__Tips);
	f.serialCont(_BorderChains);
	if (ver < 4)
	{
		for (i=0; i<NumMaxCreatureModels; ++i)
			f.serialCont(_Topologies[i]);
	}
	f.serial(_ChainQuad);
	f.serial(_BBox);
	f.serialEnum(_Type);
	f.serial(_ExteriorMesh);

	// a fix for old versions (with wrong _Type value)
	if (_Type != CLocalRetriever::Interior)	_Type = CLocalRetriever::Landscape;

	if (ver >= 1)
	{
		f.serialCont(_InteriorVertices);
		f.serialCont(_InteriorFaces);

	}
	if (ver >= 2)
	{
		f.serial(_FaceGrid);
	}
	if (ver >= 3)
	{
		f.serial(_Id);
	}

	_Loaded = true;
	LoadCheckFlag = false;
}








bool	NLPACS::CLocalRetriever::insurePosition(NLPACS::ULocalPosition &local) const
{
	if (!_Loaded)
		return false;

	if (local.Surface < 0 || local.Surface >= (sint)_Surfaces.size())
	{
		nlwarning("PACS: can't insure position to inexistant surface %d", local.Surface);
		return false;
	}

	// the surface
	const NLPACS::CRetrievableSurface	&surface = _Surfaces[local.Surface];

	uint		i, j, k;
	CVector2f	M = CVector2f(local.Estimation);
	bool		moved = false;

	// for each chain and each subchain of the surface,
	// check if point is located on the good side of the border (and far enough to avoid accuracy issues)
	for (i=0; i<surface.getChains().size(); ++i)
	{
		uint					ichain = surface.getChain(i).Chain;
		const NLPACS::CChain	&chain = _Chains[ichain];

		for (j=0; j<chain.getSubChains().size(); ++j)
		{
			uint						iochain = chain.getSubChain(j);
			const NLPACS::COrderedChain	&ochain = _OrderedChains[iochain];

			uint						isAtLeft = ((chain.getLeft() == local.Surface) ? 1 : 0);
			uint						isForward = (ochain.isForward() ? 1 : 0);
			bool						shouldBeUpper = !((isAtLeft ^ isForward) != 0);	// shouldBeAtLeft for vertical segment

			for (k=0; (sint)k<(sint)(ochain.getVertices().size()-1); ++k)
			{
				CVector2f	A = ochain[k].unpack();
				CVector2f	B = ochain[k+1].unpack();
				CVector2f	AB = B-A;

				float		lambda = ((M-A)*AB)/AB.sqrnorm();

				if (lambda<0.0f || lambda>1.0f)
					continue;

				CVector2f	n = (shouldBeUpper ? CVector2f(-AB.y, AB.x) : CVector2f(AB.y, -AB.x)).normed();
				float		d = (M-A)*n;

				// if point is too close of the border or on the wrong side
				// move it far enough
				if (d < InsurePositionThreshold && d > -InsurePositionThreshold)
				{
					M += (InsurePositionThreshold*1.1f-d)*n;
					moved = true;
				}
			}
		}
	}

	NLPACS::CRetrieverInstance::snapVector(M);

	local.Estimation.x = M.x;
	local.Estimation.y = M.y;

	{
		float	fx1024 = local.Estimation.x * 1024.0f;
		float	fy1024 = local.Estimation.x * 1024.0f;
		sint32	ix1024 = (sint32)floor(fx1024);
		sint32	iy1024 = (sint32)floor(fy1024);

		nlassert ((float)ix1024 == fx1024);
		nlassert ((float)iy1024 == fy1024);
	}

	return moved;
}


bool	NLPACS::CLocalRetriever::testPosition(NLPACS::ULocalPosition &local, CCollisionSurfaceTemp &cst) const
{
	if (!_Loaded)
		return false;

	if (local.Surface < 0 || local.Surface >= (sint)_Surfaces.size())
	{
		nlwarning("PACS: can't test inexistant surface %d", local.Surface);
		return false;
	}

	if (fabs(local.Estimation.x) >= 256.0 || fabs(local.Estimation.y) >= 256.0)
		return false;

	retrievePosition(local.Estimation, cst);

	bool	result = (cst.SurfaceLUT[local.Surface].Counter == 2 || cst.SurfaceLUT[local.Surface].OnVerticalEdge);

	uint	i;
	for (i=0; i<cst.PossibleSurfaces.size(); ++i)
				cst.SurfaceLUT[cst.PossibleSurfaces[i]].reset();

	return result;
}


void	NLPACS::CLocalRetriever::retrievePosition(CVector estimated, CCollisionSurfaceTemp &cst) const
{
	if (!_Loaded)
		return;

	CAABBox			box;
	const double	BorderThreshold = 2.0e-2f;
	box.setMinMax(CVector(estimated.x-(float)BorderThreshold, _BBox.getMin().y, 0.0f), CVector(estimated.x+(float)BorderThreshold, _BBox.getMax().y, 0.0f));
	uint			numEdges = _ChainQuad.selectEdges(box, cst);

	uint			ochain, i;
	CVector2s		estim = CVector2s(estimated);

	cst.PossibleSurfaces.clear();

	// WARNING!!
	// cst.SurfaceLUT is assumed to be 0 filled !!

	//nldebug("estim=(%d,%d)", estim.x, estim.y);

	// for each ordered chain, checks if the estimated position is between the min and max.
	for (i=0; i<numEdges; ++i)
	{
		ochain = cst.EdgeChainEntries[i].OChainId;

		const COrderedChain	&sub = _OrderedChains[ochain];
		const CVector2s	&min = sub.getMin(),
						&max = sub.getMax();

		// checks the position against the min and max of the chain
		if (estim.x < min.x || estim.x > max.x)
			continue;

		bool	isUpper;
		bool	isOnBorder = false;

		sint32	left = _Chains[sub.getParentId()].getLeft(),
				right = _Chains[sub.getParentId()].getRight();

		if (estim.y < min.y)
		{
			if (estim.x == max.x)
				continue;
			isUpper = false;
//			nlinfo("Box: min(%d,%d) max(%d,%d) forward=%d left=%d right=%d upper=false", min.x, min.y, max.x, max.y, sub.isForward(), left, right);
		}
		else if (estim.y > max.y)
		{
			if (estim.x == max.x)
				continue;
			isUpper = true;
//			nlinfo("Box: min(%d,%d) max(%d,%d) forward=%d left=%d right=%d upper=true", min.x, min.y, max.x, max.y, sub.isForward(), left, right);
		}
		else
		{
			const vector<CVector2s>	&vertices = sub.getVertices();
			uint					start = 0, stop = (uint)vertices.size()-1;

			// then finds the smallest segment of the chain that includes the estimated position.
			while (stop-start > 1)
			{
				uint	mid = (start+stop)/2;

				if (vertices[mid].x > estim.x)
					stop = mid;
				else
					start = mid;
			}

			// if a vertical edge
			if (vertices[start].x == vertices[stop].x)
			{
				// look for maximal bounds
				while (start > 0 && vertices[start].x == vertices[start-1].x)
					--start;

				while (stop < vertices.size()-1 && vertices[stop].x == vertices[stop+1].x)
					++stop;

				// if upper or lower the bounds, do nothing
				if ((estim.y > vertices[start].y && estim.y > vertices[stop].y) ||
					(estim.y < vertices[start].y && estim.y < vertices[stop].y))
					continue;

				isOnBorder = true;
				if (left >= 0)
				{
					cst.SurfaceLUT[left].FoundCloseEdge = true;
					cst.SurfaceLUT[left].OnVerticalEdge = true;
				}
				if (right >= 0)
				{
					cst.SurfaceLUT[right].FoundCloseEdge = true;
					cst.SurfaceLUT[right].OnVerticalEdge = true;
				}
//				nlinfo("Edge: start(%d,%d) stop(%d,%d) forward=%d left=%d right=%d border=true", vertices[start].x, vertices[start].y, vertices[stop].x, vertices[stop].y, sub.isForward(), left, right);
				continue;
			}
			else if (vertices[stop].x == estim.x)
			{
				// if (yes)
				continue;
			}

			// and then checks if the estimated position is up or down the chain.

			// first trivial case (up both tips)
			if (estim.y > vertices[start].y && estim.y > vertices[stop].y)
			{
				isUpper = true;
//				nlinfo("Edge: start(%d,%d) stop(%d,%d) forward=%d left=%d right=%d upper=true", vertices[start].x, vertices[start].y, vertices[stop].x, vertices[stop].y, sub.isForward(), left, right);
			}
			// second trivial case (down both tips)
			else if (estim.y < vertices[start].y && estim.y < vertices[stop].y)
			{
				isUpper = false;
//				nlinfo("Edge: start(%d,%d) stop(%d,%d) forward=%d left=%d right=%d upper=false", vertices[start].x, vertices[start].y, vertices[stop].x, vertices[stop].y, sub.isForward(), left, right);
			}
			// full test...
			else
			{
				const CVector2s	&vstart = vertices[start],
								&vstop = vertices[stop];

				sint16			intersect = vstart.y + (vstop.y-vstart.y)*(estim.x-vstart.x)/(vstop.x-vstart.x);

				isUpper = estim.y > intersect;
				//isOnBorder = (fabs(estim.y - intersect)<BorderThreshold*Vector2sAccuracy);
				isOnBorder = (fabs((double)(estim.y - intersect))<(double)(BorderThreshold*Vector2sAccuracy));
//				nlinfo("Edge: start(%d,%d) stop(%d,%d) forward=%d left=%d right=%d upper=%s border=%s", vertices[start].x, vertices[start].y, vertices[stop].x, vertices[stop].y, sub.isForward(), left, right, isUpper ? "true":"false", isOnBorder ? "true":"false");
			}
		}

		if (isOnBorder)
		{
			cst.incSurface(left);
			cst.incSurface(right);
			if (left >= 0)	cst.SurfaceLUT[left].FoundCloseEdge = true;
			if (right >= 0)	cst.SurfaceLUT[right].FoundCloseEdge = true;
			continue;
		}

		// Depending on the chain is forward, up the position, increase/decrease the surface table...
		if (sub.isForward())
		{
			if (isUpper)
			{
				cst.incSurface(left);
				cst.decSurface(right);
			}
			else
			{
				cst.decSurface(left);
				cst.incSurface(right);
			}
		}
		else
		{
			if (isUpper)
			{
				cst.decSurface(left);
				cst.incSurface(right);
			}
			else
			{
				cst.incSurface(left);
				cst.decSurface(right);
			}
		}
	}
}


void	NLPACS::CLocalRetriever::retrieveAccuratePosition(CVector2s estim, CCollisionSurfaceTemp &cst, bool &onBorder) const
{
	if (!_Loaded)
		return;

	CAABBox			box;
	CVector			estimated = estim.unpack3f();
	const double	BorderThreshold = 2.0e-2f;
	box.setMinMax(CVector(estimated.x-(float)BorderThreshold, _BBox.getMin().y, 0.0f),
				  CVector(estimated.x+(float)BorderThreshold, _BBox.getMax().y, 0.0f));
	uint			numEdges = _ChainQuad.selectEdges(box, cst);

	uint			ochain, i;

	onBorder = false;

	cst.PossibleSurfaces.clear();

	// WARNING!!
	// cst.SurfaceLUT is assumed to be 0 filled !!

	//nldebug("estim=(%d,%d)", estim.x, estim.y);

	// for each ordered chain, checks if the estimated position is between the min and max.
	for (i=0; i<numEdges; ++i)
	{
		ochain = cst.EdgeChainEntries[i].OChainId;

		const COrderedChain	&sub = _OrderedChains[ochain];
		const CVector2s	&min = sub.getMin(),
						&max = sub.getMax();

		// checks the position against the min and max of the chain
		if (estim.x < min.x || estim.x > max.x)
			continue;

		bool	isUpper;
		//bool	isOnBorder = false;

		sint32	left = _Chains[sub.getParentId()].getLeft(),
				right = _Chains[sub.getParentId()].getRight();

		if (estim.y < min.y)
		{
			if (estim.x == max.x)
				continue;
			isUpper = false;
		}
		else if (estim.y > max.y)
		{
			if (estim.x == max.x)
				continue;
			isUpper = true;
		}
		else
		{
			const vector<CVector2s>	&vertices = sub.getVertices();
			uint					start = 0, stop = (uint)vertices.size()-1;

			// then finds the smallest segment of the chain that includes the estimated position.
			while (stop-start > 1)
			{
				uint	mid = (start+stop)/2;

				if (vertices[mid].x > estim.x)
					stop = mid;
				else
					start = mid;
			}

			// if a vertical edge
			if (vertices[start].x == vertices[stop].x)
			{
				// look for maximal bounds
				while (start > 0 && vertices[start].x == vertices[start-1].x)
					--start;

				while (stop < vertices.size()-1 && vertices[stop].x == vertices[stop+1].x)
					++stop;

				// if upper or lower the bounds, do nothing
				if ((estim.y > vertices[start].y && estim.y > vertices[stop].y) ||
					(estim.y < vertices[start].y && estim.y < vertices[stop].y))
					continue;

				onBorder = true;
				continue;
			}
			else if (vertices[stop].x == estim.x)
			{
				// if (yes)
				continue;
			}

			// and then checks if the estimated position is up or down the chain.

			// first trivial case (up both tips)
			if (estim.y > vertices[start].y && estim.y > vertices[stop].y)
			{
				isUpper = true;
			}
			// second trivial case (down both tips)
			else if (estim.y < vertices[start].y && estim.y < vertices[stop].y)
			{
				isUpper = false;
			}
			// full test...
			else
			{
				const CVector2s	&vstart = vertices[start],
								&vstop = vertices[stop];

				// this test is somewhat more accurate
				// no division performed
				sint32	det = (estim.y-vstart.y)*(vstop.x-vstart.x) - (estim.x-vstart.x)*(vstop.y-vstart.y);

				isUpper = (det > 0);

				if (det == 0)
					onBorder = true;
			}
		}

		// Depending on the chain is forward, up the position, increase/decrease the surface table...
		if (sub.isForward())
		{
			if (isUpper)
			{
				cst.incSurface(left);
				cst.decSurface(right);
			}
			else
			{
				cst.decSurface(left);
				cst.incSurface(right);
			}
		}
		else
		{
			if (isUpper)
			{
				cst.decSurface(left);
				cst.incSurface(right);
			}
			else
			{
				cst.incSurface(left);
				cst.decSurface(right);
			}
		}
	}
}



void	NLPACS::CLocalRetriever::initFaceGrid()
{
	CFaceGrid::CFaceGridBuild	fgb;
	fgb.init(64, 4.0f);

	uint	i;
	for (i=0; i<_InteriorFaces.size(); ++i)
	{
		CAABBox			box;
		CInteriorFace	&f = _InteriorFaces[i];
		box.setCenter(_InteriorVertices[f.Verts[0]]);
		box.extend(_InteriorVertices[f.Verts[1]]);
		box.extend(_InteriorVertices[f.Verts[2]]);

		fgb.insert(box.getMin(), box.getMax(), i);
	}

	_FaceGrid.create(fgb);
}

void	NLPACS::CLocalRetriever::snapToInteriorGround(NLPACS::ULocalPosition &position, bool &snapped) const
{
	if (!_Loaded)
		return;

	// first preselect faces around the (x, y) position (CQuadGrid ?)
	vector<uint32>	selection;
	_FaceGrid.select(position.Estimation, selection);

	// from the preselect faces, look for the only face that belongs to the surface
	// and that contains the position
	CVector						pos = position.Estimation;
	CVector						posh = pos+CVector(0.0f, 0.0f, 1.0f);
	CVector2f					pos2d = position.Estimation;
	float						bestDist = 1.0e10f;
	CVector						best(0.0f, 0.0f, 0.0f);
	vector<uint32>::iterator	it;
	snapped = false;
	for (it=selection.begin(); it!=selection.end(); ++it)
	{
		const CInteriorFace	&f = _InteriorFaces[*it];
		if (f.Surface == (uint32)position.Surface)
		{
			CVector	v[3];
			v[0] = _InteriorVertices[f.Verts[0]];
			v[1] = _InteriorVertices[f.Verts[1]];
			v[2] = _InteriorVertices[f.Verts[2]];

			CVector2f	n;
			float		c;			// 2D cartesian coefficients of line in plane X/Y.
			// Line p0-p1.
			n = CVector2f(-(v[1].y-v[0].y), (v[1].x-v[0].x)).normed();
			c = -(v[0].x*n.x + v[0].y*n.y);
			if (n*pos2d + c < -1.0f/Vector2sAccuracy)	continue;
			// Line p1-p2.
			n = CVector2f(-(v[2].y-v[1].y), (v[2].x-v[1].x)).normed();
			c = -(v[1].x*n.x + v[1].y*n.y);
			if (n*pos2d + c < -1.0f/Vector2sAccuracy)	continue;
			//  Line p2-p0.
			n = CVector2f(-(v[0].y-v[2].y), (v[0].x-v[2].x)).normed();
			c = -(v[2].x*n.x + v[2].y*n.y);
			if (n*pos2d + c < -1.0f/Vector2sAccuracy)	continue;

			CPlane	p;
			p.make(v[0], v[1], v[2]);

			CVector i = p.intersect(pos, posh);

			float	d = (float)fabs(pos.z-i.z);

			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}
	}

	// and computes the real position on this face
	if (bestDist < 400.0f)
	{
		snapped = true;
		position.Estimation = best;
	}
}

// ***************************************************************************
float	NLPACS::CLocalRetriever::getHeight(const NLPACS::ULocalPosition &position) const
{
	if (!_Loaded)
		return 0.0f;

	if (_Type == Interior)
	{
		// first preselect faces around the (x, y) position (CQuadGrid ?)
		vector<uint32>	selection;
		_FaceGrid.select(position.Estimation, selection);

		// from the preselect faces, look for the only face that belongs to the surface
		// and that contains the position
		CVector	pos = position.Estimation;
		CVector	posh = pos+CVector(0.0f, 0.0f, 1.0f);
		float	bestDist = 1.0e10f;
		CVector	best(0.0f, 0.0f, 0.0f);
		vector<uint32>::iterator	it;
		for (it=selection.begin(); it!=selection.end(); ++it)
		{
			const CInteriorFace	&f = _InteriorFaces[*it];
			if (f.Surface == (uint32)position.Surface)
			{
				CVector	v[3];
				v[0] = _InteriorVertices[f.Verts[0]];
				v[1] = _InteriorVertices[f.Verts[1]];
				v[2] = _InteriorVertices[f.Verts[2]];

				float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
				// Line p0-p1.
				a = -(v[1].y-v[0].y);
				b =  (v[1].x-v[0].x);
				c = -(v[0].x*a + v[0].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;
				// Line p1-p2.
				a = -(v[2].y-v[1].y);
				b =  (v[2].x-v[1].x);
				c = -(v[1].x*a + v[1].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;
				//  Line p2-p0.
				a = -(v[0].y-v[2].y);
				b =  (v[0].x-v[2].x);
				c = -(v[2].x*a + v[2].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;

				CPlane	p;
				p.make(v[0], v[1], v[2]);

				CVector i = p.intersect(pos, posh);

				float	d = (float)fabs(pos.z-i.z);

				if (d < bestDist)
				{
					bestDist = d;
					best = i;
				}
			}
		}

		// and computes the real position on this face
		return (bestDist < 200.0f) ? best.z : position.Estimation.z;
	}
	else
	{
		if (_Surfaces[position.Surface].getQuadTree().getRoot() != NULL)
		{
			// find quad leaf.
			const CQuadLeaf	*leaf = _Surfaces[position.Surface].getQuadTree().getLeaf(position.Estimation);

			// if there is no acceptable leaf, just give up
			if (leaf == NULL)
			{
				//nlinfo("COL: quadtree: don't find the quadLeaf!");
				return position.Estimation.z;
			}
			else
			{
				// else return mean height.
				float	meanHeight = (leaf->getMinHeight()+leaf->getMaxHeight())*0.5f;
				return meanHeight;
			}
		}
		else if (_Surfaces[position.Surface].isUnderWater())
		{
			return _Surfaces[position.Surface].getWaterHeight();
		}
		else
		{
			sint8	qh = _Surfaces[position.Surface].getQuantHeight();
			return qh*2.0f + 1.0f;
		}
	}
}


// ***************************************************************************
float	NLPACS::CLocalRetriever::getInteriorHeightAround(const ULocalPosition &position, float outsideTolerance) const
{
	if (!_Loaded)
		return 0.0f;

	if (_Type == Interior)
	{
		// first preselect faces around the (x, y) position (CQuadGrid ?)
		vector<uint32>	selection;
		_FaceGrid.select(position.Estimation, selection);

		// from the preselect faces, look for the only face that belongs to the surface
		// and that contains the position
		CVector	pos = position.Estimation;
		CVector	posh = pos+CVector(0.0f, 0.0f, 1.0f);
		float	bestDist = 1.0e10f;
		CVector	best(0.0f, 0.0f, 0.0f);
		vector<uint32>::iterator	it;
		for (it=selection.begin(); it!=selection.end(); ++it)
		{
			const CInteriorFace	&f = _InteriorFaces[*it];
			if (f.Surface == (uint32)position.Surface)
			{
				CVector	v[3];
				v[0] = _InteriorVertices[f.Verts[0]];
				v[1] = _InteriorVertices[f.Verts[1]];
				v[2] = _InteriorVertices[f.Verts[2]];

				// Test if out of this triangle (+ tolerance)
				float		a,b;		// 2D cartesian coefficients of line in plane X/Y.
				float		len;
				// Line p0-p1.
				a = -(v[1].y-v[0].y);
				b =  (v[1].x-v[0].x);
				len= sqrtf(a*a+b*b);		// norm of the normal vector
				if (a*(pos.x-v[0].x) + b*(pos.y-v[0].y) < -len*outsideTolerance)	continue;
				// Line p1-p2.
				a = -(v[2].y-v[1].y);
				b =  (v[2].x-v[1].x);
				len= sqrtf(a*a+b*b);		// norm of the normal vector
				if (a*(pos.x-v[1].x) + b*(pos.y-v[1].y) < -len*outsideTolerance)	continue;
				//  Line p2-p0.
				a = -(v[0].y-v[2].y);
				b =  (v[0].x-v[2].x);
				len= sqrtf(a*a+b*b);		// norm of the normal vector
				if (a*(pos.x-v[2].x) + b*(pos.y-v[2].y) < -len*outsideTolerance)	continue;


				// Ok IN => compute z and keep nearest to wanted one
				CPlane	p;
				p.make(v[0], v[1], v[2]);

				CVector i = p.intersect(pos, posh);

				float	d = (float)fabs(pos.z-i.z);

				if (d < bestDist)
				{
					bestDist = d;
					best = i;
				}
			}
		}

		// and computes the real position on this face
		return (bestDist < 200.0f) ? best.z : position.Estimation.z;
	}

	return 0.f;
}


// ***************************************************************************
#ifdef NL_OS_WINDOWS
//#pragma optimize( "", off )
#endif // NL_OS_WINDOWS

void	NLPACS::CLocalRetriever::findPath(const NLPACS::CLocalRetriever::CLocalPosition &A,
										  const NLPACS::CLocalRetriever::CLocalPosition &B,
										  std::vector<NLPACS::CVector2s> &path,
										  NLPACS::CCollisionSurfaceTemp &cst) const
{
	if (A.Surface != B.Surface)
	{
		nlwarning("in NLPACS::CLocalRetriever::findPath()");
		nlerror("Try to find a path between 2 points that are not in the same surface (A=%d, B=%d)", A.Surface, B.Surface);
	}

	CVector		a = A.Estimation,
				b = B.Estimation,
				n = CVector(a.y-b.y, b.x-a.x, 0.0f);

	_ChainQuad.selectEdges(a, b, cst);

	vector<CIntersectionMarker>	intersections;

	uint	i, j;
	sint32	surfaceId = A.Surface;
	const CRetrievableSurface	&surface = _Surfaces[surfaceId];

	for (i=0; i<cst.EdgeChainEntries.size(); ++i)
	{
		CEdgeChainEntry		&entry = cst.EdgeChainEntries[i];
		const COrderedChain	&chain = _OrderedChains[entry.OChainId];

		if (_Chains[chain.getParentId()].getLeft() != surfaceId &&
			_Chains[chain.getParentId()].getRight() != surfaceId)
			continue;

		for (j=entry.EdgeStart; j<entry.EdgeEnd; ++j)
		{
			// here the edge collision test

			CVector	p0 = chain[j].unpack3f(),
					p1 = chain[j+1].unpack3f();

			float	vp0 = (p0-a)*n,
					vp1 = (p1-a)*n;

			if (vp0*vp1 <= 0.0f)
			{
				CVector	np = CVector(p0.y-p1.y, p1.x-p0.x, 0.0f);

				float	va = (a-p0)*np,
						vb = (b-p0)*np;

				// here we have an intersection
				if (va*vb <= 0.0f)
				{
					const CChain	&parent = _Chains[chain.getParentId()];
					bool			isIn = (va-vb < 0.0f) ^ (parent.getLeft() == surfaceId) ^ chain.isForward();

					intersections.push_back(CIntersectionMarker(va/(va-vb), entry.OChainId, uint16(j), isIn));
				}
			}
		}
	}

	sort(intersections.begin(), intersections.end());

	uint	intersStart = 0;
	uint	intersEnd = (uint)intersections.size();

	if (intersEnd > 0)
	{
		while (intersStart < intersections.size() &&
			   intersections[intersStart].In && intersections[intersStart].Position < 1.0e-4f)
			++intersStart;

		while (intersStart < intersEnd &&
			   !intersections[intersEnd-1].In && intersections[intersEnd-1].Position > 1.0f-1.0e-4f)
			--intersEnd;

		// Check intersections have a valid order
		if ((intersEnd-intersStart) & 1)
		{
			nlwarning("in NLPACS::CLocalRetriever::findPath()");
			nlerror("Found an odd (%d) number of intersections", intersections.size());
		}

		for (i=intersStart; i<intersEnd; )
		{
			uint	exitLoop, enterLoop;

			const CChain	&exitChain = _Chains[_OrderedChains[intersections[i].OChain].getParentId()];
			exitLoop = (exitChain.getLeft() == surfaceId) ? exitChain.getLeftLoop() : exitChain.getRightLoop();

			if (intersections[i++].In)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Entered the surface before exited", intersections.size());
			}

			const CChain	&enterChain = _Chains[_OrderedChains[intersections[i].OChain].getParentId()];
			enterLoop = (enterChain.getLeft() == surfaceId) ? enterChain.getLeftLoop() : enterChain.getRightLoop();

			if (!intersections[i++].In)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Exited twice the surface", intersections.size());
			}

			if (exitLoop != enterLoop)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Exited and rentered by a different loop");
			}
		}
	}

//	dumpSurface(surfaceId);

	path.push_back(CVector2s(A.Estimation));

	for (i=intersStart; i<intersEnd; )
	{
		uint								exitChainId = _OrderedChains[intersections[i].OChain].getParentId(),
											enterChainId = _OrderedChains[intersections[i+1].OChain].getParentId();
		const CChain						&exitChain = _Chains[exitChainId],
											&enterChain = _Chains[enterChainId];
		uint								loopId, exitLoopIndex, enterLoopIndex;

		if (exitChain.getLeft() == surfaceId)
		{
			loopId = exitChain.getLeftLoop();
			exitLoopIndex = exitChain.getLeftLoopIndex();
		}
		else
		{
			loopId = exitChain.getRightLoop();
			exitLoopIndex = exitChain.getRightLoopIndex();
		}

		const CRetrievableSurface::TLoop	&loop = surface._Loops[loopId];

		if (enterChain.getLeft() == surfaceId)
			enterLoopIndex = enterChain.getLeftLoopIndex();
		else
			enterLoopIndex = enterChain.getRightLoopIndex();

		float			forwardLength = (exitChain.getLength()+enterChain.getLength())*0.5f;

		sint	loopIndex = exitLoopIndex;
		uint	thisChainId = exitChainId;
		bool	thisChainForward = (enterChain.getLeft() == surfaceId);
		uint	thisOChainId = intersections[i].OChain;
		sint	thisOChainIndex = _OrderedChains[thisOChainId].getIndexInParent();
		bool	forward;

		if (exitChainId != enterChainId)
		{
			for (j=(exitLoopIndex+1)%loop.size(); j!=enterLoopIndex; j=(j+1)%loop.size())
				forwardLength += _Chains[surface._Chains[loop[j]].Chain].getLength();
			forward = (forwardLength <= loop.Length-forwardLength);
		}
		else
		{
			forward = !thisChainForward ^ (_OrderedChains[intersections[i].OChain].getIndexInParent() < _OrderedChains[intersections[i+1].OChain].getIndexInParent());
		}

		path.push_back(CVector2s(A.Estimation+intersections[i].Position*(B.Estimation-A.Estimation)));

		for(;;)
		{
			sint	from = (thisOChainId == intersections[i].OChain) ? intersections[i].Edge : -1,
					to = (thisOChainId == intersections[i+1].OChain) ? intersections[i+1].Edge : -1;
			bool	oforward = thisChainForward ^ forward ^ _OrderedChains[thisOChainId].isForward();

			if (from != -1 && to != -1)
				oforward = (intersections[i].Edge < intersections[i+1].Edge);

			_OrderedChains[thisOChainId].traverse(from, to, oforward, path);

			if (thisOChainId == intersections[i+1].OChain)
				break;

			thisOChainIndex = (thisChainForward ^ forward) ? thisOChainIndex-1 : thisOChainIndex+1;

			if (thisOChainIndex < 0 || thisOChainIndex >= (sint)_Chains[thisChainId]._SubChains.size())
			{
				if (forward)
				{
					loopIndex++;
					if (loopIndex == (sint)loop.size())
						loopIndex = 0;
				}
				else
				{
					loopIndex--;
					if (loopIndex < 0)
						loopIndex = (sint)loop.size()-1;
				}

				thisChainId = surface._Chains[loop[loopIndex]].Chain;
				thisChainForward = (_Chains[thisChainId].getLeft() == surfaceId);
				thisOChainIndex = (thisChainForward == forward) ?
					0 : (sint)_Chains[thisChainId]._SubChains.size()-1;
			}

			thisOChainId = _Chains[thisChainId]._SubChains[thisOChainIndex];
		}

		path.push_back(CVector2s(A.Estimation+intersections[i+1].Position*(B.Estimation-A.Estimation)));
		i += 2;
	}

	path.push_back(CVector2s(B.Estimation));
}
#ifdef NL_OS_WINDOWS
//#pragma optimize( "", on )
#endif // NL_OS_WINDOWS

// ***************************************************************************
// ***************************************************************************
// Collisions part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	NLPACS::CLocalRetriever::computeCollisionChainQuad()
{
	_ChainQuad.build(_OrderedChains);
}


// ***************************************************************************
void	NLPACS::CLocalRetriever::testCollision(CCollisionSurfaceTemp &cst, const CAABBox &bboxMove, const CVector2f &transBase) const
{
	if (!_Loaded)
		return;

//	H_AUTO(PACS_LR_testCollision);

	sint	i;

	// 0. select ordered chains in the chainquad.
	//=====================================
//	H_BEFORE(PACS_LR_testCol_selEdges);
	sint	nEce= _ChainQuad.selectEdges(bboxMove, cst);
//	H_AFTER(PACS_LR_testCol_selEdges);
	// NB: cst.OChainLUT is assured to be full of 0xFFFF after this call (if was right before).


	// 1. regroup them in chains. build cst.CollisionChains
	//=====================================
	// NB: use cst.OChainLUT to look if a Chain has been inserted before.
	uint16	*chainLUT= cst.OChainLUT;

	// bkup where we begin to add chains.
	uint	firstChainAdded= (uint)cst.CollisionChains.size();

	// For all edgechain entry.
	for(i=0;i<nEce;i++)
	{
		CEdgeChainEntry		&ece= cst.EdgeChainEntries[i];
		// this is the ordered chain in the retriever.
		const	COrderedChain	&oChain= this->getOrderedChains()[ece.OChainId];
		// this is the id of the chain is the local retriever.
		uint16				chainId= oChain.getParentId();

		// test if edge is interior and points to another instance
		if (_Type == Interior && CChain::isBorderChainId(this->getChains()[chainId].getRight()))
		{
			// then look for a door that match this edge
			uint	l;
			for (l=0; l<_ExteriorMesh.getLinks().size() && _ExteriorMesh.getLink(l).ChainId != chainId; ++l)
				;

			// if found a door, then leave the edge as is
			if (l < _ExteriorMesh.getLinks().size())
				continue;
		}


		// add/retrieve the id in cst.CollisionChains.
		//=================================
		uint				ccId;
		// if never added.
		if(chainLUT[chainId]==0xFFFF)
		{
//			H_AUTO(PACS_LR_testCol_addToLUT);
			// add a new CCollisionChain.
			ccId= (uint)cst.CollisionChains.size();
			cst.CollisionChains.push_back(CCollisionChain());
			// Fill it with default.
			cst.CollisionChains[ccId].Tested= false;
			cst.CollisionChains[ccId].ExteriorEdge = false;
			cst.CollisionChains[ccId].FirstEdgeCollide= 0xFFFFFFFF;
			cst.CollisionChains[ccId].ChainId= chainId;
			// Fill Left right info.
			cst.CollisionChains[ccId].LeftSurface.SurfaceId= this->getChains()[chainId].getLeft();
			cst.CollisionChains[ccId].RightSurface.SurfaceId= this->getChains()[chainId].getRight();
			// NB: cst.CollisionChains[ccId].*Surface.RetrieverInstanceId is not filled here because we don't have
			// this info at this level.

			// store this Id in the LUT of chains.
			chainLUT[chainId]= uint16(ccId);
		}
		else
		{
			// get the id of this collision chain.
			ccId= chainLUT[chainId];
		}

		// add edge collide to the list.
		//=================================
//		H_BEFORE(PACS_LR_testCol_addToList);
		CCollisionChain			&colChain= cst.CollisionChains[ccId];
		const std::vector<CVector2s>	&oChainVertices= oChain.getVertices();
		for(sint edge=ece.EdgeStart; edge<ece.EdgeEnd; edge++)
		{
			CVector2f	p0= oChainVertices[edge].unpack();
			CVector2f	p1= oChainVertices[edge+1].unpack();

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
//		H_AFTER(PACS_LR_testCol_addToList);
	}



	// 2. Reset LUT to 0xFFFF.
	//=====================================

//	H_BEFORE(PACS_LR_testCol_resetLUT);
	// for all collisions chains inserted (starting from firstChainAdded), reset LUT.
	for(i=firstChainAdded; i<(sint)cst.CollisionChains.size(); i++)
	{
		uint	ccId= cst.CollisionChains[i].ChainId;
		chainLUT[ccId]= 0xFFFF;
	}
//	H_AFTER(PACS_LR_testCol_resetLUT);
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	NLPACS::CLocalRetriever::buildInteriorSurfaceBBoxes(std::vector<NLMISC::CAABBox>	&surfaceBBoxes) const
{
	// resize dest, and init.
	vector<bool>	firstTriangle;
	surfaceBBoxes.clear();
	surfaceBBoxes.resize(_Surfaces.size());
	firstTriangle.resize(_Surfaces.size(), true);

	// For all _InteriorFaces.
	for(uint iIntFace=0; iIntFace<_InteriorFaces.size(); iIntFace++)
	{
		const CInteriorFace	&intFace= _InteriorFaces[iIntFace];

		// Extend the surface of this face with her 3 points.

		// check good id.
		if(intFace.Surface==(uint)-1)
			continue;
		nlassert(intFace.Surface<_Surfaces.size());

		// If first time we extend the bbox of this surface
		if(firstTriangle[intFace.Surface])
		{
			surfaceBBoxes[intFace.Surface].setCenter(_InteriorVertices[intFace.Verts[0]] );
			firstTriangle[intFace.Surface]= false;
		}
		else
			surfaceBBoxes[intFace.Surface].extend(_InteriorVertices[intFace.Verts[0]] );

		// extend with other 2 points
		surfaceBBoxes[intFace.Surface].extend(_InteriorVertices[intFace.Verts[1]] );
		surfaceBBoxes[intFace.Surface].extend(_InteriorVertices[intFace.Verts[2]] );
	}

}


// ***************************************************************************
void	NLPACS::CLocalRetriever::replaceChain(uint32 chainId, const std::vector<NLPACS::CLocalRetriever::CChainReplacement> &replacement)
{
	// free subchains
	uint		i, j;
	for (i=0; i<_Chains[chainId]._SubChains.size(); ++i)
	{
		FreeOChains.push_back(_Chains[chainId]._SubChains[i]);
		_OrderedChains[_Chains[chainId]._SubChains[i]] = COrderedChain();
		_FullOrderedChains[_Chains[chainId]._SubChains[i]] = COrderedChain3f();
	}

	// create new chains in replacement of this chain

	for (i=0; i<replacement.size(); ++i)
	{
		vector<CVector>	vertices = replacement[i].Vertices;
		sint			left = replacement[i].Left;
		sint			right = replacement[i].Right;

		if (CChain::isBorderChainId(right))
		{
			// check border already exists for this particular chain
			sint32	border = CChain::convertBorderChainId(right);

			if (border < (sint)_BorderChains.size() && (chainId != _BorderChains[border] || chainId != replacement[i].Chain))
			{
				nlwarning("replaceChain(): replacement of a border is forced whereas this border is already used and not replaced!");
			}

			if (border >= (sint)_BorderChains.size())
			{
				if (border > (sint)_BorderChains.size())
				{
					nlwarning("replaceChain(): _BorderChains size increased of more than 1 step, holes may result!");
				}

				_BorderChains.resize(border+1, 0xffff);
			}

			_BorderChains[border] = uint16(replacement[i].Chain);
		}

		nlassert(vertices.size() >= 2);

		// Remove doubled vertices due to CVector2s snapping
		vector<CVector2s>	converts;

		for (j=0; j<vertices.size(); ++j)
			converts.push_back(CVector2s(vertices[j]));

		vector<CVector2s>::iterator	next2s = converts.begin(), it2s, prev2s;
		prev2s = next2s; ++next2s;
		it2s = next2s; ++next2s;

		vector<CVector>::iterator	it3f = vertices.begin();
		CVector						prev3f = *it3f;
		++it3f;


		for (; it2s != converts.end() && next2s != converts.end(); )
		{
			// if the next point is equal to the previous
			if (*it2s == *prev2s || *it2s == *next2s)
			{
				// then remove the next point
				it2s = converts.erase(it2s);
				it3f = vertices.erase(it3f);

				prev2s = it2s;
				--prev2s;
				next2s = it2s;
				++next2s;
			}
			else
			{
				// else remember the next point, and step to the next...
				++prev2s;
				++it2s;
				++next2s;
				++it3f;
				prev3f = *it3f;
			}
		}

		nlassert(vertices.size() >= 2);

		sint32		newId = replacement[i].Chain;
		if (newId >= (sint)_Chains.size())
			_Chains.resize(newId+1);

		//CChain		&nchain = _Chains[newId];

		if (left>(sint)_Surfaces.size())
			nlerror ("left surface id MUST be id<%d (id=%d)", _Surfaces.size(), left);
		if (right>(sint)_Surfaces.size())
			nlerror ("right surface id MUST be id<%d (id=%d)", _Surfaces.size(), right);

		// checks if we can build the chain.
		if (newId > 65535)
			nlerror("in NLPACS::CLocalRetriever::addChain(): reached the maximum number of chains");

		//CRetrievableSurface	*leftSurface = (left>=0) ? &(_Surfaces[left]) : NULL;
		//CRetrievableSurface	*rightSurface = (right>=0) ? &(_Surfaces[right]) : NULL;

		CChain		&chain = _Chains[newId];

		chain._StartTip = 0xffff;
		chain._StopTip = 0xffff;

		// make the chain and its subchains.
		chain.make(vertices, left, right, _OrderedChains, (uint16)newId, _FullOrderedChains, FreeOChains);
	}

	for (i=0; i<_Surfaces.size(); ++i)
	{
		// remove old chain and replace by new chains in surface links
		for (j=0; j<_Surfaces[i]._Chains.size(); ++j)
		{
			if (_Surfaces[i]._Chains[j].Chain == (sint)chainId)
			{
				_Surfaces[i]._Chains.erase(_Surfaces[i]._Chains.begin()+j);

				uint	k;
				for (k=0; k<replacement.size(); ++k)
				{
					CRetrievableSurface::CSurfaceLink	link;

					link.Chain = replacement[k].Chain;
					link.Surface = (replacement[k].Left == (sint)i ? replacement[k].Right : replacement[k].Left);
					_Surfaces[i]._Chains.push_back(link);
				}

				break;
			}
		}

		// remove old chain and replace by new chains in surface loops
		for (j=0; j<_Surfaces[i]._Loops.size(); ++j)
		{
			uint	k;

			for (k=0; k<_Surfaces[i]._Loops[j].size(); ++k)
			{
				if (_Surfaces[i]._Loops[j][k] == chainId)
				{
					_Surfaces[i]._Loops[j].erase(_Surfaces[i]._Loops[j].begin()+k);
					uint	m;

					for (m=0; m<replacement.size(); ++m)
						_Surfaces[i]._Loops[j].insert(_Surfaces[i]._Loops[j].begin()+k+m, uint16(replacement[m].Chain));

					break;
				}
			}
		}
	}
}




/*
 * Check surface integrity
 */
bool	NLPACS::CLocalRetriever::checkSurfacesIntegrity(NLMISC::CVector translation, bool verbose) const
{
	bool	success = true;
	uint	surf;

	for (surf=0; surf<_Surfaces.size(); ++surf)
	{
		if (!checkSurfaceIntegrity(surf, translation, verbose))
		{
			success = false;
			if (verbose)
			{
				dumpSurface(surf, translation);
			}
		}

	}

	return success;
}


/**
 * Check surface integrity
 */
bool	NLPACS::CLocalRetriever::checkSurfaceIntegrity(uint surf, NLMISC::CVector translation, bool verbose) const
{
	if (surf >= _Surfaces.size())
		return false;

	const CRetrievableSurface&	surface = _Surfaces[surf];

	uint	nloops = (uint)surface.getLoops().size();

	std::vector<std::pair<CVector2s, CVector2s> >	edges;

	uint	iloop;
	uint	i, j, k;
	for (iloop=0; iloop<nloops; ++iloop)
	{
		const CRetrievableSurface::TLoop&	loop = surface.getLoop(iloop);

		for (i=0; i<loop.size(); ++i)
		{
			// loop[i] is the index in the surface list of chains
			// so surface._Chains[loop[i]].Chain is really the chain id !!!
			const CChain&	chain = _Chains[ surface._Chains[loop[i]].Chain ];
			for (j=0; j<chain.getSubChains().size(); ++j)
			{
				const COrderedChain&	ochain = _OrderedChains[chain.getSubChain(j)];

				for (k=0; k+1<ochain.getVertices().size(); ++k)
				{
					edges.push_back(std::pair<CVector2s, CVector2s>(ochain[k], ochain[k+1]));
				}
			}
		}
	}

	bool	success = true;

	for (i=0; i+1<edges.size(); ++i)
	{
		for (j=i+1; j<edges.size(); ++j)
		{
			CVector2s	a0 = edges[i].first,
						a1 = edges[i].second;
			CVector2s	b0 = edges[j].first,
						b1 = edges[j].second;

			double		a, b;
			bool		inters = CVector2s::intersect(a0, a1, b0, b1, &a, &b);

			if (!inters)
				continue;

			double		da = (a1-a0).norm();
			double		db = (b1-b0).norm();

			bool		tipa = (fabs(a)*da < 4.0e-2 || fabs(1.0-a)*da < 4.0e-2);
			bool		tipb = (fabs(b)*db < 4.0e-2 || fabs(1.0-b)*db < 4.0e-2);

			if (tipa && tipb)
				continue;

			CVector2s		ip = a0 + (a1-a0)*(float)a;
			CVector			p = ip.unpack3f() - translation;
			nlwarning("Surface %d has issue at position (%f,%f)", surf, p.x, p.y);
			if (verbose)
			{
				NLMISC::CVector	v;
				v = a0.unpack3f() - translation;
				nlwarning("  -- a0: (%f,%f)", v.x, v.y);
				v = a1.unpack3f() - translation;
				nlwarning("  -- a1: (%f,%f)", v.x, v.y);
				v = b0.unpack3f() - translation;
				nlwarning("  -- b0: (%f,%f)", v.x, v.y);
				v = b1.unpack3f() - translation;
				nlwarning("  -- b1: (%f,%f)", v.x, v.y);
			}

			success = false;
		}
	}

	return success;
}
