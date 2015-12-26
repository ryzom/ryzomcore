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
	bool	ARBOcclusionQuery;
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
	bool	GLXMESAQueryRenderer;

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
	bool	ARBMapBufferRange;
	bool	ARBVertexProgram;
	bool	ARBTextureNonPowerOfTwo;
	bool	ARBMultisample;
	bool	ARBFragmentShader;

	// NV Pixel Programs
	bool	NVFragmentProgram2;

	bool	OESDrawTexture;
	bool	OESMapBuffer;

	// extensions to get memory info

	// GL_NVX_gpu_memory_info
	bool	NVXGPUMemoryInfo;

	// GL_ATI_meminfo
	bool	ATIMeminfo;

	// WGL_AMD_gpu_association
	bool	WGLAMDGPUAssociation;

	// WGL_NV_gpu_affinity
	bool	WGLNVGPUAffinity;

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
		GLXMESAQueryRenderer= false;
		EXTBlendColor= false;
		ATIVertexArrayObject= false;
		ATIEnvMapBumpMap = false;
		ATIFragmentShader = false;
		ATIMapObjectBuffer = false;
		ATIVertexAttribArrayObject = false;
		EXTVertexShader= false;
		ARBFragmentProgram = false;
		ARBVertexBufferObject = false;
		ARBMapBufferRange = false;
		ARBVertexProgram = false;
		NVTextureRectangle = false;
		EXTTextureRectangle = false;
		EXTTextureFilterAnisotropic = false;
		EXTTextureFilterAnisotropicMaximum = 0.f;
		ARBTextureRectangle = false;
		ARBTextureNonPowerOfTwo = false;
		ARBMultisample = false;
		ARBFragmentShader = false;
		NVOcclusionQuery = false;
		ARBOcclusionQuery = false;
		FrameBufferObject = false;
		FrameBufferBlit = false;
		FrameBufferMultisample = false;
		PackedDepthStencil = false;
		NVVertexArrayRange2 = false;
		NVStateVARWithoutFlush = 0;

		OESDrawTexture = false;
		OESMapBuffer = false;

		NVXGPUMemoryInfo = false;
		ATIMeminfo = false;
		WGLAMDGPUAssociation = false;

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
		result += ARBOcclusionQuery ? "ARBOcclusionQuery " : "";
		result += NVStateVARWithoutFlush ? "NVStateVARWithoutFlush " : "";
		result += ARBMultisample ? "ARBMultisample " : "";
		result += NVXGPUMemoryInfo ? "NVXGPUMemoryInfo " : "";
		result += ATIMeminfo ? "ATIMeminfo " : "";

#ifdef NL_OS_WINDOWS
		result += "\n  WindowsGL: ";
		result += WGLARBPBuffer ? "WGLARBPBuffer " : "";
		result += WGLARBPixelFormat ? "WGLARBPixelFormat " : "";
		result += WGLEXTSwapControl ? "WGLEXTSwapControl " : "";
		result += WGLAMDGPUAssociation ? "WGLAMDGPUAssociation " : "";
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
		result += "\n  GLX: ";
		result += GLXEXTSwapControl ? "GLXEXTSwapControl " : "";
		result += GLXSGISwapControl ? "GLXSGISwapControl " : "";
		result += GLXMESASwapControl ? "GLXMESASwapControl " : "";
		result += GLXMESAQueryRenderer ? "GLXMESAQueryRenderer " : "";
#endif

		result += "\n  Array/VBO: ";
		result += NVVertexArrayRange ? ("NVVertexArrayRange (MaxVertex = " + NLMISC::toString(NVVertexArrayRangeMaxVertex) + ") ") : "";
		result += NVVertexArrayRange2 ? "NVVertexArrayRange2 " : "";
		result += ATIVertexArrayObject ? "ATIVertexArrayObject " : "";
		result += ATIVertexAttribArrayObject ? "ATIVertexAttribArrayObject " : "";
		result += ARBVertexBufferObject ? "ARBVertexBufferObject " : "";
		result += ARBMapBufferRange ? "ARBMapBufferRange " : "";
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
extern PFNGLMAPBUFFEROESPROC				nglMapBufferOES;
extern PFNGLUNMAPBUFFEROESPROC				nglUnmapBufferOES;
extern PFNGLGETBUFFERPOINTERVOESPROC		nglGetBufferPointervOES;

extern PFNGLDRAWTEXFOESPROC					nglDrawTexfOES;

// GL_OES_framebuffer_object
extern PFNGLISRENDERBUFFEROESPROC			nglIsRenderbufferOES;
extern PFNGLBINDRENDERBUFFEROESPROC			nglBindRenderbufferOES;
extern PFNGLDELETERENDERBUFFERSOESPROC		nglDeleteRenderbuffersOES;
extern PFNGLGENRENDERBUFFERSOESPROC			nglGenRenderbuffersOES;
extern PFNGLRENDERBUFFERSTORAGEOESPROC		nglRenderbufferStorageOES;
extern PFNGLGETRENDERBUFFERPARAMETERIVOESPROC	nglGetRenderbufferParameterivOES;
extern PFNGLISFRAMEBUFFEROESPROC			nglIsFramebufferOES;
extern PFNGLBINDFRAMEBUFFEROESPROC			nglBindFramebufferOES;
extern PFNGLDELETEFRAMEBUFFERSOESPROC		nglDeleteFramebuffersOES;
extern PFNGLGENFRAMEBUFFERSOESPROC			nglGenFramebuffersOES;
extern PFNGLCHECKFRAMEBUFFERSTATUSOESPROC	nglCheckFramebufferStatusOES;
extern PFNGLFRAMEBUFFERRENDERBUFFEROESPROC	nglFramebufferRenderbufferOES;
extern PFNGLFRAMEBUFFERTEXTURE2DOESPROC		nglFramebufferTexture2DOES;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC	nglGetFramebufferAttachmentParameterivOES;
extern PFNGLGENERATEMIPMAPOESPROC			nglGenerateMipmapOES;

// GL_OES_texture_cube_map
extern PFNGLTEXGENFOESPROC					nglTexGenfOES;
extern PFNGLTEXGENFVOESPROC					nglTexGenfvOES;
extern PFNGLTEXGENIOESPROC					nglTexGeniOES;
extern PFNGLTEXGENIVOESPROC					nglTexGenivOES;
extern PFNGLTEXGENXOESPROC					nglTexGenxOES;
extern PFNGLTEXGENXVOESPROC					nglTexGenxvOES;
extern PFNGLGETTEXGENFVOESPROC				nglGetTexGenfvOES;
extern PFNGLGETTEXGENIVOESPROC				nglGetTexGenivOES;
extern PFNGLGETTEXGENXVOESPROC				nglGetTexGenxvOES;

#else

// ARB_multitexture
//=================
extern PFNGLACTIVETEXTUREARBPROC		nglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC	nglClientActiveTextureARB;

extern PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

extern PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
extern PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC		nglCompressedTexImage3DARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		nglCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC		nglCompressedTexImage1DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
extern PFNGLFLUSHVERTEXARRAYRANGENVPROC		nglFlushVertexArrayRangeNV;
extern PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;

#ifdef NL_OS_WINDOWS
extern PFNWGLALLOCATEMEMORYNVPROC				nwglAllocateMemoryNV;
extern PFNWGLFREEMEMORYNVPROC					nwglFreeMemoryNV;
#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
extern PFNGLXALLOCATEMEMORYNVPROC			nglXAllocateMemoryNV;
extern PFNGLXFREEMEMORYNVPROC				nglXFreeMemoryNV;
#endif


// FenceNV.
//====================
extern PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
extern PFNGLGENFENCESNVPROC					nglGenFencesNV;
extern PFNGLISFENCENVPROC					nglIsFenceNV;
extern PFNGLTESTFENCENVPROC					nglTestFenceNV;
extern PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
extern PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
extern PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//==================
extern PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
extern PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
extern PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
extern PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
extern PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
extern PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
extern PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
extern PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
extern PFNGLGETPROGRAMPARAMETERDVNVPROC		nglGetProgramParameterdvNV;
extern PFNGLGETPROGRAMPARAMETERFVNVPROC		nglGetProgramParameterfvNV;
extern PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
extern PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
extern PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
extern PFNGLGETVERTEXATTRIBDVNVPROC			nglGetVertexAttribdvNV;
extern PFNGLGETVERTEXATTRIBFVNVPROC			nglGetVertexAttribfvNV;
extern PFNGLGETVERTEXATTRIBIVNVPROC			nglGetVertexAttribivNV;
extern PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
extern PFNGLISPROGRAMNVPROC					nglIsProgramNV;
extern PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
extern PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
extern PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
extern PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
extern PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
extern PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
extern PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
extern PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
extern PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
extern PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
extern PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
extern PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
extern PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
extern PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
extern PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
extern PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
extern PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
extern PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
extern PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
extern PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
extern PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
extern PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
extern PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
extern PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
extern PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
extern PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
extern PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
extern PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
extern PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
extern PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
extern PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
extern PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
extern PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
extern PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
extern PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
extern PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
extern PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
extern PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
extern PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
extern PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
extern PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
extern PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
extern PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
extern PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
extern PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
extern PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
extern PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
extern PFNGLVERTEXATTRIBS4UBVNVPROC			nglVertexAttribs4ubvNV;

// VertexShaderExtension.
//========================
extern PFNGLBEGINVERTEXSHADEREXTPROC		 nglBeginVertexShaderEXT;
extern PFNGLENDVERTEXSHADEREXTPROC			 nglEndVertexShaderEXT;
extern PFNGLBINDVERTEXSHADEREXTPROC			 nglBindVertexShaderEXT;
extern PFNGLGENVERTEXSHADERSEXTPROC			 nglGenVertexShadersEXT;
extern PFNGLDELETEVERTEXSHADEREXTPROC		 nglDeleteVertexShaderEXT;
extern PFNGLSHADEROP1EXTPROC				 nglShaderOp1EXT;
extern PFNGLSHADEROP2EXTPROC				 nglShaderOp2EXT;
extern PFNGLSHADEROP3EXTPROC				 nglShaderOp3EXT;
extern PFNGLSWIZZLEEXTPROC					 nglSwizzleEXT;
extern PFNGLWRITEMASKEXTPROC				 nglWriteMaskEXT;
extern PFNGLINSERTCOMPONENTEXTPROC			 nglInsertComponentEXT;
extern PFNGLEXTRACTCOMPONENTEXTPROC			 nglExtractComponentEXT;
extern PFNGLGENSYMBOLSEXTPROC				 nglGenSymbolsEXT;
extern PFNGLSETINVARIANTEXTPROC				 nglSetInvariantEXT;
extern PFNGLSETLOCALCONSTANTEXTPROC			 nglSetLocalConstantEXT;
extern PFNGLVARIANTPOINTEREXTPROC			 nglVariantPointerEXT;
extern PFNGLENABLEVARIANTCLIENTSTATEEXTPROC  nglEnableVariantClientStateEXT;
extern PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC nglDisableVariantClientStateEXT;
extern PFNGLBINDLIGHTPARAMETEREXTPROC		 nglBindLightParameterEXT;
extern PFNGLBINDMATERIALPARAMETEREXTPROC	 nglBindMaterialParameterEXT;
extern PFNGLBINDTEXGENPARAMETEREXTPROC		 nglBindTexGenParameterEXT;
extern PFNGLBINDTEXTUREUNITPARAMETEREXTPROC	 nglBindTextureUnitParameterEXT;
extern PFNGLBINDPARAMETEREXTPROC			 nglBindParameterEXT;
extern PFNGLISVARIANTENABLEDEXTPROC			 nglIsVariantEnabledEXT;
extern PFNGLGETVARIANTBOOLEANVEXTPROC		 nglGetVariantBooleanvEXT;
extern PFNGLGETVARIANTINTEGERVEXTPROC		 nglGetVariantIntegervEXT;
extern PFNGLGETVARIANTFLOATVEXTPROC			 nglGetVariantFloatvEXT;
extern PFNGLGETVARIANTPOINTERVEXTPROC		 nglGetVariantPointervEXT;
extern PFNGLGETINVARIANTBOOLEANVEXTPROC		 nglGetInvariantBooleanvEXT;
extern PFNGLGETINVARIANTINTEGERVEXTPROC		 nglGetInvariantIntegervEXT;
extern PFNGLGETINVARIANTFLOATVEXTPROC		 nglGetInvariantFloatvEXT;
extern PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC  nglGetLocalConstantBooleanvEXT;
extern PFNGLGETLOCALCONSTANTINTEGERVEXTPROC  nglGetLocalConstantIntegervEXT;
extern PFNGLGETLOCALCONSTANTFLOATVEXTPROC    nglGetLocalConstantFloatvEXT;


// ATI_envmap_bumpmap extension

extern  PFNGLTEXBUMPPARAMETERIVATIPROC nglTexBumpParameterivATI;
extern  PFNGLTEXBUMPPARAMETERFVATIPROC nglTexBumpParameterfvATI;
extern  PFNGLGETTEXBUMPPARAMETERIVATIPROC nglGetTexBumpParameterivATI;
extern  PFNGLGETTEXBUMPPARAMETERFVATIPROC nglGetTexBumpParameterfvATI;


// SecondaryColor extension
//========================
extern PFNGLSECONDARYCOLOR3BEXTPROC			nglSecondaryColor3bEXT;
extern PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
extern PFNGLSECONDARYCOLOR3DEXTPROC			nglSecondaryColor3dEXT;
extern PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
extern PFNGLSECONDARYCOLOR3FEXTPROC			nglSecondaryColor3fEXT;
extern PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
extern PFNGLSECONDARYCOLOR3IEXTPROC			nglSecondaryColor3iEXT;
extern PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
extern PFNGLSECONDARYCOLOR3SEXTPROC			nglSecondaryColor3sEXT;
extern PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
extern PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
extern PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
extern PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
extern PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
extern PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
extern PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
extern PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


// BlendColor extension
//========================
extern PFNGLBLENDCOLOREXTPROC				nglBlendColorEXT;


// GL_ATI_vertex_array_object extension
//========================
extern PFNGLNEWOBJECTBUFFERATIPROC			nglNewObjectBufferATI;
extern PFNGLISOBJECTBUFFERATIPROC			nglIsObjectBufferATI;
extern PFNGLUPDATEOBJECTBUFFERATIPROC		nglUpdateObjectBufferATI;
extern PFNGLGETOBJECTBUFFERFVATIPROC		nglGetObjectBufferfvATI;
extern PFNGLGETOBJECTBUFFERIVATIPROC		nglGetObjectBufferivATI;
extern PFNGLFREEOBJECTBUFFERATIPROC			nglFreeObjectBufferATI;
extern PFNGLARRAYOBJECTATIPROC				nglArrayObjectATI;
extern PFNGLGETARRAYOBJECTFVATIPROC			nglGetArrayObjectfvATI;
extern PFNGLGETARRAYOBJECTIVATIPROC			nglGetArrayObjectivATI;
extern PFNGLVARIANTARRAYOBJECTATIPROC		nglVariantArrayObjectATI;
extern PFNGLGETVARIANTARRAYOBJECTFVATIPROC	nglGetVariantArrayObjectfvATI;
extern PFNGLGETVARIANTARRAYOBJECTIVATIPROC	nglGetVariantArrayObjectivATI;

// GL_ATI_map_object_buffer
//===================================

extern PFNGLMAPOBJECTBUFFERATIPROC			nglMapObjectBufferATI;
extern PFNGLUNMAPOBJECTBUFFERATIPROC		nglUnmapObjectBufferATI;


// GL_ATI_fragment_shader extension
//===================================

extern PFNGLGENFRAGMENTSHADERSATIPROC			nglGenFragmentShadersATI;
extern PFNGLBINDFRAGMENTSHADERATIPROC			nglBindFragmentShaderATI;
extern PFNGLDELETEFRAGMENTSHADERATIPROC			nglDeleteFragmentShaderATI;
extern PFNGLBEGINFRAGMENTSHADERATIPROC			nglBeginFragmentShaderATI;
extern PFNGLENDFRAGMENTSHADERATIPROC			nglEndFragmentShaderATI;
extern PFNGLPASSTEXCOORDATIPROC					nglPassTexCoordATI;
extern PFNGLSAMPLEMAPATIPROC					nglSampleMapATI;
extern PFNGLCOLORFRAGMENTOP1ATIPROC				nglColorFragmentOp1ATI;
extern PFNGLCOLORFRAGMENTOP2ATIPROC				nglColorFragmentOp2ATI;
extern PFNGLCOLORFRAGMENTOP3ATIPROC				nglColorFragmentOp3ATI;
extern PFNGLALPHAFRAGMENTOP1ATIPROC				nglAlphaFragmentOp1ATI;
extern PFNGLALPHAFRAGMENTOP2ATIPROC				nglAlphaFragmentOp2ATI;
extern PFNGLALPHAFRAGMENTOP3ATIPROC				nglAlphaFragmentOp3ATI;
extern PFNGLSETFRAGMENTSHADERCONSTANTATIPROC	nglSetFragmentShaderConstantATI;

// GL_ATI_vertex_attrib_array_object
//==================================
extern PFNGLVERTEXATTRIBARRAYOBJECTATIPROC		nglVertexAttribArrayObjectATI;
extern PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC	nglGetVertexAttribArrayObjectfvATI;
extern PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC	nglGetVertexAttribArrayObjectivATI;




// GL_ARB_fragment_shader_extension
//==================================
extern PFNGLPROGRAMSTRINGARBPROC nglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC nglBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC nglDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC nglGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4DARBPROC nglProgramEnvParameter4dARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC nglProgramEnvParameter4dvARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC nglProgramEnvParameter4fARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC nglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC nglGetProgramLocalParameter4dARB;
extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC nglGetProgramLocalParameter4dvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC nglGetProgramLocalParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC nglGetProgramLocalParameter4fvARB;
extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC nglGetProgramEnvParameterdvARB;
extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC nglGetProgramEnvParameterfvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC nglGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC nglGetProgramLocalParameterfvARB;
extern PFNGLGETPROGRAMIVARBPROC nglGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC nglGetProgramStringARB;
extern PFNGLISPROGRAMARBPROC nglIsProgramARB;

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

// GL_ARB_map_buffer_range
//==================================
extern PFNGLMAPBUFFERRANGEPROC nglMapBufferRange;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC nglFlushMappedBufferRange;

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
extern PFNGLGENOCCLUSIONQUERIESNVPROC nglGenOcclusionQueriesNV;
extern PFNGLDELETEOCCLUSIONQUERIESNVPROC nglDeleteOcclusionQueriesNV;
extern PFNGLISOCCLUSIONQUERYNVPROC nglIsOcclusionQueryNV;
extern PFNGLBEGINOCCLUSIONQUERYNVPROC nglBeginOcclusionQueryNV;
extern PFNGLENDOCCLUSIONQUERYNVPROC nglEndOcclusionQueryNV;
extern PFNGLGETOCCLUSIONQUERYIVNVPROC nglGetOcclusionQueryivNV;
extern PFNGLGETOCCLUSIONQUERYUIVNVPROC nglGetOcclusionQueryuivNV;

// ARB_occlusion_query
//==================================
extern PFNGLGENQUERIESARBPROC nglGenQueriesARB;
extern PFNGLDELETEQUERIESARBPROC nglDeleteQueriesARB;
extern PFNGLISQUERYARBPROC nglIsQueryARB;
extern PFNGLBEGINQUERYARBPROC nglBeginQueryARB;
extern PFNGLENDQUERYARBPROC nglEndQueryARB;
extern PFNGLGETQUERYIVARBPROC nglGetQueryivARB;
extern PFNGLGETQUERYOBJECTIVARBPROC nglGetQueryObjectivARB;
extern PFNGLGETQUERYOBJECTUIVARBPROC nglGetQueryObjectuivARB;

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


// WGL_AMD_gpu_association
//========================
extern PFNWGLGETGPUIDSAMDPROC						nwglGetGPUIDsAMD;
extern PFNWGLGETGPUINFOAMDPROC						nwglGetGPUInfoAMD;
extern PFNWGLGETCONTEXTGPUIDAMDPROC					nwglGetContextGPUIDAMD;
extern PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC			nwglCreateAssociatedContextAMD;
extern PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC	nwglCreateAssociatedContextAttribsAMD;
extern PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC			nwglDeleteAssociatedContextAMD;
extern PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC	nwglMakeAssociatedContextCurrentAMD;
extern PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC		nwglGetCurrentAssociatedContextAMD;
extern PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC			nwglBlitContextFramebufferAMD;

// WGL_NV_gpu_affinity
//====================
extern PFNWGLENUMGPUSNVPROC							nwglEnumGpusNV;
extern PFNWGLENUMGPUDEVICESNVPROC					nwglEnumGpuDevicesNV;
extern PFNWGLCREATEAFFINITYDCNVPROC					nwglCreateAffinityDCNV;
extern PFNWGLENUMGPUSFROMAFFINITYDCNVPROC			nwglEnumGpusFromAffinityDCNV;
extern PFNWGLDELETEDCNVPROC							nwglDeleteDCNV;

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

// Swap control extensions
//===========================
extern PFNGLXSWAPINTERVALEXTPROC			nglXSwapIntervalEXT;

extern PFNGLXSWAPINTERVALSGIPROC			nglXSwapIntervalSGI;

extern PFNGLXSWAPINTERVALMESAPROC			nglXSwapIntervalMESA;
extern PFNGLXGETSWAPINTERVALMESAPROC		nglXGetSwapIntervalMESA;

// GLX_MESA_query_renderer
// =======================
extern PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC	nglXQueryCurrentRendererIntegerMESA;

#endif

// GL_EXT_framebuffer_object
extern PFNGLISRENDERBUFFEREXTPROC				nglIsRenderbufferEXT;
extern PFNGLISFRAMEBUFFEREXTPROC				nglIsFramebufferEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC		nglCheckFramebufferStatusEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC				nglGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC				nglBindFramebufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC			nglFramebufferTexture2DEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC				nglGenRenderbuffersEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC				nglBindRenderbufferEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC			nglRenderbufferStorageEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC		nglFramebufferRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC			nglDeleteRenderbuffersEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC			nglDeleteFramebuffersEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC	nglGetRenderbufferParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC				nglGenerateMipmapEXT;

// GL_EXT_framebuffer_blit
extern PFNGLBLITFRAMEBUFFEREXTPROC				nglBlitFramebufferEXT;

// GL_EXT_framebuffer_multisample
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC	nglRenderbufferStorageMultisampleEXT;

// GL_ARB_multisample
extern PFNGLSAMPLECOVERAGEARBPROC				nglSampleCoverageARB;

#endif // USE_OPENGLES

#endif // NL_OPENGL_EXTENSION_H

