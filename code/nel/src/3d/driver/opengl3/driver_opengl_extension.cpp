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
#include "driver_opengl_extension.h"

#include "nel/misc/common.h"

#include "nel/3d/material.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
#if defined(NL_OS_WINDOWS)
#define	nglGetProcAddress wglGetProcAddress
#elif defined(NL_OS_MAC)
// #include <mach-o/dyld.h>
// glXGetProcAddressARB doesn't work correctly on MAC
// void *nglGetProcAddress(const char *name)
// {
// 	NSSymbol symbol;
// 	char *symbolName;
// 	symbolName = (char*)malloc (strlen (name) + 2);
// 	strcpy(symbolName + 1, name);
// 	symbolName[0] = '_';
// 	symbol = NULL;
// 	if (NSIsSymbolNameDefined (symbolName)) symbol = NSLookupAndBindSymbol (symbolName);
// 	free (symbolName);
// 	return symbol ? NSAddressOfSymbol (symbol) : NULL;
// }

// NSAddressOfSymbol, NSIsSymbolNameDefined, NSLookupAndBindSymbol are deprecated
#include <dlfcn.h>
void *nglGetProcAddress(const char *name)
{
	return dlsym(RTLD_DEFAULT, name);
}

#elif defined (NL_OS_UNIX)
void (*nglGetProcAddress(const char *procName))()
{
	return glXGetProcAddressARB((const GLubyte *)procName);
}
#endif	// NL_OS_WINDOWS


// ***************************************************************************
// The exported function names

// Core 3.30
PFNGLGETSTRINGIPROC								nglGetStringi;

PFNGLATTACHSHADERPROC							nglAttachShader;
PFNGLCOMPILESHADERPROC							nglCompileShader;
PFNGLCREATEPROGRAMPROC							nglCreateProgram;
PFNGLCREATESHADERPROC							nglCreateShader;
PFNGLDELETEPROGRAMPROC							nglDeleteProgram;
PFNGLDELETESHADERPROC							nglDeleteShader;
PFNGLDETACHSHADERPROC							nglDetachShader;
PFNGLDISABLEVERTEXATTRIBARRAYPROC				nglDisableVertexAttribArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC				nglEnableVertexAttribArray;
PFNGLGETATTACHEDSHADERSPROC						nglGetAttachedShaders;
PFNGLGETPROGRAMIVPROC							nglGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC						nglGetProgramInfoLog;
PFNGLGETSHADERIVPROC							nglGetShaderiv;
PFNGLGETSHADERINFOLOGPROC						nglGetShaderInfoLog;
PFNGLGETUNIFORMLOCATIONPROC						nglGetUniformLocation;
PFNGLISPROGRAMPROC								nglIsProgram;
PFNGLISSHADERPROC								nglIsShader;
PFNGLLINKPROGRAMPROC							nglLinkProgram;
PFNGLSHADERSOURCEPROC							nglShaderSource;
PFNGLUSEPROGRAMPROC								nglUseProgram;
PFNGLVALIDATEPROGRAMPROC						nglValidateProgram;
/*PFNGLUNIFORM1FPROC								nglUniform1f;
PFNGLUNIFORM2FPROC								nglUniform2f;
PFNGLUNIFORM3FPROC								nglUniform3f;
PFNGLUNIFORM4FPROC								nglUniform4f;
PFNGLUNIFORM1IPROC								nglUniform1i;
PFNGLUNIFORM2IPROC								nglUniform2i;
PFNGLUNIFORM3IPROC								nglUniform3i;
PFNGLUNIFORM4IPROC								nglUniform4i;
PFNGLUNIFORM1FVPROC								nglUniform1fv;
PFNGLUNIFORM2FVPROC								nglUniform2fv;
PFNGLUNIFORM3FVPROC								nglUniform3fv;
PFNGLUNIFORM4FVPROC								nglUniform4fv;
PFNGLUNIFORM1IVPROC								nglUniform1iv;
PFNGLUNIFORM2IVPROC								nglUniform2iv;
PFNGLUNIFORM3IVPROC								nglUniform3iv;
PFNGLUNIFORM4IVPROC								nglUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC						nglUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC						nglUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC						nglUniformMatrix4fv;*/
PFNGLVERTEXATTRIBPOINTERPROC					nglVertexAttribPointer;

PFNGLBINDBUFFERPROC								nglBindBuffer;
PFNGLDELETEBUFFERSPROC							nglDeleteBuffers;
PFNGLGENBUFFERSPROC								nglGenBuffers;
PFNGLISBUFFERPROC 								nglIsBuffer;
PFNGLBUFFERDATAPROC 							nglBufferData;
PFNGLBUFFERSUBDATAPROC 							nglBufferSubData;
PFNGLGETBUFFERSUBDATAPROC 						nglGetBufferSubData;
PFNGLMAPBUFFERPROC 								nglMapBuffer;
PFNGLUNMAPBUFFERPROC 							nglUnmapBuffer;
PFNGLGETBUFFERPARAMETERIVPROC 					nglGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC 						nglGetBufferPointerv;

PFNGLGENQUERIESPROC								nglGenQueries;
PFNGLDELETEQUERIESPROC							nglDeleteQueries;
PFNGLISQUERYPROC								nglIsQuery;
PFNGLBEGINQUERYPROC								nglBeginQuery;
PFNGLENDQUERYPROC								nglEndQuery;
PFNGLGETQUERYIVPROC								nglGetQueryiv;
PFNGLGETQUERYOBJECTIVPROC						nglGetQueryObjectiv;
PFNGLGETQUERYOBJECTUIVPROC						nglGetQueryObjectuiv;

/*PFNGLISRENDERBUFFERPROC							nglIsRenderbuffer;
PFNGLBINDRENDERBUFFERPROC						nglBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC					nglDeleteRenderbuffers;
PFNGLGENRENDERBUFFERSPROC						nglGenRenderbuffers;
PFNGLRENDERBUFFERSTORAGEPROC					nglRenderbufferStorage;
PFNGLGETRENDERBUFFERPARAMETERIVPROC				nglGetRenderbufferParameteriv;
PFNGLISFRAMEBUFFERPROC							nglIsFramebuffer;
PFNGLBINDFRAMEBUFFERPROC						nglBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC						nglDeleteFramebuffers;
PFNGLGENFRAMEBUFFERSPROC						nglGenFramebuffers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC					nglCheckFramebufferStatus;
PFNGLFRAMEBUFFERTEXTURE1DPROC					nglFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DPROC					nglFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DPROC					nglFramebufferTexture3D;
PFNGLFRAMEBUFFERRENDERBUFFERPROC				nglFramebufferRenderbuffer;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC	nglGetFramebufferAttachmentParameteriv;
PFNGLGENERATEMIPMAPPROC							nglGenerateMipmap;
PFNGLBLITFRAMEBUFFERPROC						nglBlitFramebuffer;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC			nglRenderbufferStorageMultisample;
PFNGLFRAMEBUFFERTEXTURELAYERPROC				nglFramebufferTextureLayer;*/

PFNGLACTIVETEXTUREPROC							nglActiveTexture;

PFNGLCOMPRESSEDTEXIMAGE3DPROC					nglCompressedTexImage3D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC					nglCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE1DPROC					nglCompressedTexImage1D;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC				nglCompressedTexSubImage3D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC				nglCompressedTexSubImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC				nglCompressedTexSubImage1D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC					nglGetCompressedTexImage;

PFNGLBLENDCOLORPROC								nglBlendColor;

// GL_ARB_separate_shader_objects
PFNGLUSEPROGRAMSTAGESPROC						nglUseProgramStages;
PFNGLACTIVESHADERPROGRAMPROC					nglActiveShaderProgram;
PFNGLCREATESHADERPROGRAMVPROC					nglCreateShaderProgramv;
PFNGLBINDPROGRAMPIPELINEPROC					nglBindProgramPipeline;
PFNGLDELETEPROGRAMPIPELINESPROC					nglDeleteProgramPipelines;
PFNGLGENPROGRAMPIPELINESPROC					nglGenProgramPipelines;
PFNGLISPROGRAMPIPELINEPROC						nglIsProgramPipeline;
PFNGLGETPROGRAMPIPELINEIVPROC					nglGetProgramPipelineiv;
PFNGLPROGRAMUNIFORM1IPROC						nglProgramUniform1i;
PFNGLPROGRAMUNIFORM1IVPROC						nglProgramUniform1iv;
PFNGLPROGRAMUNIFORM1FPROC						nglProgramUniform1f;
PFNGLPROGRAMUNIFORM1FVPROC						nglProgramUniform1fv;
PFNGLPROGRAMUNIFORM1DPROC						nglProgramUniform1d;
PFNGLPROGRAMUNIFORM1DVPROC						nglProgramUniform1dv;
PFNGLPROGRAMUNIFORM1UIPROC						nglProgramUniform1ui;
PFNGLPROGRAMUNIFORM1UIVPROC						nglProgramUniform1uiv;
PFNGLPROGRAMUNIFORM2IPROC						nglProgramUniform2i;
PFNGLPROGRAMUNIFORM2IVPROC						nglProgramUniform2iv;
PFNGLPROGRAMUNIFORM2FPROC						nglProgramUniform2f;
PFNGLPROGRAMUNIFORM2FVPROC						nglProgramUniform2fv;
PFNGLPROGRAMUNIFORM2DPROC						nglProgramUniform2d;
PFNGLPROGRAMUNIFORM2DVPROC						nglProgramUniform2dv;
PFNGLPROGRAMUNIFORM2UIPROC						nglProgramUniform2ui;
PFNGLPROGRAMUNIFORM2UIVPROC						nglProgramUniform2uiv;
PFNGLPROGRAMUNIFORM3IPROC						nglProgramUniform3i;
PFNGLPROGRAMUNIFORM3IVPROC						nglProgramUniform3iv;
PFNGLPROGRAMUNIFORM3FPROC						nglProgramUniform3f;
PFNGLPROGRAMUNIFORM3FVPROC						nglProgramUniform3fv;
PFNGLPROGRAMUNIFORM3DPROC						nglProgramUniform3d;
PFNGLPROGRAMUNIFORM3DVPROC						nglProgramUniform3dv;
PFNGLPROGRAMUNIFORM3UIPROC						nglProgramUniform3ui;
PFNGLPROGRAMUNIFORM3UIVPROC						nglProgramUniform3uiv;
PFNGLPROGRAMUNIFORM4IPROC						nglProgramUniform4i;
PFNGLPROGRAMUNIFORM4IVPROC						nglProgramUniform4iv;
PFNGLPROGRAMUNIFORM4FPROC						nglProgramUniform4f;
PFNGLPROGRAMUNIFORM4FVPROC						nglProgramUniform4fv;
PFNGLPROGRAMUNIFORM4DPROC						nglProgramUniform4d;
PFNGLPROGRAMUNIFORM4DVPROC						nglProgramUniform4dv;
PFNGLPROGRAMUNIFORM4UIPROC						nglProgramUniform4ui;
PFNGLPROGRAMUNIFORM4UIVPROC						nglProgramUniform4uiv;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC				nglProgramUniformMatrix2fv;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC				nglProgramUniformMatrix3fv;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC				nglProgramUniformMatrix4fv;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC				nglProgramUniformMatrix2dv;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC				nglProgramUniformMatrix3dv;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC				nglProgramUniformMatrix4dv;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC				nglProgramUniformMatrix2x3fv;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC				nglProgramUniformMatrix3x2fv;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC				nglProgramUniformMatrix2x4fv;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC				nglProgramUniformMatrix4x2fv;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC				nglProgramUniformMatrix3x4fv;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC				nglProgramUniformMatrix4x3fv;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC				nglProgramUniformMatrix2x3dv;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC				nglProgramUniformMatrix3x2dv;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC				nglProgramUniformMatrix2x4dv;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC				nglProgramUniformMatrix4x2dv;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC				nglProgramUniformMatrix3x4dv;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC				nglProgramUniformMatrix4x3dv;
PFNGLVALIDATEPROGRAMPIPELINEPROC				nglValidateProgramPipeline;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC				nglGetProgramPipelineInfoLog;

#ifdef NL_OS_WINDOWS
PFNWGLALLOCATEMEMORYNVPROC						nwglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC							nwglFreeMemoryNV;

// Pbuffer extension
PFNWGLCREATEPBUFFERARBPROC						nwglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC						nwglGetPbufferDCARB;
PFNWGLRELEASEPBUFFERDCARBPROC					nwglReleasePbufferDCARB;
PFNWGLDESTROYPBUFFERARBPROC						nwglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC						nwglQueryPbufferARB;

// Get Pixel format extension
PFNWGLGETPIXELFORMATATTRIBIVARBPROC				nwglGetPixelFormatAttribivARB;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC				nwglGetPixelFormatAttribfvARB;
PFNWGLCHOOSEPIXELFORMATARBPROC					nwglChoosePixelFormatARB;

// Swap control extension
PFNWGLSWAPINTERVALEXTPROC						nwglSwapIntervalEXT;
PFNWGLGETSWAPINTERVALEXTPROC					nwglGetSwapIntervalEXT;

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC				nwglGetExtensionsStringARB;

// WGL_ARB_create_context_profile
PFNWGLCREATECONTEXTATTRIBSARBPROC				nwglCreateContextAttribsARB;

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

NEL_PFNGLXALLOCATEMEMORYNVPROC					nglXAllocateMemoryNV;
NEL_PFNGLXFREEMEMORYNVPROC						nglXFreeMemoryNV;

// Swap control extensions
NEL_PFNGLXSWAPINTERVALEXTPROC					nglXSwapIntervalEXT;

PFNGLXSWAPINTERVALSGIPROC						nglXSwapIntervalSGI;

NEL_PFNGLXSWAPINTERVALMESAPROC					nglXSwapIntervalMESA;
NEL_PFNGLXGETSWAPINTERVALMESAPROC				nglXGetSwapIntervalMESA;

#endif

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


namespace	NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

#define CHECK_EXT(ext_str) \
	if (strstr(glext, ext_str)==NULL) { nlwarning("3D: OpengGL extension '%s' was not found", ext_str); return false; } else { nldebug("3D: OpengGL Extension '%s' found", ext_str); }

bool checkExt2(std::vector<const char *> &glext, const char *ext_str)
{
	for (int i = 0; i < glext.size(); ++i)
	{
		if (strcmp(glext[i], ext_str) == 0)
			return true;
	}
	return false;
}

#define CHECK_EXT_2(ext_str) \
	if (!checkExt2(glext, ext_str)) { nlwarning("3D: OpengGL extension '%s' was not found", ext_str); return false; } else { nldebug("3D: OpengGL Extension '%s' found", ext_str); }

// Debug: don't return false if the procaddr returns 0
// It means that it can crash if nel calls this extension but at least we have a warning to know why the extension is available but not the procaddr
#define CHECK_ADDRESS(type, ext) \
	n##ext=(type)nglGetProcAddress(#ext); \
	if (!n##ext) { nlwarning("3D: GetProcAddress(\"%s\") returns NULL", #ext); return false; } else { /*nldebug("3D: GetProcAddress(\"%s\") succeed", #ext);*/ }

// ***************************************************************************
// Extensions registrations, and Windows function Registration.

// *********************************
static bool	setupEXTTextureCompressionS3TC(std::vector<const char *> &glext)
{
	H_AUTO_OGL(setupEXTTextureCompressionS3TC);

	CHECK_EXT_2("GL_EXT_texture_compression_s3tc");
	// TODO: check also for GL_S3_s3tc, GL_EXT_texture_compression_dxt1

	return true;
}

// *********************************
static bool	setupWGLARBPBuffer(const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPBuffer);
	CHECK_EXT("WGL_ARB_pbuffer");

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLCREATEPBUFFERARBPROC, wglCreatePbufferARB);
	CHECK_ADDRESS(PFNWGLGETPBUFFERDCARBPROC, wglGetPbufferDCARB);
	CHECK_ADDRESS(PFNWGLRELEASEPBUFFERDCARBPROC, wglReleasePbufferDCARB);
	CHECK_ADDRESS(PFNWGLDESTROYPBUFFERARBPROC, wglDestroyPbufferARB);
	CHECK_ADDRESS(PFNWGLQUERYPBUFFERARBPROC, wglQueryPbufferARB);
#endif

	return true;
}

#ifdef NL_OS_WINDOWS
// *********************************
static bool	setupWGLARBPixelFormat (const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPixelFormat);
	CHECK_EXT("WGL_ARB_pixel_format");

	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB);
	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBFVARBPROC, wglGetPixelFormatAttribfvARB);
	CHECK_ADDRESS(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);

	return true;
}
#endif

// ***************************************************************************
static bool	setupEXTTextureFilterAnisotropic(std::vector<const char *> &glext)
{
	H_AUTO_OGL(setupEXTTextureFilterAnisotropic);
	CHECK_EXT_2("GL_EXT_texture_filter_anisotropic");
	return true;
}

static bool setupGLCore(std::vector<const char *> &glext)
{
	CHECK_ADDRESS(PFNGLATTACHSHADERPROC, glAttachShader);
	CHECK_ADDRESS(PFNGLCOMPILESHADERPROC, glCompileShader);
	CHECK_ADDRESS(PFNGLCREATEPROGRAMPROC, glCreateProgram);
	CHECK_ADDRESS(PFNGLCREATESHADERPROC, glCreateShader);
	CHECK_ADDRESS(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
	CHECK_ADDRESS(PFNGLDELETESHADERPROC, glDeleteShader);
	CHECK_ADDRESS(PFNGLDETACHSHADERPROC, glDetachShader);
	CHECK_ADDRESS(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);
	CHECK_ADDRESS(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
	CHECK_ADDRESS(PFNGLGETATTACHEDSHADERSPROC, glGetAttachedShaders);
	CHECK_ADDRESS(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
	CHECK_ADDRESS(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
	CHECK_ADDRESS(PFNGLGETSHADERIVPROC, glGetShaderiv);
	CHECK_ADDRESS(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
	CHECK_ADDRESS(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
	CHECK_ADDRESS(PFNGLISPROGRAMPROC, glIsProgram);
	CHECK_ADDRESS(PFNGLISSHADERPROC, glIsShader);
	CHECK_ADDRESS(PFNGLLINKPROGRAMPROC, glLinkProgram);
	CHECK_ADDRESS(PFNGLSHADERSOURCEPROC, glShaderSource);
	CHECK_ADDRESS(PFNGLUSEPROGRAMPROC, glUseProgram);
	CHECK_ADDRESS(PFNGLVALIDATEPROGRAMPROC, glValidateProgram);
	/*CHECK_ADDRESS(PFNGLUNIFORM1FPROC, glUniform1f);
	CHECK_ADDRESS(PFNGLUNIFORM2FPROC, glUniform2f);
	CHECK_ADDRESS(PFNGLUNIFORM3FPROC, glUniform3f);
	CHECK_ADDRESS(PFNGLUNIFORM4FPROC, glUniform4f);
	CHECK_ADDRESS(PFNGLUNIFORM1IPROC, glUniform1i);
	CHECK_ADDRESS(PFNGLUNIFORM2IPROC, glUniform2i);
	CHECK_ADDRESS(PFNGLUNIFORM3IPROC, glUniform3i);
	CHECK_ADDRESS(PFNGLUNIFORM4IPROC, glUniform4i);
	CHECK_ADDRESS(PFNGLUNIFORM1FVPROC, glUniform1fv);
	CHECK_ADDRESS(PFNGLUNIFORM2FVPROC, glUniform2fv);
	CHECK_ADDRESS(PFNGLUNIFORM3FVPROC, glUniform3fv);
	CHECK_ADDRESS(PFNGLUNIFORM4FVPROC, glUniform4fv);
	CHECK_ADDRESS(PFNGLUNIFORM1IVPROC, glUniform1iv);
	CHECK_ADDRESS(PFNGLUNIFORM2IVPROC, glUniform2iv);
	CHECK_ADDRESS(PFNGLUNIFORM3IVPROC, glUniform3iv);
	CHECK_ADDRESS(PFNGLUNIFORM4IVPROC, glUniform4iv);
	CHECK_ADDRESS(PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);
	CHECK_ADDRESS(PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);
	CHECK_ADDRESS(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);*/
	CHECK_ADDRESS(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);

	CHECK_ADDRESS(PFNGLBINDBUFFERPROC, glBindBuffer);
	CHECK_ADDRESS(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
	CHECK_ADDRESS(PFNGLGENBUFFERSPROC, glGenBuffers);
	CHECK_ADDRESS(PFNGLISBUFFERPROC, glIsBuffer);
	CHECK_ADDRESS(PFNGLBUFFERDATAPROC, glBufferData);
	CHECK_ADDRESS(PFNGLBUFFERSUBDATAPROC, glBufferSubData);
	CHECK_ADDRESS(PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData);
	CHECK_ADDRESS(PFNGLMAPBUFFERPROC, glMapBuffer);
	CHECK_ADDRESS(PFNGLUNMAPBUFFERPROC, glUnmapBuffer);
	CHECK_ADDRESS(PFNGLGETBUFFERPARAMETERIVPROC, glGetBufferParameteriv);
	CHECK_ADDRESS(PFNGLGETBUFFERPOINTERVPROC, glGetBufferPointerv);

	CHECK_ADDRESS(PFNGLGENQUERIESPROC, glGenQueries);
	CHECK_ADDRESS(PFNGLDELETEQUERIESPROC, glDeleteQueries);
	CHECK_ADDRESS(PFNGLISQUERYPROC, glIsQuery);
	CHECK_ADDRESS(PFNGLBEGINQUERYPROC, glBeginQuery);
	CHECK_ADDRESS(PFNGLENDQUERYPROC, glEndQuery);
	CHECK_ADDRESS(PFNGLGETQUERYIVPROC, glGetQueryiv);
	CHECK_ADDRESS(PFNGLGETQUERYOBJECTIVPROC, glGetQueryObjectiv);
	CHECK_ADDRESS(PFNGLGETQUERYOBJECTUIVPROC, glGetQueryObjectuiv);

	/*CHECK_ADDRESS(PFNGLISRENDERBUFFERPROC, glIsRenderbuffer);
	CHECK_ADDRESS(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);
	CHECK_ADDRESS(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
	CHECK_ADDRESS(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
	CHECK_ADDRESS(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);
	CHECK_ADDRESS(PFNGLGETRENDERBUFFERPARAMETERIVPROC, glGetRenderbufferParameteriv);
	CHECK_ADDRESS(PFNGLISFRAMEBUFFERPROC, glIsFramebuffer);
	CHECK_ADDRESS(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
	CHECK_ADDRESS(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
	CHECK_ADDRESS(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
	CHECK_ADDRESS(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURE1DPROC, glFramebufferTexture1D);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURE3DPROC, glFramebufferTexture3D);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);
	CHECK_ADDRESS(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC, glGetFramebufferAttachmentParameteriv);
	CHECK_ADDRESS(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);
	CHECK_ADDRESS(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer);
	CHECK_ADDRESS(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURELAYERPROC, glFramebufferTextureLayer);*/

	CHECK_ADDRESS(PFNGLACTIVETEXTUREPROC, glActiveTexture);

	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE3DPROC, glCompressedTexImage3D);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE1DPROC, glCompressedTexImage1D);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, glCompressedTexSubImage3D);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC, glCompressedTexSubImage1D);
	CHECK_ADDRESS(PFNGLGETCOMPRESSEDTEXIMAGEPROC, glGetCompressedTexImage);

	CHECK_ADDRESS(PFNGLBLENDCOLORPROC, glBlendColor);

	return true;
}

static bool setupARBSeparateShaderObjects(std::vector<const char *> &glext)
{
	CHECK_EXT_2("GL_ARB_separate_shader_objects");

	CHECK_ADDRESS(PFNGLUSEPROGRAMSTAGESPROC, glUseProgramStages);
	CHECK_ADDRESS(PFNGLACTIVESHADERPROGRAMPROC, glActiveShaderProgram);
	CHECK_ADDRESS(PFNGLCREATESHADERPROGRAMVPROC, glCreateShaderProgramv);
	CHECK_ADDRESS(PFNGLBINDPROGRAMPIPELINEPROC, glBindProgramPipeline);
	CHECK_ADDRESS(PFNGLDELETEPROGRAMPIPELINESPROC, glDeleteProgramPipelines);
	CHECK_ADDRESS(PFNGLGENPROGRAMPIPELINESPROC, glGenProgramPipelines);
	CHECK_ADDRESS(PFNGLISPROGRAMPIPELINEPROC, glIsProgramPipeline);
	CHECK_ADDRESS(PFNGLGETPROGRAMPIPELINEIVPROC, glGetProgramPipelineiv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1IPROC, glProgramUniform1i);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1IVPROC, glProgramUniform1iv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1FPROC, glProgramUniform1f);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1FVPROC, glProgramUniform1fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1DPROC, glProgramUniform1d);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1DVPROC, glProgramUniform1dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1UIPROC, glProgramUniform1ui);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM1UIVPROC, glProgramUniform1uiv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2IPROC, glProgramUniform2i);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2IVPROC, glProgramUniform2iv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2FPROC, glProgramUniform2f);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2FVPROC, glProgramUniform2fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2DPROC, glProgramUniform2d);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2DVPROC, glProgramUniform2dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2UIPROC, glProgramUniform2ui);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM2UIVPROC, glProgramUniform2uiv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3IPROC, glProgramUniform3i);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3IVPROC, glProgramUniform3iv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3FPROC, glProgramUniform3f);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3FVPROC, glProgramUniform3fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3DPROC, glProgramUniform3d);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3DVPROC, glProgramUniform3dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3UIPROC, glProgramUniform3ui);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM3UIVPROC, glProgramUniform3uiv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4IPROC, glProgramUniform4i);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4IVPROC, glProgramUniform4iv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4FPROC, glProgramUniform4f);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4FVPROC, glProgramUniform4fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4DPROC, glProgramUniform4d);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4DVPROC, glProgramUniform4dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4UIPROC, glProgramUniform4ui);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORM4UIVPROC, glProgramUniform4uiv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2FVPROC, glProgramUniformMatrix2fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3FVPROC, glProgramUniformMatrix3fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4FVPROC, glProgramUniformMatrix4fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2DVPROC, glProgramUniformMatrix2dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3DVPROC, glProgramUniformMatrix3dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4DVPROC, glProgramUniformMatrix4dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC, glProgramUniformMatrix2x3fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC, glProgramUniformMatrix3x2fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC, glProgramUniformMatrix2x4fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC, glProgramUniformMatrix4x2fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC, glProgramUniformMatrix3x4fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC, glProgramUniformMatrix4x3fv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC, glProgramUniformMatrix2x3dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC, glProgramUniformMatrix3x2dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC, glProgramUniformMatrix2x4dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC, glProgramUniformMatrix4x2dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC, glProgramUniformMatrix3x4dv);
	CHECK_ADDRESS(PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC, glProgramUniformMatrix4x3dv);
	CHECK_ADDRESS(PFNGLVALIDATEPROGRAMPIPELINEPROC, glValidateProgramPipeline);
	CHECK_ADDRESS(PFNGLGETPROGRAMPIPELINEINFOLOGPROC, glGetProgramPipelineInfoLog);

	return true;
}

// ***************************************************************************
// Extension Check.
bool	registerGlExtensions(CGlExtensions &ext)
{
	H_AUTO_OGL(registerGlExtensions);

	nldebug("Register OpenGL extensions");

	const char	*nglVersion= (const char *)glGetString (GL_VERSION);
	sint	a=0, b=0;

	sscanf(nglVersion, "%d.%d", &a, &b);
	if ((a < 3) || ((a == 3) && (b < 3)))
	{
		nlinfo("OpenGL version is less than 3.3!");
		nlinfo("Version string: %s",nglVersion);
		nlassert(false);
	}

	nldebug("OpenGL version is OK");
	
	// Extensions.
	/*const char	*glext= (const char*)glGetString(GL_EXTENSIONS);
	GLint	ntext;*/

	// Get proc address
	CHECK_ADDRESS(PFNGLGETSTRINGIPROC, glGetStringi);
	nldebug("GL3: glGetStringi found!");

	std::vector<const char *> glext;
	GLint numExt;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
	nldebug("GL3: GL_NUM_EXTENSIONS = %i", numExt);
	glext.resize(numExt);
	for (GLint i = 0; i < numExt; ++i)
	{
		glext[i] = static_cast<const char *>(static_cast<const void *>(nglGetStringi(GL_EXTENSIONS, i)));
	}

	nldebug("3D: Available OpenGL Extensions:");

	if (DebugLog)
	{
		//vector<string> exts;
		//explode(string(glext), string(" "), exts);
		for (uint i = 0; i < glext.size(); i++)
		{
			if (i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(string(glext[i]) + " ").c_str());
			if (i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check 3.30 Core
	ext.GLCore = setupGLCore(glext);

	// Check GL_ARB_separate_shader_objects
	ext.ARBSeparateShaderObjects = setupARBSeparateShaderObjects(glext);

	// Compression S3TC
	ext.EXTTextureCompressionS3TC = setupEXTTextureCompressionS3TC(glext);

	// Check GL_EXT_texture_filter_anisotropic
	ext.EXTTextureFilterAnisotropic = setupEXTTextureFilterAnisotropic(glext);

	if (ext.EXTTextureFilterAnisotropic)
	{
		// get the maximum value
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &ext.EXTTextureFilterAnisotropicMaximum);
	}

	GLint nbFragmentTextureUnits;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nbFragmentTextureUnits);
	ext.NbFragmentTextureUnits = (nbFragmentTextureUnits > IDRV_MAT_MAXTEXTURES) ? IDRV_MAT_MAXTEXTURES : nbFragmentTextureUnits; // FIXME GL3

	return true;
}


// *********************************
static bool	setupWGLEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupWGLEXTSwapControl);
	CHECK_EXT("WGL_EXT_swap_control");

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
	CHECK_ADDRESS(PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);
#endif

	return true;
}

// *********************************
static bool setupWGLARBCreateContextProfile(const char	*glext)
{
	H_AUTO_OGL(setupWGLARBCreateContextProfile);
	CHECK_EXT("WGL_ARB_create_context_profile");

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);
#endif

	return true;
}

// *********************************
static bool	setupGLXEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXEXTSwapControl);
	CHECK_EXT("GLX_EXT_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(NEL_PFNGLXSWAPINTERVALEXTPROC, glXSwapIntervalEXT);
#endif

	return true;
}

// *********************************
static bool	setupGLXSGISwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXSGISwapControl);
	CHECK_EXT("GLX_SGI_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(PFNGLXSWAPINTERVALSGIPROC, glXSwapIntervalSGI);
#endif

	return true;
}

// *********************************
static bool	setupGLXMESASwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXMESASwapControl);
	CHECK_EXT("GLX_MESA_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(NEL_PFNGLXSWAPINTERVALMESAPROC, glXSwapIntervalMESA);
	CHECK_ADDRESS(NEL_PFNGLXGETSWAPINTERVALMESAPROC, glXGetSwapIntervalMESA);
#endif

	return true;
}

#if defined(NL_OS_WINDOWS)
// ***************************************************************************
bool registerWGlExtensions(CGlExtensions &ext, HDC hDC)
{
	H_AUTO_OGL(registerWGlExtensions);

	// Get proc address
	CHECK_ADDRESS(PFNWGLGETEXTENSIONSSTRINGARBPROC, wglGetExtensionsStringARB);

	// Get extension string
	const char *glext = nwglGetExtensionsStringARB (hDC);
	if (glext == NULL)
	{
		nlwarning ("nwglGetExtensionsStringARB failed");
		return false;
	}

	nldebug("3D: Available WGL Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for (uint i = 0; i < exts.size(); i++)
		{
			if (i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if (i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check for pbuffer
	ext.WGLARBPBuffer = setupWGLARBPBuffer(glext);

	// Check for pixel format
	ext.WGLARBPixelFormat = setupWGLARBPixelFormat(glext);

	// Check for swap control
	ext.WGLEXTSwapControl = setupWGLEXTSwapControl(glext);

	// Check for create context profile
	ext.WGLARBCreateContextProfile = setupWGLARBCreateContextProfile(glext);

	return true;
}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
// ***************************************************************************
bool registerGlXExtensions(CGlExtensions &ext, Display *dpy, sint screen)
{
	H_AUTO_OGL(registerGlXExtensions);

	// Get extension string
	const char *glext = glXQueryExtensionsString(dpy, screen);
	if (glext == NULL)
	{
		nlwarning ("glXQueryExtensionsString failed");
		return false;
	}

	nldebug("3D: Available GLX Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for (uint i = 0; i < exts.size(); i++)
		{
			if (i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if (i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check for pbuffer
//	ext.WGLARBPBuffer= setupWGLARBPBuffer(glext);

	// Check for pixel format
//	ext.WGLARBPixelFormat= setupWGLARBPixelFormat(glext);

	// Check for swap control
	ext.GLXEXTSwapControl= setupGLXEXTSwapControl(glext);
	ext.GLXSGISwapControl= setupGLXSGISwapControl(glext);
	ext.GLXMESASwapControl= setupGLXMESASwapControl(glext);

	return true;
}
#endif

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D
