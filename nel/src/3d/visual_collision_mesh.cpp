// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/visual_collision_mesh.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/camera_col.h"
#include "nel/3d/driver.h"
#include "nel/3d/shadow_map.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// TestYoyo. external debug flag
bool	TESTYOYO_VCM_RedShadow= false;


namespace NL3D
{


// ***************************************************************************
#define		NL3D_VCM_SHADOW_NUM_CLIP_PLANE			7
#define		NL3D_VCM_SHADOW_NUM_CLIP_PLANE_SHIFT	(1<<NL3D_VCM_SHADOW_NUM_CLIP_PLANE)
#define		NL3D_VCM_SHADOW_NUM_CLIP_PLANE_MASK		(NL3D_VCM_SHADOW_NUM_CLIP_PLANE_SHIFT-1)


// ***************************************************************************
// ***************************************************************************
// CStaticGrid
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CVisualCollisionMesh::CStaticGrid::create(uint nbQuads, uint nbElts, const NLMISC::CAABBox &gridBBox)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	nlassert(nbQuads>0 && isPowerOf2(nbQuads));

	// init the grid
	_GridSize= nbQuads;
	_GridSizePower= getPowerOf2(nbQuads);
	_Grid.resize(_GridSize*_GridSize);
	// start with 0 elt in each case
	memset(_Grid.getPtr(), 0, _Grid.size() * sizeof(CCase));

	// init the Elt Build
	_EltBuild.resize(nbElts);

	// total size is 0
	_GridDataSize= 0;

	// bbox init
	_GridPos= gridBBox.getMin();
	_GridScale= gridBBox.getSize();
	_GridScale.x= _GridSize / _GridScale.x;
	_GridScale.y= _GridSize / _GridScale.y;

	// reset intersection data
	_ItSession= 0;
}

// ***************************************************************************
void	CVisualCollisionMesh::CStaticGrid::add(uint16 id, const NLMISC::CAABBox &bbox)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	CVector	minp= bbox.getMin() - _GridPos;
	CVector	maxp= bbox.getMax() - _GridPos;

	// compute the 2D bbox
	sint	xmin= (sint)floorf(minp.x*_GridScale.x);
	sint	ymin= (sint)floorf(minp.y*_GridScale.y);
	sint	xmax= (sint)ceilf(maxp.x*_GridScale.x);
	sint	ymax= (sint)ceilf(maxp.y*_GridScale.y);
	clamp(xmin, 0, (sint)_GridSize-1);
	clamp(ymin, 0, (sint)_GridSize-1);
	clamp(xmax, xmin+1, (sint)_GridSize);
	clamp(ymax, ymin+1, (sint)_GridSize);

	// set in the elt build
	_EltBuild[id].X0= xmin;
	_EltBuild[id].Y0= ymin;
	_EltBuild[id].X1= xmax;
	_EltBuild[id].Y1= ymax;

	// for each case touched, increment NumElts
	for(uint y=ymin;y<(uint)ymax;y++)
	{
		for(uint x=xmin;x<(uint)xmax;x++)
		{
			_Grid[(y<<_GridSizePower)+x].NumElts++;
			_GridDataSize++;
		}
	}
}

// ***************************************************************************
void	CVisualCollisionMesh::CStaticGrid::compile()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	uint	i;

	// create the data
	_GridData.resize(_GridDataSize);

	// Init Start ptr for each case
	uint	idx= 0;
	for(i=0;i<_Grid.size();i++)
	{
		_Grid[i].Start= idx;
		idx+= _Grid[i].NumElts;
		// reset NumElts, because use it like an index below
		_Grid[i].NumElts= 0;
	}
	nlassert(_GridDataSize==idx);

	// For each element, fill the grid and grid data
	for(i=0;i<_EltBuild.size();i++)
	{
		CEltBuild	&eb= _EltBuild[i];

		for(uint y=eb.Y0;y<eb.Y1;y++)
		{
			for(uint x=eb.X0;x<eb.X1;x++)
			{
				CCase	&gcase= _Grid[(y<<_GridSizePower)+x];
				uint	idx= gcase.Start + gcase.NumElts;
				// store the idx
				_GridData[idx]= i;
				// increment the number of elements for this case
				gcase.NumElts++;
			}
		}
	}

	// create the temp array used for intersection test
	_Sessions.resize(_EltBuild.size());
	_Sessions.fill(0);

	// clear no more needed data
	_EltBuild.clear();
}

// ***************************************************************************
uint	CVisualCollisionMesh::CStaticGrid::select(const NLMISC::CAABBox &bbox, std::vector<uint16> &res)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// increment the intersection session
	_ItSession++;
	// enlarge the result array as needed
	if(res.size()<_Sessions.size())
		res.resize(_Sessions.size());
	// the number of selected element
	uint	numSel= 0;

	// compute the 2D bbox
	CVector	minp= bbox.getMin() - _GridPos;
	CVector	maxp= bbox.getMax() - _GridPos;
	sint	xmin= (sint)floorf(minp.x*_GridScale.x);
	sint	ymin= (sint)floorf(minp.y*_GridScale.y);
	sint	xmax= (sint)ceilf(maxp.x*_GridScale.x);
	sint	ymax= (sint)ceilf(maxp.y*_GridScale.y);
	clamp(xmin, 0, (sint)_GridSize-1);
	clamp(ymin, 0, (sint)_GridSize-1);
	clamp(xmax, xmin+1, (sint)_GridSize);
	clamp(ymax, ymin+1, (sint)_GridSize);

	// for each case touched, increment NumElts
	for(uint y=ymin;y<(uint)ymax;y++)
	{
		for(uint x=xmin;x<(uint)xmax;x++)
		{
			CCase	&gcase= _Grid[(y<<_GridSizePower)+x];
			// for each element in this case
			for(uint i= gcase.Start;i<gcase.Start + gcase.NumElts;i++)
			{
				uint	elt= _GridData[i];

				// if not alread inserted in the dest
				if(_Sessions[elt]!=_ItSession)
				{
					// mark as intersected
					_Sessions[elt]= _ItSession;
					// append
					res[numSel++]= elt;
				}
			}
		}
	}

	// return the number of selected elements
	return numSel;
}


// ***************************************************************************
// ***************************************************************************
// CVisualCollisionMesh
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CVisualCollisionMesh::CVisualCollisionMesh()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

}

// ***************************************************************************
bool					CVisualCollisionMesh::build(const std::vector<CVector> &vertices, const std::vector<uint32> &triangles, CVertexBuffer &vbForShadowRender)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	uint	i;
	// if no vertices, or no triangles, abort
	if(vertices.empty())
		return false;
	if(triangles.empty())
		return false;
	// vertices and triangles id are stored in uint16 form. so their should not be more than 65535*3 indices
	if(vertices.size()>65535 || triangles.size()>65535*3)
		return false;

	// copy
	_Vertices= vertices;

	// compress indexes to 16 bits
	_Triangles.resize(triangles.size());
	for(i=0;i<_Triangles.size();i++)
		_Triangles[i]= (uint16)triangles[i];

	// Build the Local bbox for this col mesh
	CAABBox		localBBox;
	localBBox.setCenter(vertices[0]);
	for(i=1;i<vertices.size();i++)
		localBBox.extend(vertices[i]);

	// Build the Static Grid
	uint	numTris= (uint)triangles.size()/3;
	_QuadGrid.create(16, numTris, localBBox);
	// Add all triangles
	for(i=0;i<numTris;i++)
	{
		CAABBox		bb;
		bb.setCenter(_Vertices[_Triangles[i*3+0]]);
		bb.extend(_Vertices[_Triangles[i*3+1]]);
		bb.extend(_Vertices[_Triangles[i*3+2]]);
		_QuadGrid.add(i, bb);
	}
	// compile
	_QuadGrid.compile();


	// Keep a RefPtr on the AGP vertex Buffer for shadow receiving
	_VertexBuffer= &vbForShadowRender;


	return true;
}

// ***************************************************************************
float					CVisualCollisionMesh::getCameraCollision(const CMatrix &instanceMatrix, CCameraCol &camCol)
{
	// Make the Camera Collision local to the mesh!
	CCameraCol	camColLocal;
	camColLocal.setApplyMatrix(camCol, instanceMatrix.inverted());


	// Select triangles
	static std::vector<uint16>		selection;
	uint	numSel= _QuadGrid.select(camColLocal.getBBox(), selection);

	// **** For all triangles, test if intersect the camera collision
	float		sqrMinDist= FLT_MAX;
	for(uint i=0;i<numSel;i++)
	{
		uint			triId= selection[i];
		// build the triangle
		camColLocal.minimizeDistanceAgainstTri(
			_Vertices[_Triangles[triId*3+0]],
			_Vertices[_Triangles[triId*3+1]],
			_Vertices[_Triangles[triId*3+2]],
			sqrMinDist);
	}

	// **** return the collision found, between [0,1]
	if(sqrMinDist == FLT_MAX)
		return 1;
	else
	{
		float	f= 1;
		float	d= camColLocal.getRayLen();
		if(d>0)
		{
			f= sqrtf(sqrMinDist) / d;
			f= min(f, 1.f);
		}
		return f;
	}
}

// ***************************************************************************
NLMISC::CAABBox			CVisualCollisionMesh::computeWorldBBox(const CMatrix &instanceMatrix)
{
	CAABBox		ret;
	if(!_Vertices.empty())
	{
		ret.setCenter(instanceMatrix*_Vertices[0]);
		for(uint i=1;i<_Vertices.size();i++)
		{
			ret.extend(instanceMatrix*_Vertices[i]);
		}
	}

	return ret;
}


// ***************************************************************************
void		CVisualCollisionMesh::receiveShadowMap(const NLMISC::CMatrix &instanceMatrix, const CShadowContext &shadowContext)
{
	// empty mesh => no op
	if(_Vertices.empty())
		return;

	// The VertexBuffer RefPtr has been released? quit
	if(_VertexBuffer == NULL)
		return;


	// **** Select triangles to be rendered with quadGrid
	// select with quadGrid local in mesh
	CAABBox		localBB;
	localBB= CAABBox::transformAABBox(instanceMatrix.inverted(), shadowContext.ShadowWorldBB);
	static	std::vector<uint16>		triInQuadGrid;
	uint	numTrisInQuadGrid= _QuadGrid.select(localBB, triInQuadGrid);

	// no intersection at all? quit
	if(numTrisInQuadGrid==0)
		return;


	// **** prepare more precise Clip with shadow pyramid
	// enlarge temp flag array
	static	std::vector<uint8>	vertexFlags;
	if(vertexFlags.size()<_Vertices.size())
		vertexFlags.resize(_Vertices.size());
	// reset all to 0
	memset(&vertexFlags[0], 0, _Vertices.size()*sizeof(uint8));

	// Compute the "LocalToInstance" shadow Clip Volume
	static	std::vector<CPlane>		localClipPlanes;
	/*	We want to apply to plane this matrix: IM-1 * MCasterPos,
		where IM=instanceMatrix and MCasterPos= matrix translation of "shadowContext.CasterPos"
		BUT, since to transform a plane, we must do plane * M-1, then compute this matrix:
		localMat= MCasterPos-1 * IM
	*/
	CMatrix		localMat;
	localMat.setPos(-shadowContext.CasterPos);
	localMat*= instanceMatrix;
	// Allow max bits of planes clip.
	localClipPlanes.resize(min((uint)shadowContext.ShadowMap->LocalClipPlanes.size(), (uint)NL3D_VCM_SHADOW_NUM_CLIP_PLANE));
	// Transform into Mesh local space
	for(uint i=0;i<localClipPlanes.size();i++)
	{
		localClipPlanes[i]= shadowContext.ShadowMap->LocalClipPlanes[i] * localMat;
	}


	// **** Clip and fill the triangles
	uint	currentTriIdx= 0;
	// enlarge the index buffer as max of triangles possibly intersected
	shadowContext.IndexBuffer.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	if(shadowContext.IndexBuffer.getNumIndexes()<numTrisInQuadGrid*3)
		shadowContext.IndexBuffer.setNumIndexes(numTrisInQuadGrid*3);

	// Start to clip and fill
	{
		CIndexBufferReadWrite	iba;
		shadowContext.IndexBuffer.lock(iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			uint32	*ibPtr= (uint32 *) iba.getPtr();
			// for all triangles selected with the quadgrid
			for(uint triq=0; triq<numTrisInQuadGrid;triq++)
			{
				uint			triId[3];
				triId[0]= _Triangles[uint(triInQuadGrid[triq])*3+0];
				triId[1]= _Triangles[uint(triInQuadGrid[triq])*3+1];
				triId[2]= _Triangles[uint(triInQuadGrid[triq])*3+2];
				uint			triFlag= NL3D_VCM_SHADOW_NUM_CLIP_PLANE_MASK;

				// for all vertices, clip them
				for(uint i=0;i<3;i++)
				{
					uint	vid= triId[i];
					uint	vf= vertexFlags[vid];

					// if this vertex is still not computed
					if(!vf)
					{
						// For all planes of the Clip Volume, clip this vertex.
						for(uint j=0;j<localClipPlanes.size();j++)
						{
							// out if in front
							bool	out= localClipPlanes[j]*_Vertices[vid] > 0;

							vf|= ((uint)out)<<j;
						}

						// add the bit flag to say "computed".
						vf|= NL3D_VCM_SHADOW_NUM_CLIP_PLANE_SHIFT;

						// store
						vertexFlags[vid]= vf;
					}

					// And all vertex bits.
					triFlag&= vf;
				}

				// if triangle not clipped, add the triangle
				if( (triFlag & NL3D_VCM_SHADOW_NUM_CLIP_PLANE_MASK)==0 )
				{
					// Add the 3 index to the index buffer.
					ibPtr[currentTriIdx++]= triId[0];
					ibPtr[currentTriIdx++]= triId[1];
					ibPtr[currentTriIdx++]= triId[2];
				}
			}
		}
		else
		{
			nlassert(iba.getFormat() == CIndexBuffer::Indices16);
			uint16	*ibPtr= (uint16 *) iba.getPtr();
			// for all triangles selected with the quadgrid
			for(uint triq=0; triq<numTrisInQuadGrid;triq++)
			{
				uint			triId[3];
				triId[0]= _Triangles[uint(triInQuadGrid[triq])*3+0];
				triId[1]= _Triangles[uint(triInQuadGrid[triq])*3+1];
				triId[2]= _Triangles[uint(triInQuadGrid[triq])*3+2];
				uint			triFlag= NL3D_VCM_SHADOW_NUM_CLIP_PLANE_MASK;

				// for all vertices, clip them
				for(uint i=0;i<3;i++)
				{
					uint	vid= triId[i];
					uint	vf= vertexFlags[vid];

					// if this vertex is still not computed
					if(!vf)
					{
						// For all planes of the Clip Volume, clip this vertex.
						for(uint j=0;j<localClipPlanes.size();j++)
						{
							// out if in front
							bool	out= localClipPlanes[j]*_Vertices[vid] > 0;

							vf|= ((uint)out)<<j;
						}

						// add the bit flag to say "computed".
						vf|= NL3D_VCM_SHADOW_NUM_CLIP_PLANE_SHIFT;

						// store
						vertexFlags[vid]= vf;
					}

					// And all vertex bits.
					triFlag&= vf;
				}

				// if triangle not clipped, add the triangle
				// if( (triFlag & NL3D_VCM_SHADOW_NUM_CLIP_PLANE_MASK)==0 )
				if (triFlag == 0) // previous line not useful due to init
				{
					// Add the 3 index to the index buffer.
					ibPtr[currentTriIdx++]= (uint16) triId[0];
					ibPtr[currentTriIdx++]= (uint16) triId[1];
					ibPtr[currentTriIdx++]= (uint16) triId[2];
				}
			}
		}
	}


	// **** Render
	// if some triangle to render
	if(currentTriIdx)
	{
		IDriver	*drv= shadowContext.Driver;
		// setup the collision instance matrix
		drv->setupModelMatrix(instanceMatrix);
		// update the material projection matrix, cause matrix changed
		shadowContext.ShadowMapProjector.applyToMaterial(instanceMatrix, shadowContext.ShadowMaterial);
		// render
		drv->activeVertexBuffer(*_VertexBuffer);
		drv->activeIndexBuffer(shadowContext.IndexBuffer);
		drv->renderTriangles(shadowContext.ShadowMaterial, 0, currentTriIdx/3);
		// TestYoyo. Show in Red triangles selected
		/*if(TESTYOYO_VCM_RedShadow)
		{
			static	CMaterial	tam;
			tam.initUnlit();
			tam.setColor(CRGBA(255,0,0,128));
			tam.setZFunc(CMaterial::always);
			tam.setZWrite(false);
			tam.setBlend(true);
			tam.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
			tam.setDoubleSided(true);
			drv->renderTriangles(tam, 0, currentTriIdx/3);
		}*/
	}
}


} // NL3D
