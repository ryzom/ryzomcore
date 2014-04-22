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

#include "nel/3d/stripifier.h"
// For now, don't use NVidia stripifier.
//#include "nv_tri_strip_objects.h"
#include <vector>
#include <deque>


using namespace std;

namespace NL3D
{


// ***************************************************************************
CStripifier::CStripifier()
{
}


// ***************************************************************************
/*
	NVidia(tm) 's method get better performance (8ms on 50K faces meshe, instead of 8.9ms), but
	precomputing is much slower (1'40 instead of 0'??  :)  ).
*/
/*void		CStripifier::optimizeTriangles(const CIndexBuffer &in, CIndexBuffer &out, uint cacheSize)
{
	NvStripifier	stripifier;
	WordVec			inIndices;
	NvStripInfoVec	outStrips;
	sint			i;

	// prepare inIndices.
	inIndices.resize(in.getNumTri()*3);
	for(i=0;i< (sint)inIndices.size(); i++)
	{
		inIndices[i]= in.getPtr()[i];
	}

	// build strips.
	stripifier.Stripify(inIndices, cacheSize, outStrips);

	// build triangles from strips, and release memory
	out.setNumTri(0);
	out.reserveTri(in.getNumTri());
	for(i= 0;i< (sint)outStrips.size(); i++)
	{
		NvStripInfo		*stripInfo= outStrips[i];

		// build triangle from the strip.
		for(uint j= 0;j< stripInfo->m_faces.size(); j++)
		{
			NvFaceInfo	*face= stripInfo->m_faces[j];
			out.addTri(face->m_v0, face->m_v1, face->m_v2);
			// delete this face.
			delete face;
		}

		// delete this strip.
		// Unref first the edges touched by this strip.
		NvEdgeInfo	*edgeInfo = stripInfo->m_startInfo.m_startEdge;
		while(edgeInfo)
		{
			NvEdgeInfo	*edgeInfoNext= edgeInfo->m_nextV1;
			edgeInfo->Unref();
			edgeInfo= edgeInfoNext;
		}
		// delete
		delete stripInfo;
	}

}*/

// ***************************************************************************
struct	CVertexCache
{
	CVertexCache(sint cacheSize, sint nbVerts)
	{
		_VertexInCache.resize(nbVerts, 0);
		_Cache.resize(cacheSize, 0xFFFFFFFF);
	}

	void	touchVertex(uint vert)
	{
		if(isVertexInCache(vert))
		{
			// do nothing ?????? depends of vcache implementation
		}
		else
		{
			// pop front
			uint	removed= _Cache.front();
			if(removed!=0xFFFFFFFF)
				_VertexInCache[removed]= 0;
			_Cache.pop_front();
			// push_back
			_VertexInCache[vert]= 3;
			_Cache.push_back(vert);
		}
	}

	bool	isVertexInCache(uint vert)
	{
		return _VertexInCache[vert]==3;
	}

	// return which vertex is at which place in the cache. 0xFFFFFFFF if the entry is empty
	uint	getVertexInCache(uint vertIdInCache)
	{
		return _Cache[vertIdInCache];
	}

	void	tempTouchVertex(uint vert, bool inCache)
	{
		if( _VertexInCache[vert]&1 )
		{
			if(inCache)
				_VertexInCache[vert]|= 2;
			else
				_VertexInCache[vert]&= 1;
		}
	}

private:
	// 0 if not in the cache
	vector<uint8>		_VertexInCache;
	deque<uint32>		_Cache;

};


// ***************************************************************************
struct	COrderFace
{
	sint	v[3];
	bool	Inserted;

	void	insertInPB(CIndexBuffer &out, CVertexCache &vertexCache)
	{
		uint index = out.getNumIndexes ();
		out.setNumIndexes (index+3);
		CIndexBufferReadWrite ibaWrite;
		out.lock (ibaWrite);
		ibaWrite.setTri(index, v[0], v[1], v[2]);
		vertexCache.touchVertex(v[0]);
		vertexCache.touchVertex(v[1]);
		vertexCache.touchVertex(v[2]);
		Inserted= true;
	}

	sint	countCacheMiss(CVertexCache &vertexCache)
	{
		sint	ret=0 ;
		if(!vertexCache.isVertexInCache(v[0]))	ret++;
		if(!vertexCache.isVertexInCache(v[1]))	ret++;
		if(!vertexCache.isVertexInCache(v[2]))	ret++;
		return ret;
	}
};


// ***************************************************************************
struct	CCornerNode
{
	// next in the list
	CCornerNode		*Next;
	// corner == tuple face/vertex.
	uint			FaceId;
	uint			VertexId;
};


// ***************************************************************************
void		CStripifier::optimizeTriangles(const CIndexBuffer &in, CIndexBuffer &out, uint cacheSize)
{
	vector<COrderFace>	inFaces;
	sint			i;
	sint			numTris= in.getNumIndexes()/3;

	// TestYoyo: All the same tri => perfect vertex caching...
	/*out.setNumTri(numTris);
	for(i=0;i< numTris; i++)
	{
		uint32	v0= *(in.getPtr()+0);
		uint32	v1= *(in.getPtr()+1);
		uint32	v2= *(in.getPtr()+2);
		out.setTri(i, v0, v1, v2);
	}
	return;*/


	// prepare inIndices.
	//--------------------
	{
		CIndexBufferRead ibaRead;
		in.lock (ibaRead);
		inFaces.resize(numTris);
		if (ibaRead.getFormat() == CIndexBuffer::Indices32)
		{
			for(i=0;i< numTris; i++)
			{
				const uint32 *ibaPtr = (const uint32 *) ibaRead.getPtr();
				inFaces[i].v[0]= ibaPtr[i*3 + 0];
				inFaces[i].v[1]= ibaPtr[i*3 + 1];
				inFaces[i].v[2]= ibaPtr[i*3 + 2];
				inFaces[i].Inserted= false;
			}
		}
		else
		{
			nlassert(ibaRead.getFormat() == CIndexBuffer::Indices16);
			for(i=0;i< numTris; i++)
			{
				const uint16 *ibaPtr = (const uint16 *) ibaRead.getPtr();
				inFaces[i].v[0]= ibaPtr[i*3 + 0];
				inFaces[i].v[1]= ibaPtr[i*3 + 1];
				inFaces[i].v[2]= ibaPtr[i*3 + 2];
				inFaces[i].Inserted= false;
			}
		}
	}


	// build our cache, and compute max number of vertices.
	//--------------------
	int	numVerts=0;
	for (i = 0; i < numTris; i++)
	{
		numVerts= max(numVerts, (int)inFaces[i].v[0]);
		numVerts= max(numVerts, (int)inFaces[i].v[1]);
		numVerts= max(numVerts, (int)inFaces[i].v[2]);
	}
	numVerts++;
	CVertexCache	vertexCache(cacheSize, numVerts);


	// Compute vertex connectivity.
	//--------------------
	vector<CCornerNode*>	vertexConnectivity;
	vector<CCornerNode>		cornerAllocator;
	cornerAllocator.resize(numTris * 3);
	vertexConnectivity.resize(numVerts, NULL);
	// For all triangles.
	for (i = 0; i < numTris; i++)
	{
		COrderFace	*ordFace= &inFaces[i];
		// For each corner, allocate and fill
		for(sint j=0; j<3;j++)
		{
			sint	vertexId= ordFace->v[j];

			// allocate a corner
			CCornerNode		*corner= &cornerAllocator[i*3 + j];

			// fill it.
			corner->FaceId= i;
			corner->VertexId= vertexId;
			// Link it to the vertex list of faces.
			corner->Next= vertexConnectivity[vertexId];
			vertexConnectivity[vertexId]= corner;
		}
	}


	// build output optimized triangles
	//--------------------
	out.setFormat(in.getFormat());
	out.setNumIndexes(0);
	out.reserve(3*numTris);

	for(i=0; i<numTris; i++)
	{
		// force insertion of the ith face.
		sint	nextToInsert= i;
		bool	nextToInsertFound= true;
		while( nextToInsertFound )
		{
			nextToInsertFound= false;

			// if the face is not yet inserted.
			if(!inFaces[nextToInsert].Inserted)
			{
				// must insert this face.
				inFaces[nextToInsert].insertInPB(out, vertexCache);

				sint	minC= 3;

				// look only for faces which use vertices in VertexCache, to get a face with at least one vertex.
				for(uint j=0; j<cacheSize; j++)
				{
					// get a vertex from the vertex cache.
					uint	vertexId= vertexCache.getVertexInCache(j);
					// if empty entry
					if(vertexId==0xFFFFFFFF)
						continue;

					// parse list of faces which use this vertex.
					CCornerNode		*corner= vertexConnectivity[vertexId];
					while(corner)
					{
						uint	faceId= corner->FaceId;

						// if the face is not yet inserted.
						if(!inFaces[faceId].Inserted)
						{
							sint	c= inFaces[faceId].countCacheMiss(vertexCache);
							// insert first any face which don't add any vertex in the cache.
							if(c==0)
							{
								inFaces[faceId].insertInPB(out, vertexCache);
							}
							// else the one which add the minimum of vertex possible: nextToInsert
							else
							{
								// Add cost of faces that use vertices pushed out (better results...)
								uint	numVOut= c;
								uint	k;
								for(k=cacheSize-numVOut; k<cacheSize; k++)
								{
									uint	vertexOutId= vertexCache.getVertexInCache(k);
									if(vertexOutId==0xFFFFFFFF)
										continue;
									// TempRemove the vertex from the cache
									vertexCache.tempTouchVertex(vertexOutId, false);
								}
								// parse all faces that still use those out vertices.
								for(k=cacheSize-numVOut; k<cacheSize; k++)
								{
									uint	vertexOutId= vertexCache.getVertexInCache(k);
									if(vertexOutId==0xFFFFFFFF)
										continue;
									CCornerNode		*cornerOut= vertexConnectivity[vertexOutId];
									while(cornerOut)
									{
										uint	faceOutId= cornerOut->FaceId;

										// if the face is not yet inserted AND not the one treated
										if(!inFaces[faceOutId].Inserted && faceOutId!=faceId)
										{
											// Add cache miss of this face
											c+= inFaces[faceOutId].countCacheMiss(vertexCache);
										}

										// next corner
										cornerOut= cornerOut->Next;
									}
								}
								// reset touch
								for(k=cacheSize-numVOut; k<cacheSize; k++)
								{
									uint	vertexOutId= vertexCache.getVertexInCache(k);
									if(vertexOutId==0xFFFFFFFF)
										continue;
									// restore TempTouch the vertex from the cache
									vertexCache.tempTouchVertex(vertexOutId, true);
								}


								// take the minimum cost
								if(c<minC)
								{
									nextToInsert= faceId;
									nextToInsertFound= true;
									minC= c;
								}
							}
						}

						// next corner
						corner= corner->Next;
					}

				}

				// if nextToInsertFound, then nextToInsert has the face which add the minimum of vertex possible in the cache
			}
		}

	}

}


} // NL3D
