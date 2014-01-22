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
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************
/// The extensions used by NL3D.
struct	CGlExtensions
{
	// Is this driver a correct OpenGL 1.2 driver?
	bool	Version1_2;

	// Required Extensions.
	bool	ARBMultiTexture;
	uint	NbTextureStages;
	bool	EXTTextureEnvCombine;

	// Optional Extensions.
	// NB: Fence extension is not here, because NVVertexArrayRange is false if GL_NV_fence is not here.
	bool	NVVertexArrayRange;
	uint	NVVertexArrayRangeMaxVertex;
	bool	EXTTextureCompressionS3TC;
	bool	EXTVertexWeighting;
	bool	EXTSeparateSpecularColor;
	bool	NVTextureEnvCombine4;
	bool	ARBTextureCubeMap;
	bool	NVVertexProgram;
	bool	EXTVertexShader;
	bool	NVTextureShader;
	bool	NVOcclusionQuery;
	bool	NVTextureRectangle;
	bool	EXTTextureRectangle;
	bool	ARBTextureRectangle;
	bool	FrameBufferObject;
	bool	FrameBufferBlit;
	bool	FrameBufferMultisample;
	bool	PackedDepthStencil;
	bool	EXTTextureFilterAnisotropic;
	float	EXTTextureFilterAnisotropicMaximum;

	// true if NVVertexProgram and if we know that VP is emulated
	bool	NVVertexProgramEmulated;
	bool	EXTSecondaryColor;
	bool	EXTBlendColor;
	// NVVertexArrayRange2.
	bool	NVVertexArrayRange2;
	// equal to GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV if possible, or GL_VERTEX_ARRAY_RANGE_NV
	uint	NVStateVARWithoutFlush;

	// WGL ARB extensions, true if supported
	bool	WGLARBPBuffer;
	bool	WGLARBPixelFormat;
	bool	WGLEXTSwapControl;

	// GLX extensions, true if supported
	bool	GLXEXTSwapControl;
	bool	GLXSGISwapControl;
	bool	GLXMESASwapControl;

	// ATI Extensions.
	bool	ATIVertexArrayObject;
	bool	ATIMapObjectBuffer;
	bool	ATITextureEnvCombine3;
	bool	ATIEnvMapBumpMap;
	bool	ATIFragmentShader;
	bool	ATIXTextureEnvRoute;
	bool	ATIVertexAttribArrayObject;
	// ARB Extensions
	bool	ARBTextureCompression;
	bool	ARBFragmentProgram;
	bool	ARBVertexBufferObject;
	bool	ARBVertexProgram;
	bool	ARBTextureNonPowerOfTwo;
	bool	ARBMultisample;

	// NV Pixel Programs
	bool	NVFragmentProgram2;

	bool	OESDrawTexture;
	bool	OESMapBuffer;

public:

	/// \name Disable Hardware feature. False by default. setuped by IDriver
	// @{
	bool				DisableHardwareVertexProgram;
	bool				DisableHardwarePixelProgram;
	bool				DisableHardwareVertexArrayAGP;
	bool				DisableHardwareTextureShader;
	// @}

public:
	CGlExtensions()
	{
		// Fill all false by default.
		Version1_2= false;
		ARBMultiTexture= false;
		NbTextureStages= 1;
		EXTTextureEnvCombine= false;
		ARBTextureCompression= false;
		NVVertexArrayRange= false;
		NVVertexArrayRangeMaxVertex= 0;
		EXTTextureCompressionS3TC= false;
		EXTVertexWeighting= false;
		EXTSeparateSpecularColor= false;
		NVTextureEnvCombine4= false;
		ATITextureEnvCombine3= false;
		ATIXTextureEnvRoute= false;
		ARBTextureCubeMap= false;
		NVTextureShader= false;
		NVVertexProgram= false;
		NVVertexProgramEmulated= false;
		EXTSecondaryColor= false;
		WGLARBPBuffer= false;
		WGLARBPixelFormat= false;
		WGLEXTSwapControl= false;
		GLXEXTSwapControl= false;
		GLXSGISwapControl= false;
		GLXMESASwapControl= false;
		EXTBlendColor= false;
		ATIVertexArrayObject= false;
		ATIEnvMapBumpMap = false;
		ATIFragmentShader = false;
		ATIVertexArrayObject = false;
		ATIMapObjectBuffer = false;
		ATIVertexAttribArrayObject = false;
		EXTVertexShader= false;
		ARBFragmentProgram = false;
		ARBVertexBufferObject = false;
		ARBVertexProgram = false;
		NVTextureRectangle = false;
		EXTTextureRectangle = false;
		EXTTextureFilterAnisotropic = false;
		EXTTextureFilterAnisotropicMaximum = 0.f;
		ARBTextureRectangle = false;
		ARBTextureNonPowerOfTwo = false;
		ARBMultisample = false;
		NVOcclusionQuery = false;
		FrameBufferObject = false;
		FrameBufferBlit = false;
		FrameBufferMultisample = false;
		PackedDepthStencil = false;
		NVVertexArrayRange2 = false;
		NVStateVARWithoutFlush = 0;

		OESDrawTexture = false;
		OESMapBuffer = false;

		/// \name Disable Hardware feature. False by default. setuped by IDriver
		DisableHardwareVertexProgram= false;
		DisableHardwarePixelProgram= false;
		DisableHardwareVertexArrayAGP= false;
		DisableHardwareTextureShader= false;
	}

	std::string toString()
	{
		std::string result = "OpenGL version ";
		result += Version1_2 ? "1.2 or above(*)" : "1.1 or below";
		result += "; Available extensions:";

		result += "\n  Texturing: ";
		result += ARBMultiTexture ? "ARBMultiTexture " : "";
		result += EXTTextureEnvCombine ? "EXTTextureEnvCombine(*) " : "";
		result += ARBTextureCompression ? "ARBTextureCompression " : "";
		result += EXTTextureCompressionS3TC ? "EXTTextureCompressionS3TC " : "";
		result += NVTextureEnvCombine4 ? "NVTextureEnvCombine4 " : "";
		result += ATITextureEnvCombine3 ? "ATITextureEnvCombine3 " : "";
		result += ATIXTextureEnvRoute ? "ATITextureEnvRoute " : "";
		result += ARBTextureCubeMap ? "ARBTextureCubeMap " : "";
		result += ATIEnvMapBumpMap ? "ATIEnvMapBumpMap " : "";
		result += NVTextureRectangle ? "NVTextureRectangle " : "";
		result += EXTTextureRectangle ? "EXTTextureRectangle " : "";
		result += ARBTextureRectangle ? "ARBTextureRectangle " : "";
		result += EXTTextureFilterAnisotropic ? "EXTTextureFilterAnisotropic (Maximum = " + NLMISC::toString(EXTTextureFilterAnisotropicMaximum) + ") " : "";
		result += ARBTextureNonPowerOfTwo ? "ARBTextureNonPowerOfTwo " : "";
		result += "texture stages(*) = ";
		result += NLMISC::toString(NbTextureStages);

		result += "\n  Programs:  ";
		result += NVTextureShader ? "NVTextureShader " : "";
		result += ATIFragmentShader ? "ATIFragmentShader " : "";
		result += ARBFragmentProgram ? "ARBFragmentProgram " : "";
		result += NVFragmentProgram2 ? "NVFragmentProgram2 " : "";
		result += ARBVertexProgram ? "ARBVertexProgram " : "";
		result += NVVertexProgram ? "NVVertexProgram " : "";
		result += EXTVertexShader ? "EXTVertexShader " : "";
		result += NVVertexProgramEmulated ? "NVVertexProgramEmulated " : "";

		result += "\n  Misc:      ";
		result += EXTVertexWeighting ? "EXTVertexWeighting " : "";
		result += EXTSeparateSpecularColor ? "EXTSeparateSpecularColor " : "";
		result += EXTSecondaryColor ? "EXTSecondaryColor " : "";
		result += EXTBlendColor ? "EXTBlendColor " : "";
		result += NVOcclusionQuery ? "NVOcclusionQuery " : "";
		result += NVStateVARWithoutFlush ? "NVStateVARWithoutFlush " : "";
		result += ARBMultisample ? "ARBMultisample " : "";

#ifdef NL_OS_WINDOWS
		result += "\n  WindowsGL: ";
		result += WGLARBPBuffer ? "WGLARBPBuffer " : "";
		result += WGLARBPixelFormat ? "WGLARBPixelFormat " : "";
		result += WGLEXTSwapControl ? "WGLEXTSwapControl " : "";
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
		result += "\n  GLX: ";
		result += GLXEXTSwapControl ? "GLXEXTSwapControl " : "";
		result += GLXSGISwapControl ? "GLXSGISwapControl " : "";
		result += GLXMESASwapControl ? "GLXMESASwapControl " : "";
#endif

		result += "\n  Array/VBO: ";
		result += NVVertexArrayRange ? ("NVVertexArrayRange (MaxVertex = " + NLMISC::toString(NVVertexArrayRangeMaxVertex) + ") ") : "";
		result += NVVertexArrayRange2 ? "NVVertexArrayRange2 " : "";
		result += ATIVertexArrayObject ? "ATIVertexArrayObject " : "";
		result += ATIVertexAttribArrayObject ? "ATIVertexAttribArrayObject " : "";
		result += ARBVertexBufferObject ? "ARBVertexBufferObject " : "";
		result += ATIMapObjectBuffer ? "ATIMapObjectBuffer " : "";

		result += "\n  FBO:       ";
		result += FrameBufferObject ? "FramebufferObject " : "";
		result += FrameBufferBlit ? "FrameBufferBlit " : "";
		result += FrameBufferMultisample ? "FrameBufferMultisample " : "";
		result += PackedDepthStencil ? "PackedDepthStencil " : "";

		return result;
	}

};

// ***************************************************************************

#ifdef USE_OPENGLES
/// This function will test and register EGL functions before than the gl context is created
bool registerEGlExtensions(CGlExtensions &ext, EGLDisplay dpy);
#elif defined(NL_OS_WINDOWS)
/// This function will test and register WGL functions before than the gl context is created
bool registerWGlExtensions(CGlExtensions &ext, HDC hDC);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
/// This function will test and register GLX functions before than the gl context is created
bool registerGlXExtensions(CGlExtensions &ext, Display *dpy, sint screen);
#endif // NL_OS_WINDOWS

/// This function test and register the extensions for the current GL context.
void registerGlExtensions(CGlExtensions &ext);

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D

// ***************************************************************************
// The exported function names
/* NB: We named all like nglActiveTextureARB (n for NEL :)
	to avoid compilation conflict with future version of gl.h
	eg: gl.h Version 1.2 define glActiveTextureARB so we can't use it.

	NB: we do it for all (EXT, NV, ARB extension) even it should be useful only for ARB ones.
*/

#ifdef USE_OPENGLES

// OES_mapbuffer.
//===============
extern NEL_PFNGLMAPBUFFEROESPROC				nglMapBufferOES;
extern NEL_PFNGLUNMAPBUFFEROESPROC				nglUnmapBufferOES;
extern NEL_PFNGLGETBUFFERPOINTERVOESPROC		nglGetBufferPointervOES;

extern NEL_PFNGLBUFFERSUBDATAPROC				nglBufferSubData;

extern PFNGLDRAWTEXFOESPROC						nglDrawTexfOES;

// GL_OES_framebuffer_object
extern NEL_PFNGLISRENDERBUFFEROESPROC			nglIsRenderbufferOES;
extern NEL_PFNGLBINDRENDERBUFFEROESPROC			nglBindRenderbufferOES;
extern NEL_PFNGLDELETERENDERBUFFERSOESPROC		nglDeleteRenderbuffersOES;
extern NEL_PFNGLGENRENDERBUFFERSOESPROC			nglGenRenderbuffersOES;
extern NEL_PFNGLRENDERBUFFERSTORAGEOESPROC		nglRenderbufferStorageOES;
extern NEL_PFNGLGETRENDERBUFFERPARAMETERIVOESPROC	nglGetRenderbufferParameterivOES;
extern NEL_PFNGLISFRAMEBUFFEROESPROC			nglIsFramebufferOES;
extern NEL_PFNGLBINDFRAMEBUFFEROESPROC			nglBindFramebufferOES;
extern NEL_PFNGLDELETEFRAMEBUFFERSOESPROC		nglDeleteFramebuffersOES;
extern NEL_PFNGLGENFRAMEBUFFERSOESPROC			nglGenFramebuffersOES;
extern NEL_PFNGLCHECKFRAMEBUFFERSTATUSOESPROC	nglCheckFramebufferStatusOES;
extern NEL_PFNGLFRAMEBUFFERRENDERBUFFEROESPROC	nglFramebufferRenderbufferOES;
extern NEL_PFNGLFRAMEBUFFERTEXTURE2DOESPROC		nglFramebufferTexture2DOES;
extern NEL_PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC	nglGetFramebufferAttachmentParameterivOES;
extern NEL_PFNGLGENERATEMIPMAPOESPROC			nglGenerateMipmapOES;

// GL_OES_texture_cube_map
extern NEL_PFNGLTEXGENFOESPROC					nglTexGenfOES;
extern NEL_PFNGLTEXGENFVOESPROC					nglTexGenfvOES;
extern NEL_PFNGLTEXGENIOESPROC					nglTexGeniOES;
extern NEL_PFNGLTEXGENIVOESPROC					nglTexGenivOES;
extern NEL_PFNGLTEXGENXOESPROC					nglTexGenxOES;
extern NEL_PFNGLTEXGENXVOESPROC					nglTexGenxvOES;
extern NEL_PFNGLGETTEXGENFVOESPROC				nglGetTexGenfvOES;
extern NEL_PFNGLGETTEXGENIVOESPROC				nglGetTexGenivOES;
extern NEL_PFNGLGETTEXGENXVOESPROC				nglGetTexGenxvOES;

#else

// ARB_multitexture
//=================
extern NEL_PFNGLACTIVETEXTUREARBPROC		nglActiveTextureARB;
extern NEL_PFNGLCLIENTACTIVETEXTUREARBPROC	nglClientActiveTextureARB;

extern NEL_PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
extern NEL_PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
extern NEL_PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern NEL_PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern NEL_PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
extern NEL_PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
extern NEL_PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
extern NEL_PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
extern NEL_PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
extern NEL_PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
extern NEL_PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
extern NEL_PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
extern NEL_PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
extern NEL_PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
extern NEL_PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
extern NEL_PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
extern NEL_PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

extern NEL_PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
extern NEL_PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
extern NEL_PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
extern NEL_PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
extern NEL_PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
extern NEL_PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
extern NEL_PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
extern NEL_PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
extern NEL_PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
extern NEL_PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
extern NEL_PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
extern NEL_PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
extern NEL_PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
extern NEL_PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
extern NEL_PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
extern NEL_PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
extern NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC		nglCompressedTexImage3DARB;
extern NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		nglCompressedTexImage2DARB;
extern NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC		nglCompressedTexImage1DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
extern NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
extern NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC		nglFlushVertexArrayRangeNV;
extern NEL_PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;

#ifdef NL_OS_WINDOWS
extern PFNWGLALLOCATEMEMORYNVPROC				nwglAllocateMemoryNV;
extern PFNWGLFREEMEMORYNVPROC					nwglFreeMemoryNV;
#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
extern NEL_PFNGLXALLOCATEMEMORYNVPROC			nglXAllocateMemoryNV;
extern NEL_PFNGLXFREEMEMORYNVPROC				nglXFreeMemoryNV;
#endif


// FenceNV.
//====================
extern NEL_PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
extern NEL_PFNGLGENFENCESNVPROC					nglGenFencesNV;
extern NEL_PFNGLISFENCENVPROC					nglIsFenceNV;
extern NEL_PFNGLTESTFENCENVPROC					nglTestFenceNV;
extern NEL_PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
extern NEL_PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
extern NEL_PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//==================
extern NEL_PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
extern NEL_PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
extern NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
extern NEL_PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
extern NEL_PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
extern NEL_PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
extern NEL_PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
extern NEL_PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
extern NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC		nglGetProgramParameterdvNV;
extern NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC		nglGetProgramParameterfvNV;
extern NEL_PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
extern NEL_PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
extern NEL_PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
extern NEL_PFNGLGETVERTEXATTRIBDVNVPROC			nglGetVertexAttribdvNV;
extern NEL_PFNGLGETVERTEXATTRIBFVNVPROC			nglGetVertexAttribfvNV;
extern NEL_PFNGLGETVERTEXATTRIBIVNVPROC			nglGetVertexAttribivNV;
extern NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
extern NEL_PFNGLISPROGRAMNVPROC					nglIsProgramNV;
extern NEL_PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
extern NEL_PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
extern NEL_PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
extern NEL_PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
extern NEL_PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
extern NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
extern NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
extern NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
extern NEL_PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
extern NEL_PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
extern NEL_PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
extern NEL_PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
extern NEL_PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
extern NEL_PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
extern NEL_PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
extern NEL_PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
extern NEL_PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
extern NEL_PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
extern NEL_PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
extern NEL_PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
extern NEL_PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
extern NEL_PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
extern NEL_PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
extern NEL_PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
extern NEL_PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
extern NEL_PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
extern NEL_PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
extern NEL_PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
extern NEL_PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
extern NEL_PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
extern NEL_PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
extern NEL_PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
extern NEL_PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
extern NEL_PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
extern NEL_PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
extern NEL_PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
extern NEL_PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
extern NEL_PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
extern NEL_PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
extern NEL_PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
extern NEL_PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
extern NEL_PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
extern NEL_PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
extern NEL_PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
extern NEL_PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
extern NEL_PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
extern NEL_PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
extern NEL_PFNGLVERTEXATTRIBS4UBVNVPROC			nglVertexAttribs4ubvNV;

// VertexShaderExtension.
//========================
extern NEL_PFNGLBEGINVERTEXSHADEREXTPROC		 nglBeginVertexShaderEXT;
extern NEL_PFNGLENDVERTEXSHADEREXTPROC			 nglEndVertexShaderEXT;
extern NEL_PFNGLBINDVERTEXSHADEREXTPROC			 nglBindVertexShaderEXT;
extern NEL_PFNGLGENVERTEXSHADERSEXTPROC			 nglGenVertexShadersEXT;
extern NEL_PFNGLDELETEVERTEXSHADEREXTPROC		 nglDeleteVertexShaderEXT;
extern NEL_PFNGLSHADEROP1EXTPROC				 nglShaderOp1EXT;
extern NEL_PFNGLSHADEROP2EXTPROC				 nglShaderOp2EXT;
extern NEL_PFNGLSHADEROP3EXTPROC				 nglShaderOp3EXT;
extern NEL_PFNGLSWIZZLEEXTPROC					 nglSwizzleEXT;
extern NEL_PFNGLWRITEMASKEXTPROC				 nglWriteMaskEXT;
extern NEL_PFNGLINSERTCOMPONENTEXTPROC			 nglInsertComponentEXT;
extern NEL_PFNGLEXTRACTCOMPONENTEXTPROC			 nglExtractComponentEXT;
extern NEL_PFNGLGENSYMBOLSEXTPROC				 nglGenSymbolsEXT;
extern NEL_PFNGLSETINVARIANTEXTPROC				 nglSetInvariantEXT;
extern NEL_PFNGLSETLOCALCONSTANTEXTPROC			 nglSetLocalConstantEXT;
extern NEL_PFNGLVARIANTPOINTEREXTPROC			 nglVariantPointerEXT;
extern NEL_PFNGLENABLEVARIANTCLIENTSTATEEXTPROC  nglEnableVariantClientStateEXT;
extern NEL_PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC nglDisableVariantClientStateEXT;
extern NEL_PFNGLBINDLIGHTPARAMETEREXTPROC		 nglBindLightParameterEXT;
extern NEL_PFNGLBINDMATERIALPARAMETEREXTPROC	 nglBindMaterialParameterEXT;
extern NEL_PFNGLBINDTEXGENPARAMETEREXTPROC		 nglBindTexGenParameterEXT;
extern NEL_PFNGLBINDTEXTUREUNITPARAMETEREXTPROC	 nglBindTextureUnitParameterEXT;
extern NEL_PFNGLBINDPARAMETEREXTPROC			 nglBindParameterEXT;
extern NEL_PFNGLISVARIANTENABLEDEXTPROC			 nglIsVariantEnabledEXT;
extern NEL_PFNGLGETVARIANTBOOLEANVEXTPROC		 nglGetVariantBooleanvEXT;
extern NEL_PFNGLGETVARIANTINTEGERVEXTPROC		 nglGetVariantIntegervEXT;
extern NEL_PFNGLGETVARIANTFLOATVEXTPROC			 nglGetVariantFloatvEXT;
extern NEL_PFNGLGETVARIANTPOINTERVEXTPROC		 nglGetVariantPointervEXT;
extern NEL_PFNGLGETINVARIANTBOOLEANVEXTPROC		 nglGetInvariantBooleanvEXT;
extern NEL_PFNGLGETINVARIANTINTEGERVEXTPROC		 nglGetInvariantIntegervEXT;
extern NEL_PFNGLGETINVARIANTFLOATVEXTPROC		 nglGetInvariantFloatvEXT;
extern NEL_PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC  nglGetLocalConstantBooleanvEXT;
extern NEL_PFNGLGETLOCALCONSTANTINTEGERVEXTPROC  nglGetLocalConstantIntegervEXT;
extern NEL_PFNGLGETLOCALCONSTANTFLOATVEXTPROC    nglGetLocalConstantFloatvEXT;


// ATI_envmap_bumpmap extension

extern  PFNGLTEXBUMPPARAMETERIVATIPROC nglTexBumpParameterivATI;
extern  PFNGLTEXBUMPPARAMETERFVATIPROC nglTexBumpParameterfvATI;
extern  PFNGLGETTEXBUMPPARAMETERIVATIPROC nglGetTexBumpParameterivATI;
extern  PFNGLGETTEXBUMPPARAMETERFVATIPROC nglGetTexBumpParameterfvATI;


// SecondaryColor extension
//========================
extern NEL_PFNGLSECONDARYCOLOR3BEXTPROC			nglSecondaryColor3bEXT;
extern NEL_PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
extern NEL_PFNGLSECONDARYCOLOR3DEXTPROC			nglSecondaryColor3dEXT;
extern NEL_PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
extern NEL_PFNGLSECONDARYCOLOR3FEXTPROC			nglSecondaryColor3fEXT;
extern NEL_PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
extern NEL_PFNGLSECONDARYCOLOR3IEXTPROC			nglSecondaryColor3iEXT;
extern NEL_PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
extern NEL_PFNGLSECONDARYCOLOR3SEXTPROC			nglSecondaryColor3sEXT;
extern NEL_PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
extern NEL_PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
extern NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
extern NEL_PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
extern NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
extern NEL_PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
extern NEL_PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
extern NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


// BlendColor extension
//========================
extern NEL_PFNGLBLENDCOLOREXTPROC				nglBlendColorEXT;


// GL_ATI_vertex_array_object extension
//========================
extern NEL_PFNGLNEWOBJECTBUFFERATIPROC			nglNewObjectBufferATI;
extern NEL_PFNGLISOBJECTBUFFERATIPROC			nglIsObjectBufferATI;
extern NEL_PFNGLUPDATEOBJECTBUFFERATIPROC		nglUpdateObjectBufferATI;
extern NEL_PFNGLGETOBJECTBUFFERFVATIPROC		nglGetObjectBufferfvATI;
extern NEL_PFNGLGETOBJECTBUFFERIVATIPROC		nglGetObjectBufferivATI;
extern NEL_PFNGLDELETEOBJECTBUFFERATIPROC		nglDeleteObjectBufferATI;
extern NEL_PFNGLARRAYOBJECTATIPROC				nglArrayObjectATI;
extern NEL_PFNGLGETARRAYOBJECTFVATIPROC			nglGetArrayObjectfvATI;
extern NEL_PFNGLGETARRAYOBJECTIVATIPROC			nglGetArrayObjectivATI;
extern NEL_PFNGLVARIANTARRAYOBJECTATIPROC		nglVariantArrayObjectATI;
extern NEL_PFNGLGETVARIANTARRAYOBJECTFVATIPROC	nglGetVariantArrayObjectfvATI;
extern NEL_PFNGLGETVARIANTARRAYOBJECTIVATIPROC	nglGetVariantArrayObjectivATI;

// GL_ATI_map_object_buffer
//===================================

extern NEL_PFNGLMAPOBJECTBUFFERATIPROC			nglMapObjectBufferATI;
extern NEL_PFNGLUNMAPOBJECTBUFFERATIPROC		nglUnmapObjectBufferATI;


// GL_ATI_fragment_shader extension
//===================================

extern NEL_PFNGLGENFRAGMENTSHADERSATIPROC			nglGenFragmentShadersATI;
extern NEL_PFNGLBINDFRAGMENTSHADERATIPROC			nglBindFragmentShaderATI;
extern NEL_PFNGLDELETEFRAGMENTSHADERATIPROC			nglDeleteFragmentShaderATI;
extern NEL_PFNGLBEGINFRAGMENTSHADERATIPROC			nglBeginFragmentShaderATI;
extern NEL_PFNGLENDFRAGMENTSHADERATIPROC			nglEndFragmentShaderATI;
extern NEL_PFNGLPASSTEXCOORDATIPROC					nglPassTexCoordATI;
extern NEL_PFNGLSAMPLEMAPATIPROC					nglSampleMapATI;
extern NEL_PFNGLCOLORFRAGMENTOP1ATIPROC				nglColorFragmentOp1ATI;
extern NEL_PFNGLCOLORFRAGMENTOP2ATIPROC				nglColorFragmentOp2ATI;
extern NEL_PFNGLCOLORFRAGMENTOP3ATIPROC				nglColorFragmentOp3ATI;
extern NEL_PFNGLALPHAFRAGMENTOP1ATIPROC				nglAlphaFragmentOp1ATI;
extern NEL_PFNGLALPHAFRAGMENTOP2ATIPROC				nglAlphaFragmentOp2ATI;
extern NEL_PFNGLALPHAFRAGMENTOP3ATIPROC				nglAlphaFragmentOp3ATI;
extern NEL_PFNGLSETFRAGMENTSHADERCONSTANTATIPROC	nglSetFragmentShaderConstantATI;

// GL_ATI_vertex_attrib_array_object
//==================================
extern NEL_PFNGLVERTEXATTRIBARRAYOBJECTATIPROC nglVertexAttribArrayObjectATI;
extern NEL_PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC nglGetVertexAttribArrayObjectfvATI;
extern NEL_PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC nglGetVertexAttribArrayObjectivATI;




// GL_ARB_fragment_shader_extension
//==================================
extern NEL_PFNGLPROGRAMSTRINGARBPROC nglProgramStringARB;
extern NEL_PFNGLBINDPROGRAMARBPROC nglBindProgramARB;
extern NEL_PFNGLDELETEPROGRAMSARBPROC nglDeleteProgramsARB;
extern NEL_PFNGLGENPROGRAMSARBPROC nglGenProgramsARB;
extern NEL_PFNGLPROGRAMENVPARAMETER4DARBPROC nglProgramEnvParameter4dARB;
extern NEL_PFNGLPROGRAMENVPARAMETER4DVARBPROC nglProgramEnvParameter4dvARB;
extern NEL_PFNGLPROGRAMENVPARAMETER4FARBPROC nglProgramEnvParameter4fARB;
extern NEL_PFNGLPROGRAMENVPARAMETER4FVARBPROC nglProgramEnvParameter4fvARB;
extern NEL_PFNGLPROGRAMLOCALPARAMETER4DARBPROC nglGetProgramLocalParameter4dARB;
extern NEL_PFNGLPROGRAMLOCALPARAMETER4DVARBPROC nglGetProgramLocalParameter4dvARB;
extern NEL_PFNGLPROGRAMLOCALPARAMETER4FARBPROC nglGetProgramLocalParameter4fARB;
extern NEL_PFNGLPROGRAMLOCALPARAMETER4FVARBPROC nglGetProgramLocalParameter4fvARB;
extern NEL_PFNGLGETPROGRAMENVPARAMETERDVARBPROC nglGetProgramEnvParameterdvARB;
extern NEL_PFNGLGETPROGRAMENVPARAMETERFVARBPROC nglGetProgramEnvParameterfvARB;
extern NEL_PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC nglGetProgramLocalParameterdvARB;
extern NEL_PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC nglGetProgramLocalParameterfvARB;
extern NEL_PFNGLGETPROGRAMIVARBPROC nglGetProgramivARB;
extern NEL_PFNGLGETPROGRAMSTRINGARBPROC nglGetProgramStringARB;
extern NEL_PFNGLISPROGRAMARBPROC nglIsProgramARB;

// GL_ARB_vertex_buffer_object
//==================================
extern PFNGLBINDBUFFERARBPROC nglBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC nglDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC nglGenBuffersARB;
extern PFNGLISBUFFERARBPROC nglIsBufferARB;
extern PFNGLBUFFERDATAARBPROC nglBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC nglBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC nglGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC nglMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC nglUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC nglGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC nglGetBufferPointervARB;



// GL_ARB_vertex_program
//==================================
extern PFNGLVERTEXATTRIB1SARBPROC nglVertexAttrib1sARB;
extern PFNGLVERTEXATTRIB1FARBPROC nglVertexAttrib1fARB;
extern PFNGLVERTEXATTRIB1DARBPROC nglVertexAttrib1dARB;
extern PFNGLVERTEXATTRIB2SARBPROC nglVertexAttrib2sARB;
extern PFNGLVERTEXATTRIB2FARBPROC nglVertexAttrib2fARB;
extern PFNGLVERTEXATTRIB2DARBPROC nglVertexAttrib2dARB;
extern PFNGLVERTEXATTRIB3SARBPROC nglVertexAttrib3sARB;
extern PFNGLVERTEXATTRIB3FARBPROC nglVertexAttrib3fARB;
extern PFNGLVERTEXATTRIB3DARBPROC nglVertexAttrib3dARB;
extern PFNGLVERTEXATTRIB4SARBPROC nglVertexAttrib4sARB;
extern PFNGLVERTEXATTRIB4FARBPROC nglVertexAttrib4fARB;
extern PFNGLVERTEXATTRIB4DARBPROC nglVertexAttrib4dARB;
extern PFNGLVERTEXATTRIB4NUBARBPROC nglVertexAttrib4NubARB;
extern PFNGLVERTEXATTRIB1SVARBPROC nglVertexAttrib1svARB;
extern PFNGLVERTEXATTRIB1FVARBPROC nglVertexAttrib1fvARB;
extern PFNGLVERTEXATTRIB1DVARBPROC nglVertexAttrib1dvARB;
extern PFNGLVERTEXATTRIB2SVARBPROC nglVertexAttrib2svARB;
extern PFNGLVERTEXATTRIB2FVARBPROC nglVertexAttrib2fvARB;
extern PFNGLVERTEXATTRIB2DVARBPROC nglVertexAttrib2dvARB;
extern PFNGLVERTEXATTRIB3SVARBPROC nglVertexAttrib3svARB;
extern PFNGLVERTEXATTRIB3FVARBPROC nglVertexAttrib3fvARB;
extern PFNGLVERTEXATTRIB3DVARBPROC nglVertexAttrib3dvARB;
extern PFNGLVERTEXATTRIB4BVARBPROC nglVertexAttrib4bvARB;
extern PFNGLVERTEXATTRIB4SVARBPROC nglVertexAttrib4svARB;
extern PFNGLVERTEXATTRIB4IVARBPROC nglVertexAttrib4ivARB;
extern PFNGLVERTEXATTRIB4UBVARBPROC nglVertexAttrib4ubvARB;
extern PFNGLVERTEXATTRIB4USVARBPROC nglVertexAttrib4usvARB;
extern PFNGLVERTEXATTRIB4UIVARBPROC nglVertexAttrib4uivARB;
extern PFNGLVERTEXATTRIB4FVARBPROC nglVertexAttrib4fvARB;
extern PFNGLVERTEXATTRIB4DVARBPROC nglVertexAttrib4dvARB;
extern PFNGLVERTEXATTRIB4NBVARBPROC nglVertexAttrib4NbvARB;
extern PFNGLVERTEXATTRIB4NSVARBPROC nglVertexAttrib4NsvARB;
extern PFNGLVERTEXATTRIB4NIVARBPROC nglVertexAttrib4NivARB;
extern PFNGLVERTEXATTRIB4NUBVARBPROC nglVertexAttrib4NubvARB;
extern PFNGLVERTEXATTRIB4NUSVARBPROC nglVertexAttrib4NusvARB;
extern PFNGLVERTEXATTRIB4NUIVARBPROC nglVertexAttrib4NuivARB;
extern PFNGLVERTEXATTRIBPOINTERARBPROC nglVertexAttribPointerARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC nglEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC nglDisableVertexAttribArrayARB;
extern PFNGLPROGRAMSTRINGARBPROC nglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC nglBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC nglDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC nglGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC nglProgramEnvParameter4fARB;
extern PFNGLPROGRAMENVPARAMETER4DARBPROC nglProgramEnvParameter4dARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC nglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC nglProgramEnvParameter4dvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC nglProgramLocalParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC nglProgramLocalParameter4dARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC nglProgramLocalParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC nglProgramLocalParameter4dvARB;
extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC nglGetProgramEnvParameterfvARB;
extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC nglGetProgramEnvParameterdvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC nglGetProgramLocalParameterfvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC nglGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMIVARBPROC nglGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC nglGetProgramStringARB;
extern PFNGLGETVERTEXATTRIBDVARBPROC nglGetVertexAttribdvARB;
extern PFNGLGETVERTEXATTRIBFVARBPROC nglGetVertexAttribfvARB;
extern PFNGLGETVERTEXATTRIBIVARBPROC nglGetVertexAttribivARB;
extern PFNGLGETVERTEXATTRIBPOINTERVARBPROC nglGetVertexAttribPointervARB;
extern PFNGLISPROGRAMARBPROC nglIsProgramARB;

// GL_NV_occlusion_query
//==================================
extern NEL_PFNGLGENOCCLUSIONQUERIESNVPROC nglGenOcclusionQueriesNV;
extern NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC nglDeleteOcclusionQueriesNV;
extern NEL_PFNGLISOCCLUSIONQUERYNVPROC nglIsOcclusionQueryNV;
extern NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC nglBeginOcclusionQueryNV;
extern NEL_PFNGLENDOCCLUSIONQUERYNVPROC nglEndOcclusionQueryNV;
extern NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC nglGetOcclusionQueryivNV;
extern NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC nglGetOcclusionQueryuivNV;



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
extern PFNWGLGETEXTENSIONSSTRINGARBPROC			nwglGetExtensionsStringARB;

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

// Swap control extensions
//===========================
extern NEL_PFNGLXSWAPINTERVALEXTPROC			nglXSwapIntervalEXT;

extern PFNGLXSWAPINTERVALSGIPROC				nglXSwapIntervalSGI;

extern NEL_PFNGLXSWAPINTERVALMESAPROC			nglXSwapIntervalMESA;
extern NEL_PFNGLXGETSWAPINTERVALMESAPROC		nglXGetSwapIntervalMESA;

#endif

// GL_EXT_framebuffer_object
extern NEL_PFNGLISRENDERBUFFEREXTPROC			nglIsRenderbufferEXT;
extern NEL_PFNGLISFRAMEBUFFEREXTPROC			nglIsFramebufferEXT;
extern NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	nglCheckFramebufferStatusEXT;
extern NEL_PFNGLGENFRAMEBUFFERSEXTPROC			nglGenFramebuffersEXT;
extern NEL_PFNGLBINDFRAMEBUFFEREXTPROC			nglBindFramebufferEXT;
extern NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC		nglFramebufferTexture2DEXT;
extern NEL_PFNGLGENRENDERBUFFERSEXTPROC			nglGenRenderbuffersEXT;
extern NEL_PFNGLBINDRENDERBUFFEREXTPROC			nglBindRenderbufferEXT;
extern NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC		nglRenderbufferStorageEXT;
extern NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	nglFramebufferRenderbufferEXT;
extern NEL_PFNGLDELETERENDERBUFFERSEXTPROC		nglDeleteRenderbuffersEXT;
extern NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC		nglDeleteFramebuffersEXT;
extern NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC	nglGetRenderbufferParameterivEXT;
extern NEL_PFNGENERATEMIPMAPEXTPROC				nglGenerateMipmapEXT;

// GL_EXT_framebuffer_blit
extern NEL_PFNGLBLITFRAMEBUFFEREXTPROC			nglBlitFramebufferEXT;

// GL_EXT_framebuffer_multisample
extern NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		nglRenderbufferStorageMultisampleEXT;

// GL_ARB_multisample
extern NEL_PFNGLSAMPLECOVERAGEARBPROC			nglSampleCoverageARB;

#endif // USE_OPENGLES

#endif // NL_OPENGL_EXTENSION_H

