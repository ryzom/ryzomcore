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
	// Is this driver a correct OpenGL 1.2 driver?
	bool	Version1_2;

	// Required Extensions.
	bool	ARBMultiTexture;
	uint	NbTextureStages;
	bool	EXTTextureEnvCombine;

	// Optional Extensions.
	bool	EXTTextureCompressionS3TC;
	bool	EXTVertexWeighting;
	bool	EXTSeparateSpecularColor;
	bool	NVTextureEnvCombine4;
	bool	ARBTextureCubeMap;
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

	bool	EXTSecondaryColor;
	bool	EXTBlendColor;
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
	bool	ATITextureEnvCombine3;
	bool	ATIEnvMapBumpMap;
	bool	ATIFragmentShader;
	bool	ATIXTextureEnvRoute;

	// ARB Extensions
	bool	ARBTextureCompression;
	bool	ARBFragmentProgram;
	bool	ARBVertexProgram;
	bool	ARBTextureNonPowerOfTwo;
	bool	ARBMultisample;

	bool	OESDrawTexture;
	bool	OESMapBuffer;

public:

	/// \name Disable Hardware feature. False by default. setuped by IDriver
	// @{
	bool				DisableHardwareVertexProgram;
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
		EXTTextureCompressionS3TC= false;
		EXTVertexWeighting= false;
		EXTSeparateSpecularColor= false;
		NVTextureEnvCombine4= false;
		ATITextureEnvCombine3= false;
		ATIXTextureEnvRoute= false;
		ARBTextureCubeMap= false;
		NVTextureShader= false;
		EXTSecondaryColor= false;
		WGLARBPBuffer= false;
		WGLARBPixelFormat= false;
		WGLEXTSwapControl= false;
		GLXEXTSwapControl= false;
		GLXSGISwapControl= false;
		GLXMESASwapControl= false;
		EXTBlendColor= false;
		ATIEnvMapBumpMap = false;
		ATIFragmentShader = false;
		ARBFragmentProgram = false;
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
		NVStateVARWithoutFlush = 0;

		OESDrawTexture = false;
		OESMapBuffer = false;

		/// \name Disable Hardware feature. False by default. setuped by IDriver
		DisableHardwareVertexProgram= false;
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
		result += ARBVertexProgram ? "ARBVertexProgram " : "";

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
		result += "VertexBufferObject";

		result += "\n  FBO:       ";
		result += FrameBufferObject ? "FramebufferObject " : "";
		result += FrameBufferBlit ? "FrameBufferBlit " : "";
		result += FrameBufferMultisample ? "FrameBufferMultisample " : "";
		result += PackedDepthStencil ? "PackedDepthStencil " : "";

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


// VertexWeighting.
//==================
extern NEL_PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
extern NEL_PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
extern NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


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
extern PFNGLBINDBUFFERPROC nglBindBuffer;
extern PFNGLDELETEBUFFERSPROC nglDeleteBuffers;
extern PFNGLGENBUFFERSPROC nglGenBuffers;
extern PFNGLISBUFFERPROC nglIsBuffer;
extern PFNGLBUFFERDATAPROC nglBufferData;
extern PFNGLBUFFERSUBDATAPROC nglBufferSubData;
extern PFNGLGETBUFFERSUBDATAPROC nglGetBufferSubData;
extern PFNGLMAPBUFFERPROC nglMapBuffer;
extern PFNGLUNMAPBUFFERPROC nglUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVPROC nglGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVPROC nglGetBufferPointerv;



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

#endif // NL_OPENGL_EXTENSION_H

