/** \file driver_opengl_pixel_program.cpp
 * OpenGL driver implementation for pixel program manipulation.
 *
 * $Id: driver_opengl_pixel_program.cpp,v 1.1.2.4 2007/07/09 15:29:00 legallo Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/pixel_program.h"
#include <algorithm>

// tmp
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;
 
#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{
	
#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************

CPixelProgamDrvInfosGL::CPixelProgamDrvInfosGL (CDriverGL *drv, ItGPUPrgDrvInfoPtrList it) : IProgramDrvInfos (drv, it) 
{
	H_AUTO_OGL(CPixelProgamDrvInfosGL_CPixelProgamDrvInfosGL)

#ifndef USE_OPENGLES
	// Extension must exist
	nlassert(drv->_Extensions.ARBFragmentProgram);

	if (drv->_Extensions.ARBFragmentProgram) // ARB implementation
	{
		nglGenProgramsARB(1, &ID);
	}
#endif
}

// ***************************************************************************

bool CDriverGL::supportPixelProgram(CPixelProgram::TProfile profile) const
{
	H_AUTO_OGL(CPixelProgamDrvInfosGL_supportPixelProgram_profile);

	switch (profile)
	{
	case CPixelProgram::arbfp1:
		return _Extensions.ARBFragmentProgram;
	case CPixelProgram::fp40:
		return _Extensions.NVFragmentProgram2;
	default:
		break;
	}
	return false;
}

// ***************************************************************************

bool CDriverGL::activePixelProgram(CPixelProgram *program)
{
	H_AUTO_OGL(CDriverGL_activePixelProgram)

	if (_Extensions.ARBFragmentProgram)
	{
		return activeARBPixelProgram(program);
	}

	return false;
}

// ***************************************************************************

bool CDriverGL::compilePixelProgram(NL3D::CPixelProgram *program)
{
#ifndef USE_OPENGLES
	// Program setuped ?
	if (program->m_DrvInfo == NULL)
	{
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		_PixelProgramEnabled = false;
		
		// Insert into driver list. (so it is deleted when driver is deleted).
		ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);

		// Create a driver info
		CPixelProgamDrvInfosGL *drvInfo;
		*it = drvInfo = new CPixelProgamDrvInfosGL(this, it);
		// Set the pointer
		program->m_DrvInfo = drvInfo;
	
		if (!setupPixelProgram(program, drvInfo->ID))
		{
			delete drvInfo;
			program->m_DrvInfo = NULL;
			//_GPUPrgDrvInfos.erase(it); // not needed as ~IProgramDrvInfos() already does it
			return false;
		}
	}

	return true;
#else
	return false;
#endif
}

// ***************************************************************************

bool CDriverGL::activeARBPixelProgram(CPixelProgram *program)
{
	H_AUTO_OGL(CDriverGL_activeARBPixelProgram)

#ifndef USE_OPENGLES
	// Setup or unsetup ?
	if (program)
	{
		// Program setuped ?
		if (!CDriverGL::compilePixelProgram(program)) return false;

		// Cast the driver info pointer
		CPixelProgamDrvInfosGL *drvInfo = safe_cast<CPixelProgamDrvInfosGL*>((IProgramDrvInfos*)program->m_DrvInfo);

		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		_PixelProgramEnabled = true;
		nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, drvInfo->ID);
				
		_LastSetuppedPP = program;
	}
	else
	{		
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		_PixelProgramEnabled = false;
	}
	
	return true;
#else
	return false;
#endif
}

// ***************************************************************************

bool CDriverGL::setupPixelProgram(CPixelProgram *program, GLuint id/*, bool &specularWritten*/)
{
	H_AUTO_OGL(CDriverGL_setupARBPixelProgram);
	
#ifndef USE_OPENGLES
	CPixelProgamDrvInfosGL *drvInfo = static_cast<CPixelProgamDrvInfosGL *>((IProgramDrvInfos *)program->m_DrvInfo);

	// Find a supported pixel program profile
	IProgram::CSource *source = NULL;
	for (uint i = 0; i < program->getSourceNb(); ++i)
	{
		if (supportPixelProgram(program->getSource(i)->Profile))
		{
			source = program->getSource(i);
		}
	}
	if (!source)
	{
		nlwarning("No supported source profile for pixel program");
		return false;
	}

	// Compile the program
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, id);
	glGetError();
	nglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, source->SourceLen, source->SourcePtr);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		if (err == GL_INVALID_OPERATION)
		{
			GLint position;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);
			nlassert(position != -1); // there was an error..
			nlassert(position < (GLint) source->SourceLen);
			uint line = 0;
			const char *lineStart = source->SourcePtr;
			for(uint k = 0; k < (uint) position; ++k)
			{
				if (source->SourcePtr[k] == '\n') 
				{
					lineStart = source->SourcePtr + k;
					++line;
				}
			}
			nlwarning("ARB fragment program parse error at line %d.", (int) line);
			// search end of line
			const char *lineEnd = source->SourcePtr + source->SourceLen;
			for(uint k = position; k < source->SourceLen; ++k)
			{
				if (source->SourcePtr[k] == '\n')
				{
					lineEnd = source->SourcePtr + k;
					break;
				}
			}
			nlwarning(std::string(lineStart, lineEnd).c_str());
			// display the gl error msg
			const GLubyte *errorMsg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			nlassert((const char *) errorMsg);
			nlwarning((const char *) errorMsg);
		}
		nlassert(0);
		return false;
	}

	// Set parameters for assembly programs
	drvInfo->ParamIndices = source->ParamIndices;

	// Build the feature info
	program->buildInfo(source);

	return true;
#else
	return false;
#endif
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
