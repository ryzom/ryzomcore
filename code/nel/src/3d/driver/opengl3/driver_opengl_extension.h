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

#ifndef NL_OPENGL_EXTENSION_H
#define NL_OPENGL_EXTENSION_H


#include "nel/misc/types_nl.h"
#include "nel/misc/string_common.h"

#include "driver_opengl_extension_def.h"

namespace	NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************
/// The extensions used by NL3D.
struct	CGlExtensions
{
	std::string GLVersion;

	// Required extensions
	bool	GLCore;
	bool	ARBSeparateShaderObjects;
	uint	NbFragmentTextureUnits;

	// Optional extensions
	bool	EXTTextureCompressionS3TC;
	bool	EXTTextureFilterAnisotropic;
	float	EXTTextureFilterAnisotropicMaximum;

	// Required Extensions. (old)
	bool	ARBMultiTexture;

	// WGL ARB extensions, true if supported
	bool	WGLARBPBuffer;
	bool	WGLARBPixelFormat;
	bool	WGLEXTSwapControl;
	bool	WGLARBCreateContextProfile;

	// GLX extensions, true if supported
	bool	GLXEXTSwapControl;
	bool	GLXSGISwapControl;
	bool	GLXMESASwapControl;

public:
	CGlExtensions()
	{
		// Fill all false by default.
		GLCore = false;
		ARBSeparateShaderObjects = false;
		NbFragmentTextureUnits = 0;

		EXTTextureCompressionS3TC = false;
		EXTTextureFilterAnisotropic = false;
		EXTTextureFilterAnisotropicMaximum = 0.f;

		ARBMultiTexture= false;

		WGLARBPBuffer = false;
		WGLARBPixelFormat = false;
		WGLEXTSwapControl = false;
		WGLARBCreateContextProfile = false;

		GLXEXTSwapControl = false;
		GLXSGISwapControl = false;
		GLXMESASwapControl = false;
	}

	std::string toString()
	{
		std::string result = "OpenGL version ";
		result += GLVersion;
		result += "; Available extensions:";

		result += "\n  Core: ";
		result += GLCore ? "GLCore " : "";

		result += "\n  Programs: ";
		result += ARBSeparateShaderObjects ? "ARBSeparateShaderObjects " : "";

		result += "\n  Texturing: ";
		result += ARBMultiTexture ? "ARBMultiTexture " : "";
		result += EXTTextureCompressionS3TC ? "EXTTextureCompressionS3TC " : "";
		result += EXTTextureFilterAnisotropic ? "EXTTextureFilterAnisotropic (Maximum = " + NLMISC::toString(EXTTextureFilterAnisotropicMaximum) + ") " : "";
		result += "texture stages(*) = ";
		result += NLMISC::toString(NbFragmentTextureUnits);

#ifdef NL_OS_WINDOWS
		result += "\n  WindowsGL: ";
		result += WGLARBPBuffer ? "WGLARBPBuffer " : "";
		result += WGLARBPixelFormat ? "WGLARBPixelFormat " : "";
		result += WGLEXTSwapControl ? "WGLEXTSwapControl " : "";
		result += WGLARBCreateContextProfile ? "WGLARBCreateContextProfile" : "";
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
		result += "\n  GLX: ";
		result += GLXEXTSwapControl ? "GLXEXTSwapControl " : "";
		result += GLXSGISwapControl ? "GLXSGISwapControl " : "";
		result += GLXMESASwapControl ? "GLXMESASwapControl " : "";
#endif

		return result;
	}

};

// ***************************************************************************

#if defined(NL_OS_WINDOWS)
/// This function will test and register WGL functions before than the gl context is created
bool registerWGlExtensions(CGlExtensions &ext, HDC hDC);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
/// This function will test and register GLX functions before than the gl context is created
bool registerGlXExtensions(CGlExtensions &ext, Display *dpy, sint screen);
#endif // NL_OS_WINDOWS

/// This function test and register the extensions for the current GL context.
bool registerGlExtensions(CGlExtensions &ext);

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

// ***************************************************************************
// The exported function names
/* NB: We named all like nglActiveTexture (n for NEL :)
	to avoid compilation conflict with future version of gl.h
	eg: gl.h Version 1.2 define glActiveTexture so we can't use it.

	NB: we do it for all (EXT, NV, ARB extension) even it should be useful only for ARB ones.
*/

// Core 3.30
extern PFNGLGETSTRINGIPROC								nglGetStringi;

extern PFNGLCLEARBUFFERIVPROC							nglClearBufferiv;
extern PFNGLCLEARBUFFERUIVPROC							nglClearBufferuiv;
extern PFNGLCLEARBUFFERFVPROC							nglClearBufferfv;
extern PFNGLCLEARBUFFERFIPROC							nglClearBufferfi;

extern PFNGLATTACHSHADERPROC							nglAttachShader;
extern PFNGLCOMPILESHADERPROC							nglCompileShader;
extern PFNGLCREATEPROGRAMPROC							nglCreateProgram;
extern PFNGLCREATESHADERPROC							nglCreateShader;
extern PFNGLDELETEPROGRAMPROC							nglDeleteProgram;
extern PFNGLDELETESHADERPROC							nglDeleteShader;
extern PFNGLDETACHSHADERPROC							nglDetachShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC				nglDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC					nglEnableVertexAttribArray;
extern PFNGLGETATTACHEDSHADERSPROC						nglGetAttachedShaders;
extern PFNGLGETPROGRAMIVPROC							nglGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC						nglGetProgramInfoLog;
extern PFNGLGETSHADERIVPROC								nglGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC						nglGetShaderInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC						nglGetUniformLocation;
extern PFNGLISPROGRAMPROC								nglIsProgram;
extern PFNGLISSHADERPROC								nglIsShader;
extern PFNGLLINKPROGRAMPROC								nglLinkProgram;
extern PFNGLSHADERSOURCEPROC							nglShaderSource;
extern PFNGLUSEPROGRAMPROC								nglUseProgram;
extern PFNGLVALIDATEPROGRAMPROC							nglValidateProgram;
/*extern PFNGLUNIFORM1FPROC								nglUniform1f;
extern PFNGLUNIFORM2FPROC								nglUniform2f;
extern PFNGLUNIFORM3FPROC								nglUniform3f;
extern PFNGLUNIFORM4FPROC								nglUniform4f;
extern PFNGLUNIFORM1IPROC								nglUniform1i;
extern PFNGLUNIFORM2IPROC								nglUniform2i;
extern PFNGLUNIFORM3IPROC								nglUniform3i;
extern PFNGLUNIFORM4IPROC								nglUniform4i;
extern PFNGLUNIFORM1FVPROC								nglUniform1fv;
extern PFNGLUNIFORM2FVPROC								nglUniform2fv;
extern PFNGLUNIFORM3FVPROC								nglUniform3fv;
extern PFNGLUNIFORM4FVPROC								nglUniform4fv;
extern PFNGLUNIFORM1IVPROC								nglUniform1iv;
extern PFNGLUNIFORM2IVPROC								nglUniform2iv;
extern PFNGLUNIFORM3IVPROC								nglUniform3iv;
extern PFNGLUNIFORM4IVPROC								nglUniform4iv;
extern PFNGLUNIFORMMATRIX2FVPROC						nglUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC						nglUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC						nglUniformMatrix4fv;*/
extern PFNGLVERTEXATTRIBPOINTERPROC						nglVertexAttribPointer;

extern PFNGLBINDBUFFERPROC								nglBindBuffer;
extern PFNGLDELETEBUFFERSPROC							nglDeleteBuffers;
extern PFNGLGENBUFFERSPROC								nglGenBuffers;
extern PFNGLISBUFFERPROC								nglIsBuffer;
extern PFNGLBUFFERDATAPROC								nglBufferData;
extern PFNGLBUFFERSUBDATAPROC							nglBufferSubData;
extern PFNGLGETBUFFERSUBDATAPROC						nglGetBufferSubData;
extern PFNGLMAPBUFFERPROC								nglMapBuffer;
extern PFNGLUNMAPBUFFERPROC								nglUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVPROC					nglGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVPROC						nglGetBufferPointerv;

extern PFNGLGENQUERIESPROC								nglGenQueries;
extern PFNGLDELETEQUERIESPROC							nglDeleteQueries;
extern PFNGLISQUERYPROC									nglIsQuery;
extern PFNGLBEGINQUERYPROC								nglBeginQuery;
extern PFNGLENDQUERYPROC								nglEndQuery;
extern PFNGLGETQUERYIVPROC								nglGetQueryiv;
extern PFNGLGETQUERYOBJECTIVPROC						nglGetQueryObjectiv;
extern PFNGLGETQUERYOBJECTUIVPROC						nglGetQueryObjectuiv;

/*extern PFNGLISRENDERBUFFERPROC							nglIsRenderbuffer;
extern PFNGLBINDRENDERBUFFERPROC						nglBindRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC						nglDeleteRenderbuffers;
extern PFNGLGENRENDERBUFFERSPROC						nglGenRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEPROC						nglRenderbufferStorage;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC				nglGetRenderbufferParameteriv;
extern PFNGLISFRAMEBUFFERPROC							nglIsFramebuffer;
extern PFNGLBINDFRAMEBUFFERPROC							nglBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC						nglDeleteFramebuffers;
extern PFNGLGENFRAMEBUFFERSPROC							nglGenFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC					nglCheckFramebufferStatus;
extern PFNGLFRAMEBUFFERTEXTURE1DPROC					nglFramebufferTexture1D;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC					nglFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC					nglFramebufferTexture3D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC					nglFramebufferRenderbuffer;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC		nglGetFramebufferAttachmentParameteriv;
extern PFNGLGENERATEMIPMAPPROC							nglGenerateMipmap;
extern PFNGLBLITFRAMEBUFFERPROC							nglBlitFramebuffer;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC			nglRenderbufferStorageMultisample;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC					nglFramebufferTextureLayer;*/

extern PFNGLACTIVETEXTUREPROC							nglActiveTexture;

extern PFNGLCOMPRESSEDTEXIMAGE3DPROC					nglCompressedTexImage3D;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC					nglCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXIMAGE1DPROC					nglCompressedTexImage1D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC					nglCompressedTexSubImage3D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC					nglCompressedTexSubImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC					nglCompressedTexSubImage1D;
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC					nglGetCompressedTexImage;

extern PFNGLBLENDCOLORPROC								nglBlendColor;

// GL_ARB_separate_shader_objects
extern PFNGLUSEPROGRAMSTAGESPROC						nglUseProgramStages;
extern PFNGLACTIVESHADERPROGRAMPROC						nglActiveShaderProgram;
extern PFNGLCREATESHADERPROGRAMVPROC					nglCreateShaderProgramv;
extern PFNGLBINDPROGRAMPIPELINEPROC						nglBindProgramPipeline;
extern PFNGLDELETEPROGRAMPIPELINESPROC					nglDeleteProgramPipelines;
extern PFNGLGENPROGRAMPIPELINESPROC						nglGenProgramPipelines;
extern PFNGLISPROGRAMPIPELINEPROC						nglIsProgramPipeline;
extern PFNGLGETPROGRAMPIPELINEIVPROC					nglGetProgramPipelineiv;
extern PFNGLPROGRAMUNIFORM1IPROC						nglProgramUniform1i;
extern PFNGLPROGRAMUNIFORM1IVPROC						nglProgramUniform1iv;
extern PFNGLPROGRAMUNIFORM1FPROC						nglProgramUniform1f;
extern PFNGLPROGRAMUNIFORM1FVPROC						nglProgramUniform1fv;
extern PFNGLPROGRAMUNIFORM1DPROC						nglProgramUniform1d;
extern PFNGLPROGRAMUNIFORM1DVPROC						nglProgramUniform1dv;
extern PFNGLPROGRAMUNIFORM1UIPROC						nglProgramUniform1ui;
extern PFNGLPROGRAMUNIFORM1UIVPROC						nglProgramUniform1uiv;
extern PFNGLPROGRAMUNIFORM2IPROC						nglProgramUniform2i;
extern PFNGLPROGRAMUNIFORM2IVPROC						nglProgramUniform2iv;
extern PFNGLPROGRAMUNIFORM2FPROC						nglProgramUniform2f;
extern PFNGLPROGRAMUNIFORM2FVPROC						nglProgramUniform2fv;
extern PFNGLPROGRAMUNIFORM2DPROC						nglProgramUniform2d;
extern PFNGLPROGRAMUNIFORM2DVPROC						nglProgramUniform2dv;
extern PFNGLPROGRAMUNIFORM2UIPROC						nglProgramUniform2ui;
extern PFNGLPROGRAMUNIFORM2UIVPROC						nglProgramUniform2uiv;
extern PFNGLPROGRAMUNIFORM3IPROC						nglProgramUniform3i;
extern PFNGLPROGRAMUNIFORM3IVPROC						nglProgramUniform3iv;
extern PFNGLPROGRAMUNIFORM3FPROC						nglProgramUniform3f;
extern PFNGLPROGRAMUNIFORM3FVPROC						nglProgramUniform3fv;
extern PFNGLPROGRAMUNIFORM3DPROC						nglProgramUniform3d;
extern PFNGLPROGRAMUNIFORM3DVPROC						nglProgramUniform3dv;
extern PFNGLPROGRAMUNIFORM3UIPROC						nglProgramUniform3ui;
extern PFNGLPROGRAMUNIFORM3UIVPROC						nglProgramUniform3uiv;
extern PFNGLPROGRAMUNIFORM4IPROC						nglProgramUniform4i;
extern PFNGLPROGRAMUNIFORM4IVPROC						nglProgramUniform4iv;
extern PFNGLPROGRAMUNIFORM4FPROC						nglProgramUniform4f;
extern PFNGLPROGRAMUNIFORM4FVPROC						nglProgramUniform4fv;
extern PFNGLPROGRAMUNIFORM4DPROC						nglProgramUniform4d;
extern PFNGLPROGRAMUNIFORM4DVPROC						nglProgramUniform4dv;
extern PFNGLPROGRAMUNIFORM4UIPROC						nglProgramUniform4ui;
extern PFNGLPROGRAMUNIFORM4UIVPROC						nglProgramUniform4uiv;
extern PFNGLPROGRAMUNIFORMMATRIX2FVPROC					nglProgramUniformMatrix2fv;
extern PFNGLPROGRAMUNIFORMMATRIX3FVPROC					nglProgramUniformMatrix3fv;
extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC					nglProgramUniformMatrix4fv;
extern PFNGLPROGRAMUNIFORMMATRIX2DVPROC					nglProgramUniformMatrix2dv;
extern PFNGLPROGRAMUNIFORMMATRIX3DVPROC					nglProgramUniformMatrix3dv;
extern PFNGLPROGRAMUNIFORMMATRIX4DVPROC					nglProgramUniformMatrix4dv;
extern PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC				nglProgramUniformMatrix2x3fv;
extern PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC				nglProgramUniformMatrix3x2fv;
extern PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC				nglProgramUniformMatrix2x4fv;
extern PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC				nglProgramUniformMatrix4x2fv;
extern PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC				nglProgramUniformMatrix3x4fv;
extern PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC				nglProgramUniformMatrix4x3fv;
extern PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC				nglProgramUniformMatrix2x3dv;
extern PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC				nglProgramUniformMatrix3x2dv;
extern PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC				nglProgramUniformMatrix2x4dv;
extern PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC				nglProgramUniformMatrix4x2dv;
extern PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC				nglProgramUniformMatrix3x4dv;
extern PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC				nglProgramUniformMatrix4x3dv;
extern PFNGLVALIDATEPROGRAMPIPELINEPROC					nglValidateProgramPipeline;
extern PFNGLGETPROGRAMPIPELINEINFOLOGPROC				nglGetProgramPipelineInfoLog;

#ifdef NL_OS_WINDOWS

// Pbuffer extension
//==================
extern PFNWGLCREATEPBUFFERARBPROC			nwglCreatePbufferARB;
extern PFNWGLGETPBUFFERDCARBPROC			nwglGetPbufferDCARB;
extern PFNWGLRELEASEPBUFFERDCARBPROC		nwglReleasePbufferDCARB;
extern PFNWGLDESTROYPBUFFERARBPROC			nwglDestroyPbufferARB;
extern PFNWGLQUERYPBUFFERARBPROC			nwglQueryPbufferARB;


// Get Pixel format extension
//===========================
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC	nwglGetPixelFormatAttribivARB;
extern PFNWGLGETPIXELFORMATATTRIBFVARBPROC	nwglGetPixelFormatAttribfvARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC		nwglChoosePixelFormatARB;


// Swap control extension
//===========================
extern PFNWGLSWAPINTERVALEXTPROC			nwglSwapIntervalEXT;
extern PFNWGLGETSWAPINTERVALEXTPROC			nwglGetSwapIntervalEXT;

// WGL_ARB_extensions_string
extern PFNWGLGETEXTENSIONSSTRINGARBPROC		nwglGetExtensionsStringARB;

// WGL_ARB_create_context_profile
extern PFNWGLCREATECONTEXTATTRIBSARBPROC	nwglCreateContextAttribsARB;


#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

// Swap control extensions
//===========================
extern NEL_PFNGLXSWAPINTERVALEXTPROC			nglXSwapIntervalEXT;

extern PFNGLXSWAPINTERVALSGIPROC				nglXSwapIntervalSGI;

extern NEL_PFNGLXSWAPINTERVALMESAPROC			nglXSwapIntervalMESA;
extern NEL_PFNGLXGETSWAPINTERVALMESAPROC		nglXGetSwapIntervalMESA;

#endif



#endif // NL_OPENGL_EXTENSION_H

