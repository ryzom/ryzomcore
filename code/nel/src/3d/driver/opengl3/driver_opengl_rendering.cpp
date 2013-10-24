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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/index_buffer.h"
#include "driver_opengl_vertex_buffer_hard.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************

bool CDriverGL3::renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines)
{
	H_AUTO_OGL(CDriverGL3_renderLines)

	// setup material
	if ( !setupMaterial(mat) || _LastIB._Values == NULL )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		// draw the primitives.
		if(nlines)
		{
			if (_LastIB._Format == CIndexBuffer::Indices16)
			{
				glDrawElements(GL_LINES,2*nlines,GL_UNSIGNED_SHORT,((uint16 *) _LastIB._Values)+firstIndex);
			}
			else
			{
				nlassert(_LastIB._Format == CIndexBuffer::Indices32);
				glDrawElements(GL_LINES,2*nlines,GL_UNSIGNED_INT,((uint32 *) _LastIB._Values)+firstIndex);
			}
		}
	}
	// end multipass.
	endMultiPass();

	// Profiling.
	_PrimitiveProfileIn.NLines+= nlines;
	_PrimitiveProfileOut.NLines+= nlines;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderTriangles(CMaterial& mat, uint32 firstIndex, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderTriangles);

	// setup material
	if ( !setupMaterial(mat) || _LastIB._Values == NULL )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;

	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;

	nPass= beginMultiPass();

	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		// draw the primitives.
		if(ntris)
		{
			if (_LastIB._Format == CIndexBuffer::Indices16)
			{
				glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_SHORT, ((uint16 *) _LastIB._Values)+firstIndex);
			}
			else
			{
				nlassert(_LastIB._Format == CIndexBuffer::Indices32);
				glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, ((uint32 *) _LastIB._Values)+firstIndex);
			}
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris * nPass;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderSimpleTriangles(uint32 firstTri, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderSimpleTriangles);

	nlassert(ntris>0);

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;
	// Don't setup any material here.

	// render primitives.
	//==============================
	// NO MULTIPASS HERE!!
	// draw the primitives. (nb: ntris>0).

	if (_LastIB._Format == CIndexBuffer::Indices16)
	{
		glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_SHORT, ((uint16 *) _LastIB._Values)+firstTri);
	}
	else
	{
		nlassert(_LastIB._Format == CIndexBuffer::Indices32);
		glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, ((uint32 *) _LastIB._Values)+firstTri);
	}

	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawPoints(CMaterial& mat, uint32 startIndex, uint32 numPoints)
{
	H_AUTO_OGL(CDriverGL3_renderRawPoints)

	// setup material
	if ( !setupMaterial(mat) )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		// draw the primitives.
		if(numPoints)
			glDrawArrays(GL_POINTS, startIndex, numPoints);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NPoints+= numPoints;
	_PrimitiveProfileOut.NPoints+= numPoints * nPass;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawLines(CMaterial& mat, uint32 startIndex, uint32 numLines)
{
	H_AUTO_OGL(CDriverGL3_renderRawLines)

	// setup material
	if ( !setupMaterial(mat) )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		// draw the primitives.
		if(numLines)
			glDrawArrays(GL_LINES, startIndex << 1, numLines << 1);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NLines  += numLines ;
	_PrimitiveProfileOut.NLines += numLines  * nPass;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawTriangles(CMaterial& mat, uint32 startIndex, uint32 numTris)
{
	H_AUTO_OGL(CDriverGL3_renderRawTriangles)

	// setup material
	if ( !setupMaterial(mat) )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		// draw the primitives.
		if(numTris)
		{
			glDrawArrays(GL_TRIANGLES, startIndex*3, numTris*3);
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NTriangles  += numTris ;
	_PrimitiveProfileOut.NTriangles += numTris  * nPass;

	nullPrograms();

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawQuads(CMaterial& mat, uint32 startIndex, uint32 numQuads)
{
	H_AUTO_OGL(CDriverGL3_renderRawQuads)
	if (!numQuads) return true;

	// setup material
	if ( !setupMaterial(mat) )
		return false;

	if( !setupProgram( mat ) )
		return false;

	if (_CurrentVertexBufferHard && _CurrentVertexBufferHard->isInvalid()) return true;

	const uint32 QUAD_BATCH_SIZE = 2048;
	static GLshort defaultIndices[QUAD_BATCH_SIZE * 6];
	static bool	init = false;
	if (!init)
	{
		// setup the base index buffer
		for(uint k = 0; k < QUAD_BATCH_SIZE; ++k)
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

	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		if( !setupPass(pass) )
			return false;

		uint32 currIndex = startIndex;
		uint32 numLeftQuads = numQuads;

		// draw first batch of quads using the static setupped array
		if (startIndex < QUAD_BATCH_SIZE)
		{
			// draw first quads (as pair of tri to have guaranteed orientation)
			uint numQuadsToDraw = std::min(QUAD_BATCH_SIZE - startIndex, numQuads);
			glDrawElements(GL_TRIANGLES, 6 * numQuadsToDraw, GL_UNSIGNED_SHORT, defaultIndices + 6 * startIndex);
			numLeftQuads -= numQuadsToDraw;
			currIndex += 4 * numQuadsToDraw;
		}

		// draw remaining quads
		while (numLeftQuads)
		{
			// TODO : resetting vertex pointer would avoid the need to rebuild indices each times
			uint32 numQuadsToDraw = std::min(numLeftQuads, QUAD_BATCH_SIZE);
			// draw all quads
			if (4 * numQuadsToDraw + currIndex <= (1 << 16))
			{
				// indices fits on 16 bits
				GLshort indices[QUAD_BATCH_SIZE * 6];
				GLshort *curr = indices;
				GLshort *end = indices + 6 * numQuadsToDraw;
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
				while(curr != end);
				glDrawElements(GL_TRIANGLES, 6 * numQuadsToDraw, GL_UNSIGNED_SHORT, indices);
			}
			else
			{
				// indices fits on 32 bits
				GLint indices[QUAD_BATCH_SIZE];
				GLint *curr = indices;
				GLint *end = indices + 6 * numQuadsToDraw;
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
				while(curr != end);
				glDrawElements(GL_TRIANGLES, 6 * numQuadsToDraw, GL_UNSIGNED_INT, indices);
			}
			numLeftQuads -= numQuadsToDraw;
			currIndex += 4 * numQuadsToDraw;
		}
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NQuads  += numQuads ;
	_PrimitiveProfileOut.NQuads += numQuads  * nPass;

	nullPrograms();

	return true;
}

#ifdef NL_STATIC
}
#endif

}

