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

// ARB_multitexture
NEL_PFNGLACTIVETEXTUREARBPROC					nglActiveTextureARB;
NEL_PFNGLCLIENTACTIVETEXTUREARBPROC				nglClientActiveTextureARB;

NEL_PFNGLMULTITEXCOORD1SARBPROC					nglMultiTexCoord1sARB;
NEL_PFNGLMULTITEXCOORD1IARBPROC					nglMultiTexCoord1iARB;
NEL_PFNGLMULTITEXCOORD1FARBPROC					nglMultiTexCoord1fARB;
NEL_PFNGLMULTITEXCOORD1DARBPROC					nglMultiTexCoord1dARB;
NEL_PFNGLMULTITEXCOORD2SARBPROC					nglMultiTexCoord2sARB;
NEL_PFNGLMULTITEXCOORD2IARBPROC					nglMultiTexCoord2iARB;
NEL_PFNGLMULTITEXCOORD2FARBPROC					nglMultiTexCoord2fARB;
NEL_PFNGLMULTITEXCOORD2DARBPROC					nglMultiTexCoord2dARB;
NEL_PFNGLMULTITEXCOORD3SARBPROC					nglMultiTexCoord3sARB;
NEL_PFNGLMULTITEXCOORD3IARBPROC					nglMultiTexCoord3iARB;
NEL_PFNGLMULTITEXCOORD3FARBPROC					nglMultiTexCoord3fARB;
NEL_PFNGLMULTITEXCOORD3DARBPROC					nglMultiTexCoord3dARB;
NEL_PFNGLMULTITEXCOORD4SARBPROC					nglMultiTexCoord4sARB;
NEL_PFNGLMULTITEXCOORD4IARBPROC					nglMultiTexCoord4iARB;
NEL_PFNGLMULTITEXCOORD4FARBPROC					nglMultiTexCoord4fARB;
NEL_PFNGLMULTITEXCOORD4DARBPROC					nglMultiTexCoord4dARB;

NEL_PFNGLMULTITEXCOORD1SVARBPROC				nglMultiTexCoord1svARB;
NEL_PFNGLMULTITEXCOORD1IVARBPROC				nglMultiTexCoord1ivARB;
NEL_PFNGLMULTITEXCOORD1FVARBPROC				nglMultiTexCoord1fvARB;
NEL_PFNGLMULTITEXCOORD1DVARBPROC				nglMultiTexCoord1dvARB;
NEL_PFNGLMULTITEXCOORD2SVARBPROC				nglMultiTexCoord2svARB;
NEL_PFNGLMULTITEXCOORD2IVARBPROC				nglMultiTexCoord2ivARB;
NEL_PFNGLMULTITEXCOORD2FVARBPROC				nglMultiTexCoord2fvARB;
NEL_PFNGLMULTITEXCOORD2DVARBPROC				nglMultiTexCoord2dvARB;
NEL_PFNGLMULTITEXCOORD3SVARBPROC				nglMultiTexCoord3svARB;
NEL_PFNGLMULTITEXCOORD3IVARBPROC				nglMultiTexCoord3ivARB;
NEL_PFNGLMULTITEXCOORD3FVARBPROC				nglMultiTexCoord3fvARB;
NEL_PFNGLMULTITEXCOORD3DVARBPROC				nglMultiTexCoord3dvARB;
NEL_PFNGLMULTITEXCOORD4SVARBPROC				nglMultiTexCoord4svARB;
NEL_PFNGLMULTITEXCOORD4IVARBPROC				nglMultiTexCoord4ivARB;
NEL_PFNGLMULTITEXCOORD4FVARBPROC				nglMultiTexCoord4fvARB;
NEL_PFNGLMULTITEXCOORD4DVARBPROC				nglMultiTexCoord4dvARB;

// ARB_TextureCompression.
NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC			nglCompressedTexImage3DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC			nglCompressedTexImage2DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC			nglCompressedTexImage1DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC			nglCompressedTexSubImage3DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC			nglCompressedTexSubImage2DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC			nglCompressedTexSubImage1DARB;
NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC			nglGetCompressedTexImageARB;

// VertexWeighting.
NEL_PFNGLVERTEXWEIGHTFEXTPROC					nglVertexWeightfEXT;
NEL_PFNGLVERTEXWEIGHTFVEXTPROC					nglVertexWeightfvEXT;
NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC				nglVertexWeightPointerEXT;

// SecondaryColor extension
NEL_PFNGLSECONDARYCOLOR3BEXTPROC				nglSecondaryColor3bEXT;
NEL_PFNGLSECONDARYCOLOR3BVEXTPROC				nglSecondaryColor3bvEXT;
NEL_PFNGLSECONDARYCOLOR3DEXTPROC				nglSecondaryColor3dEXT;
NEL_PFNGLSECONDARYCOLOR3DVEXTPROC				nglSecondaryColor3dvEXT;
NEL_PFNGLSECONDARYCOLOR3FEXTPROC				nglSecondaryColor3fEXT;
NEL_PFNGLSECONDARYCOLOR3FVEXTPROC				nglSecondaryColor3fvEXT;
NEL_PFNGLSECONDARYCOLOR3IEXTPROC				nglSecondaryColor3iEXT;
NEL_PFNGLSECONDARYCOLOR3IVEXTPROC				nglSecondaryColor3ivEXT;
NEL_PFNGLSECONDARYCOLOR3SEXTPROC				nglSecondaryColor3sEXT;
NEL_PFNGLSECONDARYCOLOR3SVEXTPROC				nglSecondaryColor3svEXT;
NEL_PFNGLSECONDARYCOLOR3UBEXTPROC				nglSecondaryColor3ubEXT;
NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC				nglSecondaryColor3ubvEXT;
NEL_PFNGLSECONDARYCOLOR3UIEXTPROC				nglSecondaryColor3uiEXT;
NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC				nglSecondaryColor3uivEXT;
NEL_PFNGLSECONDARYCOLOR3USEXTPROC				nglSecondaryColor3usEXT;
NEL_PFNGLSECONDARYCOLOR3USVEXTPROC				nglSecondaryColor3usvEXT;
NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC			nglSecondaryColorPointerEXT;

// BlendColor extension
NEL_PFNGLBLENDCOLOREXTPROC						nglBlendColorEXT;

// GL_ATI_envmap_bumpmap extension
PFNGLTEXBUMPPARAMETERIVATIPROC					nglTexBumpParameterivATI;
PFNGLTEXBUMPPARAMETERFVATIPROC					nglTexBumpParameterfvATI;
PFNGLGETTEXBUMPPARAMETERIVATIPROC				nglGetTexBumpParameterivATI;
PFNGLGETTEXBUMPPARAMETERFVATIPROC				nglGetTexBumpParameterfvATI;

// GL_ATI_fragment_shader extension
NEL_PFNGLGENFRAGMENTSHADERSATIPROC				nglGenFragmentShadersATI;
NEL_PFNGLBINDFRAGMENTSHADERATIPROC				nglBindFragmentShaderATI;
NEL_PFNGLDELETEFRAGMENTSHADERATIPROC			nglDeleteFragmentShaderATI;
NEL_PFNGLBEGINFRAGMENTSHADERATIPROC				nglBeginFragmentShaderATI;
NEL_PFNGLENDFRAGMENTSHADERATIPROC				nglEndFragmentShaderATI;
NEL_PFNGLPASSTEXCOORDATIPROC					nglPassTexCoordATI;
NEL_PFNGLSAMPLEMAPATIPROC						nglSampleMapATI;
NEL_PFNGLCOLORFRAGMENTOP1ATIPROC				nglColorFragmentOp1ATI;
NEL_PFNGLCOLORFRAGMENTOP2ATIPROC				nglColorFragmentOp2ATI;
NEL_PFNGLCOLORFRAGMENTOP3ATIPROC				nglColorFragmentOp3ATI;
NEL_PFNGLALPHAFRAGMENTOP1ATIPROC				nglAlphaFragmentOp1ATI;
NEL_PFNGLALPHAFRAGMENTOP2ATIPROC				nglAlphaFragmentOp2ATI;
NEL_PFNGLALPHAFRAGMENTOP3ATIPROC				nglAlphaFragmentOp3ATI;
NEL_PFNGLSETFRAGMENTSHADERCONSTANTATIPROC		nglSetFragmentShaderConstantATI;

// GL_ARB_fragment_program
// the following functions are the sames than with GL_ARB_vertex_program
//NEL_PFNGLPROGRAMSTRINGARBPROC					nglProgramStringARB;
//NEL_PFNGLBINDPROGRAMARBPROC					nglBindProgramARB;
//NEL_PFNGLDELETEPROGRAMSARBPROC				nglDeleteProgramsARB;
//NEL_PFNGLGENPROGRAMSARBPROC					nglGenProgramsARB;
//NEL_PFNGLPROGRAMENVPARAMETER4DARBPROC			nglProgramEnvParameter4dARB;
//NEL_PFNGLPROGRAMENVPARAMETER4DVARBPROC		nglProgramEnvParameter4dvARB;
//NEL_PFNGLPROGRAMENVPARAMETER4FARBPROC			nglProgramEnvParameter4fARB;
//NEL_PFNGLPROGRAMENVPARAMETER4FVARBPROC		nglProgramEnvParameter4fvARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4DARBPROC			nglGetProgramLocalParameter4dARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4DVARBPROC		nglGetProgramLocalParameter4dvARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4FARBPROC			nglGetProgramLocalParameter4fARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4FVARBPROC		nglGetProgramLocalParameter4fvARB;
//NEL_PFNGLGETPROGRAMENVPARAMETERDVARBPROC		nglGetProgramEnvParameterdvARB;
//NEL_PFNGLGETPROGRAMENVPARAMETERFVARBPROC		nglGetProgramEnvParameterfvARB;
//NEL_PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC	nglGetProgramLocalParameterdvARB;
//NEL_PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC	nglGetProgramLocalParameterfvARB;
//NEL_PFNGLGETPROGRAMIVARBPROC					nglGetProgramivARB;
//NEL_PFNGLGETPROGRAMSTRINGARBPROC				nglGetProgramStringARB;
//NEL_PFNGLISPROGRAMARBPROC						nglIsProgramARB;

// GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERPROC							nglBindBuffer;
PFNGLDELETEBUFFERSPROC						nglDeleteBuffers;
PFNGLGENBUFFERSPROC							nglGenBuffers;
PFNGLISBUFFERPROC 							nglIsBuffer;
PFNGLBUFFERDATAPROC 							nglBufferData;
PFNGLBUFFERSUBDATAPROC 						nglBufferSubData;
PFNGLGETBUFFERSUBDATAPROC 					nglGetBufferSubData;
PFNGLMAPBUFFERPROC 							nglMapBuffer;
PFNGLUNMAPBUFFERPROC 						nglUnmapBuffer;
PFNGLGETBUFFERPARAMETERIVPROC 				nglGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC 					nglGetBufferPointerv;

// GL_ARB_vertex_program
PFNGLVERTEXATTRIB1SARBPROC						nglVertexAttrib1sARB;
PFNGLVERTEXATTRIB1FARBPROC						nglVertexAttrib1fARB;
PFNGLVERTEXATTRIB1DARBPROC						nglVertexAttrib1dARB;
PFNGLVERTEXATTRIB2SARBPROC						nglVertexAttrib2sARB;
PFNGLVERTEXATTRIB2FARBPROC						nglVertexAttrib2fARB;
PFNGLVERTEXATTRIB2DARBPROC						nglVertexAttrib2dARB;
PFNGLVERTEXATTRIB3SARBPROC						nglVertexAttrib3sARB;
PFNGLVERTEXATTRIB3FARBPROC						nglVertexAttrib3fARB;
PFNGLVERTEXATTRIB3DARBPROC						nglVertexAttrib3dARB;
PFNGLVERTEXATTRIB4SARBPROC						nglVertexAttrib4sARB;
PFNGLVERTEXATTRIB4FARBPROC						nglVertexAttrib4fARB;
PFNGLVERTEXATTRIB4DARBPROC						nglVertexAttrib4dARB;
PFNGLVERTEXATTRIB4NUBARBPROC					nglVertexAttrib4NubARB;
PFNGLVERTEXATTRIB1SVARBPROC						nglVertexAttrib1svARB;
PFNGLVERTEXATTRIB1FVARBPROC						nglVertexAttrib1fvARB;
PFNGLVERTEXATTRIB1DVARBPROC						nglVertexAttrib1dvARB;
PFNGLVERTEXATTRIB2SVARBPROC						nglVertexAttrib2svARB;
PFNGLVERTEXATTRIB2FVARBPROC						nglVertexAttrib2fvARB;
PFNGLVERTEXATTRIB2DVARBPROC						nglVertexAttrib2dvARB;
PFNGLVERTEXATTRIB3SVARBPROC						nglVertexAttrib3svARB;
PFNGLVERTEXATTRIB3FVARBPROC						nglVertexAttrib3fvARB;
PFNGLVERTEXATTRIB3DVARBPROC						nglVertexAttrib3dvARB;
PFNGLVERTEXATTRIB4BVARBPROC						nglVertexAttrib4bvARB;
PFNGLVERTEXATTRIB4SVARBPROC						nglVertexAttrib4svARB;
PFNGLVERTEXATTRIB4IVARBPROC						nglVertexAttrib4ivARB;
PFNGLVERTEXATTRIB4UBVARBPROC					nglVertexAttrib4ubvARB;
PFNGLVERTEXATTRIB4USVARBPROC					nglVertexAttrib4usvARB;
PFNGLVERTEXATTRIB4UIVARBPROC					nglVertexAttrib4uivARB;
PFNGLVERTEXATTRIB4FVARBPROC						nglVertexAttrib4fvARB;
PFNGLVERTEXATTRIB4DVARBPROC						nglVertexAttrib4dvARB;
PFNGLVERTEXATTRIB4NBVARBPROC					nglVertexAttrib4NbvARB;
PFNGLVERTEXATTRIB4NSVARBPROC					nglVertexAttrib4NsvARB;
PFNGLVERTEXATTRIB4NIVARBPROC					nglVertexAttrib4NivARB;
PFNGLVERTEXATTRIB4NUBVARBPROC					nglVertexAttrib4NubvARB;
PFNGLVERTEXATTRIB4NUSVARBPROC					nglVertexAttrib4NusvARB;
PFNGLVERTEXATTRIB4NUIVARBPROC					nglVertexAttrib4NuivARB;
PFNGLVERTEXATTRIBPOINTERARBPROC					nglVertexAttribPointerARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC				nglEnableVertexAttribArrayARB;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC			nglDisableVertexAttribArrayARB;
PFNGLPROGRAMSTRINGARBPROC						nglProgramStringARB;
PFNGLBINDPROGRAMARBPROC							nglBindProgramARB;
PFNGLDELETEPROGRAMSARBPROC						nglDeleteProgramsARB;
PFNGLGENPROGRAMSARBPROC							nglGenProgramsARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC				nglProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4DARBPROC				nglProgramEnvParameter4dARB;
PFNGLPROGRAMENVPARAMETER4FVARBPROC				nglProgramEnvParameter4fvARB;
PFNGLPROGRAMENVPARAMETER4DVARBPROC				nglProgramEnvParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC				nglProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC				nglProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC			nglProgramLocalParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC			nglProgramLocalParameter4dvARB;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC			nglGetProgramEnvParameterfvARB;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC			nglGetProgramEnvParameterdvARB;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC			nglGetProgramLocalParameterfvARB;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC			nglGetProgramLocalParameterdvARB;
PFNGLGETPROGRAMIVARBPROC						nglGetProgramivARB;
PFNGLGETPROGRAMSTRINGARBPROC					nglGetProgramStringARB;
PFNGLGETVERTEXATTRIBDVARBPROC					nglGetVertexAttribdvARB;
PFNGLGETVERTEXATTRIBFVARBPROC					nglGetVertexAttribfvARB;
PFNGLGETVERTEXATTRIBIVARBPROC					nglGetVertexAttribivARB;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC				nglGetVertexAttribPointervARB;
PFNGLISPROGRAMARBPROC							nglIsProgramARB;

// NV_occlusion_query
NEL_PFNGLGENOCCLUSIONQUERIESNVPROC				nglGenOcclusionQueriesNV;
NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC			nglDeleteOcclusionQueriesNV;
NEL_PFNGLISOCCLUSIONQUERYNVPROC					nglIsOcclusionQueryNV;
NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC				nglBeginOcclusionQueryNV;
NEL_PFNGLENDOCCLUSIONQUERYNVPROC				nglEndOcclusionQueryNV;
NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC				nglGetOcclusionQueryivNV;
NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC				nglGetOcclusionQueryuivNV;

// GL_EXT_framebuffer_object
NEL_PFNGLISRENDERBUFFEREXTPROC					nglIsRenderbufferEXT;
NEL_PFNGLISFRAMEBUFFEREXTPROC					nglIsFramebufferEXT;
NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC			nglCheckFramebufferStatusEXT;
NEL_PFNGLGENFRAMEBUFFERSEXTPROC					nglGenFramebuffersEXT;
NEL_PFNGLBINDFRAMEBUFFEREXTPROC					nglBindFramebufferEXT;
NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC			nglFramebufferTexture2DEXT;
NEL_PFNGLGENRENDERBUFFERSEXTPROC				nglGenRenderbuffersEXT;
NEL_PFNGLBINDRENDERBUFFEREXTPROC				nglBindRenderbufferEXT;
NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC				nglRenderbufferStorageEXT;
NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC			nglFramebufferRenderbufferEXT;
NEL_PFNGLDELETERENDERBUFFERSEXTPROC				nglDeleteRenderbuffersEXT;
NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC				nglDeleteFramebuffersEXT;
NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC		nglGetRenderbufferParameterivEXT;
NEL_PFNGENERATEMIPMAPEXTPROC					nglGenerateMipmapEXT;

// GL_EXT_framebuffer_blit
NEL_PFNGLBLITFRAMEBUFFEREXTPROC					nglBlitFramebufferEXT;

// GL_EXT_framebuffer_multisample
NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC	nglRenderbufferStorageMultisampleEXT;

// GL_ARB_multisample
NEL_PFNGLSAMPLECOVERAGEARBPROC					nglSampleCoverageARB;

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
	if(strstr(glext, ext_str)==NULL) { nlwarning("3D: OpengGL extension '%s' was not found", ext_str); return false; } else { nldebug("3D: OpengGL Extension '%s' found", ext_str); }

// Debug: don't return false if the procaddr returns 0
// It means that it can crash if nel calls this extension but at least we have a warning to know why the extension is available but not the procaddr
#define CHECK_ADDRESS(type, ext) \
	n##ext=(type)nglGetProcAddress(#ext); \
	if(!n##ext) { nlwarning("3D: GetProcAddress(\"%s\") returns NULL", #ext); return false; } else { /*nldebug("3D: GetProcAddress(\"%s\") succeed", #ext);*/ }

// ***************************************************************************
// Extensions registrations, and Windows function Registration.

// *********************************
static bool setupARBMultiTexture(const char	*glext)
{
	H_AUTO_OGL(setupARBMultiTexture);

	CHECK_EXT("GL_ARB_multitexture");

	CHECK_ADDRESS(NEL_PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);
	CHECK_ADDRESS(NEL_PFNGLCLIENTACTIVETEXTUREARBPROC, glClientActiveTextureARB);

	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1SARBPROC, glMultiTexCoord1sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1IARBPROC, glMultiTexCoord1iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1FARBPROC, glMultiTexCoord1fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1DARBPROC, glMultiTexCoord1dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2SARBPROC, glMultiTexCoord2sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2IARBPROC, glMultiTexCoord2iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2DARBPROC, glMultiTexCoord2dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3SARBPROC, glMultiTexCoord3sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3IARBPROC, glMultiTexCoord3iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3FARBPROC, glMultiTexCoord3fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3DARBPROC, glMultiTexCoord3dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4SARBPROC, glMultiTexCoord4sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4IARBPROC, glMultiTexCoord4iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4FARBPROC, glMultiTexCoord4fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4DARBPROC, glMultiTexCoord4dARB);

	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1SVARBPROC, glMultiTexCoord1svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1IVARBPROC, glMultiTexCoord1ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1FVARBPROC, glMultiTexCoord1fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1DVARBPROC, glMultiTexCoord1dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2SVARBPROC, glMultiTexCoord2svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2IVARBPROC, glMultiTexCoord2ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2DVARBPROC, glMultiTexCoord2dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3SVARBPROC, glMultiTexCoord3svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3IVARBPROC, glMultiTexCoord3ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3FVARBPROC, glMultiTexCoord3fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3DVARBPROC, glMultiTexCoord3dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4SVARBPROC, glMultiTexCoord4svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4IVARBPROC, glMultiTexCoord4ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4FVARBPROC, glMultiTexCoord4fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4DVARBPROC, glMultiTexCoord4dvARB);

	return true;
}

// *********************************
static bool setupEXTTextureEnvCombine(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureEnvCombine);

	return (strstr(glext, "GL_EXT_texture_env_combine")!=NULL || strstr(glext, "GL_ARB_texture_env_combine")!=NULL);
}


// *********************************
static bool	setupARBTextureCompression(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

	CHECK_EXT("GL_ARB_texture_compression");

	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC, glCompressedTexImage3DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC, glCompressedTexImage2DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC, glCompressedTexImage1DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC, glCompressedTexSubImage3DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC, glCompressedTexSubImage2DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC, glCompressedTexSubImage1DARB);
	CHECK_ADDRESS(NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC, glGetCompressedTexImageARB);

	return true;
}

// *********************************
static bool	setupARBTextureNonPowerOfTwo(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

	CHECK_EXT("GL_ARB_texture_non_power_of_two");

	return true;
}

// *********************************
static bool	setupEXTTextureCompressionS3TC(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureCompressionS3TC);

	CHECK_EXT("GL_EXT_texture_compression_s3tc");
	// TODO: check also for GL_S3_s3tc, GL_EXT_texture_compression_dxt1

	return true;
}

// *********************************
static bool	setupEXTVertexWeighting(const char	*glext)
{
	H_AUTO_OGL(setupEXTVertexWeighting);
	CHECK_EXT("GL_EXT_vertex_weighting");

	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTFEXTPROC, glVertexWeightfEXT);
	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTFVEXTPROC, glVertexWeightfvEXT);
	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC, glVertexWeightPointerEXT);

	return true;
}


// *********************************
static bool	setupEXTSeparateSpecularColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTSeparateSpecularColor);
	CHECK_EXT("GL_EXT_separate_specular_color");
	return true;
}

// *********************************
static bool	setupATITextureEnvCombine3(const char	*glext)
{
	H_AUTO_OGL(setupATITextureEnvCombine3);

// reenabled to allow bloom on mac, TODO: cleanly fix the water issue
// i think this issue was mtp target related - is this the case in ryzom too?
// #ifdef NL_OS_MAC
// // Water doesn't render on GeForce 8600M GT (on MAC OS X) if this extension is enabled
// 	return false;
// #endif

	CHECK_EXT("GL_ATI_texture_env_combine3");
	return true;
}

// *********************************
static bool	setupATIXTextureEnvRoute(const char * /* glext */)
{
	H_AUTO_OGL(setupATIXTextureEnvRoute);
	return false;
//	CHECK_EXT("GL_ATIX_texture_env_route");
//	return true;
}

// *********************************
static bool	setupATIEnvMapBumpMap(const char	*glext)
{
	H_AUTO_OGL(setupATIEnvMapBumpMap);
	CHECK_EXT("GL_ATI_envmap_bumpmap");

	GLint num = -1;

	CHECK_ADDRESS(PFNGLTEXBUMPPARAMETERIVATIPROC, glTexBumpParameterivATI);
	CHECK_ADDRESS(PFNGLTEXBUMPPARAMETERFVATIPROC, glTexBumpParameterfvATI);
	CHECK_ADDRESS(PFNGLGETTEXBUMPPARAMETERIVATIPROC, glGetTexBumpParameterivATI);
	CHECK_ADDRESS(PFNGLGETTEXBUMPPARAMETERFVATIPROC, glGetTexBumpParameterfvATI);

	// Check for broken ATI drivers and disable EMBM if we caught one.
	// Reminder: This code crashes with Catalyst 7.11 fglrx drivers!
	nglGetTexBumpParameterivATI(GL_BUMP_NUM_TEX_UNITS_ATI, &num);

	return num > 0;
}

// *********************************
static bool	setupARBTextureCubeMap(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCubeMap);

	CHECK_EXT("GL_ARB_texture_cube_map");

	return true;
}

// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTSecondaryColor);
	CHECK_EXT("GL_EXT_secondary_color");

	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3BEXTPROC, glSecondaryColor3bEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3BVEXTPROC, glSecondaryColor3bvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3DEXTPROC, glSecondaryColor3dEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3DVEXTPROC, glSecondaryColor3dvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3FEXTPROC, glSecondaryColor3fEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3FVEXTPROC, glSecondaryColor3fvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3IEXTPROC, glSecondaryColor3iEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3IVEXTPROC, glSecondaryColor3ivEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3SEXTPROC, glSecondaryColor3sEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3SVEXTPROC, glSecondaryColor3svEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UBEXTPROC, glSecondaryColor3ubEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC, glSecondaryColor3ubvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UIEXTPROC, glSecondaryColor3uiEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC, glSecondaryColor3uivEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3USEXTPROC, glSecondaryColor3usEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3USVEXTPROC, glSecondaryColor3usvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC, glSecondaryColorPointerEXT);

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

// *********************************
static bool	setupARBMultisample(const char	*glext)
{
	H_AUTO_OGL(setupARBMultisample);
	CHECK_EXT("GL_ARB_multisample");

	CHECK_ADDRESS(NEL_PFNGLSAMPLECOVERAGEARBPROC, glSampleCoverageARB);

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

// *********************************
static bool	setupEXTBlendColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTBlendColor);
	CHECK_EXT("GL_EXT_blend_color");

	CHECK_ADDRESS(NEL_PFNGLBLENDCOLOREXTPROC, glBlendColorEXT);

	return true;
}

// *********************************
static bool	setupATIFragmentShader(const char *glext)
{
	H_AUTO_OGL(setupATIFragmentShader);
	CHECK_EXT("GL_ATI_fragment_shader");

	CHECK_ADDRESS(NEL_PFNGLGENFRAGMENTSHADERSATIPROC, glGenFragmentShadersATI);
	CHECK_ADDRESS(NEL_PFNGLBINDFRAGMENTSHADERATIPROC, glBindFragmentShaderATI);
	CHECK_ADDRESS(NEL_PFNGLDELETEFRAGMENTSHADERATIPROC, glDeleteFragmentShaderATI);
	CHECK_ADDRESS(NEL_PFNGLBEGINFRAGMENTSHADERATIPROC, glBeginFragmentShaderATI);
	CHECK_ADDRESS(NEL_PFNGLENDFRAGMENTSHADERATIPROC, glEndFragmentShaderATI);
	CHECK_ADDRESS(NEL_PFNGLPASSTEXCOORDATIPROC, glPassTexCoordATI);
	CHECK_ADDRESS(NEL_PFNGLSAMPLEMAPATIPROC, glSampleMapATI);
	CHECK_ADDRESS(NEL_PFNGLCOLORFRAGMENTOP1ATIPROC, glColorFragmentOp1ATI);
	CHECK_ADDRESS(NEL_PFNGLCOLORFRAGMENTOP2ATIPROC, glColorFragmentOp2ATI);
	CHECK_ADDRESS(NEL_PFNGLCOLORFRAGMENTOP3ATIPROC, glColorFragmentOp3ATI);
	CHECK_ADDRESS(NEL_PFNGLALPHAFRAGMENTOP1ATIPROC, glAlphaFragmentOp1ATI);
	CHECK_ADDRESS(NEL_PFNGLALPHAFRAGMENTOP2ATIPROC, glAlphaFragmentOp2ATI);
	CHECK_ADDRESS(NEL_PFNGLALPHAFRAGMENTOP3ATIPROC, glAlphaFragmentOp3ATI);
	CHECK_ADDRESS(NEL_PFNGLSETFRAGMENTSHADERCONSTANTATIPROC, glSetFragmentShaderConstantATI);

	return true;
}

// *********************************
static bool	setupARBFragmentProgram(const char *glext)
{
	H_AUTO_OGL(setupARBFragmentProgram);
	CHECK_EXT("GL_ARB_fragment_program");

	CHECK_ADDRESS(NEL_PFNGLPROGRAMSTRINGARBPROC, glProgramStringARB);
	CHECK_ADDRESS(NEL_PFNGLBINDPROGRAMARBPROC, glBindProgramARB);
	CHECK_ADDRESS(NEL_PFNGLDELETEPROGRAMSARBPROC, glDeleteProgramsARB);
	CHECK_ADDRESS(NEL_PFNGLGENPROGRAMSARBPROC, glGenProgramsARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMENVPARAMETER4DARBPROC, glProgramEnvParameter4dARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMENVPARAMETER4DVARBPROC, glProgramEnvParameter4dvARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMENVPARAMETER4FARBPROC, glProgramEnvParameter4fARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMENVPARAMETER4FVARBPROC, glProgramEnvParameter4fvARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMLOCALPARAMETER4DARBPROC, glProgramLocalParameter4dARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMLOCALPARAMETER4DVARBPROC, glProgramLocalParameter4dvARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMLOCALPARAMETER4FARBPROC, glProgramLocalParameter4fARB);
	CHECK_ADDRESS(NEL_PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMENVPARAMETERDVARBPROC, glGetProgramEnvParameterdvARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMENVPARAMETERFVARBPROC, glGetProgramEnvParameterfvARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC, glGetProgramLocalParameterdvARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC, glGetProgramLocalParameterfvARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMIVARBPROC, glGetProgramivARB);
	CHECK_ADDRESS(NEL_PFNGLGETPROGRAMSTRINGARBPROC, glGetProgramStringARB);
	CHECK_ADDRESS(NEL_PFNGLISPROGRAMARBPROC, glIsProgramARB);

	return true;
}

// ***************************************************************************
static bool	setupARBVertexBufferObject(const char	*glext)
{
	H_AUTO_OGL(setupARBVertexBufferObject);

	CHECK_EXT("GL_ARB_vertex_buffer_object");

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

	return true;
}

// ***************************************************************************
static bool	setupARBVertexProgram(const char	*glext)
{
	H_AUTO_OGL(setupARBVertexProgram);
	CHECK_EXT("GL_ARB_vertex_program");

	CHECK_ADDRESS(PFNGLVERTEXATTRIB1SARBPROC, glVertexAttrib1sARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1FARBPROC, glVertexAttrib1fARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1DARBPROC, glVertexAttrib1dARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2SARBPROC, glVertexAttrib2sARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2FARBPROC, glVertexAttrib2fARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2DARBPROC, glVertexAttrib2dARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3SARBPROC, glVertexAttrib3sARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3FARBPROC, glVertexAttrib3fARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3DARBPROC, glVertexAttrib3dARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4SARBPROC, glVertexAttrib4sARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4FARBPROC, glVertexAttrib4fARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4DARBPROC, glVertexAttrib4dARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NUBARBPROC, glVertexAttrib4NubARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1SVARBPROC, glVertexAttrib1svARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1FVARBPROC, glVertexAttrib1fvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1DVARBPROC, glVertexAttrib1dvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2SVARBPROC, glVertexAttrib2svARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2FVARBPROC, glVertexAttrib2fvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2DVARBPROC, glVertexAttrib2dvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3SVARBPROC, glVertexAttrib3svARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3FVARBPROC, glVertexAttrib3fvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3DVARBPROC, glVertexAttrib3dvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4BVARBPROC, glVertexAttrib4bvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4SVARBPROC, glVertexAttrib4svARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4IVARBPROC, glVertexAttrib4ivARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4UBVARBPROC, glVertexAttrib4ubvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4USVARBPROC, glVertexAttrib4usvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4UIVARBPROC, glVertexAttrib4uivARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4FVARBPROC, glVertexAttrib4fvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4DVARBPROC, glVertexAttrib4dvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NBVARBPROC, glVertexAttrib4NbvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NSVARBPROC, glVertexAttrib4NsvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NIVARBPROC, glVertexAttrib4NivARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NUBVARBPROC, glVertexAttrib4NubvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NUSVARBPROC, glVertexAttrib4NusvARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4NUIVARBPROC, glVertexAttrib4NuivARB);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBPOINTERARBPROC, glVertexAttribPointerARB);
	CHECK_ADDRESS(PFNGLENABLEVERTEXATTRIBARRAYARBPROC, glEnableVertexAttribArrayARB);
	CHECK_ADDRESS(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC, glDisableVertexAttribArrayARB);
	CHECK_ADDRESS(PFNGLPROGRAMSTRINGARBPROC, glProgramStringARB);
	CHECK_ADDRESS(PFNGLBINDPROGRAMARBPROC, glBindProgramARB);
	CHECK_ADDRESS(PFNGLDELETEPROGRAMSARBPROC, glDeleteProgramsARB);
	CHECK_ADDRESS(PFNGLGENPROGRAMSARBPROC, glGenProgramsARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4FARBPROC, glProgramEnvParameter4fARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4DARBPROC, glProgramEnvParameter4dARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4FVARBPROC, glProgramEnvParameter4fvARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4DVARBPROC, glProgramEnvParameter4dvARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4FARBPROC, glProgramLocalParameter4fARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4DARBPROC, glProgramLocalParameter4dARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4DVARBPROC, glProgramLocalParameter4dvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMENVPARAMETERFVARBPROC, glGetProgramEnvParameterfvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMENVPARAMETERDVARBPROC, glGetProgramEnvParameterdvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC, glGetProgramLocalParameterfvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC, glGetProgramLocalParameterdvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMIVARBPROC, glGetProgramivARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMSTRINGARBPROC, glGetProgramStringARB);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBDVARBPROC, glGetVertexAttribdvARB);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBFVARBPROC, glGetVertexAttribfvARB);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBIVARBPROC, glGetVertexAttribivARB);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBPOINTERVARBPROC, glGetVertexAttribPointervARB);
	CHECK_ADDRESS(PFNGLISPROGRAMARBPROC, glIsProgramARB);

	return true;
}

// ***************************************************************************
static bool	setupNVOcclusionQuery(const char	*glext)
{
	H_AUTO_OGL(setupNVOcclusionQuery);
	CHECK_EXT("GL_NV_occlusion_query");

	CHECK_ADDRESS(NEL_PFNGLGENOCCLUSIONQUERIESNVPROC, glGenOcclusionQueriesNV);
	CHECK_ADDRESS(NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC, glDeleteOcclusionQueriesNV);
	CHECK_ADDRESS(NEL_PFNGLISOCCLUSIONQUERYNVPROC, glIsOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC, glBeginOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLENDOCCLUSIONQUERYNVPROC, glEndOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC, glGetOcclusionQueryivNV);
	CHECK_ADDRESS(NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC, glGetOcclusionQueryuivNV);

	return true;
}


// ***************************************************************************
static bool	setupNVTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupNVTextureRectangle);
	CHECK_EXT("GL_NV_texture_rectangle");
	return true;
}

// ***************************************************************************
static bool	setupEXTTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureRectangle);
	CHECK_EXT("GL_EXT_texture_rectangle");
	return true;
}

// ***************************************************************************
static bool	setupARBTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureRectangle);

	CHECK_EXT("GL_ARB_texture_rectangle");

	return true;
}

// ***************************************************************************
static bool	setupEXTTextureFilterAnisotropic(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureFilterAnisotropic);
	CHECK_EXT("GL_EXT_texture_filter_anisotropic");
	return true;
}

// ***************************************************************************
static bool	setupFrameBufferObject(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferObject);

	CHECK_EXT("GL_EXT_framebuffer_object");

	CHECK_ADDRESS(NEL_PFNGLISRENDERBUFFEREXTPROC, glIsRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLISFRAMEBUFFEREXTPROC, glIsFramebufferEXT);
	CHECK_ADDRESS(NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
	CHECK_ADDRESS(NEL_PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT);
	CHECK_ADDRESS(NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);
	CHECK_ADDRESS(NEL_PFNGLGENRENDERBUFFERSEXTPROC, glGenRenderbuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLBINDRENDERBUFFEREXTPROC, glBindRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC, glRenderbufferStorageEXT);
	CHECK_ADDRESS(NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC, glFramebufferRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLDELETERENDERBUFFERSEXTPROC, glDeleteRenderbuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC, glDeleteFramebuffersEXT);
	CHECK_ADDRESS(NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC, glGetRenderbufferParameterivEXT);
	CHECK_ADDRESS(NEL_PFNGENERATEMIPMAPEXTPROC, glGenerateMipmapEXT);

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferBlit(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferBlit);
	CHECK_EXT("GL_EXT_framebuffer_blit");

	CHECK_ADDRESS(NEL_PFNGLBLITFRAMEBUFFEREXTPROC, glBlitFramebufferEXT);

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferMultisample(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferMultisample);
	CHECK_EXT("GL_EXT_framebuffer_multisample");

	CHECK_ADDRESS(NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC, glRenderbufferStorageMultisampleEXT);

	return true;
}

// ***************************************************************************
static bool	setupPackedDepthStencil(const char	*glext)
{
	H_AUTO_OGL(setupPackedDepthStencil);

	CHECK_EXT("GL_EXT_packed_depth_stencil");

	return true;
}

// ***************************************************************************
// Extension Check.
void	registerGlExtensions(CGlExtensions &ext)
{
	H_AUTO_OGL(registerGlExtensions);

	const char	*nglVersion= (const char *)glGetString (GL_VERSION);
	sint	a=0, b=0;

	sscanf(nglVersion, "%d.%d", &a, &b);
	if( ( a < 3 ) || ( ( a == 3 ) && ( b < 3 ) ) )
	{
		nlinfo( "OpenGL version is less than 3.3!" );
		nlinfo( "Version string: %s",nglVersion );
		nlassert( false );
	}
	
	// Extensions.
	const char	*glext= (const char*)glGetString(GL_EXTENSIONS);
	GLint	ntext;

	nldebug("3D: Available OpenGL Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check ARBMultiTexture
	ext.ARBMultiTexture= setupARBMultiTexture(glext);
	if(ext.ARBMultiTexture)
	{
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &ntext);
		// We could have more than IDRV_MAT_MAXTEXTURES but the interface only
		// support IDRV_MAT_MAXTEXTURES texture stages so take min
		ext.NbTextureStages= (ntext<((GLint)IDRV_MAT_MAXTEXTURES)?ntext:IDRV_MAT_MAXTEXTURES);
	}

	// Check EXTTextureEnvCombine
	ext.EXTTextureEnvCombine= setupEXTTextureEnvCombine(glext);

	// Check ARBTextureCompression
	ext.ARBTextureCompression= setupARBTextureCompression(glext);

	// Check ARBTextureNonPowerOfTwo
	ext.ARBTextureNonPowerOfTwo= setupARBTextureNonPowerOfTwo(glext);

	// Check ARBMultisample
	ext.ARBMultisample = setupARBMultisample(glext);

	// Compression S3TC OK iff ARBTextureCompression.
	ext.EXTTextureCompressionS3TC= (ext.ARBTextureCompression && setupEXTTextureCompressionS3TC(glext));

	// Check if NVidia GL_EXT_vertex_weighting is available.
	ext.EXTVertexWeighting= setupEXTVertexWeighting(glext);

	// Check EXTSeparateSpecularColor.
	ext.EXTSeparateSpecularColor= setupEXTSeparateSpecularColor(glext);

	// Check for cube mapping
	ext.ARBTextureCubeMap = setupARBTextureCubeMap(glext);

	setupARBVertexProgram(glext);

	// Check texture shaders
	// Disable feature ???
	if(!ext.DisableHardwareTextureShader)
	{
		ext.ATIEnvMapBumpMap = setupATIEnvMapBumpMap(glext);
		ext.ATIFragmentShader = setupATIFragmentShader(glext);
		ext.ARBFragmentProgram = setupARBFragmentProgram(glext);
	}
	else
	{
		ext.ATIEnvMapBumpMap = false;
		ext.ATIFragmentShader = false;
		ext.ARBFragmentProgram = false;
	}

	// Check EXTSecondaryColor
	ext.EXTSecondaryColor= setupEXTSecondaryColor(glext);

	// Check EXTBlendColor
	ext.EXTBlendColor= setupEXTBlendColor(glext);

	// Check NV_occlusion_query
	ext.NVOcclusionQuery = setupNVOcclusionQuery(glext);

	// Check GL_NV_texture_rectangle
	ext.NVTextureRectangle = setupNVTextureRectangle(glext);

	// Check GL_EXT_texture_rectangle
	ext.EXTTextureRectangle = setupEXTTextureRectangle(glext);

	// Check GL_ARB_texture_rectangle
	ext.ARBTextureRectangle = setupARBTextureRectangle(glext);

	// Check GL_EXT_texture_filter_anisotropic
	ext.EXTTextureFilterAnisotropic = setupEXTTextureFilterAnisotropic(glext);

	if (ext.EXTTextureFilterAnisotropic)
	{
		// get the maximum value
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &ext.EXTTextureFilterAnisotropicMaximum);
	}

	// Check GL_EXT_framebuffer_object
	ext.FrameBufferObject = setupFrameBufferObject(glext);

	// Check GL_EXT_framebuffer_blit
	ext.FrameBufferBlit = setupFrameBufferBlit(glext);

	// Check GL_EXT_framebuffer_multisample
	ext.FrameBufferMultisample = setupFrameBufferMultisample(glext);

	// Check GL_EXT_packed_depth_stencil
	ext.PackedDepthStencil = setupPackedDepthStencil(glext);

	// ATI extensions
	// -------------

	// Check ATIXTextureEnvCombine3.
	ext.ATITextureEnvCombine3= setupATITextureEnvCombine3(glext);
	// Check ATIXTextureEnvRoute
	ext.ATIXTextureEnvRoute= setupATIXTextureEnvRoute(glext);

	setupARBVertexBufferObject(glext);
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
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check for pbuffer
	ext.WGLARBPBuffer= setupWGLARBPBuffer(glext);

	// Check for pixel format
	ext.WGLARBPixelFormat= setupWGLARBPixelFormat(glext);

	// Check for swap control
	ext.WGLEXTSwapControl= setupWGLEXTSwapControl(glext);

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
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
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
} // NLDRIVERGL/ES
#endif

} // NL3D
