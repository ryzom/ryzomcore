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

#include "nel/3d/computed_string.h"
#include "nel/3d/texture.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/frustum.h"
#include "nel/3d/viewport.h"

#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"
#include "nel/misc/fast_mem.h"

using namespace std;

namespace NL3D {


/*------------------------------------------------------------------*\
							getHotSpotVector()
\*------------------------------------------------------------------*/
CVector CComputedString::getHotSpotVector(THotSpot hotspot)
{
	CVector hotspotVector(0,0,0);

	if (hotspot==MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);

	if (hotspot==TopLeft)
		hotspotVector = CVector(0,0,-StringHeight);

	if (hotspot==MiddleBottom)
		hotspotVector = CVector(-StringWidth/2,0,0);

	if (hotspot==MiddleMiddle)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight/2);

	if (hotspot==MiddleTop)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight);

	if (hotspot==BottomRight)
		hotspotVector = CVector(-StringWidth,0,0);

	if (hotspot==MiddleRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight/2);

	if (hotspot==TopRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight);

	return hotspotVector;
}


/*------------------------------------------------------------------*\
							render2D()
\*------------------------------------------------------------------*/
void CComputedString::render2D (IDriver& driver,
								float x, float z,
								THotSpot hotspot,
								float scaleX, float scaleZ,
								float rotateY,
								bool useScreenAR43, bool roundToNearestPixel
								)
{
	if (Vertices.getNumVertices() == 0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	CViewport vp;
	driver.getViewport(vp);
	wndWidth = (uint32)((float)wndWidth * vp.getWidth());
	wndHeight = (uint32)((float)wndHeight * vp.getHeight());
	// scale to window size.
	x*= wndWidth;
	z*= wndHeight;

	driver.setFrustum(0, (float)wndWidth, 0, (float)wndHeight, -1, 1, false);  // resX/resY

	// Computing hotspot translation vector
	CVector hotspotVector = getHotSpotVector(hotspot);

	// transformation matrix initialized to identity
	CMatrix matrix;
	matrix.identity();

	// view matrix <-> identity
	driver.setupViewMatrix(matrix);

	// model matrix :
	// centering to hotspot, then scaling, rotating, and translating.
	matrix.translate(CVector(x,0,z));
	matrix.rotateY(rotateY);
	matrix.scale(CVector(scaleX,1,scaleZ));
	// scale the string to follow window aspect Ratio
	if(useScreenAR43)
	{
		matrix.scale(CVector((3.0f*wndWidth)/(4.0f*wndHeight),1,1));
	}
	matrix.translate(hotspotVector);
	// if roundToNearestPixel, then snap the position to the nearest pixel
	if( roundToNearestPixel)
	{
		CVector	pos= matrix.getPos();
		pos.x= (float)floor(pos.x+0.5f);
		pos.z= (float)floor(pos.z+0.5f);
		matrix.setPos(pos);
	}
	// setup the matrix
	driver.setupModelMatrix(matrix);

	driver.activeVertexBuffer(Vertices);

	// rendering each primitives
	Material->setZFunc (CMaterial::always);
	Material->setZWrite (false);
	Material->setColor (Color);
	// Clamp for selection
	uint32	nNumQuad= Vertices.getNumVertices()/4;
	driver.renderRawQuads (*Material, SelectStart*4, min(nNumQuad, SelectSize) );
}


/*------------------------------------------------------------------*\
							render3D()
\*------------------------------------------------------------------*/
void CComputedString::render3D (IDriver& driver,CMatrix matrix,THotSpot hotspot)
{
	if (Vertices.getNumVertices() == 0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale according to window height (backward compatibility)
	matrix.scale(1.0f/wndHeight);

	// Computing hotspot translation vector
	CVector hotspotVector = getHotSpotVector(hotspot);
	matrix.translate(hotspotVector);

	// render
	driver.setupModelMatrix(matrix);
	driver.activeVertexBuffer(Vertices);

	// Rendering each primitive blocks
	Material->setZFunc (CMaterial::lessequal);
	Material->setZWrite (true);
	Material->setColor (Color);
	// Clamp for selection
	uint32	nNumQuad= Vertices.getNumVertices()/4;
	driver.renderRawQuads (*Material, SelectStart*4, min(nNumQuad, SelectSize) );
}


/*------------------------------------------------------------------*\
							render2DClip()
\*------------------------------------------------------------------*/
void CComputedString::render2DClip (IDriver& driver, CRenderStringBuffer &rdrBuffer,
					float x, float z,
					float xmin, float zmin, float xmax, float zmax)
{
	if (Vertices.getNumVertices() == 0)
		return;
	if(SelectSize==0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale to window size.
	x*= wndWidth;
	z*= wndHeight;
	xmin*= wndWidth;
	xmax*= wndWidth;
	zmin*= wndHeight;
	zmax*= wndHeight;

	// Test String Bound against clip window
	// If entirely out skip
	if (((x+XMin) > xmax) || ((x+XMax) < xmin) ||
		((z+ZMin) > zmax) || ((z+ZMax) < zmin))
		return;

	// test if entirely in.
	bool	allIn;
	allIn=	((x+XMin) >= xmin) && ((x+XMax) <= xmax) &&
			((z+ZMin) >= zmin) && ((z+ZMax) <= zmax);


	// How many quad to render?
	uint	nNumQuadSrc= Vertices.getNumVertices()/4;
	nNumQuadSrc= min(nNumQuadSrc, (uint)SelectSize);

	// Enlarge dest Buffer if needed
	if( (rdrBuffer.NumQuads+nNumQuadSrc)*4 > rdrBuffer.Vertices.getNumVertices() )
	{
		rdrBuffer.Vertices.setNumVertices( (rdrBuffer.NumQuads+nNumQuadSrc)*4 );
	}

	// prepare copy.
	CVertexBuffer::TVertexColorType vtype = driver.getVertexColorFormat();
	Vertices.setVertexColorFormat (vtype);
	rdrBuffer.Vertices.setVertexColorFormat (vtype);
	CVertexBufferReadWrite srcvba;
	Vertices.lock (srcvba);
	CVertexBufferReadWrite dstvba;
	rdrBuffer.Vertices.lock (dstvba);
	sint	ofsSrcUV= Vertices.getTexCoordOff();
	sint	ofsDstUV= rdrBuffer.Vertices.getTexCoordOff();
	sint	ofsDstColor= rdrBuffer.Vertices.getColorOff();
	uint8	*srcPtr= (uint8*)srcvba.getVertexCoordPointer();
	uint8	*dstPtr= (uint8*)dstvba.getVertexCoordPointer(rdrBuffer.NumQuads*4);
	sint	srcSize= Vertices.getVertexSize();
	sint	dstSize= rdrBuffer.Vertices.getVertexSize();

	// decal src for selection
	srcPtr+= SelectStart*4 * srcSize;

	CRGBA	mCol = Color;

	// **** clipping?
	if(allIn)
	{
		// copy All vertices
		uint numVerts= nNumQuadSrc*4;
		uint count = 4;
		uint lastIndex = 0;
		for(uint i=0;i<numVerts;i++)
		{
			if(count==4)
			{
				if(!LetterColors.empty())
				{
					if(LetterColors.getIndex(lastIndex)==i/4)
					{
						mCol.modulateFromColor(Color, LetterColors.getColor(lastIndex));

						if(lastIndex+1<LetterColors.size())
						{
							lastIndex++;
						}
					}
				}

				count = 0;
			}

			// copy and translate pos
			CHECK_VBA_RANGE(srcvba, srcPtr, Vertices.getVertexSize());
			CHECK_VBA_RANGE(dstvba, dstPtr, rdrBuffer.Vertices.getVertexSize())
			((CVector*)dstPtr)->x= x + ((CVector*)srcPtr)->x;
			((CVector*)dstPtr)->y= ((CVector*)srcPtr)->y;
			((CVector*)dstPtr)->z= z + ((CVector*)srcPtr)->z;
			// uv
			*((CUV*)(dstPtr+ofsDstUV))= *((CUV*)(srcPtr+ofsSrcUV));
			// color
			if (vtype == CVertexBuffer::TRGBA)
				*((CRGBA*)(dstPtr+ofsDstColor))= mCol;
			else
				*((CBGRA*)(dstPtr+ofsDstColor))= mCol;

			// next
			srcPtr+= srcSize;
			dstPtr+= dstSize;
			count++;
		}

		// update the rdrBuffer
		rdrBuffer.NumQuads+= nNumQuadSrc;
	}
	else
	{
		uint	nNumQuadClipped= 0;

		// the real number of vertices to compute (with selection)
		uint	numVerts= nNumQuadSrc*4;

		// clip into VerticesClipped
		CVector *pIniPos0 = (CVector*)srcPtr;
		CVector *pIniPos2 = (CVector*)(((uint8*)pIniPos0) + srcSize*2);
		CVector *pClipPos0 = (CVector*)dstPtr;
		CVector *pClipPos1 = (CVector*)(((uint8*)pClipPos0) + dstSize);
		CVector *pClipPos2 = (CVector*)(((uint8*)pClipPos1) + dstSize);
		CVector *pClipPos3 = (CVector*)(((uint8*)pClipPos2) + dstSize);
		CUV *pClipUV0 = (CUV*)(dstPtr + ofsDstUV );
		CUV *pClipUV1 = (CUV*)(((uint8*)pClipUV0) + dstSize);
		CUV *pClipUV2 = (CUV*)(((uint8*)pClipUV1) + dstSize);
		CUV *pClipUV3 = (CUV*)(((uint8*)pClipUV2) + dstSize);
		float ratio;

		uint lastIndex = 0;

		for (uint32 i = 0; i < numVerts; i+=4)
		{
			if (((x+pIniPos0->x) > xmax) || ((x+pIniPos2->x) < xmin) ||
				((z+pIniPos0->z) > zmax) || ((z+pIniPos2->z) < zmin))
			{
				// Totally clipped do nothing
			}
			else
			{
				if(!LetterColors.empty())
				{
					if(LetterColors.getIndex(lastIndex)==(i/4))
					{
						mCol.modulateFromColor(Color, LetterColors.getColor(lastIndex));

						if(lastIndex+1<LetterColors.size())
						{
							lastIndex++;
						}
					}
				}

				// copy with no clip
				// v0
				*((CVector*) (dstPtr + dstSize*0))= *((CVector*) (srcPtr + srcSize*0));
				*((CUV*)	 (dstPtr + dstSize*0 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*0 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*0 + ofsDstColor))= mCol;
				else
					*((CBGRA*)	 (dstPtr + dstSize*0 + ofsDstColor))= mCol;
				// v1
				*((CVector*) (dstPtr + dstSize*1))= *((CVector*) (srcPtr + srcSize*1));
				*((CUV*)	 (dstPtr + dstSize*1 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*1 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*1 + ofsDstColor))= mCol;
				else
					*((CBGRA*)	 (dstPtr + dstSize*1 + ofsDstColor))= mCol;
				// v2
				*((CVector*) (dstPtr + dstSize*2))= *((CVector*) (srcPtr + srcSize*2));
				*((CUV*)	 (dstPtr + dstSize*2 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*2 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*2 + ofsDstColor))= mCol;
				else
					*((CBGRA*)	 (dstPtr + dstSize*2 + ofsDstColor))= mCol;
				// v3
				*((CVector*) (dstPtr + dstSize*3))= *((CVector*) (srcPtr + srcSize*3));
				*((CUV*)	 (dstPtr + dstSize*3 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*3 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*3 + ofsDstColor))= mCol;
				else
					*((CBGRA*)	 (dstPtr + dstSize*3 + ofsDstColor))= mCol;

				// translate dest
				pClipPos0->x += x; pClipPos1->x += x; pClipPos2->x += x; pClipPos3->x += x;
				pClipPos0->z += z; pClipPos1->z += z; pClipPos2->z += z; pClipPos3->z += z;
				if ((pClipPos0->x >= xmin) && (pClipPos0->z >= zmin) && (pClipPos2->x <= xmax) && (pClipPos2->z <= zmax))
				{
					// Not clipped
				}
				else
				{
					// Partially clipped

					if (pClipPos0->x < xmin)
					{
						ratio = ((float)(xmin - pClipPos0->x))/((float)(pClipPos1->x - pClipPos0->x));
						pClipPos3->x = pClipPos0->x = xmin;
						pClipUV0->U += ratio*(pClipUV1->U - pClipUV0->U);
						pClipUV3->U += ratio*(pClipUV2->U - pClipUV3->U);
					}

					if (pClipPos0->z < zmin)
					{
						ratio = ((float)(zmin - pClipPos0->z))/((float)(pClipPos3->z - pClipPos0->z));
						pClipPos1->z = pClipPos0->z = zmin;
						pClipUV0->V += ratio*(pClipUV3->V - pClipUV0->V);
						pClipUV1->V += ratio*(pClipUV2->V - pClipUV1->V);
					}

					if (pClipPos2->x > xmax)
					{
						ratio = ((float)(xmax - pClipPos2->x))/((float)(pClipPos3->x - pClipPos2->x));
						pClipPos2->x = pClipPos1->x = xmax;
						pClipUV2->U += ratio*(pClipUV3->U - pClipUV2->U);
						pClipUV1->U += ratio*(pClipUV0->U - pClipUV1->U);
					}

					if (pClipPos2->z > zmax)
					{
						ratio = ((float)(zmax - pClipPos2->z))/((float)(pClipPos1->z - pClipPos2->z));
						pClipPos2->z = pClipPos3->z = zmax;
						pClipUV2->V += ratio*(pClipUV1->V - pClipUV2->V);
						pClipUV3->V += ratio*(pClipUV0->V - pClipUV3->V);
					}
				}

				// next quad out
				++nNumQuadClipped;
				pClipPos0 = (CVector*)(((uint8*)pClipPos0) + dstSize*4);
				pClipPos1 = (CVector*)(((uint8*)pClipPos0) + dstSize);
				pClipPos2 = (CVector*)(((uint8*)pClipPos1) + dstSize);
				pClipPos3 = (CVector*)(((uint8*)pClipPos2) + dstSize);
				pClipUV0 = (CUV*)( ((uint8*)pClipUV0) + dstSize*4 );
				pClipUV1 = (CUV*)(((uint8*)pClipUV0) + dstSize);
				pClipUV2 = (CUV*)(((uint8*)pClipUV1) + dstSize);
				pClipUV3 = (CUV*)(((uint8*)pClipUV2) + dstSize);
				dstPtr+=  4*dstSize;
			}
			// next quad in
			pIniPos0 = (CVector*)(((uint8*)pIniPos0) + srcSize*4);
			pIniPos2 = (CVector*)(((uint8*)pIniPos0) + srcSize*2);
			srcPtr+=  4*srcSize;
		}

		// update the rdrBuffer
		rdrBuffer.NumQuads+= nNumQuadClipped;
	}
}

/*------------------------------------------------------------------*\
							render2DUnProjected()
\*------------------------------------------------------------------*/
void CComputedString::render2DUnProjected (IDriver& driver, CRenderStringBuffer &rdrBuffer, class NL3D::CFrustum &frustum,
	const NLMISC::CMatrix &scaleMatrix, float x, float z, float depth, float xmin, float zmin, float xmax, float zmax)
{
	if (Vertices.getNumVertices() == 0)
		return;
	if(SelectSize==0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale to window size.
	x*= wndWidth;
	z*= wndHeight;
	xmin*= wndWidth;
	xmax*= wndWidth;
	zmin*= wndHeight;
	zmax*= wndHeight;

	// Test String Bound against clip window
	// If entirely out skip
	if (((x+XMin) > xmax) || ((x+XMax) < xmin) ||
		((z+ZMin) > zmax) || ((z+ZMax) < zmin))
		return;

	// test if entirely in.
	bool	allIn;
	allIn=	((x+XMin) >= (xmin-0.001f)) && ((x+XMax) <= (xmax+0.001f)) &&
			((z+ZMin) >= (zmin-0.001f)) && ((z+ZMax) <= (zmax+0.001f));


	// How many quad to render?
	uint	nNumQuadSrc= Vertices.getNumVertices()/4;
	nNumQuadSrc= min(nNumQuadSrc, (uint)SelectSize);

	// Enlarge dest Buffer if needed
	if( (rdrBuffer.NumQuads+nNumQuadSrc)*4 > rdrBuffer.Vertices.getNumVertices() )
	{
		rdrBuffer.Vertices.setNumVertices( (rdrBuffer.NumQuads+nNumQuadSrc)*4 );
	}

	// prepare copy.
	CVertexBuffer::TVertexColorType vtype = driver.getVertexColorFormat();
	Vertices.setVertexColorFormat (vtype);
	rdrBuffer.Vertices.setVertexColorFormat (vtype);
	CVertexBufferReadWrite srcvba;
	Vertices.lock (srcvba);
	CVertexBufferReadWrite dstvba;
	rdrBuffer.Vertices.lock (dstvba);
	sint	ofsSrcUV= Vertices.getTexCoordOff();
	sint	ofsDstUV= rdrBuffer.Vertices.getTexCoordOff();
	sint	ofsDstColor= rdrBuffer.Vertices.getColorOff();
	uint8	*srcPtr= (uint8*)srcvba.getVertexCoordPointer();
	uint8	*dstPtr= (uint8*)dstvba.getVertexCoordPointer(rdrBuffer.NumQuads*4);
	sint	srcSize= Vertices.getVertexSize();
	sint	dstSize= rdrBuffer.Vertices.getVertexSize();

	// decal src for selection
	srcPtr+= SelectStart*4 * srcSize;

	uint8	*dstPtrBackup = dstPtr;

	// **** clipping?
	if(allIn)
	{
		// copy All vertices
		uint numVerts= nNumQuadSrc*4;
		for(uint i=0;i<numVerts;i++)
		{
			// copy and translate pos
			CHECK_VBA_RANGE(dstvba, dstPtr, Vertices.getVertexSize());
			CHECK_VBA_RANGE(srcvba, srcPtr, rdrBuffer.Vertices.getVertexSize());
			((CVector*)dstPtr)->x= x + ((CVector*)srcPtr)->x;
			((CVector*)dstPtr)->z= z + ((CVector*)srcPtr)->z;

			// uv
			*((CUV*)(dstPtr+ofsDstUV))= *((CUV*)(srcPtr+ofsSrcUV));
			// color
			if (vtype == CVertexBuffer::TRGBA)
				*((CRGBA*)(dstPtr+ofsDstColor))= Color;
			else
				*((CBGRA*)(dstPtr+ofsDstColor))= Color;

			// next
			srcPtr+= srcSize;
			dstPtr+= dstSize;
		}

		// update the rdrBuffer
		rdrBuffer.NumQuads+= nNumQuadSrc;
	}
	else
	{
		uint	nNumQuadClipped= 0;

		// the real number of vertices to compute (with selection)
		uint	numVerts= nNumQuadSrc*4;

		// clip into VerticesClipped
		CVector *pIniPos0 = (CVector*)srcPtr;
		CVector *pIniPos2 = (CVector*)(((uint8*)pIniPos0) + srcSize*2);
		CVector *pClipPos0 = (CVector*)dstPtr;
		CVector *pClipPos1 = (CVector*)(((uint8*)pClipPos0) + dstSize);
		CVector *pClipPos2 = (CVector*)(((uint8*)pClipPos1) + dstSize);
		CVector *pClipPos3 = (CVector*)(((uint8*)pClipPos2) + dstSize);
		CUV *pClipUV0 = (CUV*)(dstPtr + ofsDstUV );
		CUV *pClipUV1 = (CUV*)(((uint8*)pClipUV0) + dstSize);
		CUV *pClipUV2 = (CUV*)(((uint8*)pClipUV1) + dstSize);
		CUV *pClipUV3 = (CUV*)(((uint8*)pClipUV2) + dstSize);
		float ratio;
		for (uint32 i = 0; i < numVerts; i+=4)
		{
			if (((x+pIniPos0->x) > xmax) || ((x+pIniPos2->x) < xmin) ||
				((z+pIniPos0->z) > zmax) || ((z+pIniPos2->z) < zmin))
			{
				// Totally clipped do nothing
			}
			else
			{
				// copy with no clip
				// v0
				*((CVector*) (dstPtr + dstSize*0))= *((CVector*) (srcPtr + srcSize*0));
				*((CUV*)	 (dstPtr + dstSize*0 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*0 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*0 + ofsDstColor))= Color;
				else
					*((CBGRA*)	 (dstPtr + dstSize*0 + ofsDstColor))= Color;
				// v1
				*((CVector*) (dstPtr + dstSize*1))= *((CVector*) (srcPtr + srcSize*1));
				*((CUV*)	 (dstPtr + dstSize*1 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*1 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*1 + ofsDstColor))= Color;
				else
					*((CBGRA*)	 (dstPtr + dstSize*1 + ofsDstColor))= Color;
				// v2
				*((CVector*) (dstPtr + dstSize*2))= *((CVector*) (srcPtr + srcSize*2));
				*((CUV*)	 (dstPtr + dstSize*2 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*2 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*2 + ofsDstColor))= Color;
				else
					*((CBGRA*)	 (dstPtr + dstSize*2 + ofsDstColor))= Color;
				// v3
				*((CVector*) (dstPtr + dstSize*3))= *((CVector*) (srcPtr + srcSize*3));
				*((CUV*)	 (dstPtr + dstSize*3 + ofsDstUV))= *((CUV*)(srcPtr + srcSize*3 + ofsSrcUV));
				if (vtype == CVertexBuffer::TRGBA)
					*((CRGBA*)	 (dstPtr + dstSize*3 + ofsDstColor))= Color;
				else
					*((CBGRA*)	 (dstPtr + dstSize*3 + ofsDstColor))= Color;


				// translate dest
				pClipPos0->x += x; pClipPos1->x += x; pClipPos2->x += x; pClipPos3->x += x;
				pClipPos0->z += z; pClipPos1->z += z; pClipPos2->z += z; pClipPos3->z += z;
				if ((pClipPos0->x >= xmin) && (pClipPos0->z >= zmin) && (pClipPos2->x <= xmax) && (pClipPos2->z <= zmax))
				{
					// Not clipped
				}
				else
				{
					// Partially clipped

					if (pClipPos0->x < xmin)
					{
						ratio = ((float)(xmin - pClipPos0->x))/((float)(pClipPos1->x - pClipPos0->x));
						pClipPos3->x = pClipPos0->x = xmin;
						pClipUV0->U += ratio*(pClipUV1->U - pClipUV0->U);
						pClipUV3->U += ratio*(pClipUV2->U - pClipUV3->U);
					}

					if (pClipPos0->z < zmin)
					{
						ratio = ((float)(zmin - pClipPos0->z))/((float)(pClipPos3->z - pClipPos0->z));
						pClipPos1->z = pClipPos0->z = zmin;
						pClipUV0->V += ratio*(pClipUV3->V - pClipUV0->V);
						pClipUV1->V += ratio*(pClipUV2->V - pClipUV1->V);
					}

					if (pClipPos2->x > xmax)
					{
						ratio = ((float)(xmax - pClipPos2->x))/((float)(pClipPos3->x - pClipPos2->x));
						pClipPos2->x = pClipPos1->x = xmax;
						pClipUV2->U += ratio*(pClipUV3->U - pClipUV2->U);
						pClipUV1->U += ratio*(pClipUV0->U - pClipUV1->U);
					}

					if (pClipPos2->z > zmax)
					{
						ratio = ((float)(zmax - pClipPos2->z))/((float)(pClipPos1->z - pClipPos2->z));
						pClipPos2->z = pClipPos3->z = zmax;
						pClipUV2->V += ratio*(pClipUV1->V - pClipUV2->V);
						pClipUV3->V += ratio*(pClipUV0->V - pClipUV3->V);
					}
				}

				// next quad out
				++nNumQuadClipped;
				pClipPos0 = (CVector*)(((uint8*)pClipPos0) + dstSize*4);
				pClipPos1 = (CVector*)(((uint8*)pClipPos0) + dstSize);
				pClipPos2 = (CVector*)(((uint8*)pClipPos1) + dstSize);
				pClipPos3 = (CVector*)(((uint8*)pClipPos2) + dstSize);
				pClipUV0 = (CUV*)( ((uint8*)pClipUV0) + dstSize*4 );
				pClipUV1 = (CUV*)(((uint8*)pClipUV0) + dstSize);
				pClipUV2 = (CUV*)(((uint8*)pClipUV1) + dstSize);
				pClipUV3 = (CUV*)(((uint8*)pClipUV2) + dstSize);
				dstPtr+=  4*dstSize;
			}
			// next quad in
			pIniPos0 = (CVector*)(((uint8*)pIniPos0) + srcSize*4);
			pIniPos2 = (CVector*)(((uint8*)pIniPos0) + srcSize*2);
			srcPtr+=  4*srcSize;
		}

		// update the rdrBuffer
		rdrBuffer.NumQuads+= nNumQuadClipped;
	}

	const float OOW = 1.f / (float)wndWidth;
	const float OOH = 1.f / (float)wndHeight;

	while (dstPtrBackup != dstPtr)
	{
		// preset unprojection
		CVector tmp;
		tmp.x = ((CVector*)dstPtrBackup)->x * OOW;
		tmp.y = ((CVector*)dstPtrBackup)->z * OOH;
		tmp.z = depth;
		// mul by user scale matrix
		tmp= scaleMatrix * tmp;
		// Unproject it
		*((CVector*)dstPtrBackup) = frustum.unProjectZ(tmp);
		dstPtrBackup += dstSize;
	}

}

// ***************************************************************************
CRenderStringBuffer::CRenderStringBuffer()
{
	// Use color per vertex
	Vertices.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::PrimaryColorFlag);
	Vertices.setPreferredMemory (CVertexBuffer::RAMVolatile, true);
	Vertices.setName("CRenderStringBuffer");
	NumQuads= 0;
}


// ***************************************************************************
CRenderStringBuffer::~CRenderStringBuffer()
{
}


// ***************************************************************************
void	CRenderStringBuffer::flush(IDriver& driver, CMaterial *fontMat)
{
	if(NumQuads==0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);

	// **** setup driver context
	driver.setFrustum(0, (float)wndWidth, 0, (float)wndHeight, -1, 1, false);  // resX/resY

	// view matrix and model matrix <-> identity
	driver.setupViewMatrix (CMatrix::Identity);
	driver.setupModelMatrix (CMatrix::Identity);

	// setup material
	fontMat->setZFunc (CMaterial::always);
	fontMat->setZWrite (false);

	// setup vertices clipped
	driver.activeVertexBuffer (Vertices);

	// *** rendering
	driver.renderRawQuads (*fontMat, 0, NumQuads );

	// *** reset
	NumQuads= 0;
}


// ***************************************************************************
void	CRenderStringBuffer::flushUnProjected(IDriver& driver, CMaterial *fontMat, bool zwrite)
{
	if(NumQuads==0)
		return;

	// setup material
	fontMat->setZFunc (CMaterial::lessequal);
	fontMat->setZWrite (zwrite);

	// setup vertices clipped
	driver.activeVertexBuffer (Vertices);

	// *** rendering
	driver.renderRawQuads (*fontMat, 0, NumQuads );

	// *** reset
	NumQuads= 0;
}


} // NL3D
