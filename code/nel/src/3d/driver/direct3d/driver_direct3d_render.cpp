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

#include "stddirect3d.h"

#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/misc/fast_mem.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;




namespace NL3D
{




// convert 32 bit indices to 16 bit indices on devices that don't have 32 bit indices
// ***************************************************************************
void CDriverD3D::convertToIndices16(uint firstIndex, uint numIndices)
{
	nlassert(numIndices > 0);
	// Lock the good buffer
	uint offset;
	CVolatileIndexBuffer *&buffer = _VolatileIndexBuffer16RAM[_CurrentRenderPass&1];
	uint16 *ptr = (uint16*)buffer->lock (numIndices*sizeof(uint16), offset);
	if (!ptr)
	{
		// index buffer full, swap with other buffer
		CVolatileIndexBuffer *&bufferOther = _VolatileIndexBuffer16RAM[(_CurrentRenderPass + 1) &1];
		std::swap(buffer, bufferOther);
		buffer->reset();
		ptr = (uint16*)buffer->lock (numIndices*sizeof(uint16), offset);
	}
	const uint32 *currIndex = &_LastIndexBufferInfo->RamVersion[firstIndex];
	const uint32 *indexEnd = currIndex + numIndices;
	do
	{
		#ifdef NL_DEBUG
			nlassertex(*currIndex < _MaxVertexIndex, ("In this implementation, only 16 bit indices are supported"));
		#endif
		*ptr++ = (uint16&) (*currIndex++);
	}
	while (currIndex != indexEnd);
	buffer->unlock();
	setIndexBuffer(buffer->IndexBuffer, offset);
}

// ***************************************************************************
#ifndef NL_DEBUG
	inline
#endif
bool CDriverD3D::renderPrimitives(D3DPRIMITIVETYPE primitiveType, uint /* numVertexPerPrim */, CMaterial& mat, uint firstVertex, uint32 nPrims)
{
	// Setup material
	if ( !setupMaterial(mat) )
		return false;
	if (nPrims == 0)
		return false;
	nlassertex(nPrims < _MaxPrimitiveCount, ("Number of max primitive at each calls limited in this implementation on current hradware"));
	if (_VertexBufferCache.VertexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass<_CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);
			HRESULT r = _DeviceInterface->DrawPrimitive (primitiveType, firstVertex + _VertexBufferOffset, nPrims);
			nlassert(r == D3D_OK);
		}
		endMultiPass ();
	}
	return true;
}




// ***************************************************************************
#ifndef NL_DEBUG
	inline
#endif
bool CDriverD3D::renderIndexedPrimitives(D3DPRIMITIVETYPE primitiveType, uint numVertexPerPrim, CMaterial& mat, uint32 firstIndex, uint32 nPrims, uint indexOffset /*= 0*/)
{
	// Setup material
	if ( !setupMaterial(mat) )
		return false;
	if (nPrims == 0)
		return false;
	nlassertex(nPrims < _MaxPrimitiveCount, ("Number of max primitive at each calls limited in this implementation on current hardware"));
	nlassert(_CurrIndexBufferFormat != CIndexBuffer::IndicesUnknownFormat);
	if (_MaxVertexIndex <= 0xffff && _CurrIndexBufferFormat == CIndexBuffer::Indices32)
	{
		convertToIndices16(firstIndex, numVertexPerPrim * nPrims);
		firstIndex = 0;
	}
	if (_VertexBufferCache.VertexBuffer && _IndexBufferCache.IndexBuffer)
	{
		uint pass;
		beginMultiPass ();
		for (pass=0; pass< _CurrentShaderPassCount; pass++)
		{
			// Active the pass
			activePass (pass);
			// NB : indexOffset is actually a constant added to each index in the current index buffer (actually may be implemented
			// by moving vertex pointer in the driver ...), whereas _IndexBufferOffset+firstIndex gives an offset into the index buffer
			HRESULT r = _DeviceInterface->DrawIndexedPrimitive (primitiveType, _VertexBufferOffset + indexOffset, 0, _VertexBufferSize,
				firstIndex+_IndexBufferOffset, nPrims);
			nlassert(r == D3D_OK);

		}
		endMultiPass ();
	}
	return true;
}

// ***************************************************************************
#ifndef NL_DEBUG
	inline
#endif
bool CDriverD3D::renderSimpleIndexedPrimitives(D3DPRIMITIVETYPE primitiveType, uint numVertexPerPrim, uint32 firstIndex, uint32 nPrims, uint indexOffset /*= 0*/)
{
	if (nPrims == 0)
		return false;
	nlassertex(nPrims < _MaxPrimitiveCount, ("Number of max primitive at each calls limited in this implementation on current hardware"));
	nlassert(_CurrIndexBufferFormat != CIndexBuffer::IndicesUnknownFormat);
	if (_MaxVertexIndex <= 0xffff && _CurrIndexBufferFormat == CIndexBuffer::Indices32)
	{
		convertToIndices16(firstIndex, numVertexPerPrim * nPrims);
		firstIndex = 0;
	}
	if (_VertexBufferCache.VertexBuffer && _IndexBufferCache.IndexBuffer)
	{
		updateRenderVariablesInternal();
		//fixVB(nPrims, numVertexPerPrim);
		// NB : indexOffset is actually a constant added to each index in the current index buffer (actually may be implemented
			// by moving vertex pointer in the driver ...), whereas _IndexBufferOffset+firstIndex gives an offset into the index buffer
		HRESULT r = _DeviceInterface->DrawIndexedPrimitive (primitiveType, _VertexBufferOffset + indexOffset, 0, _VertexBufferSize,
			firstIndex+_IndexBufferOffset, nPrims);
		nlassert(r == D3D_OK);
	}
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines)
{
	H_AUTO_D3D(CDriverD3D_renderLines)
	if (!renderIndexedPrimitives(D3DPT_LINELIST, 2, mat, firstIndex, nlines)) return false;
	// Stats
	_PrimitiveProfileIn.NLines += nlines;
	_PrimitiveProfileOut.NLines += nlines*_CurrentShaderPassCount;

	return true;
}


// ***************************************************************************

bool CDriverD3D::renderTriangles(CMaterial& mat, uint32 firstIndex, uint32 ntris)
{
	H_AUTO_D3D(CDriverD3D_renderTriangles)
	if (!renderIndexedPrimitives(D3DPT_TRIANGLELIST, 3, mat, firstIndex, ntris)) return false;
	// Stats
	_PrimitiveProfileIn.NTriangles += ntris;
	_PrimitiveProfileOut.NTriangles += ntris*_CurrentShaderPassCount;
	return true;
}

// ***************************************************************************

bool CDriverD3D::renderSimpleTriangles(uint32 firstIndex, uint32 ntris)
{
	H_AUTO_D3D(CDriverD3D_renderSimpleTriangles)
	nlassert (ntris != 0);
	nlassert (_VertexBufferCache.VertexBuffer);
	nlassert (_IndexBufferCache.IndexBuffer);
	if (!renderSimpleIndexedPrimitives(D3DPT_TRIANGLELIST, 3,  firstIndex, ntris)) return false;
	// Stats
	_PrimitiveProfileIn.NTriangles += ntris;
	_PrimitiveProfileOut.NTriangles += ntris;

	return true;
}

// ***************************************************************************
bool CDriverD3D::renderLinesWithIndexOffset(CMaterial& mat, uint32 firstIndex, uint32 nlines, uint indexOffset)
{
	H_AUTO_D3D(CDriverD3D_renderLinesWithIndexOffset)
	if (!renderIndexedPrimitives(D3DPT_LINELIST, 2, mat, firstIndex, nlines, indexOffset)) return false;
	// Stats
	_PrimitiveProfileIn.NLines += nlines;
	_PrimitiveProfileOut.NLines += nlines*_CurrentShaderPassCount;
	return true;
}

// ***************************************************************************
bool CDriverD3D::renderTrianglesWithIndexOffset(CMaterial& mat, uint32 firstIndex, uint32 ntris, uint indexOffset)
{
	H_AUTO_D3D(CDriverD3D_renderTrianglesWithIndexOffset)
	if (!renderIndexedPrimitives(D3DPT_TRIANGLELIST, 3, mat, firstIndex, ntris, indexOffset)) return false;
	// Stats
	_PrimitiveProfileIn.NTriangles += ntris;
	_PrimitiveProfileOut.NTriangles += ntris*_CurrentShaderPassCount;
	return true;
}

// ***************************************************************************
bool CDriverD3D::renderSimpleTrianglesWithIndexOffset(uint32 firstIndex, uint32 ntris, uint indexOffset)
{
	H_AUTO_D3D(CDriverD3D_renderSimpleTrianglesWithIndexOffset)
	nlassert (ntris != 0);
	nlassert (_VertexBufferCache.VertexBuffer);
	nlassert (_IndexBufferCache.IndexBuffer);
	if (!renderSimpleIndexedPrimitives(D3DPT_TRIANGLELIST, 3,  firstIndex, ntris, indexOffset)) return false;
	// Stats
	_PrimitiveProfileIn.NTriangles += ntris;
	_PrimitiveProfileOut.NTriangles += ntris;

	return true;
}

// ***************************************************************************
bool CDriverD3D::renderRawPoints(CMaterial& mat, uint32 firstIndex, uint32 numPoints)
{
	H_AUTO_D3D(CDriverD3D_renderRawPoints);
	if (!renderPrimitives(D3DPT_POINTLIST, 1, mat, firstIndex, numPoints))
	// Stats
	_PrimitiveProfileIn.NPoints += numPoints;
	_PrimitiveProfileOut.NPoints += numPoints*_CurrentShaderPassCount;
	return true;
}

// ***************************************************************************
bool CDriverD3D::renderRawLines(CMaterial& mat, uint32 firstIndex, uint32 numLines)
{
	H_AUTO_D3D(CDriverD3D_renderRawLines);
	if (!renderPrimitives(D3DPT_LINELIST, 2, mat, firstIndex, numLines)) return false;
	// Stats
	_PrimitiveProfileIn.NLines += numLines;
	_PrimitiveProfileOut.NLines += numLines*_CurrentShaderPassCount;

	return true;
}

// ***************************************************************************

bool CDriverD3D::renderRawTriangles(CMaterial& mat, uint32 firstIndex, uint32 numTris)
{
	H_AUTO_D3D(CDriverD3D_renderRawTriangles)
	if (!renderPrimitives(D3DPT_TRIANGLELIST, 3, mat, 3 * firstIndex, numTris)) return false;
	// Stats
	_PrimitiveProfileIn.NTriangles += numTris;
	_PrimitiveProfileOut.NTriangles += numTris*_CurrentShaderPassCount;

	return true;
}

// ***************************************************************************
// 32 bits version
void fillQuadIndexes (uint32 *indexes, uint first, uint last)
{
	H_AUTO_D3D(fillQuadIndexes)
	uint firstQuad = (first / 6) * 4;
	for (;first<last; first+=6, firstQuad+=4)
	{
		indexes[first+0] = firstQuad+0;
		indexes[first+1] = firstQuad+1;
		indexes[first+2] = firstQuad+2;
		indexes[first+3] = firstQuad+0;
		indexes[first+4] = firstQuad+2;
		indexes[first+5] = firstQuad+3;
	}
}

// ***************************************************************************
// 16 bits version
void fillQuadIndexes (uint16 *indexes, uint first, uint last)
{
	H_AUTO_D3D(fillQuadIndexes)
	uint16 firstQuad = (uint16) ((first / 6) * 4);
	for (;first<last; first+=6, firstQuad+=4)
	{
		indexes[first+0] = firstQuad+0;
		indexes[first+1] = firstQuad+1;
		indexes[first+2] = firstQuad+2;
		indexes[first+3] = firstQuad+0;
		indexes[first+4] = firstQuad+2;
		indexes[first+5] = firstQuad+3;
	}
}

// ***************************************************************************

void CDriverD3D::setDebugMaterial()
{
	H_AUTO_D3D(CDriverD3D_setDebugMaterial)
	_DeviceInterface->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
	_DeviceInterface->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_ZERO);
	_DeviceInterface->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_ALPHAREF, 128);
	_DeviceInterface->SetRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	_DeviceInterface->SetRenderState (D3DRS_LIGHTING, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_TEXTUREFACTOR, NL_D3DCOLOR_RGBA(CRGBA(255,0,255,255)));
	_DeviceInterface->SetRenderState (D3DRS_CULLMODE, D3DCULL_CW);
	_DeviceInterface->SetRenderState (D3DRS_COLORVERTEX, FALSE);
	_DeviceInterface->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
	_DeviceInterface->SetRenderState (D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	_DeviceInterface->SetRenderState (D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetRenderState (D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	_DeviceInterface->SetTexture (0, NULL);
	_DeviceInterface->SetTexture (1, NULL);
	_DeviceInterface->SetTexture (2, NULL);
	_DeviceInterface->SetTexture (3, NULL);
	_DeviceInterface->SetTexture (4, NULL);
	_DeviceInterface->SetTexture (5, NULL);
	_DeviceInterface->SetTexture (6, NULL);
	_DeviceInterface->SetTexture (7, NULL);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG0, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	_DeviceInterface->SetTextureStageState (0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	// _DeviceInterface->SetTextureStageState (0, D3DTSS_CONSTANT, 0x0);
	uint i;
	for (i=1; i<8; i++)
	{
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG0, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		_DeviceInterface->SetTextureStageState (i, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
		// _DeviceInterface->SetTextureStageState (i, D3DTSS_CONSTANT, 0x0);
	}
}


// ***************************************************************************
bool CDriverD3D::renderRawQuads(CMaterial& mat, uint32 startIndex, uint32 numQuads)
{
	H_AUTO_D3D(CDriverD3D_renderRawQuads)
	if (numQuads == 0)	return false;
	else
	if (_VertexBufferCache.VertexBuffer)
	{
		// Num of indexes needed
		const uint numQuadsNeeded = numQuads*6;
		nlassert(numQuads < MAX_NUM_QUADS); // this limitation should suffice for now
		{
			if (_MaxVertexIndex <= 0xffff)
			{
				nlassert(_QuadIB);
				setIndexBuffer(_QuadIB, 0);
				_CurrIndexBufferFormat = CIndexBuffer::Indices16; // must set the format because we don't call activeIndexBuffer
			}
			else
			{
				const uint IB_RESIZE_STRIDE = 6 * 256;
				nlctassert(IB_RESIZE_STRIDE % 6 == 0);
				// Need to resize the quad indexes array ?
				if (_QuadIndexes.getNumIndexes() < numQuadsNeeded)
				{
					// Resize it
					uint32 numIndexResize = IB_RESIZE_STRIDE * ((numQuadsNeeded + (IB_RESIZE_STRIDE - 1)) / IB_RESIZE_STRIDE);
					_QuadIndexes.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
					_QuadIndexes.setNumIndexes(numIndexResize); // snap to nearest size
					// Fill the index buffer in VRAM
					CIndexBufferReadWrite iba;
					_QuadIndexes.lock (iba);
					if (_QuadIndexes.getFormat() == CIndexBuffer::Indices32)
					{
						fillQuadIndexes ((uint32 *) iba.getPtr(), 0, numIndexResize);
					}
					else
					{
						fillQuadIndexes ((uint16 *) iba.getPtr(), 0, numIndexResize);
					}
				}
				activeIndexBuffer (_QuadIndexes);
			}
			// Setup material
			if ( !setupMaterial(mat) )
				return false;

			uint pass;
			beginMultiPass ();
			for (pass=0; pass<_CurrentShaderPassCount; pass++)
			{
				// Active the pass
				activePass (pass);
				//fixVB(numQuads, 4);
				_DeviceInterface->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, _VertexBufferOffset + startIndex, 0, numQuads * 4,
					0, numQuads*2);
			}
			endMultiPass ();
		}
	}

	// Stats
	_PrimitiveProfileIn.NTriangles += numQuads*2;
	_PrimitiveProfileOut.NTriangles += numQuads*2*_CurrentShaderPassCount;

	return true;
}

// ***************************************************************************

} // NL3D





















