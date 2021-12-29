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

#include "nel/pacs/edge_quad.h"
#include "nel/pacs/global_retriever.h"

using	namespace std;
using	namespace NLMISC;


namespace NLPACS
{


// ***************************************************************************
const	float	CEdgeQuad::_QuadElementSize= 4;	// = 4 meters.


// ***************************************************************************
CEdgeQuad::CEdgeQuad()
{
	_QuadData= NULL;
	_QuadDataLen= 0;
}
// ***************************************************************************
CEdgeQuad::~CEdgeQuad()
{
	clear();
}
// ***************************************************************************
CEdgeQuad::CEdgeQuad(const CEdgeQuad &o)
{
	_QuadData= NULL;
	_QuadDataLen= 0;
	*this= o;
}
// ***************************************************************************
CEdgeQuad	&CEdgeQuad::operator=(const CEdgeQuad &o)
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
	_EdgeEntries = o._EdgeEntries;

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
void	CEdgeQuad::clear()
{
	delete [] _QuadData;
	_QuadData= NULL;
	_QuadDataLen= 0;

	_Quad.clear();
	_EdgeEntries.clear();
	_Width = 0;
	_Height = 0;
	_X = 0;
	_Y = 0;
}

// ***************************************************************************
void			CEdgeQuad::getGridBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const CVector &minP, const CVector &maxP) const
{
	x0= (sint32)floor(minP.x / _QuadElementSize) - _X;
	y0= (sint32)floor(minP.y / _QuadElementSize) - _Y;
	x1= (sint32) ceil(maxP.x / _QuadElementSize) - _X;
	y1= (sint32) ceil(maxP.y / _QuadElementSize) - _Y;
	x0= max(x0, (sint32)0);
	y0= max(y0, (sint32)0);
	x1= min(x1, (sint32)_Width);
	y1= min(y1, (sint32)_Height);
}



// ***************************************************************************
void			CEdgeQuad::build(const CExteriorMesh &em,
								 const CGlobalRetriever &global,
								 CCollisionSurfaceTemp &cst,
								 uint32 thisInstance)
{
	const std::vector<CExteriorMesh::CEdge> &edges = em.getEdges();

	vector< list<uint16> >	tempQuad;
	sint					i, j;

	// first, clear any pr-build.
	contReset(_Quad);
	delete [] _QuadData;
	_QuadData= NULL;
	_QuadDataLen= 0;

	// don't care about the origin of the instance
	CVector	origin = global.getInstance(thisInstance).getOrigin();

	// 0. Find BBox of the grid. Allocate grid.
	//=========================================
	bool		first=true;
	CAABBox		chainquadBBox;
	// run all chains.
	for (i=0; i<(sint)edges.size()-1; i++)
	{
		// enlarge bbox.
		if (first)
			first= false, chainquadBBox.setCenter(edges[i].Start);
		else
			chainquadBBox.extend(edges[i].Start);
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
	for (i=0; i<(sint)edges.size()-1; i++)
	{
		if (edges[i].Link == -2)
			continue;

		float		dnorm = (edges[i+1].Start-edges[i].Start).norm();
		uint		numStep = (uint)(dnorm/0.1f)+1;
		uint		step;

		CVector		pbegin = edges[i].Start+origin,
					pend = edges[i+1].Start+origin;

		CVector		opbegin = edges[i].Start,
					opend = edges[i+1].Start;

		for (step=0; step<numStep; ++step)
		{
			float		lambda0 = (float)(step)/(float)(numStep);
			float		lambda1 = (float)(step+1)/(float)(numStep);
			CVector		p0 = pbegin*(1.0f-lambda0)+pend*(lambda0),
						p1 = pbegin*(1.0f-lambda1)+pend*(lambda1);
			CVector		op0 = opbegin*(1.0f-lambda0)+opend*(lambda0),
						op1 = opbegin*(1.0f-lambda1)+opend*(lambda1);
			CVector		s0, s1,
						mins, maxs;

			uint		prevEdge = (i-1)%(edges.size()-1);
			bool		prio0 = (edges[i].Link!=-1) || (edges[prevEdge].Link!=-1);

			UGlobalPosition	gp0 = global.retrievePosition(p0);
			global.updateHeight(gp0);
			UGlobalPosition	gp1 = global.retrievePosition(p1);
			global.updateHeight(gp1);

			if (!prio0)
			{
				swap(p0, p1);
				swap(op0, op1);
				swap(gp0, gp1);
			}

			if (gp0.InstanceId == -1)
			{
				swap(p0, p1);
				swap(op0, op1);
				swap(gp0, gp1);
			}

			const TCollisionSurfaceDescVector	*pcd = global.testCylinderMove(gp0, p1-p0, 0.01f, cst);

			if (pcd == NULL)
			{
//				nlwarning("in CEdgeQuad::build(): testCylinderMove() returned NULL");
				continue;
			}

			TCollisionSurfaceDescVector	cd = (*pcd);

			if (edges[i].Link != -1 && !cd.empty())
			{
				nlwarning ("In NLPACS::CEdgeQuad::build()");
				nlwarning ("ERROR: exterior edge %d with interior link crosses some surfaces", i);
				cd.clear ();
			}

			// add start surface to the collision description
			CCollisionSurfaceDesc	stcd;
			stcd.ContactTime = 0.0f;
			stcd.ContactSurface.RetrieverInstanceId = gp0.InstanceId;
			stcd.ContactSurface.SurfaceId = gp0.LocalPosition.Surface;
			cd.insert(cd.begin(), stcd);

			// get the surface, chain ...
			sint	edgeId = i;
			uint16	chainId;

			CSurfaceIdent	interior;
			if (edges[i].Link == -1)
			{
				interior.RetrieverInstanceId = -1;
				interior.SurfaceId = -1;
				chainId = 0xFFFF;
			}
			else
			{
				interior.RetrieverInstanceId = thisInstance;
				interior.SurfaceId = em.getLink(edges[i].Link).SurfaceId;
				chainId = em.getLink(edges[i].Link).ChainId;
			}


			// add end point to the collision description
			stcd = cd.back();
			stcd.ContactTime = 1.0f;
			cd.push_back(stcd);

			for (j=0; j<(sint)cd.size()-1; ++j)
			{
				s0 = op0*(float)(1.0-cd[j].ContactTime) + op1*(float)(cd[j].ContactTime);
				s1 = op0*(float)(1.0-cd[j+1].ContactTime) + op1*(float)(cd[j+1].ContactTime);

				mins.minof(s0, s1);
				maxs.maxof(s0, s1);

				// PrecisionPb: extend a little this edge. This is important for special case like borders on zones.
				if(mins.x-maxs.x==0)
					mins.x-=0.001f, maxs.x+=0.001f;
				if(mins.y-maxs.y==0)
					mins.y-=0.001f, maxs.y+=0.001f;

				// get bounding coordinate of this edge in the quadgrid.
				sint32	x0, y0, x1, y1;
				sint	x, y;
				getGridBounds(x0, y0, x1, y1, mins, maxs);

				CSurfaceIdent	exterior = cd[j].ContactSurface;

				uint	entry;
				for (entry=0; entry<_EdgeEntries.size(); ++entry)
				{
					if (_EdgeEntries[entry].EdgeId == edgeId &&
						_EdgeEntries[entry].Exterior == exterior)
					{
						if (_EdgeEntries[entry].ChainId != chainId ||
							_EdgeEntries[entry].Interior != interior)
						{
							nlwarning("In NLPACS::CEdgeQuad::build()");
							nlerror("exterior edge %d has different interior linkage", edgeId);
						}

						break;
					}
				}

				// if this entry didn't exist before create a new one...
				if (entry == _EdgeEntries.size())
				{
					_EdgeEntries.push_back(CExteriorEdgeEntry());
					_EdgeEntries.back().EdgeId = uint16(edgeId);
					_EdgeEntries.back().ChainId = chainId;
					_EdgeEntries.back().Interior = interior;
					_EdgeEntries.back().Exterior = exterior;
				}

				// add this edge to all the quadnode it touches.
				for(y=y0; y<y1; y++)
				{
					for(x=x0; x<x1; x++)
					{
						// check we don't push this entry twice
						list<uint16>::iterator	it;
						for (it=tempQuad[y*_Width+x].begin(); it!=tempQuad[y*_Width+x].end(); ++it)
							if (entry == *it)
								break;
						if (it == tempQuad[y*_Width+x].end())
							tempQuad[y*_Width+x].push_back(uint16(entry));
					}
				}
			}
		}

	}

	nlinfo("Built ExteriorEdgeQuad, linked following doors:");
	for (i=0; i<(sint)_EdgeEntries.size(); ++i)
	{
		if (edges[_EdgeEntries[i].EdgeId].Link != -1 &&
			(_EdgeEntries[i].Interior.RetrieverInstanceId == -1 || _EdgeEntries[i].Interior.SurfaceId == -1 ||
			 _EdgeEntries[i].Exterior.RetrieverInstanceId == -1 || _EdgeEntries[i].Exterior.SurfaceId == -1))
		{
			nlwarning("In NLPACS::CEdgeQuad::build(): exterior door %d has corrupted link", i);
		}
		else if (edges[_EdgeEntries[i].EdgeId].Link != -1)
		{
			nlinfo("Inst=%d ExtEdge=%d IntInst=%d IntSurf=%d IntChain=%d ExtInst=%d ExtSurf=%d", thisInstance, _EdgeEntries[i].EdgeId,
				_EdgeEntries[i].Interior.RetrieverInstanceId, _EdgeEntries[i].Interior.SurfaceId, _EdgeEntries[i].ChainId,
				_EdgeEntries[i].Exterior.RetrieverInstanceId, _EdgeEntries[i].Exterior.SurfaceId);
		}
	}

	// 2. Mem optimisation: Use only 1 block for ALL quads of the grid.
	//=========================================
	sint	memSize= 0;
	// run all quads.
	for(i=0;i<(sint)tempQuad.size();i++)
	{
		list<uint16>	&quadNode= tempQuad[i];

		if(!quadNode.empty())
		{
			// add an entry for Len.
			memSize+= sizeof(uint16);
			// add N entry of CEdgeChainEntry.
			memSize+= (sint)quadNode.size()*sizeof(uint16);
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
		list<uint16>			&srcQuadNode= tempQuad[i];
		list<uint16>::iterator	it;

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
				*((uint16 *)ptr)= *it;
				ptr+= sizeof(uint16);
			}
		}
	}

	// End.
}


// ***************************************************************************
sint			CEdgeQuad::selectEdges(const NLMISC::CAABBox &bbox, CCollisionSurfaceTemp &cst) const
{
	sint	nRes=0;
	sint	i;
	uint16	*indexLUT = cst.OChainLUT;

	// start: no edge found.
	cst.ExteriorEdgeIndexes.clear();

	// get bounding coordinate of this bbox in the quadgrid.
	sint32	x0, y0, x1, y1;
	getGridBounds(x0, y0, x1, y1, bbox.getMin(), bbox.getMax());


	// run all intersected quads.
	for (sint y= y0; y<y1; y++)
	{
		for (sint x= x0; x<x1; x++)
		{
			uint8	*quadNode= _Quad[y*_Width+x];

			// no edgechain entry??
			if(!quadNode)
				continue;

			// get edgechain entries
			sint	numExteriorEdgeIndexes= *((uint16*)quadNode);
			quadNode+= sizeof(uint16);
			uint16	*ptrExteriorEdgeIndex= (uint16*)quadNode;

			// For each one, add it to the result list.
			for (i=0;i<numExteriorEdgeIndexes;i++)
			{
				uint16	index = ptrExteriorEdgeIndex[i];

				// if ochain not yet inserted.
				if (indexLUT[index]==0xFFFF)
				{
					// inc the list.
					indexLUT[index]= uint16(nRes);
					cst.ExteriorEdgeIndexes.push_back(index);
					nRes++;
				}
			}
		}
	}


	// reset LUT to 0xFFFF for all ochains selected.
	for(i=0;i<nRes;i++)
		indexLUT[cst.ExteriorEdgeIndexes[i]]= 0xFFFF;

	return nRes;
}

sint		CEdgeQuad::selectEdges(CVector start, CVector end, CCollisionSurfaceTemp &cst) const
{
	sint	nRes=0;
	sint	i;
	uint16	*indexLUT= cst.OChainLUT;

	// start: no edge found.
	cst.ExteriorEdgeIndexes.clear();

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
			sint	numExteriorEdgeIndexes= *((uint16 *)quadNode);
			quadNode+= sizeof(uint16);
			uint16	*ptrExteriorEdgeIndex = (uint16 *)quadNode;

			// For each one, add it to the result list.
			for(i=0;i<numExteriorEdgeIndexes;i++)
			{
				uint16	index = ptrExteriorEdgeIndex[i];

				// if ochain not yet inserted.
				if(indexLUT[index]==0xFFFF)
				{
					// inc the list.
					indexLUT[index]= uint16(nRes);
					cst.ExteriorEdgeIndexes.push_back(ptrExteriorEdgeIndex[i]);
					nRes++;
				}
			}
		}
	}

	// reset LUT to 0xFFFF for all ochains selected.
	for(i=0;i<nRes;i++)
		indexLUT[cst.ExteriorEdgeIndexes[i]]= 0xFFFF;

	return nRes;
}

// ***************************************************************************
void		CEdgeQuad::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);
	uint	i;

	// serial basics.
	f.serial(_X, _Y, _Width, _Height, _QuadDataLen);
	f.serialCont(_EdgeEntries);

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
