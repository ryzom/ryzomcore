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

using namespace std;
using namespace NLMISC;

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

inline void CDriverGL::setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3)
{
	H_AUTO_OGL(CDriverGL_setUniform4f);

#ifndef USE_OPENGLES
	switch (program)
	{
	case VertexProgram:
		if (_Extensions.NVVertexProgram)
		{
			// Setup constant
			nglProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, index, f0, f1, f2, f3);
		}
		else if (_Extensions.ARBVertexProgram)
		{
			nglProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, index, f0, f1, f2, f3);
		}
		else if (_Extensions.EXTVertexShader)
		{
			float datas[] = { f0, f1, f2, f3 };
			nglSetInvariantEXT(_EVSConstantHandle + index, GL_FLOAT, datas);
		}
		break;
	case PixelProgram:
		if (_Extensions.ARBFragmentProgram)
		{
			nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, index, f0, f1, f2, f3);
		}
		break;
	}
#endif
}

inline void CDriverGL::setUniform4fv(TProgram program, uint index, size_t num, const float *src)
{
	H_AUTO_OGL(CDriverGL_setUniform4fv);

#ifndef USE_OPENGLES
	switch (program)
	{
	case VertexProgram:
		if (_Extensions.NVVertexProgram)
		{
			nglProgramParameters4fvNV(GL_VERTEX_PROGRAM_NV, index, num, src);
		}
		else if (_Extensions.ARBVertexProgram) // ARB pixel and geometry program will only exist when ARB vertex program exists
		{
			for (uint k = 0; k < num; ++k)
			{
				nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + k, src + 4 * k);
			}
		}
		else if (_Extensions.EXTVertexShader)
		{
			for (uint k = 0; k < num; ++k)
			{
				nglSetInvariantEXT(_EVSConstantHandle + index + k, GL_FLOAT, (void *)(src + 4 * k));
			}
		}
		break;
	case PixelProgram:
		if (_Extensions.ARBFragmentProgram) // ARB pixel and geometry program will only exist when ARB vertex program exists
		{
			for (uint k = 0; k < num; ++k)
			{
				nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + k, src + 4 * k);
			}
		}
		break;
	}
#endif
}

void CDriverGL::setUniform1f(TProgram program, uint index, float f0)
{
	CDriverGL::setUniform4f(program, index, f0, 0.f, 0.f, 0.f);
}

void CDriverGL::setUniform2f(TProgram program, uint index, float f0, float f1)
{
	CDriverGL::setUniform4f(program, index, f0, f1, 0.f, 0.f);
}

void CDriverGL::setUniform3f(TProgram program, uint index, float f0, float f1, float f2)
{
	CDriverGL::setUniform4f(program, index, f0, f1, f2, 0.0f);
}

void CDriverGL::setUniform1i(TProgram program, uint index, sint32 i0)
{

}

void CDriverGL::setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1)
{

}

void CDriverGL::setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2)
{

}

void CDriverGL::setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{

}

void CDriverGL::setUniform1ui(TProgram program, uint index, uint32 ui0)
{

}

void CDriverGL::setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1)
{

}

void CDriverGL::setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2)
{

}

void CDriverGL::setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{

}

void CDriverGL::setUniform3f(TProgram program, uint index, const NLMISC::CVector& v)
{
	CDriverGL::setUniform4f(program, index, v.x, v.y, v.z, 0.f);
}

void CDriverGL::setUniform4f(TProgram program, uint index, const NLMISC::CVector& v, float f3)
{
	CDriverGL::setUniform4f(program, index, v.x, v.y, v.z, f3);
}

void CDriverGL::setUniform4x4f(TProgram program, uint index, const NLMISC::CMatrix& m)
{
	H_AUTO_OGL(CDriverGL_setUniform4x4f);

	// TODO: Verify this!
	NLMISC::CMatrix mat = m;
	mat.transpose();
	const float *md = mat.get();

	CDriverGL::setUniform4fv(program, index, 4, md);
}

void CDriverGL::setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src)
{
	
}

void CDriverGL::setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src)
{
	
}

const uint CDriverGL::GLMatrix[IDriver::NumMatrix]=
{
	GL_MODELVIEW,
	GL_PROJECTION,
#ifdef USE_OPENGLES
	GL_MODELVIEW
#else
	GL_MODELVIEW_PROJECTION_NV
#endif
};

const uint CDriverGL::GLTransform[IDriver::NumTransform]=
{
#ifdef USE_OPENGLES
	0,
	0,
	0,
	0
#else
	GL_IDENTITY_NV,
	GL_INVERSE_NV,
	GL_TRANSPOSE_NV,
	GL_INVERSE_TRANSPOSE_NV
#endif
};

void CDriverGL::setUniformMatrix(NL3D::IDriver::TProgram program, uint index, NL3D::IDriver::TMatrix matrix, NL3D::IDriver::TTransform transform)
{
	H_AUTO_OGL(CDriverGL_setUniformMatrix);

#ifndef USE_OPENGLES
	// Vertex program exist ?
	if (program == VertexProgram && _Extensions.NVVertexProgram)
	{
		// First, ensure that the render setup is correclty setuped.
		refreshRenderSetup();

		// Track the matrix
		nglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, index, GLMatrix[matrix], GLTransform[transform]);
		// Release Track => matrix data is copied.
		nglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, index, GL_NONE, GL_IDENTITY_NV);
	}
	else
	{
		// First, ensure that the render setup is correctly setuped.
		refreshRenderSetup();

		CMatrix mat;
		switch (matrix)
		{
			case IDriver::ModelView:
				mat = _ModelViewMatrix;
			break;
			case IDriver::Projection:
				{
					refreshProjMatrixFromGL();
					mat = _GLProjMat;
				}
			break;
			case IDriver::ModelViewProjection:
				refreshProjMatrixFromGL();
				mat = _GLProjMat * _ModelViewMatrix;
			break;
            default:
                break;
		}

		switch(transform)
		{
			case IDriver::Identity: break;
			case IDriver::Inverse:
				mat.invert();
			break;
			case IDriver::Transpose:
				mat.transpose();
			break;
			case IDriver::InverseTranspose:
				mat.invert();
				mat.transpose();
			break;
            default:
                break;
		}

		mat.transpose();
		const float *md = mat.get();
		
		CDriverGL::setUniform4fv(program, index, 4, md);
	}
#endif
}

void CDriverGL::setUniformFog(NL3D::IDriver::TProgram program, uint index)
{
	H_AUTO_OGL(CDriverGL_setUniformFog)
	
	const float *values = _ModelViewMatrix.get();
	CDriverGL::setUniform4f(program, index, -values[2], -values[6], -values[10], -values[14]);
}

bool CDriverGL::setUniformDriver(TProgram program)
{
	IGPUProgram *prog = NULL;
	switch (program)
	{
	case VertexProgram:
		prog = _LastSetuppedVP;
		break;
	case PixelProgram:
		prog = _LastSetuppedPP;
		break;
	}
	if (!prog) return false;

	const CGPUProgramFeatures &features = prog->features();

	if (features.DriverFlags)
	{
		// todo
	}

	return true;
}

bool CDriverGL::setUniformMaterial(TProgram program, CMaterial &material)
{
	IGPUProgram *prog = NULL;
	switch (program)
	{
	case VertexProgram:
		prog = _LastSetuppedVP;
		break;
	case PixelProgram:
		prog = _LastSetuppedPP;
		break;
	}
	if (!prog) return false;

	const CGPUProgramFeatures &features = prog->features();

	// These are also already set by setupMaterial, so setupMaterial uses setUniformMaterialInternal instead
	if (features.MaterialFlags & (CGPUProgramFeatures::TextureStages | CGPUProgramFeatures::TextureMatrices))
	{
		if (features.MaterialFlags & CGPUProgramFeatures::TextureStages)
		{
			for (uint stage = 0; stage < inlGetNumTextStages(); ++stage)
			{
				ITexture *text= material.getTexture(uint8(stage));

				// Must setup textures each frame. (need to test if touched).
				if (text != NULL && !setupTexture(*text))
					return false;

				// activate the texture, or disable texturing if NULL.
				activateTexture(stage, text);

				// If texture not NULL, Change texture env function.
				setTextureEnvFunction(stage, material);
			}

			
		}
		if (features.MaterialFlags & CGPUProgramFeatures::TextureMatrices)
		{
			// Textures user matrix
			setupUserTextureMatrix(inlGetNumTextStages(), material);
		}
	}

	return true;
}

bool CDriverGL::setUniformMaterialInternal(TProgram program, CMaterial &material)
{
	IGPUProgram *prog = NULL;
	switch (program)
	{
	case VertexProgram:
		prog = _LastSetuppedVP;
		break;
	case PixelProgram:
		prog = _LastSetuppedPP;
		break;
	}
	if (!prog) return false;

	const CGPUProgramFeatures &features = prog->features();

	if (features.MaterialFlags & ~(CGPUProgramFeatures::TextureStages | CGPUProgramFeatures::TextureMatrices))
	{
		// todo
	}

	return true;
}

void CDriverGL::setUniformParams(TProgram program, const CGPUProgramParams &params)
{
	// todo
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
