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

#include "nel/pacs/chain_quad.h"

using	namespace std;
using	namespace NLMISC;


namespace NLPACS
{


// ***************************************************************************
const	float	CChainQuad::_QuadElementSize= 4;	// = 4 meters.


// ***************************************************************************
CChainQuad::CChainQuad()
{
	_QuadData= NULL;
	_QuadDataLen= 0;
}
// ***************************************************************************
CChainQuad::~CChainQuad()
{
	delete [] _QuadData;
	_QuadData= NULL;
	_QuadDataLen= 0;
}
// ***************************************************************************
CChainQuad::CChainQuad(const CChainQuad &o)
{
	_QuadData= NULL;
	_QuadDataLen= 0;
	*this= o;
}
// ***************************************************************************
CChainQuad	&CChainQuad::operator=(const CChainQuad &o)
{
	// Alloc good quaddata.
	_QuadDataLen= o._QuadDataLen;
	delete [] _QuadData;
	if(_QuadDataLen>0)
	{
		_QuadData= (uint8*)new uint8[_QuadDataLen];
		// copy contents.
		memcpy(_QuadData, o._QuadData, _QuadDataLen);
	}
	else
		_QuadData= NULL;

	// copy infos.
	_Width= o._Width;
	_Height= o._Height;
	_X= o._X;
	_Y= o._Y;

	// copy good pointers.
	_Quad.clear();
	_Quad.resize(o._Quad.size(), NULL);
	for(sint i=0; i<(sint)_Quad.size(); i++)
	{
		if(o._Quad[i])
		{
			uint32	off= (uint32)(o._Quad[i]-o._QuadData);
			_Quad[i]= _QuadData+off;
		}
	}


	return *this;
}



// ***************************************************************************
void			CChainQuad::getGridBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const CVector &minP, const CVector &maxP) const
{
	x0= (sint32)floor(minP.x / _QuadElementSize) - _X;
	y0= (sint32)floor(minP.y / _QuadElementSize) - _Y;
	x1= (sint32) ceil(maxP.x / _QuadElementSize) - _X;
	y1= (sint32) ceil(maxP.y / _QuadElementSize) - _Y;
	// Manage selection of a point exactly on a quad bound
	if(x1-x0==0)
		x0--, x1++;
	if(y1-y0==0)
		y0--, y1++;
	// clamp
	x0= max(x0, (sint32)0);
	y0= max(y0, (sint32)0);
	x1= min(x1, (sint32)_Width);
	y1= min(y1, (sint32)_Height);
}


// ***************************************************************************
void			CChainQuad::build(const std::vector<COrderedChain> &ochains)
{
	vector< list<CEdgeChainEntry> >	tempQuad;
	sint	i,j;

	// first, clear any pr-build.
	contReset(_Quad);
	delete [] _QuadData;
	_QuadData= NULL;
	_QuadDataLen= 0;


	// 0. Find BBox of the grid. Allocate grid.
	//=========================================
	bool		first=true;
	CAABBox		chainquadBBox;
	// run all chains.
	for(i=0;i<(sint)ochains.size();i++)
	{
		const std::vector<CVector2s>	&vertices= ochains[i].getVertices();

		// run all vertices.
		for(j= 0; j<(sint)vertices.size();j++)
		{
			// enlarge bbox.
			if(first)
				first= false, chainquadBBox.setCenter(vertices[j].unpack3f());
			else
				chainquadBBox.extend(vertices[j].unpack3f());
		}
	}

	// compute X,Y,Width, Height.
	_X= (sint32)floor(chainquadBBox.getMin().x / _QuadElementSize);
	_Y= (sint32)floor(chainquadBBox.getMin().y / _QuadElementSize);
	_Width= (sint32)ceil(chainquadBBox.getMax().x / _QuadElementSize) - _X;
	_Height= (sint32)ceil(chainquadBBox.getMax().y / _QuadElementSize) - _Y;

	tempQuad.resize(_Width*_Height);
	_Quad.resize(_Width*_Height, NULL);


	// 1. For each edge, add them to the quadgrid.
	//=========================================
	// run all chains.
	for(i=0;i<(sint)ochains.size();i++)
	{
		const std::vector<CVector2s>	&vertices= ochains[i].getVertices();

		sint	numEdges= (sint)vertices.size()-1;

		// run all edges.
		for(j= 0; j<numEdges; j++)
		{
			const CVector		p0= vertices[j].unpack3f();
			const CVector		p1= vertices[j+1].unpack3f();
			CVector		minP,maxP;
			minP.minof(p0, p1);
			maxP.maxof(p0, p1);
			// PrecisionPb: extend a little this edge. This is important for special case like borders on zones.
			if(minP.x-maxP.x==0)
				minP.x-=0.001f, maxP.x+=0.001f;
			if(minP.y-maxP.y==0)
				minP.y-=0.001f, maxP.y+=0.001f;


			// get bounding coordinate of this edge in the quadgrid.
			sint32	x0, y0, x1, y1;
			getGridBounds(x0, y0, x1, y1, minP, maxP);

			// add this edge to all the quadnode it touch.
			for(sint y= y0; y<y1; y++)
			{
				for(sint x= x0; x<x1; x++)
				{
					list<CEdgeChainEntry>	&quadNode= tempQuad[y*_Width+x];

					addEdgeToQuadNode(quadNode, i, j);
				}
			}
		}
	}


	// 2. Mem optimisation: Use only 1 block for ALL quads of the grid.
	//=========================================
	sint	memSize= 0;
	// run all quads.
	for(i=0;i<(sint)tempQuad.size();i++)
	{
		list<CEdgeChainEntry>	&quadNode= tempQuad[i];

		if(!quadNode.empty())
		{
			// add an entry for Len.
			memSize+= sizeof(uint16);
			// add N entry of CEdgeChainEntry.
			memSize+= (sint)quadNode.size()*sizeof(CEdgeChainEntry);
		}
	}

	// allocate.
	_QuadData= (uint8*)new uint8[memSize];
	_QuadDataLen= memSize;


	// 3. Fill _QuadData with lists.
	//=========================================
	uint8	*ptr= _QuadData;
	for(i=0;i<(sint)tempQuad.size();i++)
	{
		list<CEdgeChainEntry>			&srcQuadNode= tempQuad[i];
		list<CEdgeChainEntry>::iterator	it;

		if(!srcQuadNode.empty())
		{
			_Quad[i]= ptr;

			// write len.
			uint16	len= uint16(srcQuadNode.size());
			*((uint16*)ptr)= len;
			ptr+= sizeof(uint16);

			// add entries.
			it= srcQuadNode.begin();
			for(j=0; j<len; j++, it++)
			{
				*((CEdgeChainEntry*)ptr)= *it;
				ptr+= sizeof(CEdgeChainEntry);
			}
		}
	}


	// End.
}


// ***************************************************************************
void			CChainQuad::addEdgeToQuadNode(list<CEdgeChainEntry> &quadNode, sint ochainId, sint edgeId)
{
	// 0. try to find, insert an edge in an existing CEdgeChainEntry.
	//=========================================
	list<CEdgeChainEntry>::iterator		it;
	for(it= quadNode.begin(); it!=quadNode.end();it++)
	{
		if(it->OChainId==ochainId)
		{
			// selection is faster if we only manages a single start/end block.
			it->EdgeStart= min(it->EdgeStart, (uint16)edgeId);
			it->EdgeEnd= max(it->EdgeEnd, (uint16)(edgeId+1));
			return;
		}
	}


	// 1. else, create new one.
	//=========================================
	CEdgeChainEntry		entry;
	entry.OChainId= uint16(ochainId);
	entry.EdgeStart= uint16(edgeId);
	entry.EdgeEnd= uint16(edgeId+1);
	quadNode.push_back(entry);
}


// ***************************************************************************
sint			CChainQuad::selectEdges(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst) const
{
	sint	nRes=0;
	sint	i;
	uint16	*ochainLUT= cst.OChainLUT;

	// start: no edge found.
	cst.EdgeChainEntries.clear();

	// get bounding coordinate of this bbox in the quadgrid.
	sint32	x0, y0, x1, y1;
	getGridBounds(x0, y0, x1, y1, bbox.getMin(), bbox.getMax());


	// run all intersected quads.
	for(sint y= y0; y<y1; y++)
	{
		for(sint x= x0; x<x1; x++)
		{
			uint8	*quadNode= _Quad[y*_Width+x];

			// no edgechain entry??
			if(!quadNode)
				continue;

			// get edgechain entries
			sint	numEdgeChainEntries= *((uint16*)quadNode);
			quadNode+= sizeof(uint16);
			CEdgeChainEntry		*ptrEdgeChainEntry= (CEdgeChainEntry*)quadNode;

			// For each one, add it to the result list.
			for(i=0;i<numEdgeChainEntries;i++)
			{
				uint16	ochainId= ptrEdgeChainEntry[i].OChainId;

				// if ochain not yet inserted.
				if(ochainLUT[ochainId]==0xFFFF)
				{
					// inc the list.
					ochainLUT[ochainId]= uint16(nRes);
					cst.EdgeChainEntries.push_back(ptrEdgeChainEntry[i]);
					nRes++;
				}
				else
				{
					// extend the entry in the list.
					uint16 id= ochainLUT[ochainId];
					CEdgeChainEntry		&ece= cst.EdgeChainEntries[id];
					ece.EdgeStart= min(ece.EdgeStart, ptrEdgeChainEntry[i].EdgeStart);
					ece.EdgeEnd= max(ece.EdgeEnd, ptrEdgeChainEntry[i].EdgeEnd);
				}
			}
		}
	}


	// reset LUT to 0xFFFF for all ochains selected.
	for(i=0;i<nRes;i++)
	{
		uint16	ochainId= cst.EdgeChainEntries[i].OChainId;
		ochainLUT[ochainId]= 0xFFFF;
	}


	return nRes;
}

sint		CChainQuad::selectEdges(CVector start, CVector end, CCollisionSurfaceTemp &cst) const
{
	sint	nRes=0;
	sint	i;
	uint16	*ochainLUT= cst.OChainLUT;

	// start: no edge found.
	cst.EdgeChainEntries.clear();

	if (end.x < start.x)
		swap(start, end);

	float	minx = _X*_QuadElementSize,
			miny = _Y*_QuadElementSize,
			maxx = minx + _Width*_QuadElementSize,
			maxy = miny + _Height*_QuadElementSize;

	if (start.x > maxx || end.x < minx || start.y > maxy || end.y < miny)
		return nRes;

	if (start.x < minx)
	{
		start.y = start.y+(end.y-start.y)*(minx-start.x)/(end.x-start.x);
		start.x = minx;
	}

	if (start.y < miny)
	{
		start.x = start.x+(end.x-start.x)*(miny-start.y)/(end.y-start.y);
		start.y = miny;
	}

	if (end.x > maxx)
	{
		end.y = start.y+(end.y-start.y)*(minx-start.x)/(end.x-start.x);
		end.x = maxx;
	}

	if (end.y > maxy)
	{
		end.x = start.x+(end.x-start.x)*(miny-start.y)/(end.y-start.y);
		end.y = maxy;
	}

	sint32	x0, x1, ya, yb;
	sint	x, y;
	float	fx, fxa, fxb, fya, fyb;

	x0 = (sint32)floor(start.x / _QuadElementSize) - _X;
	x1 = (sint32)ceil(end.x / _QuadElementSize) - _X;
	fx = (x0+_X)*_QuadElementSize;

	for (x=x0; x<x1; ++x)
	{
		fxa = (fx < start.x) ? start.x : fx;
		fxb = (fx+_QuadElementSize > end.x) ? end.x : fx+_QuadElementSize;

		fya = start.y+(end.y-start.y)*(fxa-start.x)/(end.x-start.x);
		fyb = start.y+(end.y-start.y)*(fxb-start.x)/(end.x-start.x);

		if (fya > fyb)
			swap (fya, fyb);

		ya = (sint32)floor(fya / _QuadElementSize) - _Y;
		yb = (sint32)ceil(fyb / _QuadElementSize) - _Y;

		fx += _QuadElementSize;

		for (y=ya; y<yb; ++y)
		{
			uint8	*quadNode= _Quad[y*_Width+x];

			// no edgechain entry??
			if(!quadNode)
				continue;

			// get edgechain entries
			sint	numEdgeChainEntries= *((uint16*)quadNode);
			quadNode+= sizeof(uint16);
			CEdgeChainEntry		*ptrEdgeChainEntry= (CEdgeChainEntry*)quadNode;

			// For each one, add it to the result list.
			for(i=0;i<numEdgeChainEntries;i++)
			{
				uint16	ochainId= ptrEdgeChainEntry[i].OChainId;

				// if ochain not yet inserted.
				if(ochainLUT[ochainId]==0xFFFF)
				{
					// inc the list.
					ochainLUT[ochainId]= uint16(nRes);
					cst.EdgeChainEntries.push_back(ptrEdgeChainEntry[i]);
					nRes++;
				}
				else
				{
					// extend the entry in the list.
					uint16 id= ochainLUT[ochainId];
					CEdgeChainEntry		&ece= cst.EdgeChainEntries[id];
					ece.EdgeStart= min(ece.EdgeStart, ptrEdgeChainEntry[i].EdgeStart);
					ece.EdgeEnd= max(ece.EdgeEnd, ptrEdgeChainEntry[i].EdgeEnd);
				}
			}
		}
	}

	// reset LUT to 0xFFFF for all ochains selected.
	for(i=0;i<nRes;i++)
	{
		uint16	ochainId= cst.EdgeChainEntries[i].OChainId;
		ochainLUT[ochainId]= 0xFFFF;
	}

	return nRes;
}

// ***************************************************************************
void		CChainQuad::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);
	uint	i;

	// serial basics.
	f.serial(_X, _Y, _Width, _Height, _QuadDataLen);


	// serial _QuadData.
	if(f.isReading())
	{
		delete [] _QuadData;
		if(_QuadDataLen>0)
			_QuadData= (uint8*)new uint8[_QuadDataLen];
		else
			_QuadData= NULL;
	}
	// Since we have only uint16 (see CEdgeChainEntry), serial them in a single block.
	uint16	*ptrQData= (uint16*)_QuadData;
	for(i=0;i<_QuadDataLen/2; i++, ptrQData++)
	{
		f.serial(*ptrQData);
	}


	// serial _Quad.
	std::vector<uint32>		offsets;
	uint32		len;
	uint32		val;
	if(f.isReading())
	{
		// len/resize.
		f.serial(len);
		offsets.resize(len);
		contReset(_Quad);
		_Quad.resize(len);

		// read offsets -> ptrs.
		for(i=0; i<len; i++)
		{
			f.serial(val);
			if(val== 0xFFFFFFFF)
				_Quad[i]= NULL;
			else
				_Quad[i]= _QuadData+val;
		}
	}
	else
	{
		// len/resize.
		len= (uint32)_Quad.size();
		f.serial(len);

		// write offsets.
		for(i=0; i<len; i++)
		{
			uint8	*ptr= _Quad[i];
			if(ptr==NULL)
				val= 0xFFFFFFFF;
			else
				val= (uint32)(ptr-_QuadData);
			f.serial(val);
		}
	}

}



} // NLPACS
