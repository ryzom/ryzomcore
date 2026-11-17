// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl3.h"

#include "driver_opengl3.h"
#include "nel/3d/index_buffer.h"
#include "driver_opengl3_vertex_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
// Indices of the draws, in the index stream or in a GL buffer
// ***************************************************************************

bool CDriverGL3::useIndices(CIndexBlock &block, const void *data, uint32 size, bool write, CIndexRange &range)
{
	H_AUTO_OGL(CDriverGL3_useIndices)

	if (_StreamMode)
	{
		// The stream gives the chunk of a handle to other data without telling, so a kept
		// handle is asked for on every use, and the indices written again when they are gone
		if (write || !_IndexStream.isValid(block.Handle))
		{
			block.Handle = _IndexStream.write(data, size, size);
			if (!_IndexStream.isValid(block.Handle))
				return false; // the stream has logged what failed
		}

		// The chunk now belongs to the current frame and render call. The buffer and the
		// offset stay right for the draws of the call, whatever the stream does with the
		// handle while the call writes more indices.
		range.Buffer = _IndexStream.use(block.Handle);
		range.Offset = block.Handle.Offset;
		return true;
	}

	// Per-buffer mode: the indices have a GL buffer to themselves, which keeps them until
	// the next write. A write gives the buffer new storage with the indices in one call,
	// so that nothing waits for the draws that still read the old storage.
	if (write || !block.Buffer)
	{
		if (!block.Buffer)
			nglGenBuffers(1, &block.Buffer);
		_DriverGLStates.bindElementArrayBuffer(block.Buffer);
		nglBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, write ? GL_STREAM_DRAW : GL_STATIC_DRAW);
#if NL3D_GL3_STREAM_STATS
		++_StreamStats.IndexBufferWrites;
		_StreamStats.IndexBufferBytes += size;
#endif
	}
	range.Buffer = block.Buffer;
	range.Offset = 0;
	return true;
}

void CDriverGL3::releaseIndexBlock(CIndexBlock &block)
{
	H_AUTO_OGL(CDriverGL3_releaseIndexBlock)

	if (block.Buffer)
	{
		// GL takes a deleted buffer out of the bound vertex array object, the state cache follows
		_DriverGLStates.elementArrayBufferDeleted(block.Buffer);
		nglDeleteBuffers(1, &block.Buffer);
		block.Buffer = 0;
	}
	block.Handle = CStreamHandleGL3();
}

bool CDriverGL3::streamIndices(uint32 firstIndex, uint32 count, CIndexRange &range, GLenum &type)
{
	H_AUTO_OGL(CDriverGL3_streamIndices)

	// A new render call: the index stream no longer has to keep the indices of the one before
	_IndexStream.nextDraw();

	// The indices are written for this call only. Nothing is kept per index buffer:
	// in stream mode a large body of indices that do not change would be written again
	// every few frames, as the stream moves on.
	if (_LastIB._Format == CIndexBuffer::Indices16)
	{
		type = GL_UNSIGNED_SHORT;
		return useIndices(_ScratchIndices, ((const uint16 *)_LastIB._Values) + firstIndex, count * (uint32)sizeof(uint16), true, range);
	}
	else
	{
		nlassert(_LastIB._Format == CIndexBuffer::Indices32);
		type = GL_UNSIGNED_INT;
		return useIndices(_ScratchIndices, ((const uint32 *)_LastIB._Values) + firstIndex, count * (uint32)sizeof(uint32), true, range);
	}
}

void CDriverGL3::drawIndices(GLenum mode, GLsizei count, GLenum type, const CIndexRange &range, uint32 offset)
{
	// The uniform buffer ranges of the draw
	_DriverGLStates.flushUniformBufferBindings();
	// With a buffer bound, the pointer of glDrawElements is the offset into it
	_DriverGLStates.bindElementArrayBuffer(range.Buffer);
	glDrawElements(mode, count, type, (const GLvoid *)(size_t)(range.Offset + offset));
}

// ***************************************************************************

bool CDriverGL3::renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines)
{
	H_AUTO_OGL(CDriverGL3_renderLines)

	// setup material
	if (!setupMaterial(mat) || _LastIB._Values == nullptr)
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;

	// the indices, once for all passes
	CIndexRange indices;
	GLenum indexType = GL_UNSIGNED_SHORT;
	if (nlines && !streamIndices(firstIndex, 2 * nlines, indices, indexType))
		return false;

	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the primitives.
		if (nlines)
			drawIndices(GL_LINES, 2 * nlines, indexType, indices);
	}
	// end multipass.
	endMultiPass();

	// Profiling.
	_PrimitiveProfileIn.NLines+= nlines;
	_PrimitiveProfileOut.NLines+= nlines;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderTriangles(CMaterial& mat, uint32 firstIndex, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderTriangles);

	// setup material
	if (!setupMaterial(mat) || _LastIB._Values == nullptr)
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;

	// the indices, once for all passes
	CIndexRange indices;
	GLenum indexType = GL_UNSIGNED_SHORT;
	if (ntris && !streamIndices(firstIndex, 3 * ntris, indices, indexType))
		return false;

	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;

	nPass= beginMultiPass();

	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the primitives.
		if (ntris)
			drawIndices(GL_TRIANGLES, 3 * ntris, indexType, indices);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris * nPass;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderSimpleTriangles(uint32 firstTri, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderSimpleTriangles);

	// Without its programs and uniform blocks set up, the draw would read what an earlier draw left bound
	if (!setupBuiltinPrograms())
		return false;

	nlassert(ntris>0);

	if (_LastIB._Values == nullptr)
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;
	// Don't setup any material here.

	// render primitives.
	//==============================
	// NO MULTIPASS HERE!!
	// draw the primitives. (nb: ntris>0).

	CIndexRange indices;
	GLenum indexType = GL_UNSIGNED_SHORT;
	if (!streamIndices(firstTri, 3 * ntris, indices, indexType))
		return false;
	drawIndices(GL_TRIANGLES, 3 * ntris, indexType, indices);

	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawPoints(CMaterial& mat, uint32 startIndex, uint32 numPoints)
{
	H_AUTO_OGL(CDriverGL3_renderRawPoints)

	// setup material
	if (!setupMaterial(mat))
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the primitives.
		if (numPoints)
		{
			_DriverGLStates.flushUniformBufferBindings();
			glDrawArrays(GL_POINTS, startIndex, numPoints);
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NPoints+= numPoints;
	_PrimitiveProfileOut.NPoints+= numPoints * nPass;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawLines(CMaterial& mat, uint32 startIndex, uint32 numLines)
{
	H_AUTO_OGL(CDriverGL3_renderRawLines)

	// setup material
	if (!setupMaterial(mat))
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the primitives.
		if (numLines)
		{
			_DriverGLStates.flushUniformBufferBindings();
			glDrawArrays(GL_LINES, startIndex << 1, numLines << 1);
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NLines  += numLines ;
	_PrimitiveProfileOut.NLines += numLines  * nPass;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawTriangles(CMaterial& mat, uint32 startIndex, uint32 numTris)
{
	H_AUTO_OGL(CDriverGL3_renderRawTriangles)

	// setup material
	if (!setupMaterial(mat))
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the primitives.
		if (numTris)
		{
			_DriverGLStates.flushUniformBufferBindings();
			glDrawArrays(GL_TRIANGLES, startIndex*3, numTris*3);
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NTriangles  += numTris ;
	_PrimitiveProfileOut.NTriangles += numTris  * nPass;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawQuads(CMaterial& mat, uint32 startIndex, uint32 numQuads)
{
	H_AUTO_OGL(CDriverGL3_renderRawQuads)
	if (!numQuads) return true;

	// setup material
	if (!setupMaterial(mat))
		return false;

	if (_CurrentVertexBufferGL && _CurrentVertexBufferGL->isInvalid()) return true;

	const uint32 QUAD_BATCH_SIZE = 2048;
	static GLshort defaultIndices[QUAD_BATCH_SIZE * 6];
	static bool	init = false;
	if (!init)
	{
		// setup the base index buffer
		for (uint k = 0; k < QUAD_BATCH_SIZE; ++k)
		{
			// first tri
			defaultIndices[k * 6] = (GLshort) (k * 4);
			defaultIndices[k * 6 + 1] = (GLshort) (k * 4 + 1);
			defaultIndices[k * 6 + 2] = (GLshort) (k * 4 + 2);
			// second tri
			defaultIndices[k * 6 + 3] = (GLshort) (k * 4);
			defaultIndices[k * 6 + 4] = (GLshort) (k * 4 + 2);
			defaultIndices[k * 6 + 5] = (GLshort) (k * 4 + 3);

		}
		init = true;
	}

	// The indices of the call go to the driver's buffers before the passes, once for all of them
	_IndexStream.nextDraw();

	// startIndex is the first vertex of the first quad (IDriver::renderRawQuads)
	uint32 currIndex = startIndex;
	uint32 numLeftQuads = numQuads;

	// first batch of quads using the static setupped array, which has quad k at vertex 4 * k:
	// it serves a first vertex that is a multiple of 4
	CIndexRange tableRange;
	uint tableFirst = 0;
	uint tableQuads = 0;
	const uint32 startQuad = startIndex / 4;
	if ((startIndex & 3) == 0 && startQuad < QUAD_BATCH_SIZE)
	{
		// first quads (as pair of tri to have guaranteed orientation)
		tableFirst = startQuad;
		tableQuads = std::min(QUAD_BATCH_SIZE - startQuad, numQuads);
		// The whole table, the same for every call: written when the driver does not have it
		if (!useIndices(_QuadIndices, defaultIndices, sizeof(defaultIndices), false, tableRange))
			return false;
		numLeftQuads -= tableQuads;
		currIndex += 4 * tableQuads;
	}

	// remaining quads: the indices of their batches are built one behind the other, and written at once
	CIndexRange restRange;
	_QuadDraws.clear();
	_QuadIndexData.clear();
	while (numLeftQuads)
	{
		// TODO : resetting vertex pointer would avoid the need to rebuild indices each times
		uint32 numQuadsToDraw = std::min(numLeftQuads, QUAD_BATCH_SIZE);
		CIndexDraw draw;
		draw.Offset = (uint32)_QuadIndexData.size();
		draw.Count = 6 * numQuadsToDraw;
		if (4 * numQuadsToDraw + currIndex <= (1 << 16))
		{
			// indices fits on 16 bits
			draw.Type = GL_UNSIGNED_SHORT;
			// a whole number of 32 bit words, so that every batch starts at a multiple of its index size
			_QuadIndexData.resize(draw.Offset + ((draw.Count * sizeof(uint16) + 3) & ~(size_t)3));
			uint16 *curr = (uint16 *)&_QuadIndexData[draw.Offset];
			uint16 *end = curr + 6 * numQuadsToDraw;
			uint16 vertexIndex = (uint16) currIndex;
			do
			{
				*curr++ = vertexIndex;
				*curr++ = vertexIndex + 1;
				*curr++ = vertexIndex + 2;
				*curr++ = vertexIndex;
				*curr++ = vertexIndex + 2;
				*curr++ = vertexIndex + 3;
				vertexIndex += 4;
			}
			while (curr != end);
		}
		else
		{
			// indices fits on 32 bits
			draw.Type = GL_UNSIGNED_INT;
			_QuadIndexData.resize(draw.Offset + draw.Count * sizeof(uint32));
			uint32 *curr = (uint32 *)&_QuadIndexData[draw.Offset];
			uint32 *end = curr + 6 * numQuadsToDraw;
			uint32 vertexIndex = currIndex;
			do
			{
				*curr++ = vertexIndex;
				*curr++ = vertexIndex + 1;
				*curr++ = vertexIndex + 2;
				*curr++ = vertexIndex;
				*curr++ = vertexIndex + 2;
				*curr++ = vertexIndex + 3;
				vertexIndex += 4;
			}
			while (curr != end);
		}
		_QuadDraws.push_back(draw);
		numLeftQuads -= numQuadsToDraw;
		currIndex += 4 * numQuadsToDraw;
	}
	if (!_QuadIndexData.empty()
	    && !useIndices(_ScratchIndices, &_QuadIndexData[0], (uint32)_QuadIndexData.size(), true, restRange))
		return false;

	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for (uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if (!setupPass(pass))
			return false;

		// draw the quads of the table, then the batches of the others
		if (tableQuads)
			drawIndices(GL_TRIANGLES, 6 * tableQuads, GL_UNSIGNED_SHORT, tableRange, 6 * tableFirst * (uint32)sizeof(GLshort));
		for (size_t i = 0; i < _QuadDraws.size(); ++i)
			drawIndices(GL_TRIANGLES, _QuadDraws[i].Count, _QuadDraws[i].Type, restRange, _QuadDraws[i].Offset);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NQuads  += numQuads ;
	_PrimitiveProfileOut.NQuads += numQuads  * nPass;

	if (_CurrentVertexBufferGL)
		_CurrentVertexBufferGL->setFrameInFlight(_SwapBufferCounter);

	return true;
}

}
}

