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

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/polygon.h"
#include "nel/3d/shadow_poly_receiver.h"
#include "nel/3d/shadow_map.h"
#include "nel/3d/driver.h"
#include "nel/3d/camera_col.h"


using	namespace std;
using	namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


// ***************************************************************************
CShadowPolyReceiver::CShadowPolyReceiver(uint quadGridSize, float quadGridCellSize)
{
	_Vertices.reserve(64);
	_FreeVertices.reserve(64);
	_FreeTriangles.reserve(64);
	_Triangles.reserve(64);

	_TriangleGrid.create(quadGridSize, quadGridCellSize);

	_VB.setVertexFormat(CVertexBuffer::PositionFlag);
	_VB.setName("CShadowPolyReceiver");
	// lock volatile, to avoid cpu stall when rendering multiple shadows in the same polyReceiver
	_VB.setPreferredMemory(CVertexBuffer::RAMVolatile, false);
	_RenderTriangles.setPreferredMemory(CIndexBuffer::RAMVolatile, false);
	_RenderTriangles.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	NL_SET_IB_NAME(_RenderTriangles, "CShadowPolyReceiver");
}


// ***************************************************************************
uint			CShadowPolyReceiver::addTriangle(const NLMISC::CTriangle &tri)
{
	uint	id;

	// Look for a free triangle entry.
	if(_FreeTriangles.empty())
	{
		_Triangles.push_back(TTriangleGrid::CIterator());
		id= (uint)_Triangles.size()-1;
		// enlarge render size.
		_RenderTriangles.setNumIndexes((uint32)_Triangles.size() * 3);
	}
	else
	{
		id= _FreeTriangles.back();
		_FreeTriangles.pop_back();
	}

	// Allocate vertices, reusing same ones.
	CTriangleId		triId;
	CVector			v[3];
	v[0]= tri.V0;
	v[1]= tri.V1;
	v[2]= tri.V2;
	for(uint i=0;i<3;i++)
	{
		// Find the vertex in the map.
		TVertexMap::iterator	it;
		it= _VertexMap.find(v[i]);
		// if not found, allocate it
		if(it==_VertexMap.end())
		{
			triId.Vertex[i]= allocateVertex(v[i]);
		}
		// else get its id
		else
		{
			triId.Vertex[i]= it->second;
		}

		// increment the reference of this vertex
		incVertexRefCount(triId.Vertex[i]);
	}

	// Insert the triangle in the quadGrid.
	CAABBox		bb;
	bb.setCenter(tri.V0);
	bb.extend(tri.V1);
	bb.extend(tri.V2);
	// insert in QuadGrid and store iterator for future remove
	_Triangles[id]= _TriangleGrid.insert(bb.getMin(), bb.getMax(), triId);

	return id;
}

// ***************************************************************************
void			CShadowPolyReceiver::removeTriangle(uint id)
{
	nlassert(id<_Triangles.size());
	// Must not be NULL iterator.
	nlassert(_Triangles[id]!=_TriangleGrid.end());

	// Release Vertices
	const CTriangleId		&triId= *_Triangles[id];
	releaseVertex(triId.Vertex[0]);
	releaseVertex(triId.Vertex[1]);
	releaseVertex(triId.Vertex[2]);

	// Delete Triangle.
	_TriangleGrid.erase(_Triangles[id]);
	_Triangles[id]= _TriangleGrid.end();
	// Append to free list.
	_FreeTriangles.push_back(id);
}


// ***************************************************************************
uint			CShadowPolyReceiver::allocateVertex(const CVector &v)
{
	uint	id;

	// if not valid double, will crash cause map<float,...> crash when float are not valid
	nlassert(isValidDouble(v.x) && isValidDouble(v.y) && isValidDouble(v.z));

	// Look for a free vertex entry.
	if(_FreeVertices.empty())
	{
		// Add the vertex, and init refCount to 0.
		_Vertices.push_back(v);
		id= (uint)_Vertices.size()-1;

		// Resize the VBuffer at max possible
		_VB.setNumVertices((uint32)_Vertices.size());
	}
	else
	{
		id= _FreeVertices.back();
		_FreeVertices.pop_back();
		// init entry
		_Vertices[id]= v;
		_Vertices[id].RefCount= 0;
	}

	// insert in the map (should not be here)
	_VertexMap.insert( make_pair(v, id) );

	return id;
}

// ***************************************************************************
void			CShadowPolyReceiver::releaseVertex(uint id)
{
	nlassert(id<_Vertices.size());
	// dec ref
	nlassert(_Vertices[id].RefCount>0);
	_Vertices[id].RefCount--;
	// no more used?
	if(_Vertices[id].RefCount==0)
	{
		// Free it.
		_FreeVertices.push_back(id);
		// Remove it from map.
		TVertexMap::iterator	it= _VertexMap.find(_Vertices[id]);
		if (it!=_VertexMap.end())
			_VertexMap.erase(it);
		else
			nlwarning("vertex %u doesn't exist in _VertexMap, this should not happen", id);
	}
}

// ***************************************************************************
void			CShadowPolyReceiver::incVertexRefCount(uint id)
{
	nlassert(id<_Vertices.size());
	nlassert(_Vertices[id].RefCount < NL3D_SPR_MAX_REF_COUNT);
	_Vertices[id].RefCount++;
}



// ***************************************************************************
inline void	CShadowPolyReceiver::renderSelection(IDriver *drv, CMaterial &shadowMat, const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta)
{
	uint	i, j;
	// For all triangles, reset vertices flags.
	TTriangleGrid::CIterator	it;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		for(i=0;i<3;i++)
		{
			_Vertices[triId.Vertex[i]].Flags= 0;
			_Vertices[triId.Vertex[i]].VBIdx= -1;
		}
	}

	// Compute the world Clip Volume
	static	std::vector<CPlane>		worldClipPlanes;
	CMatrix		worldMat;
	// set -casterPos, because to transform a plane, we must do plane * M-1
	worldMat.setPos(-casterPos);
	// Allow max bits of planes clip.
	worldClipPlanes.resize(min((uint)shadowMap->LocalClipPlanes.size(), (uint)NL3D_SPR_NUM_CLIP_PLANE));
	// Transform into world
	for(i=0;i<worldClipPlanes.size();i++)
	{
		worldClipPlanes[i]= shadowMap->LocalClipPlanes[i] * worldMat;
	}

	uint	currentTriIdx= 0;
	{
		// Volatile: must resize before lock
		_VB.setNumVertices((uint32)_Vertices.size());
		_RenderTriangles.setNumIndexes((uint32)_Triangles.size() * 3);

		// lock volatile, to avoid cpu stall
		CVertexBufferReadWrite vba;
		_VB.lock(vba);
		CIndexBufferReadWrite iba;
		_RenderTriangles.lock (iba);
		TIndexType *triPtr = (TIndexType *) iba.getPtr();

		// For All triangles, clip them.
		uint	currentVbIdx= 0;
		for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
		{
			CTriangleId		&triId= *it;
			uint			triFlag= NL3D_SPR_NUM_CLIP_PLANE_MASK;

			// for all vertices, clip them
			for(i=0;i<3;i++)
			{
				uint	vid= triId.Vertex[i];
				uint	vertexFlags= _Vertices[vid].Flags;

				// if this vertex is still not computed
				if(!vertexFlags)
				{
					// For all planes of the Clip Volume, clip this vertex.
					for(j=0;j<worldClipPlanes.size();j++)
					{
						// out if in front
						bool	out= worldClipPlanes[j]*_Vertices[vid] > 0;

						vertexFlags|= ((uint)out)<<j;
					}

					// add the bit flag to say "computed".
					vertexFlags|= NL3D_SPR_NUM_CLIP_PLANE_SHIFT;

					// store
					_Vertices[vid].Flags= vertexFlags;
				}

				// And all vertex bits.
				triFlag&= vertexFlags;
			}

			// if triangle not clipped, add the triangle
			if( (triFlag & NL3D_SPR_NUM_CLIP_PLANE_MASK)==0 )
			{
				// Add the 3 vertices to the VB, and to the index buffer.
				for(i=0;i<3;i++)
				{
					uint	vid= triId.Vertex[i];
					sint	vbId= _Vertices[vid].VBIdx;

					// if not yet inserted in the VB, do it.
					if(vbId==-1)
					{
						// allocate a new place in the VBuffer
						vbId= currentVbIdx++;
						_Vertices[vid].VBIdx= vbId;
						// set the coord
						vba.setVertexCoord(vbId, _Vertices[vid]+vertDelta);
					}

					// add the index to the tri list.
					triPtr[currentTriIdx++]= (TIndexType) vbId;
				}
			}
		}
	}


	// **** Render
	drv->activeVertexBuffer(_VB);
	drv->activeIndexBuffer(_RenderTriangles);
	drv->renderTriangles(shadowMat, 0, currentTriIdx/3);
	// TestYoyo. Show in Red triangles selected
	/*static	CMaterial	tam;
	tam.initUnlit();
	tam.setColor(CRGBA(255,0,0,128));
	tam.setZFunc(CMaterial::always);
	tam.setZWrite(false);
	tam.setBlend(true);
	tam.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	tam.setDoubleSided(true);
	drv->renderTriangles(tam, &_RenderTriangles[0], currentTriIdx/3);*/
}

// ***************************************************************************
void CShadowPolyReceiver::computeClippedTrisWithPolyClip(const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta, const NLMISC::CPolygon2D &poly, std::vector<CRGBAVertex> &destTris, bool colorUpfacingVertices)
{
	nlctassert(sizeof(CRGBAVertex) == 12 + 4); // ensure padding works as expected
	destTris.clear();
	selectPolygon(poly);
	if (_TriangleGrid.begin() == _TriangleGrid.end()) return;
	uint	i, j;


	static	std::vector<CVector>	vertexNormals; // normal for each vertex
	static	std::vector<uint8>	vertexNormalsUndefined; // normal for each vertex
	static  std::vector<CTriangleId *> visibleTris; // triangles that passed the clip
	vertexNormals.resize(_Vertices.size());
	vertexNormalsUndefined.resize(_Vertices.size(), 1);
	visibleTris.clear();

	// For all triangles, reset vertices flags.
	TTriangleGrid::CIterator	it;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		for(i=0;i<3;i++)
		{
			_Vertices[triId.Vertex[i]].Flags= 0;
		}
	}

	// Compute the world Clip Volume
	static	std::vector<CPlane>		worldClipPlanes;
	CMatrix		worldMat;
	// set -casterPos, because to transform a plane, we must do plane * M-1
	worldMat.setPos(-casterPos);
	// Allow max bits of planes clip.
	worldClipPlanes.resize(min((uint)shadowMap->LocalClipPlanes.size(), (uint)NL3D_SPR_NUM_CLIP_PLANE));
	// Transform into world
	for(i=0;i<worldClipPlanes.size();i++)
	{
		worldClipPlanes[i]= shadowMap->LocalClipPlanes[i] * worldMat;
	}

	static NLMISC::CPolygon clippedTri;

	CVector triNormal;


	// For All triangles, clip them.
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		uint			triFlag= NL3D_SPR_NUM_CLIP_PLANE_MASK;


		CVectorId *vid[3] = { &_Vertices[triId.Vertex[0]],
							  &_Vertices[triId.Vertex[1]],
							  &_Vertices[triId.Vertex[2]]
							};

		// for all vertices, clip them
		for(i=0;i<3;i++)
		{

			// if this vertex is still not computed
			if(!vid[i]->Flags)
			{
				// For all planes of the Clip Volume, clip this vertex.
				for(j=0;j<worldClipPlanes.size();j++)
				{
					// out if in front
					bool	out= worldClipPlanes[j]* *vid[i] > 0;

					vid[i]->Flags |= ((uint)out)<<j;
				}
				// add the bit flag to say "computed".
				vid[i]->Flags |= NL3D_SPR_NUM_CLIP_PLANE_SHIFT;
			}

			vertexNormalsUndefined[triId.Vertex[i]] = 1; // invalidate normal for next pass

			// And all vertex bits.
			triFlag&= vid[i]->Flags;
		}
		// if triangle not clipped, do finer clip then add resulting triangles
		if( (triFlag & NL3D_SPR_NUM_CLIP_PLANE_MASK)==0 )
		{
			visibleTris.push_back(&triId);
		}
	}

	uint numVisibleTris = (uint)visibleTris.size();
	// compute normals if needed
	if (colorUpfacingVertices)
	{
		for (uint triIndex = 0; triIndex < numVisibleTris; ++triIndex)
		{
			CTriangleId		&triId= *visibleTris[triIndex];
			CVectorId *vid[3] = { &_Vertices[triId.Vertex[0]],
								  &_Vertices[triId.Vertex[1]],
								  &_Vertices[triId.Vertex[2]]
								};
			// compute normal for this tri
			triNormal = ((*vid[1] - *vid[0]) ^ (*vid[2] - *vid[0])).normed();

			// for all vertices, clip them
			for(i=0;i<3;i++)
			{
				sint vertIndex = triId.Vertex[i];
				if (vertexNormalsUndefined[vertIndex])
				{
					vertexNormals[vertIndex] = triNormal;
					vertexNormalsUndefined[vertIndex] = 0;
				}
				else
				{
					vertexNormals[vertIndex] += triNormal;
				}
			}
		}
	}


	if (colorUpfacingVertices)
	{
		for (uint triIndex = 0; triIndex < numVisibleTris; ++triIndex)
		{
			CTriangleId		&triId= *visibleTris[triIndex];
			// use CPlane 'uv cliping', store 'color' in 'U'
			static std::vector<CVector> corner0;
			static std::vector<CVector> corner1;
			static std::vector<CUV> uv0;
			static std::vector<CUV> uv1;
			uv0.resize(3 + worldClipPlanes.size());
			uv1.resize(3 + worldClipPlanes.size());
			corner0.resize(3 + worldClipPlanes.size());
			corner1.resize(3 + worldClipPlanes.size());
			//
			corner0[0] = _Vertices[triId.Vertex[0]];
			corner0[1] = _Vertices[triId.Vertex[1]];
			corner0[2] = _Vertices[triId.Vertex[2]];
			//
			uv0[0].set(vertexNormals[triId.Vertex[0]].z >= 0.f ? 1.f : 0.f, 0.f);
			uv0[1].set(vertexNormals[triId.Vertex[1]].z >= 0.f ? 1.f : 0.f, 0.f);
			uv0[2].set(vertexNormals[triId.Vertex[2]].z >= 0.f ? 1.f : 0.f, 0.f);
			//
			sint numVerts = 3;
			//
			for (uint k = 0; k < worldClipPlanes.size(); ++k)
			{
				numVerts = worldClipPlanes[k].clipPolygonBack(&corner0[0], &uv0[0], &corner1[0], &uv1[0], numVerts);
				nlassert(numVerts <= (sint) corner1.size());
				if (numVerts == 0) break;
				uv0.swap(uv1);
				corner0.swap(corner1);
			}
			for (sint k = 0; k < numVerts - 2; ++k)
			{
				uint8 alpha[3] =
				{
					(uint8) (255.f * uv0[0].U),
					(uint8) (255.f * uv0[k + 1].U),
					(uint8) (255.f * uv0[k + 2].U)
				};
				if (alpha[0] != 0 || alpha[1] != 0 || alpha[2] != 0)
				{
					destTris.push_back(CRGBAVertex(corner0[0] + vertDelta, CRGBA(255, 255, 255, alpha[0])));
					destTris.push_back(CRGBAVertex(corner0[k + 1] + vertDelta, CRGBA(255, 255, 255, alpha[1])));
					destTris.push_back(CRGBAVertex(corner0[k + 2] + vertDelta, CRGBA(255, 255, 255, alpha[2])));
				}
			}
		}
	}
	else
	{
		for (uint triIndex = 0; triIndex < numVisibleTris; ++triIndex)
		{
			CTriangleId		&triId= *visibleTris[triIndex];
			clippedTri.Vertices.resize(3);
			clippedTri.Vertices[0] = _Vertices[triId.Vertex[0]];
			clippedTri.Vertices[1] = _Vertices[triId.Vertex[1]];
			clippedTri.Vertices[2] = _Vertices[triId.Vertex[2]];
			clippedTri.clip(worldClipPlanes);
			if (clippedTri.Vertices.size() >= 3)
			{
				for(uint k = 0; k < clippedTri.Vertices.size() - 2; ++k)
				{
					destTris.push_back(CRGBAVertex(clippedTri.Vertices[0] + vertDelta, CRGBA::White));
					destTris.push_back(CRGBAVertex(clippedTri.Vertices[k + 1] + vertDelta,	CRGBA::White));
					destTris.push_back(CRGBAVertex(clippedTri.Vertices[k + 2] + vertDelta,	CRGBA::White));
				}
			}
		}
	}

}

// ***************************************************************************
void			CShadowPolyReceiver::render(IDriver *drv, CMaterial &shadowMat, const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta)
{

	// **** Fill Triangles that are hit by the Caster
	// First select with quadGrid
	CAABBox		worldBB;
	worldBB= shadowMap->LocalBoundingBox;
	worldBB.setCenter(worldBB.getCenter() + casterPos);
	_TriangleGrid.select(worldBB.getMin(), worldBB.getMax());
	if (_TriangleGrid.begin() == _TriangleGrid.end()) return;
	renderSelection(drv, shadowMat, shadowMap, casterPos, vertDelta);
}


// ***************************************************************************
void CShadowPolyReceiver::selectPolygon(const NLMISC::CPolygon2D &poly)
{
	static TTriangleGrid::TSelectionShape selectionShape;
	_TriangleGrid.buildSelectionShape(selectionShape, poly);
	_TriangleGrid.select(selectionShape);
}

// ***************************************************************************
void			CShadowPolyReceiver::renderWithPolyClip(IDriver *drv, CMaterial &shadowMat, const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta, const NLMISC::CPolygon2D &poly)
{

	selectPolygon(poly);
	renderSelection(drv, shadowMat, shadowMap, casterPos, vertDelta);
}

// ***************************************************************************
float			CShadowPolyReceiver::getCameraCollision(const CVector &start, const CVector &end, TCameraColTest testType, float radius)
{
	// **** build the camera collision info
	CCameraCol	camCol;
	if(testType==CameraColSimpleRay)
		camCol.buildRay(start, end);
	else
		camCol.build(start, end, radius, testType==CameraColCone);

	// select with quadGrid
	if(testType==CameraColSimpleRay)
	{
		_TriangleGrid.selectRay(start, end);
	}
	else
	{
		_TriangleGrid.select(camCol.getBBox().getMin(), camCol.getBBox().getMax());
	}

	// **** For all triangles, test if intersect the camera collision
	TTriangleGrid::CIterator	it;
	float		sqrMinDist= FLT_MAX;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		camCol.minimizeDistanceAgainstTri(
			_Vertices[triId.Vertex[0]],
			_Vertices[triId.Vertex[1]],
			_Vertices[triId.Vertex[2]],
			sqrMinDist);
	}

	// **** return the collision found, between [0,1]
	if(sqrMinDist == FLT_MAX)
		return 1;
	else
	{
		float	f= 1;
		float	d= camCol.getRayLen();
		if(d>0)
		{
			f= sqrtf(sqrMinDist) / d;
			f= min(f, 1.f);
		}
		return f;
	}
}





} // NL3D
