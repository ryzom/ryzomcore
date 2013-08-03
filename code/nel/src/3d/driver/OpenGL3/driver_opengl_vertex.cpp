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




using namespace std;
using namespace NLMISC;




// ***************************************************************************
// Flags for software vertex skinning.
#define	NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE	3
#define	NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE	1
#define	NL3D_DRV_SOFTSKIN_VCOMPUTED		0
// 3 means "vertex may need compute".
// 1 means "Primitive say vertex must be computed".
// 0 means "vertex is computed".


// 500K min.
#define	NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE		(512*1024)





namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************

CVBDrvInfosGL3::CVBDrvInfosGL3(CDriverGL3 *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb) : IVBDrvInfos(drv, it, vb)
{
	H_AUTO_OGL(CVBDrvInfosGL_CVBDrvInfosGL)
	_DriverGL = drv;
	_VBHard = NULL;
}

// ***************************************************************************

CVBDrvInfosGL3::~CVBDrvInfosGL3()
{
	H_AUTO_OGL(CVBDrvInfosGL_CVBDrvInfosGLDtor)
	// Restaure non resident memory
	if (VertexBufferPtr)
	{
		VertexBufferPtr->setLocation(CVertexBuffer::NotResident);
		VertexBufferPtr = NULL;
	}

	if (_VBHard)
	{
		_VBHard->disable();
		_DriverGL->_VertexBufferHardSet.erase(_VBHard);
	}

	_VBHard = NULL;
}

// ***************************************************************************
uint8 *CVBDrvInfosGL3::lock (uint /* first */, uint /* last */, bool /* readOnly */)
{
	H_AUTO_OGL(CVBDrvInfosGL_lock)
	return (uint8*)_VBHard->lock ();
}

// ***************************************************************************
void CVBDrvInfosGL3::unlock (uint first, uint last)
{
	H_AUTO_OGL(CVBDrvInfosGL_unlock)
	_VBHard->unlock(first, last);
}

// ***************************************************************************
bool CDriverGL3::setupVertexBuffer(CVertexBuffer& VB)
{
	H_AUTO_OGL(CDriverGL3_setupVertexBuffer)
	// 2. If necessary, do modifications.
	//==================================
	const bool touched = (VB.getTouchFlags() & (CVertexBuffer::TouchedReserve|CVertexBuffer::TouchedVertexFormat)) != 0;
	if( touched || (VB.DrvInfos == NULL))
	{
		// delete first
		if(VB.DrvInfos)
			delete VB.DrvInfos;
		VB.DrvInfos = NULL;

		// create only if some vertices
		if(VB.getNumVertices())
		{
			// 1. Retrieve/Create driver shader.
			//==================================
			// insert into driver list. (so it is deleted when driver is deleted).
			ItVBDrvInfoPtrList	it= _VBDrvInfos.insert(_VBDrvInfos.end(), (NL3D::IVBDrvInfos*)NULL);
			// create and set iterator, for future deletion.
			CVBDrvInfosGL3 *info = new CVBDrvInfosGL3(this, it, &VB);
			*it= VB.DrvInfos = info;

			// Preferred memory
			CVertexBuffer::TPreferredMemory preferred = VB.getPreferredMemory ();
			if ((preferred == CVertexBuffer::RAMVolatile) ||
				(preferred == CVertexBuffer::AGPVolatile) ||
				(preferred == CVertexBuffer::RAMPreferred ) )
				preferred = CVertexBuffer::AGPPreferred;

			const uint size = VB.capacity()*VB.getVertexSize();
			
			// Vertex buffer hard
			info->_VBHard = createVertexBufferHard(size, VB.capacity(), preferred, &VB);

			// Upload the data
			VB.setLocation ((CVertexBuffer::TLocation)preferred);
		}
	}

	return true;
}


// ***************************************************************************
bool		CDriverGL3::activeVertexBuffer(CVertexBuffer& VB)
{
	H_AUTO_OGL(CDriverGL3_activeVertexBuffer)
	// NB: must duplicate changes in activeVertexBufferHard()
	uint32	flags;

	// In any case, we'll erase any special vertex setup for Lightmap Material
	_LastVertexSetupIsLightMap= false;

	// setup
	if (!setupVertexBuffer(VB))
		return false;

	if (VB.getNumVertices()==0)
		return true;

	// Fill the buffer if in local memory
	VB.fillBuffer ();

	// Get VB flags, to setup matrixes and arrays.
	flags=VB.getVertexFormat();


	// 2. Setup Arrays.
	//===================
	// For MultiPass Material.
	CVBDrvInfosGL3		*info= safe_cast<CVBDrvInfosGL3*>((IVBDrvInfos*)VB.DrvInfos);
	if (!info->_VBHard ||  (info->_VBHard && !info->_VBHard->isInvalid()))
		_LastVB.setupVertexBuffer(VB);

	if (info->_VBHard == NULL)
	{
		// Disable the current vertexBufferHard if setuped.
		if(_CurrentVertexBufferHard)
			_CurrentVertexBufferHard->disable();
	}
	else
	{
		// 2. Setup Arrays.
		//===================

		// Enable the vertexArrayRange of this array.
		info->_VBHard->enable();
	}
	if (!info->_VBHard ||  (info->_VBHard && !info->_VBHard->isInvalid()))
	{
		setupGlArrays(_LastVB);
	}
	return true;
}

// ***************************************************************************
bool CDriverGL3::activeIndexBuffer(CIndexBuffer& IB)
{
	H_AUTO_OGL(CDriverGL3_activeIndexBuffer)
	_LastIB.setupIndexBuffer(IB);
	return true;
}

// ***************************************************************************

bool CDriverGL3::renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines)
{
	H_AUTO_OGL(CDriverGL3_renderLines)
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) || _LastIB._Values == NULL )
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
		setupPass(pass);

		refreshTexMatrices();

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

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderTriangles(CMaterial& mat, uint32 firstIndex, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderTriangles);

	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) || _LastIB._Values == NULL )
		return false;

	refreshTexMatrices();

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
		setupPass(pass);

		refreshTexMatrices();

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

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderSimpleTriangles(uint32 firstTri, uint32 ntris)
{
	H_AUTO_OGL(CDriverGL3_renderSimpleTriangles);

	nlassert(ntris>0);

	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

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

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawPoints(CMaterial& mat, uint32 startIndex, uint32 numPoints)
{
	H_AUTO_OGL(CDriverGL3_renderRawPoints)
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) )
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
		setupPass(pass);

		refreshTexMatrices();

		// draw the primitives.
		if(numPoints)
			glDrawArrays(GL_POINTS, startIndex, numPoints);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NPoints+= numPoints;
	_PrimitiveProfileOut.NPoints+= numPoints * nPass;

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawLines(CMaterial& mat, uint32 startIndex, uint32 numLines)
{
	H_AUTO_OGL(CDriverGL3_renderRawLines)
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) )
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
		setupPass(pass);

		refreshTexMatrices();

		// draw the primitives.
		if(numLines)
			glDrawArrays(GL_LINES, startIndex << 1, numLines << 1);
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NLines  += numLines ;
	_PrimitiveProfileOut.NLines += numLines  * nPass;

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawTriangles(CMaterial& mat, uint32 startIndex, uint32 numTris)
{
	H_AUTO_OGL(CDriverGL3_renderRawTriangles)
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) )
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
		setupPass(pass);

		refreshTexMatrices();

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

	return true;
}

// ***************************************************************************

bool CDriverGL3::renderRawQuads(CMaterial& mat, uint32 startIndex, uint32 numQuads)
{
	H_AUTO_OGL(CDriverGL3_renderRawQuads)
	if (!numQuads) return true;
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(mat) )
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
		setupPass(pass);

		refreshTexMatrices();

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

	return true;
}

// ***************************************************************************

void		CDriverGL3::setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId)
{
	H_AUTO_OGL(CDriverGL3_setupUVPtr)
	// sould not be called with vertex program Array setuped.
	nlassert(!_LastSetupGLArrayVertexProgram);

	_DriverGLStates.clientActiveTextureARB(stage);
	if (VB.VertexFormat & (CVertexBuffer::TexCoord0Flag<<uvId))
	{
		// Check type, if not supported, just ignore
		CVertexBuffer::TType uvType = VB.Type[CVertexBuffer::TexCoord0+uvId];
		if (uvType == CVertexBuffer::Float2 ||
			uvType == CVertexBuffer::Float3)
		{
			_DriverGLStates.enableTexCoordArray(true);
			uint numTexCoord = (uvType == CVertexBuffer::Float2) ? 2 : 3;
			_DriverGLStates.bindARBVertexBuffer(VB.VertexObjectId);
			// with arb buffers, position is relative to the start of the stream
			glTexCoordPointer(numTexCoord,GL_FLOAT,VB.VertexSize, VB.ValuePtr[CVertexBuffer::TexCoord0+uvId]);

		}
		else
		{
			_DriverGLStates.enableTexCoordArray(false);
		}
	}
	else
		_DriverGLStates.enableTexCoordArray(false);
}


// ***************************************************************************
void		CDriverGL3::mapTextureStageToUV(uint stage, uint uv)
{
	H_AUTO_OGL(CDriverGL3_mapTextureStageToUV)
	// Just call it for last VertexBuffer setuped.
	setupUVPtr(stage, _LastVB, uv);
}



// ***************************************************************************
// ***************************************************************************
// VertexBufferHard
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool			CDriverGL3::supportVertexBufferHard() const
{
	H_AUTO_OGL(CDriverGL3_supportVertexBufferHard)
	return true;
}

// ***************************************************************************
bool			CDriverGL3::supportVolatileVertexBuffer() const
{
	H_AUTO_OGL(CDriverGL3_supportVolatileVertexBuffer)
		return false;
}



// ***************************************************************************
bool			CDriverGL3::slowUnlockVertexBufferHard() const
{
	H_AUTO_OGL(CDriverGL3_slowUnlockVertexBufferHard)
	return _SlowUnlockVBHard;
}


// ***************************************************************************
uint			CDriverGL3::getMaxVerticesByVertexBufferHard() const
{
	H_AUTO_OGL(CDriverGL3_getMaxVerticesByVertexBufferHard)
	return std::numeric_limits<uint32>::max();
}


// ***************************************************************************
IVertexBufferHardGL	*CDriverGL3::createVertexBufferHard(uint size, uint numVertices, CVertexBuffer::TPreferredMemory vbType, CVertexBuffer *vb)
{
	H_AUTO_OGL(CDriverGL3_createVertexBufferHard)
	// choose the VertexArrayRange of good type
	IVertexArrayRange	*vertexArrayRange= NULL;
	switch(vbType)
	{
	case CVertexBuffer::AGPPreferred:
		vertexArrayRange= _AGPVertexArrayRange;
		break;
	case CVertexBuffer::StaticPreferred:
		if (getStaticMemoryToVRAM())
			vertexArrayRange= _VRAMVertexArrayRange;
		else
			vertexArrayRange= _AGPVertexArrayRange;
		break;
        default:
            break;
	}

	// If this one at least created (an extension support it).
	if( !vertexArrayRange )
		return NULL;
	else
	{
		// Create a CVertexBufferHardGL
		IVertexBufferHardGL		*vbHard = NULL;
		// let the VAR create the vbhard.
		vbHard= vertexArrayRange->createVBHardGL(size, vb);
		// if fails
		if(!vbHard)
		{
			return NULL;
		}
		else
		{
			// insert in list.
			return _VertexBufferHardSet.insert(vbHard);
		}
	}
}


// ***************************************************************************
const uint		CDriverGL3::NumCoordinatesType[CVertexBuffer::NumType]=
{
	1,	// Double1
	1,	// Float1
	1,	// Short1
	2,	// Double2
	2,	// Float2
	2,	// Short2
	3,	// Double3
	3,	// Float3
	3,	// Short3
	4,	// Double4
	4,	// Float4
	4,	// Short4
	4	// UChar4
};


// ***************************************************************************
const uint		CDriverGL3::GLType[CVertexBuffer::NumType]=
{
	GL_DOUBLE,	// Double1
	GL_FLOAT,	// Float1
	GL_SHORT,	// Short1
	GL_DOUBLE,	// Double2
	GL_FLOAT,	// Float2
	GL_SHORT,	// Short2
	GL_DOUBLE,	// Double3
	GL_FLOAT,	// Float3
	GL_SHORT,	// Short3
	GL_DOUBLE,	// Double4
	GL_FLOAT,	// Float4
	GL_SHORT,	// Short4
	GL_UNSIGNED_BYTE	// UChar4
};

// ***************************************************************************
const bool CDriverGL3::GLTypeIsIntegral[CVertexBuffer::NumType] =
{
	false,	// Double1
	false,	// Float1
	true,	// Short1
	false,	// Double2
	false,	// Float2
	true,	// Short2
	false,	// Double3
	false,	// Float3
	true,	// Short3
	false,	// Double4
	false,	// Float4
	true,	// Short4
	true	// UChar4
};



// ***************************************************************************
const uint		CDriverGL3::GLVertexAttribIndex[CVertexBuffer::NumValue]=
{
	0,	// Position
	2,	// Normal
	8,	// TexCoord0
	9,	// TexCoord1
	10,	// TexCoord2
	11,	// TexCoord3
	12,	// TexCoord4
	13,	// TexCoord5
	14,	// TexCoord6
	15,	// TexCoord7
	3,	// PrimaryColor
	4,	// SecondaryColor
	1,	// Weight
	6,	// Empty (PaletteSkin)
	5,	// Fog
	7,	// Empty
};



// ***************************************************************************
void		CDriverGL3::setupGlArraysStd(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArraysStd)
	uint32	flags= vb.VertexFormat;

	_DriverGLStates.bindARBVertexBuffer(vb.VertexObjectId);

	{
		{
			// setup vertex ptr.
			//-----------
			uint numVertexCoord = CVertexBuffer::NumComponentsType[vb.Type[CVertexBuffer::Position]];
			nlassert (numVertexCoord >= 2);
			_DriverGLStates.enableVertexArray(true);
			glVertexPointer(numVertexCoord, GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Position]);

			// setup normal ptr.
			//-----------
			// Check for normal param in vertex buffer
			if (flags & CVertexBuffer::NormalFlag)
			{
				// Check type
				nlassert (vb.Type[CVertexBuffer::Normal]==CVertexBuffer::Float3);

				_DriverGLStates.enableNormalArray(true);
				glNormalPointer(GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Normal]);
			}
			else
			{
				_DriverGLStates.enableNormalArray(false);
			}

			// Setup Color
			//-----------
			// Check for color param in vertex buffer
			if (flags & CVertexBuffer::PrimaryColorFlag)
			{
				// Check type
				nlassert (vb.Type[CVertexBuffer::PrimaryColor]==CVertexBuffer::UChar4);
				_DriverGLStates.enableColorArray(true);
				// Setup ATI VBHard or std ptr.
				glColorPointer(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[CVertexBuffer::PrimaryColor]);
			}
			else
			{
				_DriverGLStates.enableColorArray(false);
			}
		}
	}

	// Setup Uvs
	//-----------
	// Get the routing
	for(uint i=0; i<inlGetNumTextStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, vb, vb.UVRouting[i]);
	}
}


// ***************************************************************************
void		CDriverGL3::toggleGlArraysForNVVertexProgram()
{
	H_AUTO_OGL(CDriverGL3_toggleGlArraysForNVVertexProgram)
	// If change of setup type, must disable olds.
	//=======================

	// If last was a VertexProgram setup, and now it is a standard GL array setup.
	if( _LastSetupGLArrayVertexProgram && !isVertexProgramEnabled () )
	{

		// Disable all VertexAttribs.
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Index
			uint glIndex=GLVertexAttribIndex[value];
			_DriverGLStates.enableVertexAttribArray(glIndex, false);
		}
		_DriverGLStates.enableColorArray(false);
		_DriverGLStates.enableSecondaryColorArray(false);

		// no more a vertex program setup.
		_LastSetupGLArrayVertexProgram= false;
	}

	// If last was a standard GL array setup, and now it is a VertexProgram setup.
	if( !_LastSetupGLArrayVertexProgram && isVertexProgramEnabled () )
	{
		// Disable all standards ptrs.
		_DriverGLStates.enableVertexArray(false);
		_DriverGLStates.enableNormalArray(false);
		_DriverGLStates.enableColorArray(false);
		for(uint i=0; i<inlGetNumTextStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}

		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}
}

// ***************************************************************************
void		CDriverGL3::toggleGlArraysForARBVertexProgram()
{
	H_AUTO_OGL(CDriverGL3_toggleGlArraysForARBVertexProgram)
	// If change of setup type, must disable olds.
	//=======================

	// If last was a VertexProgram setup, and now it is a standard GL array setup.
	if( _LastSetupGLArrayVertexProgram && !isVertexProgramEnabled () )
	{

		if (_Extensions.ATITextureEnvCombine3)
		{
			// fix for ATI : when switching from Vertex Program to fixed Pipe, must clean texture, otherwise texture may be disabled in next render
			// (seems to be a driver bug)
			ITexture *oldTex[IDRV_MAT_MAXTEXTURES];
			for(uint stage=0 ; stage < inlGetNumTextStages() ; stage++)
			{
				oldTex[stage] = _CurrentTexture[stage];
				// activate the texture, or disable texturing if NULL.
				activateTexture(stage, NULL);
			}

			glBegin(GL_QUADS);
			glVertex4f(0.f, 0.f, 0.f, 1.f);
			glVertex4f(0.f, 0.f, 0.f, 1.f);
			glVertex4f(0.f, 0.f, 0.f, 1.f);
			glVertex4f(0.f, 0.f, 0.f, 1.f);
			glEnd();


			for(uint stage=0 ; stage<inlGetNumTextStages() ; stage++)
			{
				// activate the texture, or disable texturing if NULL.
				activateTexture(stage, oldTex[stage]);
			}
		}

		// Disable all VertexAttribs.
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Index
			uint glIndex=GLVertexAttribIndex[value];
			_DriverGLStates.enableVertexAttribArrayARB(glIndex, false);
		}
		// no more a vertex program setup.
		_LastSetupGLArrayVertexProgram= false;
	}

	// If last was a standard GL array setup, and now it is a VertexProgram setup.
	if( !_LastSetupGLArrayVertexProgram && isVertexProgramEnabled () )
	{
		// Disable all standards ptrs.
		_DriverGLStates.enableVertexArray(false);
		_DriverGLStates.enableNormalArray(false);
		_DriverGLStates.enableColorArray(false);
		_DriverGLStates.enableSecondaryColorArray(false);
		for(uint i=0; i<inlGetNumTextStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}

		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}
}



// ***************************************************************************
void		CDriverGL3::toggleGlArraysForEXTVertexShader()
{
	H_AUTO_OGL(CDriverGL3_toggleGlArraysForEXTVertexShader)
	// If change of setup type, must disable olds.
	//=======================


	// If last was a VertexProgram setup, and now it is a standard GL array setup.
	if( _LastSetupGLArrayVertexProgram && !isVertexProgramEnabled () )
	{
		CVertexProgram *vp = _LastSetuppedVP;
		if (vp)
		{
			CVertexProgamDrvInfosGL3 *drvInfo = NLMISC::safe_cast<CVertexProgamDrvInfosGL3 *>((IVertexProgramDrvInfos *) vp->_DrvInfo);
			if (drvInfo)
			{
				// Disable all VertexAttribs.
				for (uint value=0; value<CVertexBuffer::NumValue; value++)
				{
					_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(value, false, drvInfo->Variants);
				}
			}
		}
		// no more a vertex program setup.
		_LastSetupGLArrayVertexProgram= false;
	}

	// If last was a standard GL array setup, and now it is a VertexProgram setup.
	if( !_LastSetupGLArrayVertexProgram && isVertexProgramEnabled () )
	{
		// Disable all standards ptrs.
		_DriverGLStates.enableVertexArray(false);
		_DriverGLStates.enableNormalArray(false);
		_DriverGLStates.enableColorArray(false);
		_DriverGLStates.enableSecondaryColorArray(false);
		for(uint i=0; i<inlGetNumTextStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}

		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}
}

// ***************************************************************************
void		CDriverGL3::setupGlArraysForNVVertexProgram(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArraysForNVVertexProgram)
	uint16	flags= vb.VertexFormat;

	_DriverGLStates.bindARBVertexBuffer(vb.VertexObjectId);

	// For each value
	for (uint value=0; value<CVertexBuffer::NumValue; value++)
	{
		//nldebug("3D:   value = %d", value);
		// Flag
		uint16 flag=1<<value;

		// Type
		CVertexBuffer::TType type=vb.Type[value];

		// Index
		uint glIndex=GLVertexAttribIndex[value];

		// Not setuped value and used
		if (flags & flag)
		{
			/* OpenGL Driver Bug with VertexProgram, UChar4 type, and VertexArrayRange.
				Don't work and lead to very poor performance (1/10) (VAR is "disabled").
			*/
			// Test if can use glColorPointer() / glSecondaryColorPointerEXT() instead.
			if( (glIndex==3 || glIndex==4) )
			{
				if( type == CVertexBuffer::UChar4 )
				{
					// Must disable VertexAttrib array.
					_DriverGLStates.enableVertexAttribArray(glIndex, false);

					// Active this value, with standard gl calls
					if(glIndex==3)
					{
						// Primary color
						_DriverGLStates.enableColorArray(true);
						glColorPointer(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[value]);
					}
					else
					{
						// Secondary color
						_DriverGLStates.enableSecondaryColorArray(true);
						nglSecondaryColorPointerEXT(4, GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[value]);
					}
				}
				else
				{
					// Can use normal VertexAttribArray.
					// Disable first standard Color Array.
					if(glIndex==3)
						_DriverGLStates.enableColorArray(false);
					else
						_DriverGLStates.enableSecondaryColorArray(false);

					// Active this value
					_DriverGLStates.enableVertexAttribArray(glIndex, true);
					nglVertexAttribPointerNV (glIndex, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
				}
			}
			// Else normal case, can't do anything for other values with UChar4....
			else
			{
				// Active this value
				_DriverGLStates.enableVertexAttribArray(glIndex, true);
				nglVertexAttribPointerNV (glIndex, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
			}
		}
		else
		{
			_DriverGLStates.enableVertexAttribArray(glIndex, false);
			/* OpenGL Driver Bug with VertexProgram, UChar4 type, and VertexArrayRange.
				Must also disable colorArray in standard gl calls.
			*/
			if(glIndex==3)
				_DriverGLStates.enableColorArray(false);
			else if(glIndex==4)
				_DriverGLStates.enableSecondaryColorArray(false);
		}
	}

	_DriverGLStates.bindARBVertexBuffer(0);

}

// tells for each vertex argument if it must be normalized when it is an integral type
static const GLboolean ARBVertexProgramMustNormalizeAttrib[] =
{
	GL_FALSE, // Position
	GL_TRUE,  // Normal
	GL_FALSE, // TexCoord0
	GL_FALSE, // TexCoord1
	GL_FALSE, // TexCoord2
	GL_FALSE, // TexCoord3
	GL_FALSE, // TexCoord4
	GL_FALSE, // TexCoord5
	GL_FALSE, // TexCoord6
	GL_FALSE, // TexCoord7
	GL_TRUE,  // PrimaryColor
	GL_TRUE,  // SecondaryColor
	GL_TRUE,  // Weight
	GL_FALSE, // PaletteSkin
	GL_FALSE, // Fog
	GL_FALSE, // Empty
};

// ***************************************************************************
void		CDriverGL3::setupGlArraysForARBVertexProgram(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArraysForARBVertexProgram)

	uint32	flags= vb.VertexFormat;

	nlctassert(CVertexBuffer::NumValue == sizeof(ARBVertexProgramMustNormalizeAttrib) / sizeof(ARBVertexProgramMustNormalizeAttrib[0]));
	_DriverGLStates.bindARBVertexBuffer(vb.VertexObjectId);

	{
		// For each value
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Flag
			uint16 flag=1<<value;

			// Type
			CVertexBuffer::TType type=vb.Type[value];
			{
				// Index
				uint glIndex=GLVertexAttribIndex[value];
				// Not setuped value and used
				if (flags & flag)
				{
					_DriverGLStates.enableVertexAttribArrayARB(glIndex, true);
					GLboolean mustNormalize = GL_FALSE;
					if (GLTypeIsIntegral[type])
					{
						mustNormalize = ARBVertexProgramMustNormalizeAttrib[value];
					}
					nglVertexAttribPointerARB(glIndex, NumCoordinatesType[type], GLType[type], mustNormalize, vb.VertexSize, vb.ValuePtr[value]);
				}
				else
				{
					_DriverGLStates.enableVertexAttribArrayARB(glIndex, false);
				}
			}
		}
	}
}



// ***************************************************************************
void		CDriverGL3::setupGlArraysForEXTVertexShader(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArraysForEXTVertexShader)


	CVertexProgram *vp = _LastSetuppedVP;
	if (!vp) return;
	CVertexProgamDrvInfosGL3 *drvInfo = NLMISC::safe_cast<CVertexProgamDrvInfosGL3 *>((IVertexProgramDrvInfos *) vp->_DrvInfo);
	if (!drvInfo) return;

	uint32	flags= vb.VertexFormat;


	_DriverGLStates.bindARBVertexBuffer(vb.VertexObjectId);

	// For each value
	for (uint value=0; value<CVertexBuffer::NumValue; value++)
	{
		// Flag
		uint16 flag=1<<value;

		// Type
		CVertexBuffer::TType type=vb.Type[value];

		// Index
		uint glIndex=GLVertexAttribIndex[value];

		// Not setuped value and used
		if (flags & flag & drvInfo->UsedVertexComponents)
		{
			_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(glIndex, true, drvInfo->Variants);

			// use variant or open gl standard array

			{
				switch(value)
				{
					case CVertexBuffer::Position: // position
					{
						nlassert(NumCoordinatesType[type] >= 2);
						glVertexPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Weight: // skin weight
					{
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						nglVariantPointerEXT(drvInfo->Variants[CDriverGL3::EVSSkinWeightVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Normal: // normal
					{
						nlassert(NumCoordinatesType[type] == 3); // must have 3 components for normals
						glNormalPointer(GLType[type], vb.VertexSize, vb.ValuePtr[CVertexBuffer::Normal]);
					}
					break;
					case CVertexBuffer::PrimaryColor: // color
					{
						nlassert(NumCoordinatesType[type] >= 3); // must have 3 or 4 components for primary color
						glColorPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::SecondaryColor: // secondary color
					{
						// implemented using a variant, as not available with EXTVertexShader
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						nglVariantPointerEXT(drvInfo->Variants[CDriverGL3::EVSSecondaryColorVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Fog: // fog coordinate
					{
						// implemented using a variant
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						nglVariantPointerEXT(drvInfo->Variants[CDriverGL3::EVSFogCoordsVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::PaletteSkin: // palette skin
					{
						// implemented using a variant
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						nglVariantPointerEXT(drvInfo->Variants[CDriverGL3::EVSPaletteSkinVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Empty: // empty
						nlstop;
					break;
					case CVertexBuffer::TexCoord0:
					case CVertexBuffer::TexCoord1:
					case CVertexBuffer::TexCoord2:
					case CVertexBuffer::TexCoord3:
					case CVertexBuffer::TexCoord4:
					case CVertexBuffer::TexCoord5:
					case CVertexBuffer::TexCoord6:
					case CVertexBuffer::TexCoord7:
					{
						_DriverGLStates.clientActiveTextureARB(value - CVertexBuffer::TexCoord0);
						glTexCoordPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					default:
						nlstop; // invalid value
					break;
				}
			}
		}
		else
		{
			_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(glIndex, false, drvInfo->Variants);
		}
	}
}



// ***************************************************************************
void		CDriverGL3::setupGlArrays(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArrays)

	// Standard case (NVVertexProgram or no vertex program case)
	if (_Extensions.NVVertexProgram)
	{
		toggleGlArraysForNVVertexProgram();
		// Use a vertex program ?
		if (!isVertexProgramEnabled ())
		{
			setupGlArraysStd(vb);
		}
		else
		{
			setupGlArraysForNVVertexProgram(vb);
		}
	}
	else if (_Extensions.ARBVertexProgram)
	{
		toggleGlArraysForARBVertexProgram();
		// Use a vertex program ?
		if (!isVertexProgramEnabled ())
		{
			setupGlArraysStd(vb);
		}
		else
		{
			setupGlArraysForARBVertexProgram(vb);
		}
	}
	else if (_Extensions.EXTVertexShader)
	{
		toggleGlArraysForEXTVertexShader();
		// Use a vertex program ?
		if (!isVertexProgramEnabled ())
		{
			setupGlArraysStd(vb);
		}
		else
		{
			setupGlArraysForEXTVertexShader(vb);
		}
	}
	else
	{
		// no vertex programs
		setupGlArraysStd(vb);
	}
}


// ***************************************************************************
void		CVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb)
{
	H_AUTO_OGL(CDriverGL3_setupVertexBuffer)
	sint	i;
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	// Lock the buffer
	CVertexBufferReadWrite access;
	uint8 *ptr;
	CVBDrvInfosGL3 *info= safe_cast<CVBDrvInfosGL3*>((IVBDrvInfos*)vb.DrvInfos);
	nlassert (info);

	ptr = (uint8*)info->_VBHard->getPointer();
	info->_VBHard->setupVBInfos(*this);

	// Get value pointer
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Value used ?
		if (VertexFormat&(1<<i))
		{
			// Get the pointer
			ValuePtr[i]= ptr+vb.getValueOffEx((CVertexBuffer::TValue)i);

			// Type of the value
			Type[i]=vb.getValueType (i);
		}
	}

	// Copy the UVRouting table
	const uint8 *uvRouting = vb.getUVRouting();
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		UVRouting[i] = uvRouting[i];
	}
}


// ***************************************************************************
void			CDriverGL3::resetVertexArrayRange()
{
	H_AUTO_OGL(CDriverGL3_resetVertexArrayRange)
	if(_CurrentVertexBufferHard)
	{
		// Must ensure it has ended any drawing
		_CurrentVertexBufferHard->lock();
		_CurrentVertexBufferHard->unlock();
		// disable it
		_CurrentVertexBufferHard->disable();
	}
	// Clear any VertexBufferHard created.
	_VertexBufferHardSet.clear();
}


// ***************************************************************************
bool			CDriverGL3::initVertexBufferHard(uint agpMem, uint vramMem)
{
	H_AUTO_OGL(CDriverGL3_initVertexBufferHard)
	if(!supportVertexBufferHard())
		return false;

	// must be supported
	if(!_AGPVertexArrayRange || !_VRAMVertexArrayRange)
		return false;

	// First, reset any VBHard created.
	resetVertexArrayRange();

	return true;
}


// ***************************************************************************

CIndexBufferInfo::CIndexBufferInfo()
{
	H_AUTO_OGL(CIndexBufferInfo_CIndexBufferInfo)
	_Values = NULL;
}

// ***************************************************************************

void CIndexBufferInfo::setupIndexBuffer(CIndexBuffer &ib)
{
	H_AUTO_OGL(CIndexBufferInfo_setupIndexBuffer)
	CIndexBufferReadWrite access;
	ib.lock (access);
	_Values = access.getPtr();
	_Format = access.getFormat();
}

// ***************************************************************************

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
