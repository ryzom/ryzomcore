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

#include "nel/pacs/chain.h"

using namespace std;
using namespace NLMISC;


// Functions for vertices comparison.
// total order relation
static inline bool	isStrictlyLess(const CVector &a, const CVector &b)
{
	if (a.x < b.x)	return true;
	if (a.x > b.x)	return false;
	if (a.y < b.y)	return true;
	if (a.y > b.y)	return false;
	if (a.z < b.y)	return true;
	return false;
}

static inline bool	isStrictlyGreater(const CVector &a, const CVector &b)
{
	if (a.x > b.x)	return true;
	if (a.x < b.x)	return false;
	if (a.y > b.y)	return true;
	if (a.y < b.y)	return false;
	if (a.z > b.y)	return true;
	return false;
}

static inline bool	isEqual(const CVector &a, const CVector &b)
{
	return (a == b);
}


// COrderedChain3f methods implementation

void	NLPACS::COrderedChain3f::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
	f.serial(_IndexInParent);
}

// end of COrderedChain3f methods implementation

// COrderedChain methods implementation

// translates the ordered chain by the vector translation
void	NLPACS::COrderedChain::translate(const CVector &translation)
{
	uint	i;
	CVector2s	translat;
	translat.pack(translation);
	for (i=0; i<_Vertices.size(); ++i)
		_Vertices[i] += translat;
}

//
void	NLPACS::COrderedChain::traverse(sint from, sint to, bool forward, vector<NLPACS::CVector2s> &path) const
{
	sint	i;
	if (forward)
	{
		if (from < 0)	from = 0;
		if (to < 0)		to = (sint)_Vertices.size()-1;

		for (i=from+1; i<=to; ++i)
			path.push_back(_Vertices[i]);
	}
	else
	{
		if (from < 0)	from = (sint)_Vertices.size()-2;
		if (to < 0)		to = -1;

		for (i=from; i>to; --i)
			path.push_back(_Vertices[i]);
	}
}

//
float	NLPACS::COrderedChain::distance(const CVector &position) const
{
	float		minDist = 1.0e10f;
	uint		i;
	CVector2f	pos = CVector2f(position);

	for (i=0; i+1<_Vertices.size(); ++i)
	{
		CVector2f	a = _Vertices[i].unpack(),
					b = _Vertices[i+1].unpack();

		CVector2f	d = (b-a);
		float		len = d.norm();
		d /= len;
		CVector2f	n = CVector2f(d.y, -d.x);

		float		l = (pos-a)*d;
		float		dist;

		if (l < 0.0f)
		{
			dist = (pos-a).norm();
		}
		else if (l > len)
		{
			dist = (pos-b).norm();
		}
		else
		{
			dist = (float)fabs((pos-a)*n);
		}

		if (dist < minDist)
		{
			minDist = dist;
		}
	}

	return minDist;
}


// serializes the ordered chain
void	NLPACS::COrderedChain::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	Version 1:
		- added _Min and _Max vectors
	*/
	sint	ver= f.serialVersion(1);

	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
	f.serial(_IndexInParent);
	f.serial(_Length);

	if (ver >= 1)
	{
		f.serial(_Min, _Max);
	}
	else if (f.isReading() && !_Vertices.empty())
	{
		uint	i;
		_Max = _Min = _Vertices[0];
		for (i=1; i<_Vertices.size(); ++i)
		{
			_Min.minof(_Min, _Vertices[i]);
			_Max.maxof(_Max, _Vertices[i]);
		}
	}
}

// end of COrderedChain methods implementation


// CChain methods implementation

// builds the CChain from a list of vertices and a left and right surfaces id.
// the chains vector is the vector where to store generated ordered chains.
// thisId is the current id of the CChain, and edge is the number of the edge the CChain belongs to (-1
// if none.)
void	NLPACS::CChain::make(const vector<CVector> &vertices, sint32 left, sint32 right, vector<COrderedChain> &chains, uint16 thisId,
							 vector<COrderedChain3f> &fullChains, vector<uint> &useOChainId)
{
	sint		first = 0, last = 0, i;

	_Left = left;
	_Right = right;
	_Length = 0.0f;

	// splits the vertices list in ordered sub chains.
	while (first < (sint)vertices.size()-1)
	{
		last = first+1;
		bool	forward = isStrictlyLess(vertices[first], vertices[last]);

		// first checks if the subchain goes forward or backward.
		if (forward)
			for (; last < (sint)vertices.size() && isStrictlyLess(vertices[last-1], vertices[last]); ++last)
				;
		else
			for (; last < (sint)vertices.size() && isStrictlyGreater(vertices[last-1], vertices[last]); ++last)
				;
		--last;

		// inserts the new subchain id within the CChain.
		uint32	subChainId;

		if (useOChainId.empty())
		{
			subChainId = (uint32)chains.size();
			if (subChainId > 65535)
				nlerror("in NLPACS::CChain::make(): reached the maximum number of ordered chains");

			chains.resize(chains.size()+1);
			fullChains.resize(fullChains.size()+1);
		}
		else
		{
			subChainId = useOChainId.back();
			useOChainId.pop_back();
		}

		_SubChains.push_back((uint16)subChainId);

		// and creates a new COrderedChain
		COrderedChain3f	&subchain3f = fullChains[subChainId];
		subchain3f._Vertices.reserve(last-first+1);
		subchain3f._Forward = forward;
		subchain3f._ParentId = thisId;
		subchain3f._IndexInParent = uint16(_SubChains.size()-1);

		// and then copies the vertices (sorted, btw!)
		if (forward)
			for (i=first; i<=last; ++i)
				subchain3f._Vertices.push_back(vertices[i]);
		else
			for (i=last; i>=first; --i)
				subchain3f._Vertices.push_back(vertices[i]);

		first = last;

		COrderedChain	&subchain = chains[subChainId];
		subchain.pack(subchain3f);
		subchain.computeMinMax();

		float	length = 0.0f;
		for (i=0; i<(sint)subchain._Vertices.size()-1; ++i)
			length += (subchain._Vertices[i+1]-subchain._Vertices[i]).norm();

		subchain._Length = length;
		_Length += length;
	}
}

// serializes the CChain
void	NLPACS::CChain::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	f.serialCont(_SubChains);
	f.serial(_Left, _Right);
	f.serial(_StartTip, _StopTip);
	f.serial(_Length);
	f.serial(_LeftLoop, _LeftLoopIndex);
	f.serial(_RightLoop, _RightLoopIndex);
}



// unifiies the chain
void	NLPACS::CChain::unify(vector<NLPACS::COrderedChain> &ochains)
{
	CVector2s	snap;
	uint		i;

	snap = (ochains[_SubChains[0]].isForward()) ? ochains[_SubChains[0]]._Vertices.back() : ochains[_SubChains[0]]._Vertices.front();

	for (i=1; i<_SubChains.size(); ++i)
	{
		if (ochains[_SubChains[i]].isForward())
		{
			if (ochains[_SubChains[i]]._Vertices.front() != snap)
				nlwarning("ochain %d and %d are not stuck together", _SubChains[i-1], _SubChains[i]);
			ochains[_SubChains[i]]._Vertices.front() = snap;
			snap = ochains[_SubChains[i]]._Vertices.back();
		}
		else
		{
			if (ochains[_SubChains[i]]._Vertices.back() != snap)
				nlwarning("ochain %d and %d are not stuck together", _SubChains[i-1], _SubChains[i]);
			ochains[_SubChains[i]]._Vertices.back() = snap;
			snap = ochains[_SubChains[i]]._Vertices.front();
		}
	}

}

//
void	NLPACS::CChain::setStartVector(const NLPACS::CVector2s &v, vector<NLPACS::COrderedChain> &ochains)
{
	if (ochains[_SubChains.front()].isForward())
		ochains[_SubChains.front()]._Vertices.front() = v;
	else
		ochains[_SubChains.front()]._Vertices.back() = v;
}

//
void	NLPACS::CChain::setStopVector(const NLPACS::CVector2s &v, vector<NLPACS::COrderedChain> &ochains)
{
	if (ochains[_SubChains.back()].isForward())
		ochains[_SubChains.back()]._Vertices.back() = v;
	else
		ochains[_SubChains.back()]._Vertices.front() = v;
}

//
NLPACS::CVector2s	NLPACS::CChain::getStartVector(vector<NLPACS::COrderedChain> &ochains)
{
	if (ochains[_SubChains.front()].isForward())
		return ochains[_SubChains.front()]._Vertices.front();
	else
		return ochains[_SubChains.front()]._Vertices.back();
}

//
NLPACS::CVector2s	NLPACS::CChain::getStopVector(vector<NLPACS::COrderedChain> &ochains)
{
	if (ochains[_SubChains.back()].isForward())
		return ochains[_SubChains.back()]._Vertices.back();
	else
		return ochains[_SubChains.back()]._Vertices.front();
}

// end of CChain methods implementation

