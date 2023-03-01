// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2020  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#ifdef USE_OPENGLES
#define	nglGetProcAddress eglGetProcAddress
#elif defined(NL_OS_WINDOWS)
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

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// ***************************************************************************
// The exported function names

#ifdef USE_OPENGLES

// GL_OES_mapbuffer
PFNGLMAPBUFFEROESPROC						nglMapBufferOES;
PFNGLUNMAPBUFFEROESPROC						nglUnmapBufferOES;
PFNGLGETBUFFERPOINTERVOESPROC				nglGetBufferPointervOES;

PFNGLDRAWTEXFOESPROC						nglDrawTexfOES;

// GL_OES_framebuffer_object
PFNGLISRENDERBUFFEROESPROC					nglIsRenderbufferOES;
PFNGLBINDRENDERBUFFEROESPROC				nglBindRenderbufferOES;
PFNGLDELETERENDERBUFFERSOESPROC				nglDeleteRenderbuffersOES;
PFNGLGENRENDERBUFFERSOESPROC				nglGenRenderbuffersOES;
PFNGLRENDERBUFFERSTORAGEOESPROC				nglRenderbufferStorageOES;
PFNGLGETRENDERBUFFERPARAMETERIVOESPROC		nglGetRenderbufferParameterivOES;
PFNGLISFRAMEBUFFEROESPROC					nglIsFramebufferOES;
PFNGLBINDFRAMEBUFFEROESPROC					nglBindFramebufferOES;
PFNGLDELETEFRAMEBUFFERSOESPROC				nglDeleteFramebuffersOES;
PFNGLGENFRAMEBUFFERSOESPROC					nglGenFramebuffersOES;
PFNGLCHECKFRAMEBUFFERSTATUSOESPROC			nglCheckFramebufferStatusOES;
PFNGLFRAMEBUFFERRENDERBUFFEROESPROC			nglFramebufferRenderbufferOES;
PFNGLFRAMEBUFFERTEXTURE2DOESPROC			nglFramebufferTexture2DOES;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC	nglGetFramebufferAttachmentParameterivOES;
PFNGLGENERATEMIPMAPOESPROC					nglGenerateMipmapOES;

// GL_OES_texture_cube_map
PFNGLTEXGENFOESPROC							nglTexGenfOES;
PFNGLTEXGENFVOESPROC						nglTexGenfvOES;
PFNGLTEXGENIOESPROC							nglTexGeniOES;
PFNGLTEXGENIVOESPROC						nglTexGenivOES;
PFNGLTEXGENXOESPROC							nglTexGenxOES;
PFNGLTEXGENXVOESPROC						nglTexGenxvOES;
PFNGLGETTEXGENFVOESPROC						nglGetTexGenfvOES;
PFNGLGETTEXGENIVOESPROC						nglGetTexGenivOES;
PFNGLGETTEXGENXVOESPROC						nglGetTexGenxvOES;

#else

// ARB_multitexture
PFNGLACTIVETEXTUREARBPROC					nglActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC				nglClientActiveTextureARB;

PFNGLMULTITEXCOORD1SARBPROC					nglMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1IARBPROC					nglMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1FARBPROC					nglMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1DARBPROC					nglMultiTexCoord1dARB;
PFNGLMULTITEXCOORD2SARBPROC					nglMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2IARBPROC					nglMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2FARBPROC					nglMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2DARBPROC					nglMultiTexCoord2dARB;
PFNGLMULTITEXCOORD3SARBPROC					nglMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3IARBPROC					nglMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3FARBPROC					nglMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3DARBPROC					nglMultiTexCoord3dARB;
PFNGLMULTITEXCOORD4SARBPROC					nglMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4IARBPROC					nglMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4FARBPROC					nglMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4DARBPROC					nglMultiTexCoord4dARB;

PFNGLMULTITEXCOORD1SVARBPROC				nglMultiTexCoord1svARB;
PFNGLMULTITEXCOORD1IVARBPROC				nglMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1FVARBPROC				nglMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1DVARBPROC				nglMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD2SVARBPROC				nglMultiTexCoord2svARB;
PFNGLMULTITEXCOORD2IVARBPROC				nglMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2FVARBPROC				nglMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2DVARBPROC				nglMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD3SVARBPROC				nglMultiTexCoord3svARB;
PFNGLMULTITEXCOORD3IVARBPROC				nglMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3FVARBPROC				nglMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3DVARBPROC				nglMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD4SVARBPROC				nglMultiTexCoord4svARB;
PFNGLMULTITEXCOORD4IVARBPROC				nglMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4FVARBPROC				nglMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4DVARBPROC				nglMultiTexCoord4dvARB;

// ARB_TextureCompression.
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC			nglCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC			nglCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC			nglCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC			nglCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC			nglCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC			nglCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC			nglGetCompressedTexImageARB;

// VertexArrayRangeNV.
PFNGLFLUSHVERTEXARRAYRANGENVPROC			nglFlushVertexArrayRangeNV;
PFNGLVERTEXARRAYRANGENVPROC					nglVertexArrayRangeNV;

// FenceNV.
PFNGLDELETEFENCESNVPROC						nglDeleteFencesNV;
PFNGLGENFENCESNVPROC						nglGenFencesNV;
PFNGLISFENCENVPROC							nglIsFenceNV;
PFNGLTESTFENCENVPROC						nglTestFenceNV;
PFNGLGETFENCEIVNVPROC						nglGetFenceivNV;
PFNGLFINISHFENCENVPROC						nglFinishFenceNV;
PFNGLSETFENCENVPROC							nglSetFenceNV;

// VertexWeighting.
PFNGLVERTEXWEIGHTFEXTPROC					nglVertexWeightfEXT;
PFNGLVERTEXWEIGHTFVEXTPROC					nglVertexWeightfvEXT;
PFNGLVERTEXWEIGHTPOINTEREXTPROC				nglVertexWeightPointerEXT;

// VertexProgramExtension.
PFNGLAREPROGRAMSRESIDENTNVPROC				nglAreProgramsResidentNV;
PFNGLBINDPROGRAMNVPROC						nglBindProgramNV;
PFNGLDELETEPROGRAMSNVPROC					nglDeleteProgramsNV;
PFNGLEXECUTEPROGRAMNVPROC					nglExecuteProgramNV;
PFNGLGENPROGRAMSNVPROC						nglGenProgramsNV;
PFNGLGETPROGRAMPARAMETERDVNVPROC			nglGetProgramParameterdvNV;
PFNGLGETPROGRAMPARAMETERFVNVPROC			nglGetProgramParameterfvNV;
PFNGLGETPROGRAMIVNVPROC						nglGetProgramivNV;
PFNGLGETPROGRAMSTRINGNVPROC					nglGetProgramStringNV;
PFNGLGETTRACKMATRIXIVNVPROC					nglGetTrackMatrixivNV;
PFNGLGETVERTEXATTRIBDVNVPROC				nglGetVertexAttribdvNV;
PFNGLGETVERTEXATTRIBFVNVPROC				nglGetVertexAttribfvNV;
PFNGLGETVERTEXATTRIBIVNVPROC				nglGetVertexAttribivNV;
PFNGLGETVERTEXATTRIBPOINTERVNVPROC			nglGetVertexAttribPointervNV;
PFNGLISPROGRAMNVPROC						nglIsProgramNV;
PFNGLLOADPROGRAMNVPROC						nglLoadProgramNV;
PFNGLPROGRAMPARAMETER4DNVPROC				nglProgramParameter4dNV;
PFNGLPROGRAMPARAMETER4DVNVPROC				nglProgramParameter4dvNV;
PFNGLPROGRAMPARAMETER4FNVPROC				nglProgramParameter4fNV;
PFNGLPROGRAMPARAMETER4FVNVPROC				nglProgramParameter4fvNV;
PFNGLPROGRAMPARAMETERS4DVNVPROC				nglProgramParameters4dvNV;
PFNGLPROGRAMPARAMETERS4FVNVPROC				nglProgramParameters4fvNV;
PFNGLREQUESTRESIDENTPROGRAMSNVPROC			nglRequestResidentProgramsNV;
PFNGLTRACKMATRIXNVPROC						nglTrackMatrixNV;
PFNGLVERTEXATTRIBPOINTERNVPROC				nglVertexAttribPointerNV;
PFNGLVERTEXATTRIB1DNVPROC					nglVertexAttrib1dNV;
PFNGLVERTEXATTRIB1DVNVPROC					nglVertexAttrib1dvNV;
PFNGLVERTEXATTRIB1FNVPROC					nglVertexAttrib1fNV;
PFNGLVERTEXATTRIB1FVNVPROC					nglVertexAttrib1fvNV;
PFNGLVERTEXATTRIB1SNVPROC					nglVertexAttrib1sNV;
PFNGLVERTEXATTRIB1SVNVPROC					nglVertexAttrib1svNV;
PFNGLVERTEXATTRIB2DNVPROC					nglVertexAttrib2dNV;
PFNGLVERTEXATTRIB2DVNVPROC					nglVertexAttrib2dvNV;
PFNGLVERTEXATTRIB2FNVPROC					nglVertexAttrib2fNV;
PFNGLVERTEXATTRIB2FVNVPROC					nglVertexAttrib2fvNV;
PFNGLVERTEXATTRIB2SNVPROC					nglVertexAttrib2sNV;
PFNGLVERTEXATTRIB2SVNVPROC					nglVertexAttrib2svNV;
PFNGLVERTEXATTRIB3DNVPROC					nglVertexAttrib3dNV;
PFNGLVERTEXATTRIB3DVNVPROC					nglVertexAttrib3dvNV;
PFNGLVERTEXATTRIB3FNVPROC					nglVertexAttrib3fNV;
PFNGLVERTEXATTRIB3FVNVPROC					nglVertexAttrib3fvNV;
PFNGLVERTEXATTRIB3SNVPROC					nglVertexAttrib3sNV;
PFNGLVERTEXATTRIB3SVNVPROC					nglVertexAttrib3svNV;
PFNGLVERTEXATTRIB4DNVPROC					nglVertexAttrib4dNV;
PFNGLVERTEXATTRIB4DVNVPROC					nglVertexAttrib4dvNV;
PFNGLVERTEXATTRIB4FNVPROC					nglVertexAttrib4fNV;
PFNGLVERTEXATTRIB4FVNVPROC					nglVertexAttrib4fvNV;
PFNGLVERTEXATTRIB4SNVPROC					nglVertexAttrib4sNV;
PFNGLVERTEXATTRIB4SVNVPROC					nglVertexAttrib4svNV;
PFNGLVERTEXATTRIB4UBVNVPROC					nglVertexAttrib4ubvNV;
PFNGLVERTEXATTRIBS1DVNVPROC					nglVertexAttribs1dvNV;
PFNGLVERTEXATTRIBS1FVNVPROC					nglVertexAttribs1fvNV;
PFNGLVERTEXATTRIBS1SVNVPROC					nglVertexAttribs1svNV;
PFNGLVERTEXATTRIBS2DVNVPROC					nglVertexAttribs2dvNV;
PFNGLVERTEXATTRIBS2FVNVPROC					nglVertexAttribs2fvNV;
PFNGLVERTEXATTRIBS2SVNVPROC					nglVertexAttribs2svNV;
PFNGLVERTEXATTRIBS3DVNVPROC					nglVertexAttribs3dvNV;
PFNGLVERTEXATTRIBS3FVNVPROC					nglVertexAttribs3fvNV;
PFNGLVERTEXATTRIBS3SVNVPROC					nglVertexAttribs3svNV;
PFNGLVERTEXATTRIBS4DVNVPROC					nglVertexAttribs4dvNV;
PFNGLVERTEXATTRIBS4FVNVPROC					nglVertexAttribs4fvNV;
PFNGLVERTEXATTRIBS4SVNVPROC					nglVertexAttribs4svNV;
PFNGLVERTEXATTRIBS4UBVNVPROC				nglVertexAttribs4ubvNV;

// VertexShaderExt extension
PFNGLBEGINVERTEXSHADEREXTPROC				nglBeginVertexShaderEXT;
PFNGLENDVERTEXSHADEREXTPROC					nglEndVertexShaderEXT;
PFNGLBINDVERTEXSHADEREXTPROC				nglBindVertexShaderEXT;
PFNGLGENVERTEXSHADERSEXTPROC				nglGenVertexShadersEXT;
PFNGLDELETEVERTEXSHADEREXTPROC				nglDeleteVertexShaderEXT;
PFNGLSHADEROP1EXTPROC						nglShaderOp1EXT;
PFNGLSHADEROP2EXTPROC						nglShaderOp2EXT;
PFNGLSHADEROP3EXTPROC						nglShaderOp3EXT;
PFNGLSWIZZLEEXTPROC							nglSwizzleEXT;
PFNGLWRITEMASKEXTPROC						nglWriteMaskEXT;
PFNGLINSERTCOMPONENTEXTPROC					nglInsertComponentEXT;
PFNGLEXTRACTCOMPONENTEXTPROC				nglExtractComponentEXT;
PFNGLGENSYMBOLSEXTPROC						nglGenSymbolsEXT;
PFNGLSETINVARIANTEXTPROC					nglSetInvariantEXT;
PFNGLSETLOCALCONSTANTEXTPROC				nglSetLocalConstantEXT;
PFNGLVARIANTPOINTEREXTPROC					nglVariantPointerEXT;
PFNGLENABLEVARIANTCLIENTSTATEEXTPROC		nglEnableVariantClientStateEXT;
PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC		nglDisableVariantClientStateEXT;
PFNGLBINDLIGHTPARAMETEREXTPROC				nglBindLightParameterEXT;
PFNGLBINDMATERIALPARAMETEREXTPROC			nglBindMaterialParameterEXT;
PFNGLBINDTEXGENPARAMETEREXTPROC				nglBindTexGenParameterEXT;
PFNGLBINDTEXTUREUNITPARAMETEREXTPROC		nglBindTextureUnitParameterEXT;
PFNGLBINDPARAMETEREXTPROC					nglBindParameterEXT;
PFNGLISVARIANTENABLEDEXTPROC				nglIsVariantEnabledEXT;
PFNGLGETVARIANTBOOLEANVEXTPROC				nglGetVariantBooleanvEXT;
PFNGLGETVARIANTINTEGERVEXTPROC				nglGetVariantIntegervEXT;
PFNGLGETVARIANTFLOATVEXTPROC				nglGetVariantFloatvEXT;
PFNGLGETVARIANTPOINTERVEXTPROC				nglGetVariantPointervEXT;
PFNGLGETINVARIANTBOOLEANVEXTPROC			nglGetInvariantBooleanvEXT;
PFNGLGETINVARIANTINTEGERVEXTPROC			nglGetInvariantIntegervEXT;
PFNGLGETINVARIANTFLOATVEXTPROC				nglGetInvariantFloatvEXT;
PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC		nglGetLocalConstantBooleanvEXT;
PFNGLGETLOCALCONSTANTINTEGERVEXTPROC		nglGetLocalConstantIntegervEXT;
PFNGLGETLOCALCONSTANTFLOATVEXTPROC			nglGetLocalConstantFloatvEXT;

// SecondaryColor extension
PFNGLSECONDARYCOLOR3BEXTPROC				nglSecondaryColor3bEXT;
PFNGLSECONDARYCOLOR3BVEXTPROC				nglSecondaryColor3bvEXT;
PFNGLSECONDARYCOLOR3DEXTPROC				nglSecondaryColor3dEXT;
PFNGLSECONDARYCOLOR3DVEXTPROC				nglSecondaryColor3dvEXT;
PFNGLSECONDARYCOLOR3FEXTPROC				nglSecondaryColor3fEXT;
PFNGLSECONDARYCOLOR3FVEXTPROC				nglSecondaryColor3fvEXT;
PFNGLSECONDARYCOLOR3IEXTPROC				nglSecondaryColor3iEXT;
PFNGLSECONDARYCOLOR3IVEXTPROC				nglSecondaryColor3ivEXT;
PFNGLSECONDARYCOLOR3SEXTPROC				nglSecondaryColor3sEXT;
PFNGLSECONDARYCOLOR3SVEXTPROC				nglSecondaryColor3svEXT;
PFNGLSECONDARYCOLOR3UBEXTPROC				nglSecondaryColor3ubEXT;
PFNGLSECONDARYCOLOR3UBVEXTPROC				nglSecondaryColor3ubvEXT;
PFNGLSECONDARYCOLOR3UIEXTPROC				nglSecondaryColor3uiEXT;
PFNGLSECONDARYCOLOR3UIVEXTPROC				nglSecondaryColor3uivEXT;
PFNGLSECONDARYCOLOR3USEXTPROC				nglSecondaryColor3usEXT;
PFNGLSECONDARYCOLOR3USVEXTPROC				nglSecondaryColor3usvEXT;
PFNGLSECONDARYCOLORPOINTEREXTPROC			nglSecondaryColorPointerEXT;

// BlendColor extension
PFNGLBLENDCOLOREXTPROC						nglBlendColorEXT;

//========================
PFNGLNEWOBJECTBUFFERATIPROC					nglNewObjectBufferATI;
PFNGLISOBJECTBUFFERATIPROC					nglIsObjectBufferATI;
PFNGLUPDATEOBJECTBUFFERATIPROC				nglUpdateObjectBufferATI;
PFNGLGETOBJECTBUFFERFVATIPROC				nglGetObjectBufferfvATI;
PFNGLGETOBJECTBUFFERIVATIPROC				nglGetObjectBufferivATI;
PFNGLFREEOBJECTBUFFERATIPROC				nglFreeObjectBufferATI;
PFNGLARRAYOBJECTATIPROC						nglArrayObjectATI;
PFNGLGETARRAYOBJECTFVATIPROC				nglGetArrayObjectfvATI;
PFNGLGETARRAYOBJECTIVATIPROC				nglGetArrayObjectivATI;
PFNGLVARIANTARRAYOBJECTATIPROC				nglVariantArrayObjectATI;
PFNGLGETVARIANTARRAYOBJECTFVATIPROC			nglGetVariantArrayObjectfvATI;
PFNGLGETVARIANTARRAYOBJECTIVATIPROC			nglGetVariantArrayObjectivATI;

// GL_ATI_map_object_buffer
PFNGLMAPOBJECTBUFFERATIPROC   				nglMapObjectBufferATI;
PFNGLUNMAPOBJECTBUFFERATIPROC 				nglUnmapObjectBufferATI;

// GL_ATI_vertex_attrib_array_object
PFNGLVERTEXATTRIBARRAYOBJECTATIPROC			nglVertexAttribArrayObjectATI;
PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC	nglGetVertexAttribArrayObjectfvATI;
PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC	nglGetVertexAttribArrayObjectivATI;

// GL_ATI_envmap_bumpmap extension
PFNGLTEXBUMPPARAMETERIVATIPROC					nglTexBumpParameterivATI;
PFNGLTEXBUMPPARAMETERFVATIPROC					nglTexBumpParameterfvATI;
PFNGLGETTEXBUMPPARAMETERIVATIPROC				nglGetTexBumpParameterivATI;
PFNGLGETTEXBUMPPARAMETERFVATIPROC				nglGetTexBumpParameterfvATI;

// GL_ATI_fragment_shader extension
PFNGLGENFRAGMENTSHADERSATIPROC				nglGenFragmentShadersATI;
PFNGLBINDFRAGMENTSHADERATIPROC				nglBindFragmentShaderATI;
PFNGLDELETEFRAGMENTSHADERATIPROC			nglDeleteFragmentShaderATI;
PFNGLBEGINFRAGMENTSHADERATIPROC				nglBeginFragmentShaderATI;
PFNGLENDFRAGMENTSHADERATIPROC				nglEndFragmentShaderATI;
PFNGLPASSTEXCOORDATIPROC					nglPassTexCoordATI;
PFNGLSAMPLEMAPATIPROC						nglSampleMapATI;
PFNGLCOLORFRAGMENTOP1ATIPROC				nglColorFragmentOp1ATI;
PFNGLCOLORFRAGMENTOP2ATIPROC				nglColorFragmentOp2ATI;
PFNGLCOLORFRAGMENTOP3ATIPROC				nglColorFragmentOp3ATI;
PFNGLALPHAFRAGMENTOP1ATIPROC				nglAlphaFragmentOp1ATI;
PFNGLALPHAFRAGMENTOP2ATIPROC				nglAlphaFragmentOp2ATI;
PFNGLALPHAFRAGMENTOP3ATIPROC				nglAlphaFragmentOp3ATI;
PFNGLSETFRAGMENTSHADERCONSTANTATIPROC		nglSetFragmentShaderConstantATI;

// GL_ARB_fragment_program
// the following functions are the sames than with GL_ARB_vertex_program
//PFNGLPROGRAMSTRINGARBPROC					nglProgramStringARB;
//PFNGLBINDPROGRAMARBPROC					nglBindProgramARB;
//PFNGLDELETEPROGRAMSARBPROC				nglDeleteProgramsARB;
//PFNGLGENPROGRAMSARBPROC					nglGenProgramsARB;
//PFNGLPROGRAMENVPARAMETER4DARBPROC			nglProgramEnvParameter4dARB;
//PFNGLPROGRAMENVPARAMETER4DVARBPROC		nglProgramEnvParameter4dvARB;
//PFNGLPROGRAMENVPARAMETER4FARBPROC			nglProgramEnvParameter4fARB;
//PFNGLPROGRAMENVPARAMETER4FVARBPROC		nglProgramEnvParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC			nglGetProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC		nglGetProgramLocalParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC			nglGetProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC		nglGetProgramLocalParameter4fvARB;
//PFNGLGETPROGRAMENVPARAMETERDVARBPROC		nglGetProgramEnvParameterdvARB;
//PFNGLGETPROGRAMENVPARAMETERFVARBPROC		nglGetProgramEnvParameterfvARB;
//PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC	nglGetProgramLocalParameterdvARB;
//PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC	nglGetProgramLocalParameterfvARB;
//PFNGLGETPROGRAMIVARBPROC					nglGetProgramivARB;
//PFNGLGETPROGRAMSTRINGARBPROC				nglGetProgramStringARB;
//PFNGLISPROGRAMARBPROC						nglIsProgramARB;

// GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC							nglBindBufferARB;
PFNGLDELETEBUFFERSARBPROC						nglDeleteBuffersARB;
PFNGLGENBUFFERSARBPROC							nglGenBuffersARB;
PFNGLISBUFFERARBPROC 							nglIsBufferARB;
PFNGLBUFFERDATAARBPROC 							nglBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC 						nglBufferSubDataARB;
PFNGLGETBUFFERSUBDATAARBPROC 					nglGetBufferSubDataARB;
PFNGLMAPBUFFERARBPROC 							nglMapBufferARB;
PFNGLUNMAPBUFFERARBPROC 						nglUnmapBufferARB;
PFNGLGETBUFFERPARAMETERIVARBPROC 				nglGetBufferParameterivARB;
PFNGLGETBUFFERPOINTERVARBPROC 					nglGetBufferPointervARB;

// GL_ARB_map_buffer_range
PFNGLMAPBUFFERRANGEPROC							nglMapBufferRange;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC					nglFlushMappedBufferRange;

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
PFNGLGENOCCLUSIONQUERIESNVPROC				nglGenOcclusionQueriesNV;
PFNGLDELETEOCCLUSIONQUERIESNVPROC			nglDeleteOcclusionQueriesNV;
PFNGLISOCCLUSIONQUERYNVPROC					nglIsOcclusionQueryNV;
PFNGLBEGINOCCLUSIONQUERYNVPROC				nglBeginOcclusionQueryNV;
PFNGLENDOCCLUSIONQUERYNVPROC				nglEndOcclusionQueryNV;
PFNGLGETOCCLUSIONQUERYIVNVPROC				nglGetOcclusionQueryivNV;
PFNGLGETOCCLUSIONQUERYUIVNVPROC				nglGetOcclusionQueryuivNV;

// ARB_occlusion_query
PFNGLGENQUERIESARBPROC						nglGenQueriesARB;
PFNGLDELETEQUERIESARBPROC					nglDeleteQueriesARB;
PFNGLISQUERYARBPROC							nglIsQueryARB;
PFNGLBEGINQUERYARBPROC						nglBeginQueryARB;
PFNGLENDQUERYARBPROC						nglEndQueryARB;
PFNGLGETQUERYIVARBPROC						nglGetQueryivARB;
PFNGLGETQUERYOBJECTIVARBPROC				nglGetQueryObjectivARB;
PFNGLGETQUERYOBJECTUIVARBPROC				nglGetQueryObjectuivARB;

// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC					nglIsRenderbufferEXT;
PFNGLISFRAMEBUFFEREXTPROC					nglIsFramebufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC			nglCheckFramebufferStatusEXT;
PFNGLGENFRAMEBUFFERSEXTPROC					nglGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC					nglBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC			nglFramebufferTexture2DEXT;
PFNGLGENRENDERBUFFERSEXTPROC				nglGenRenderbuffersEXT;
PFNGLBINDRENDERBUFFEREXTPROC				nglBindRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC				nglRenderbufferStorageEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC			nglFramebufferRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC				nglDeleteRenderbuffersEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC				nglDeleteFramebuffersEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC		nglGetRenderbufferParameterivEXT;
PFNGLGENERATEMIPMAPEXTPROC					nglGenerateMipmapEXT;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC					nglBlitFramebufferEXT;

// GL_EXT_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC	nglRenderbufferStorageMultisampleEXT;

// GL_ARB_multisample
PFNGLSAMPLECOVERAGEARBPROC					nglSampleCoverageARB;

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

// WGL_AMD_gpu_association
//========================
PFNWGLGETGPUIDSAMDPROC							nwglGetGPUIDsAMD;
PFNWGLGETGPUINFOAMDPROC							nwglGetGPUInfoAMD;
PFNWGLGETCONTEXTGPUIDAMDPROC					nwglGetContextGPUIDAMD;
PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC			nwglCreateAssociatedContextAMD;
PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC		nwglCreateAssociatedContextAttribsAMD;
PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC			nwglDeleteAssociatedContextAMD;
PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC		nwglMakeAssociatedContextCurrentAMD;
PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC		nwglGetCurrentAssociatedContextAMD;
PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC				nwglBlitContextFramebufferAMD;

// WGL_NV_gpu_affinity
//====================
PFNWGLENUMGPUSNVPROC							nwglEnumGpusNV;
PFNWGLENUMGPUDEVICESNVPROC						nwglEnumGpuDevicesNV;
PFNWGLCREATEAFFINITYDCNVPROC					nwglCreateAffinityDCNV;
PFNWGLENUMGPUSFROMAFFINITYDCNVPROC				nwglEnumGpusFromAffinityDCNV;
PFNWGLDELETEDCNVPROC							nwglDeleteDCNV;

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

PFNGLXALLOCATEMEMORYNVPROC					nglXAllocateMemoryNV;
PFNGLXFREEMEMORYNVPROC						nglXFreeMemoryNV;

// Swap control extensions
PFNGLXSWAPINTERVALEXTPROC					nglXSwapIntervalEXT;

PFNGLXSWAPINTERVALSGIPROC					nglXSwapIntervalSGI;

PFNGLXSWAPINTERVALMESAPROC					nglXSwapIntervalMESA;
PFNGLXGETSWAPINTERVALMESAPROC				nglXGetSwapIntervalMESA;

// GLX_MESA_query_renderer
// =======================
PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC	nglXQueryCurrentRendererIntegerMESA;
#endif

#endif // USE_OPENGLES

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


namespace	NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
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

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_multitexture");

	CHECK_ADDRESS(PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);
	CHECK_ADDRESS(PFNGLCLIENTACTIVETEXTUREARBPROC, glClientActiveTextureARB);

	CHECK_ADDRESS(PFNGLMULTITEXCOORD1SARBPROC, glMultiTexCoord1sARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1IARBPROC, glMultiTexCoord1iARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1FARBPROC, glMultiTexCoord1fARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1DARBPROC, glMultiTexCoord1dARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2SARBPROC, glMultiTexCoord2sARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2IARBPROC, glMultiTexCoord2iARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2DARBPROC, glMultiTexCoord2dARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3SARBPROC, glMultiTexCoord3sARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3IARBPROC, glMultiTexCoord3iARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3FARBPROC, glMultiTexCoord3fARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3DARBPROC, glMultiTexCoord3dARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4SARBPROC, glMultiTexCoord4sARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4IARBPROC, glMultiTexCoord4iARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4FARBPROC, glMultiTexCoord4fARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4DARBPROC, glMultiTexCoord4dARB);

	CHECK_ADDRESS(PFNGLMULTITEXCOORD1SVARBPROC, glMultiTexCoord1svARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1IVARBPROC, glMultiTexCoord1ivARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1FVARBPROC, glMultiTexCoord1fvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD1DVARBPROC, glMultiTexCoord1dvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2SVARBPROC, glMultiTexCoord2svARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2IVARBPROC, glMultiTexCoord2ivARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD2DVARBPROC, glMultiTexCoord2dvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3SVARBPROC, glMultiTexCoord3svARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3IVARBPROC, glMultiTexCoord3ivARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3FVARBPROC, glMultiTexCoord3fvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD3DVARBPROC, glMultiTexCoord3dvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4SVARBPROC, glMultiTexCoord4svARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4IVARBPROC, glMultiTexCoord4ivARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4FVARBPROC, glMultiTexCoord4fvARB);
	CHECK_ADDRESS(PFNGLMULTITEXCOORD4DVARBPROC, glMultiTexCoord4dvARB);
#endif

	return true;
}

// *********************************
static bool setupEXTTextureEnvCombine(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureEnvCombine);

#ifdef USE_OPENGLES
	return true;
#else
	return (strstr(glext, "GL_EXT_texture_env_combine")!=NULL || strstr(glext, "GL_ARB_texture_env_combine")!=NULL);
#endif
}


// *********************************
static bool	setupARBTextureCompression(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_texture_compression");

	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE3DARBPROC, glCompressedTexImage3DARB);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC, glCompressedTexImage2DARB);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXIMAGE1DARBPROC, glCompressedTexImage1DARB);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC, glCompressedTexSubImage3DARB);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC, glCompressedTexSubImage2DARB);
	CHECK_ADDRESS(PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC, glCompressedTexSubImage1DARB);
	CHECK_ADDRESS(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC, glGetCompressedTexImageARB);
#endif

	return true;
}

// *********************************
static bool	setupARBTextureNonPowerOfTwo(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_texture_non_power_of_two");
#endif

	return true;
}

// ***************************************************************************
static bool	setupOESMapBuffer(const char *glext)
{
	H_AUTO_OGL(setupOESMapBuffer);

	CHECK_EXT("OES_mapbuffer");

#ifdef USE_OPENGLES
	CHECK_ADDRESS(PFNGLMAPBUFFEROESPROC, glMapBufferOES);
	CHECK_ADDRESS(PFNGLUNMAPBUFFEROESPROC, glUnmapBufferOES);
	CHECK_ADDRESS(PFNGLGETBUFFERPOINTERVOESPROC, glGetBufferPointervOES);
#endif

	return true;
}

// ***************************************************************************
static bool	setupOESDrawTexture(const char *glext)
{
	H_AUTO_OGL(setupOESDrawTexture);

	CHECK_EXT("OES_draw_texture");

#ifdef USE_OPENGLES
	CHECK_ADDRESS(PFNGLDRAWTEXFOESPROC, glDrawTexfOES);
#endif

	return true;
}

// *********************************
static bool	setupNVVertexArrayRange(const char	*glext)
{
	H_AUTO_OGL(setupNVVertexArrayRange);

	// Test if VAR is present.
	CHECK_EXT("GL_NV_vertex_array_range");

	// Tess Fence too.
	CHECK_EXT("GL_NV_fence");

#ifndef USE_OPENGLES
	// Get VAR address.
	CHECK_ADDRESS(PFNGLFLUSHVERTEXARRAYRANGENVPROC, glFlushVertexArrayRangeNV);
	CHECK_ADDRESS(PFNGLVERTEXARRAYRANGENVPROC, glVertexArrayRangeNV);

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLALLOCATEMEMORYNVPROC, wglAllocateMemoryNV);
	CHECK_ADDRESS(PFNWGLFREEMEMORYNVPROC, wglFreeMemoryNV);
#elif defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(PFNGLXALLOCATEMEMORYNVPROC, glXAllocateMemoryNV);
	CHECK_ADDRESS(PFNGLXFREEMEMORYNVPROC, glXFreeMemoryNV);
#endif

	// Get fence address.
	CHECK_ADDRESS(PFNGLDELETEFENCESNVPROC, glDeleteFencesNV);
	CHECK_ADDRESS(PFNGLGENFENCESNVPROC, glGenFencesNV);
	CHECK_ADDRESS(PFNGLISFENCENVPROC, glIsFenceNV);
	CHECK_ADDRESS(PFNGLTESTFENCENVPROC, glTestFenceNV);
	CHECK_ADDRESS(PFNGLGETFENCEIVNVPROC, glGetFenceivNV);
	CHECK_ADDRESS(PFNGLFINISHFENCENVPROC, glFinishFenceNV);
	CHECK_ADDRESS(PFNGLSETFENCENVPROC, glSetFenceNV);
#endif

	return true;
}

// *********************************
static bool	setupEXTTextureCompressionS3TC(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureCompressionS3TC);

#ifdef USE_OPENGLES
	CHECK_EXT("EXT_texture_compression_s3tc");
	// TODO: check also for EXT_texture_compression_dxt1
#else
	CHECK_EXT("GL_EXT_texture_compression_s3tc");
	// TODO: check also for GL_S3_s3tc, GL_EXT_texture_compression_dxt1
#endif

	return true;
}

// *********************************
static bool	setupEXTVertexWeighting(const char	*glext)
{
	H_AUTO_OGL(setupEXTVertexWeighting);
	CHECK_EXT("GL_EXT_vertex_weighting");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLVERTEXWEIGHTFEXTPROC, glVertexWeightfEXT);
	CHECK_ADDRESS(PFNGLVERTEXWEIGHTFVEXTPROC, glVertexWeightfvEXT);
	CHECK_ADDRESS(PFNGLVERTEXWEIGHTPOINTEREXTPROC, glVertexWeightPointerEXT);
#endif

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
static bool	setupNVTextureEnvCombine4(const char	*glext)
{
	H_AUTO_OGL(setupNVTextureEnvCombine4);
	CHECK_EXT("GL_NV_texture_env_combine4");
#ifdef DEBUG_OGL_COMBINE43_DISABLE
	// issue 310: disable extension to debug bug around CDriverGL::setupSpecularPass()
	nlwarning("GL_NV_texture_env_combine4 disabled by request (DEBUG_OGL_COMBINE43_DISABLE)");
	return false;
#else
	return true;
#endif
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
#ifdef DEBUG_OGL_COMBINE43_DISABLE
	// issue 310: disable extension to debug bug around CDriverGL::setupSpecularPass()
	nlwarning("GL_ATI_texture_env_combine3 disabled by request (DEBUG_OGL_COMBINE43_DISABLE)");
	return false;
#else
	return true;
#endif
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

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLTEXBUMPPARAMETERIVATIPROC, glTexBumpParameterivATI);
	CHECK_ADDRESS(PFNGLTEXBUMPPARAMETERFVATIPROC, glTexBumpParameterfvATI);
	CHECK_ADDRESS(PFNGLGETTEXBUMPPARAMETERIVATIPROC, glGetTexBumpParameterivATI);
	CHECK_ADDRESS(PFNGLGETTEXBUMPPARAMETERFVATIPROC, glGetTexBumpParameterfvATI);

	// Check for broken ATI drivers and disable EMBM if we caught one.
	// Reminder: This code crashes with Catalyst 7.11 fglrx drivers!
	nglGetTexBumpParameterivATI(GL_BUMP_NUM_TEX_UNITS_ATI, &num);
#endif

	return num > 0;
}

// *********************************
static bool	setupARBTextureCubeMap(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCubeMap);

#ifdef USE_OPENGLES
	CHECK_EXT("OES_texture_cube_map");

	CHECK_ADDRESS(PFNGLTEXGENFOESPROC, glTexGenfOES);
	CHECK_ADDRESS(PFNGLTEXGENFVOESPROC, glTexGenfvOES);
	CHECK_ADDRESS(PFNGLTEXGENIOESPROC, glTexGeniOES);
	CHECK_ADDRESS(PFNGLTEXGENIVOESPROC, glTexGenivOES);
	CHECK_ADDRESS(PFNGLTEXGENXOESPROC, glTexGenxOES);
	CHECK_ADDRESS(PFNGLTEXGENXVOESPROC, glTexGenxvOES);
	CHECK_ADDRESS(PFNGLGETTEXGENFVOESPROC, glGetTexGenfvOES);
	CHECK_ADDRESS(PFNGLGETTEXGENIVOESPROC, glGetTexGenivOES);
	CHECK_ADDRESS(PFNGLGETTEXGENXVOESPROC, glGetTexGenxvOES);
#else
	CHECK_EXT("GL_ARB_texture_cube_map");
#endif

	return true;
}


// *********************************
static bool	setupNVVertexProgram(const char	*glext)
{
	H_AUTO_OGL(setupNVVertexProgram);

// reenabled to allow bloom on mac, TODO: cleanly fix the water issue
// i think this issue was mtp target related - is this the case in ryzom too?
// #ifdef NL_OS_MAC
// // Water doesn't render on GeForce 8600M GT (on MAC OS X) if this extension is enabled
// 	return false;
// #endif

	CHECK_EXT("GL_NV_vertex_program");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLAREPROGRAMSRESIDENTNVPROC, glAreProgramsResidentNV);
	CHECK_ADDRESS(PFNGLBINDPROGRAMNVPROC, glBindProgramNV);
	CHECK_ADDRESS(PFNGLDELETEPROGRAMSNVPROC, glDeleteProgramsNV);
	CHECK_ADDRESS(PFNGLEXECUTEPROGRAMNVPROC, glExecuteProgramNV);
	CHECK_ADDRESS(PFNGLGENPROGRAMSNVPROC, glGenProgramsNV);
	CHECK_ADDRESS(PFNGLGETPROGRAMPARAMETERDVNVPROC, glGetProgramParameterdvNV);
	CHECK_ADDRESS(PFNGLGETPROGRAMPARAMETERFVNVPROC, glGetProgramParameterfvNV);
	CHECK_ADDRESS(PFNGLGETPROGRAMIVNVPROC, glGetProgramivNV);
	CHECK_ADDRESS(PFNGLGETPROGRAMSTRINGNVPROC, glGetProgramStringNV);
	CHECK_ADDRESS(PFNGLGETTRACKMATRIXIVNVPROC, glGetTrackMatrixivNV);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBDVNVPROC, glGetVertexAttribdvNV);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBFVNVPROC, glGetVertexAttribfvNV);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBIVNVPROC, glGetVertexAttribivNV);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBPOINTERVNVPROC, glGetVertexAttribPointervNV);
	CHECK_ADDRESS(PFNGLISPROGRAMNVPROC, glIsProgramNV);
	CHECK_ADDRESS(PFNGLLOADPROGRAMNVPROC, glLoadProgramNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETER4DNVPROC, glProgramParameter4dNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETER4DVNVPROC, glProgramParameter4dvNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETER4FNVPROC, glProgramParameter4fNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETER4FVNVPROC, glProgramParameter4fvNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETERS4DVNVPROC, glProgramParameters4dvNV);
	CHECK_ADDRESS(PFNGLPROGRAMPARAMETERS4FVNVPROC, glProgramParameters4fvNV);
	CHECK_ADDRESS(PFNGLREQUESTRESIDENTPROGRAMSNVPROC, glRequestResidentProgramsNV);
	CHECK_ADDRESS(PFNGLTRACKMATRIXNVPROC, glTrackMatrixNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBPOINTERNVPROC, glVertexAttribPointerNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1DNVPROC, glVertexAttrib1dNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1DVNVPROC, glVertexAttrib1dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1FNVPROC, glVertexAttrib1fNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1FVNVPROC, glVertexAttrib1fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1SNVPROC, glVertexAttrib1sNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB1SVNVPROC, glVertexAttrib1svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2DNVPROC, glVertexAttrib2dNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2DVNVPROC, glVertexAttrib2dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2FNVPROC, glVertexAttrib2fNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2FVNVPROC, glVertexAttrib2fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2SNVPROC, glVertexAttrib2sNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB2SVNVPROC, glVertexAttrib2svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3DNVPROC, glVertexAttrib3dNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3DVNVPROC, glVertexAttrib3dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3FNVPROC, glVertexAttrib3fNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3FVNVPROC, glVertexAttrib3fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3SNVPROC, glVertexAttrib3sNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB3SVNVPROC, glVertexAttrib3svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4DNVPROC, glVertexAttrib4dNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4DVNVPROC, glVertexAttrib4dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4FNVPROC, glVertexAttrib4fNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4FVNVPROC, glVertexAttrib4fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4SNVPROC, glVertexAttrib4sNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4SVNVPROC, glVertexAttrib4svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIB4UBVNVPROC, glVertexAttrib4ubvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS1DVNVPROC, glVertexAttribs1dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS1FVNVPROC, glVertexAttribs1fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS1SVNVPROC, glVertexAttribs1svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS2DVNVPROC, glVertexAttribs2dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS2FVNVPROC, glVertexAttribs2fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS2SVNVPROC, glVertexAttribs2svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS3DVNVPROC, glVertexAttribs3dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS3FVNVPROC, glVertexAttribs3fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS3SVNVPROC, glVertexAttribs3svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS4DVNVPROC, glVertexAttribs4dvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS4FVNVPROC, glVertexAttribs4fvNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS4SVNVPROC, glVertexAttribs4svNV);
	CHECK_ADDRESS(PFNGLVERTEXATTRIBS4UBVNVPROC, glVertexAttribs4ubvNV);
#endif

	return true;
}

// *********************************
static bool	setupEXTVertexShader(const char	*glext)
{
	H_AUTO_OGL(setupEXTVertexShader);
	CHECK_EXT("GL_EXT_vertex_shader");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLBEGINVERTEXSHADEREXTPROC, glBeginVertexShaderEXT);
	CHECK_ADDRESS(PFNGLENDVERTEXSHADEREXTPROC, glEndVertexShaderEXT);
	CHECK_ADDRESS(PFNGLBINDVERTEXSHADEREXTPROC, glBindVertexShaderEXT);
	CHECK_ADDRESS(PFNGLGENVERTEXSHADERSEXTPROC, glGenVertexShadersEXT);
	CHECK_ADDRESS(PFNGLDELETEVERTEXSHADEREXTPROC, glDeleteVertexShaderEXT);
	CHECK_ADDRESS(PFNGLSHADEROP1EXTPROC, glShaderOp1EXT);
	CHECK_ADDRESS(PFNGLSHADEROP2EXTPROC, glShaderOp2EXT);
	CHECK_ADDRESS(PFNGLSHADEROP3EXTPROC, glShaderOp3EXT);
	CHECK_ADDRESS(PFNGLSWIZZLEEXTPROC, glSwizzleEXT);
	CHECK_ADDRESS(PFNGLWRITEMASKEXTPROC, glWriteMaskEXT);
	CHECK_ADDRESS(PFNGLINSERTCOMPONENTEXTPROC, glInsertComponentEXT);
	CHECK_ADDRESS(PFNGLEXTRACTCOMPONENTEXTPROC, glExtractComponentEXT);
	CHECK_ADDRESS(PFNGLGENSYMBOLSEXTPROC, glGenSymbolsEXT);
	CHECK_ADDRESS(PFNGLSETINVARIANTEXTPROC, glSetInvariantEXT);
	CHECK_ADDRESS(PFNGLSETLOCALCONSTANTEXTPROC, glSetLocalConstantEXT);
	CHECK_ADDRESS(PFNGLVARIANTPOINTEREXTPROC, glVariantPointerEXT);
	CHECK_ADDRESS(PFNGLENABLEVARIANTCLIENTSTATEEXTPROC, glEnableVariantClientStateEXT);
	CHECK_ADDRESS(PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC, glDisableVariantClientStateEXT);
	CHECK_ADDRESS(PFNGLBINDLIGHTPARAMETEREXTPROC, glBindLightParameterEXT);
	CHECK_ADDRESS(PFNGLBINDMATERIALPARAMETEREXTPROC, glBindMaterialParameterEXT);
	CHECK_ADDRESS(PFNGLBINDTEXGENPARAMETEREXTPROC, glBindTexGenParameterEXT);
	CHECK_ADDRESS(PFNGLBINDTEXTUREUNITPARAMETEREXTPROC, glBindTextureUnitParameterEXT);
	CHECK_ADDRESS(PFNGLBINDPARAMETEREXTPROC, glBindParameterEXT);
	CHECK_ADDRESS(PFNGLISVARIANTENABLEDEXTPROC, glIsVariantEnabledEXT);
	CHECK_ADDRESS(PFNGLGETVARIANTBOOLEANVEXTPROC, glGetVariantBooleanvEXT);
	CHECK_ADDRESS(PFNGLGETVARIANTINTEGERVEXTPROC, glGetVariantIntegervEXT);
	CHECK_ADDRESS(PFNGLGETVARIANTFLOATVEXTPROC, glGetVariantFloatvEXT);
	CHECK_ADDRESS(PFNGLGETVARIANTPOINTERVEXTPROC, glGetVariantPointervEXT);
	CHECK_ADDRESS(PFNGLGETINVARIANTBOOLEANVEXTPROC, glGetInvariantBooleanvEXT);
	CHECK_ADDRESS(PFNGLGETINVARIANTINTEGERVEXTPROC, glGetInvariantIntegervEXT);
	CHECK_ADDRESS(PFNGLGETINVARIANTFLOATVEXTPROC, glGetInvariantFloatvEXT);
	CHECK_ADDRESS(PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC, glGetLocalConstantBooleanvEXT);
	CHECK_ADDRESS(PFNGLGETLOCALCONSTANTINTEGERVEXTPROC, glGetLocalConstantIntegervEXT);
	CHECK_ADDRESS(PFNGLGETLOCALCONSTANTFLOATVEXTPROC, glGetLocalConstantFloatvEXT);

	// we require at least 128 instructions, 15 local register (r0, r1,..,r11) + 3 temporary vector for swizzle emulation + 1 vector for indexing temp + 3 temporary scalar for LOGG, EXPP and LIT emulation,  1 address register
	// we require 11 variants (4 textures + position + normal + primary color + secondary color + weight + palette skin + fog)
	// we also require 2 local constants (0 and 1)
	// 96 invariants (c[0], c[1] ..) + 1 invariants for fog emulation (fog coordinate must range from 0 to 1 with EXT_VERTEX_shader)
	GLint numVSInst;
	glGetIntegerv(GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT, &numVSInst);
	if (numVSInst < 128) return false;
	//
	GLint numVSLocals;
	glGetIntegerv(GL_MAX_VERTEX_SHADER_LOCALS_EXT, &numVSLocals);
	if (numVSLocals < 4 * (12 + 4) + 1 + 3)
	{
		nlwarning("EXT_vertex_shader extension has not much register. Some vertex program may fail loading");
		return false;
	}
	//
	GLint numVSLocalConstants;
	glGetIntegerv(GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT, &numVSLocalConstants);
	if (numVSLocalConstants < 2) return false;
	//
	GLint numVSInvariants;
	glGetIntegerv(GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT, &numVSInvariants);
	if (numVSInvariants < 96 + 1) return false;
	//
	GLint numVSVariants;
	glGetIntegerv(GL_MAX_VERTEX_SHADER_VARIANTS_EXT, &numVSVariants);
	if (numVSInvariants < 4) return false;
#endif

	return true;

}


// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTSecondaryColor);
	CHECK_EXT("GL_EXT_secondary_color");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3BEXTPROC, glSecondaryColor3bEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3BVEXTPROC, glSecondaryColor3bvEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3DEXTPROC, glSecondaryColor3dEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3DVEXTPROC, glSecondaryColor3dvEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3FEXTPROC, glSecondaryColor3fEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3FVEXTPROC, glSecondaryColor3fvEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3IEXTPROC, glSecondaryColor3iEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3IVEXTPROC, glSecondaryColor3ivEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3SEXTPROC, glSecondaryColor3sEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3SVEXTPROC, glSecondaryColor3svEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3UBEXTPROC, glSecondaryColor3ubEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3UBVEXTPROC, glSecondaryColor3ubvEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3UIEXTPROC, glSecondaryColor3uiEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3UIVEXTPROC, glSecondaryColor3uivEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3USEXTPROC, glSecondaryColor3usEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLOR3USVEXTPROC, glSecondaryColor3usvEXT);
	CHECK_ADDRESS(PFNGLSECONDARYCOLORPOINTEREXTPROC, glSecondaryColorPointerEXT);
#endif

	return true;
}

// *********************************
static bool	setupWGLARBPBuffer(const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPBuffer);
	CHECK_EXT("WGL_ARB_pbuffer");

#ifndef USE_OPENGLES
#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLCREATEPBUFFERARBPROC, wglCreatePbufferARB);
	CHECK_ADDRESS(PFNWGLGETPBUFFERDCARBPROC, wglGetPbufferDCARB);
	CHECK_ADDRESS(PFNWGLRELEASEPBUFFERDCARBPROC, wglReleasePbufferDCARB);
	CHECK_ADDRESS(PFNWGLDESTROYPBUFFERARBPROC, wglDestroyPbufferARB);
	CHECK_ADDRESS(PFNWGLQUERYPBUFFERARBPROC, wglQueryPbufferARB);
#endif
#endif

	return true;
}

// *********************************
static bool	setupARBMultisample(const char	*glext)
{
	H_AUTO_OGL(setupARBMultisample);
	CHECK_EXT("GL_ARB_multisample");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLSAMPLECOVERAGEARBPROC, glSampleCoverageARB);
#endif

	return true;
}

#ifdef NL_OS_WINDOWS
// *********************************
static bool	setupWGLARBPixelFormat (const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPixelFormat);
	CHECK_EXT("WGL_ARB_pixel_format");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB);
	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBFVARBPROC, wglGetPixelFormatAttribfvARB);
	CHECK_ADDRESS(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);
#endif

	return true;
}
#endif

// *********************************
static bool	setupNVTextureShader(const char	*glext)
{
	H_AUTO_OGL(setupNVTextureShader);

// reenabled to allow bloom on mac, TODO: cleanly fix the water issue
// i think this issue was mtp target related - is this the case in ryzom too?
// #ifdef NL_OS_MAC
// // Water doesn't render on GeForce 8600M GT (on MAC OS X) if this extension is enabled
// 	return false;
// #endif

	CHECK_EXT("GL_NV_texture_shader");
	return true;
}


// *********************************
static bool	setupEXTBlendColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTBlendColor);
	CHECK_EXT("GL_EXT_blend_color");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLBLENDCOLOREXTPROC, glBlendColorEXT);
#endif

	return true;
}

// *********************************
static bool	setupNVVertexArrayRange2(const char	*glext)
{
	H_AUTO_OGL(setupNVVertexArrayRange2);
	CHECK_EXT("GL_NV_vertex_array_range2");
	return true;
}


// *********************************
static bool	setupATIVertexArrayObject(const char *glext)
{
	H_AUTO_OGL(setupATIVertexArrayObject);
	CHECK_EXT("GL_ATI_vertex_array_object");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLNEWOBJECTBUFFERATIPROC, glNewObjectBufferATI);
	CHECK_ADDRESS(PFNGLISOBJECTBUFFERATIPROC, glIsObjectBufferATI);
	CHECK_ADDRESS(PFNGLUPDATEOBJECTBUFFERATIPROC, glUpdateObjectBufferATI);
	CHECK_ADDRESS(PFNGLGETOBJECTBUFFERFVATIPROC, glGetObjectBufferfvATI);
	CHECK_ADDRESS(PFNGLGETOBJECTBUFFERIVATIPROC, glGetObjectBufferivATI);
	CHECK_ADDRESS(PFNGLFREEOBJECTBUFFERATIPROC, glFreeObjectBufferATI);
	CHECK_ADDRESS(PFNGLARRAYOBJECTATIPROC, glArrayObjectATI);
	CHECK_ADDRESS(PFNGLGETARRAYOBJECTFVATIPROC, glGetArrayObjectfvATI);
	CHECK_ADDRESS(PFNGLGETARRAYOBJECTIVATIPROC, glGetArrayObjectivATI);

	if(strstr(glext, "GL_EXT_vertex_shader") != NULL)
	{
		// the following exist only if ext vertex shader is present
		CHECK_ADDRESS(PFNGLVARIANTARRAYOBJECTATIPROC, glVariantArrayObjectATI);
		CHECK_ADDRESS(PFNGLGETVARIANTARRAYOBJECTFVATIPROC, glGetVariantArrayObjectfvATI);
		CHECK_ADDRESS(PFNGLGETVARIANTARRAYOBJECTIVATIPROC, glGetVariantArrayObjectivATI);
	}
#endif

	return true;
}


static bool	setupATIMapObjectBuffer(const char *glext)
{
	H_AUTO_OGL(setupATIMapObjectBuffer);
	CHECK_EXT("GL_ATI_map_object_buffer");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLMAPOBJECTBUFFERATIPROC, glMapObjectBufferATI);
	CHECK_ADDRESS(PFNGLUNMAPOBJECTBUFFERATIPROC, glUnmapObjectBufferATI);
#endif

	return true;
}



// *********************************
static bool	setupATIFragmentShader(const char *glext)
{
	H_AUTO_OGL(setupATIFragmentShader);
	CHECK_EXT("GL_ATI_fragment_shader");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLGENFRAGMENTSHADERSATIPROC, glGenFragmentShadersATI);
	CHECK_ADDRESS(PFNGLBINDFRAGMENTSHADERATIPROC, glBindFragmentShaderATI);
	CHECK_ADDRESS(PFNGLDELETEFRAGMENTSHADERATIPROC, glDeleteFragmentShaderATI);
	CHECK_ADDRESS(PFNGLBEGINFRAGMENTSHADERATIPROC, glBeginFragmentShaderATI);
	CHECK_ADDRESS(PFNGLENDFRAGMENTSHADERATIPROC, glEndFragmentShaderATI);
	CHECK_ADDRESS(PFNGLPASSTEXCOORDATIPROC, glPassTexCoordATI);
	CHECK_ADDRESS(PFNGLSAMPLEMAPATIPROC, glSampleMapATI);
	CHECK_ADDRESS(PFNGLCOLORFRAGMENTOP1ATIPROC, glColorFragmentOp1ATI);
	CHECK_ADDRESS(PFNGLCOLORFRAGMENTOP2ATIPROC, glColorFragmentOp2ATI);
	CHECK_ADDRESS(PFNGLCOLORFRAGMENTOP3ATIPROC, glColorFragmentOp3ATI);
	CHECK_ADDRESS(PFNGLALPHAFRAGMENTOP1ATIPROC, glAlphaFragmentOp1ATI);
	CHECK_ADDRESS(PFNGLALPHAFRAGMENTOP2ATIPROC, glAlphaFragmentOp2ATI);
	CHECK_ADDRESS(PFNGLALPHAFRAGMENTOP3ATIPROC, glAlphaFragmentOp3ATI);
	CHECK_ADDRESS(PFNGLSETFRAGMENTSHADERCONSTANTATIPROC, glSetFragmentShaderConstantATI);
#endif

	return true;
}

// *********************************
static bool setupATIVertexAttribArrayObject(const char *glext)
{
	H_AUTO_OGL(setupATIVertexAttribArrayObject);
	CHECK_EXT("GL_ATI_vertex_attrib_array_object");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLVERTEXATTRIBARRAYOBJECTATIPROC, glVertexAttribArrayObjectATI);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC, glGetVertexAttribArrayObjectfvATI);
	CHECK_ADDRESS(PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC, glGetVertexAttribArrayObjectivATI);
#endif

	return true;
}

// *********************************
static bool	setupARBFragmentProgram(const char *glext)
{
	H_AUTO_OGL(setupARBFragmentProgram);
	CHECK_EXT("GL_ARB_fragment_program");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLPROGRAMSTRINGARBPROC, glProgramStringARB);
	CHECK_ADDRESS(PFNGLBINDPROGRAMARBPROC, glBindProgramARB);
	CHECK_ADDRESS(PFNGLDELETEPROGRAMSARBPROC, glDeleteProgramsARB);
	CHECK_ADDRESS(PFNGLGENPROGRAMSARBPROC, glGenProgramsARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4DARBPROC, glProgramEnvParameter4dARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4DVARBPROC, glProgramEnvParameter4dvARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4FARBPROC, glProgramEnvParameter4fARB);
	CHECK_ADDRESS(PFNGLPROGRAMENVPARAMETER4FVARBPROC, glProgramEnvParameter4fvARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4DARBPROC, glProgramLocalParameter4dARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4DVARBPROC, glProgramLocalParameter4dvARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4FARBPROC, glProgramLocalParameter4fARB);
	CHECK_ADDRESS(PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMENVPARAMETERDVARBPROC, glGetProgramEnvParameterdvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMENVPARAMETERFVARBPROC, glGetProgramEnvParameterfvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC, glGetProgramLocalParameterdvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC, glGetProgramLocalParameterfvARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMIVARBPROC, glGetProgramivARB);
	CHECK_ADDRESS(PFNGLGETPROGRAMSTRINGARBPROC, glGetProgramStringARB);
	CHECK_ADDRESS(PFNGLISPROGRAMARBPROC, glIsProgramARB);
#endif

	return true;
}

// *********************************
static bool	setupNVFragmentProgram2(const char *glext)
{
	H_AUTO_OGL(setupNVFragmentProgram2);
	CHECK_EXT("GL_NV_fragment_program2");

	return true;
}

// *********************************
static bool	setupARBFragmentShader(const char *glext)
{
	H_AUTO_OGL(setupNVFragmentProgram2);
	CHECK_EXT("GL_ARB_fragment_shader");

	return true;
}

// ***************************************************************************
static bool	setupARBVertexBufferObject(const char	*glext)
{
	H_AUTO_OGL(setupARBVertexBufferObject);

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_vertex_buffer_object");

	CHECK_ADDRESS(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
	CHECK_ADDRESS(PFNGLDELETEBUFFERSARBPROC, glDeleteBuffersARB);
	CHECK_ADDRESS(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
	CHECK_ADDRESS(PFNGLISBUFFERARBPROC, glIsBufferARB);
	CHECK_ADDRESS(PFNGLBUFFERDATAARBPROC, glBufferDataARB);
	CHECK_ADDRESS(PFNGLBUFFERSUBDATAARBPROC, glBufferSubDataARB);
	CHECK_ADDRESS(PFNGLGETBUFFERSUBDATAARBPROC, glGetBufferSubDataARB);
	CHECK_ADDRESS(PFNGLMAPBUFFERARBPROC, glMapBufferARB);
	CHECK_ADDRESS(PFNGLUNMAPBUFFERARBPROC, glUnmapBufferARB);
	CHECK_ADDRESS(PFNGLGETBUFFERPARAMETERIVARBPROC, glGetBufferParameterivARB);
	CHECK_ADDRESS(PFNGLGETBUFFERPOINTERVARBPROC, glGetBufferPointervARB);
#endif

	return true;
}

// ***************************************************************************
static bool	setupARBMapBufferRange(const char	*glext)
{
	H_AUTO_OGL(setupARBMapBufferRange);

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_map_buffer_range");

	CHECK_ADDRESS(PFNGLMAPBUFFERRANGEPROC, glMapBufferRange);
	CHECK_ADDRESS(PFNGLFLUSHMAPPEDBUFFERRANGEPROC, glFlushMappedBufferRange);
#endif

	return true;
}

// ***************************************************************************
static bool	setupARBVertexProgram(const char	*glext)
{
	H_AUTO_OGL(setupARBVertexProgram);
	CHECK_EXT("GL_ARB_vertex_program");

#ifndef USE_OPENGLES
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
#endif

	return true;
}

// ***************************************************************************
static bool	setupNVOcclusionQuery(const char	*glext)
{
	H_AUTO_OGL(setupNVOcclusionQuery);
	CHECK_EXT("GL_NV_occlusion_query");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLGENOCCLUSIONQUERIESNVPROC, glGenOcclusionQueriesNV);
	CHECK_ADDRESS(PFNGLDELETEOCCLUSIONQUERIESNVPROC, glDeleteOcclusionQueriesNV);
	CHECK_ADDRESS(PFNGLISOCCLUSIONQUERYNVPROC, glIsOcclusionQueryNV);
	CHECK_ADDRESS(PFNGLBEGINOCCLUSIONQUERYNVPROC, glBeginOcclusionQueryNV);
	CHECK_ADDRESS(PFNGLENDOCCLUSIONQUERYNVPROC, glEndOcclusionQueryNV);
	CHECK_ADDRESS(PFNGLGETOCCLUSIONQUERYIVNVPROC, glGetOcclusionQueryivNV);
	CHECK_ADDRESS(PFNGLGETOCCLUSIONQUERYUIVNVPROC, glGetOcclusionQueryuivNV);
#endif

	return true;
}

// ***************************************************************************
static bool	setupARBOcclusionQuery(const char	*glext)
{
	H_AUTO_OGL(setupARBOcclusionQuery);
	CHECK_EXT("ARB_occlusion_query");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLGENQUERIESARBPROC, glGenQueriesARB);
	CHECK_ADDRESS(PFNGLDELETEQUERIESARBPROC, glDeleteQueriesARB);
	CHECK_ADDRESS(PFNGLISQUERYARBPROC, glIsQueryARB);
	CHECK_ADDRESS(PFNGLBEGINQUERYARBPROC, glBeginQueryARB);
	CHECK_ADDRESS(PFNGLENDQUERYARBPROC, glEndQueryARB);
	CHECK_ADDRESS(PFNGLGETQUERYIVARBPROC, glGetQueryivARB);
	CHECK_ADDRESS(PFNGLGETQUERYOBJECTIVARBPROC, glGetQueryObjectivARB);
	CHECK_ADDRESS(PFNGLGETQUERYOBJECTUIVARBPROC, glGetQueryObjectuivARB);
#endif

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

#ifndef USE_OPENGLES
	CHECK_EXT("GL_ARB_texture_rectangle");
#endif

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

#ifdef USE_OPENGLES
	CHECK_EXT("GL_OES_framebuffer_object");

	CHECK_ADDRESS(PFNGLISRENDERBUFFEROESPROC, glIsRenderbufferOES);
	CHECK_ADDRESS(PFNGLBINDRENDERBUFFEROESPROC, glBindRenderbufferOES);
	CHECK_ADDRESS(PFNGLDELETERENDERBUFFERSOESPROC, glDeleteRenderbuffersOES);
	CHECK_ADDRESS(PFNGLGENRENDERBUFFERSOESPROC, glGenRenderbuffersOES);
	CHECK_ADDRESS(PFNGLRENDERBUFFERSTORAGEOESPROC, glRenderbufferStorageOES);
	CHECK_ADDRESS(PFNGLGETRENDERBUFFERPARAMETERIVOESPROC, glGetRenderbufferParameterivOES);
	CHECK_ADDRESS(PFNGLISFRAMEBUFFEROESPROC, glIsFramebufferOES);
	CHECK_ADDRESS(PFNGLBINDFRAMEBUFFEROESPROC, glBindFramebufferOES);
	CHECK_ADDRESS(PFNGLDELETEFRAMEBUFFERSOESPROC, glDeleteFramebuffersOES);
	CHECK_ADDRESS(PFNGLGENFRAMEBUFFERSOESPROC, glGenFramebuffersOES);
	CHECK_ADDRESS(PFNGLCHECKFRAMEBUFFERSTATUSOESPROC, glCheckFramebufferStatusOES);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERRENDERBUFFEROESPROC, glFramebufferRenderbufferOES);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURE2DOESPROC, glFramebufferTexture2DOES);
	CHECK_ADDRESS(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC, glGetFramebufferAttachmentParameterivOES);
	CHECK_ADDRESS(PFNGLGENERATEMIPMAPOESPROC, glGenerateMipmapOES);
#else
	CHECK_EXT("GL_EXT_framebuffer_object");

	CHECK_ADDRESS(PFNGLISRENDERBUFFEREXTPROC, glIsRenderbufferEXT);
	CHECK_ADDRESS(PFNGLISFRAMEBUFFEREXTPROC, glIsFramebufferEXT);
	CHECK_ADDRESS(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
	CHECK_ADDRESS(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT);
	CHECK_ADDRESS(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);
	CHECK_ADDRESS(PFNGLGENRENDERBUFFERSEXTPROC, glGenRenderbuffersEXT);
	CHECK_ADDRESS(PFNGLBINDRENDERBUFFEREXTPROC, glBindRenderbufferEXT);
	CHECK_ADDRESS(PFNGLRENDERBUFFERSTORAGEEXTPROC, glRenderbufferStorageEXT);
	CHECK_ADDRESS(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC, glFramebufferRenderbufferEXT);
	CHECK_ADDRESS(PFNGLDELETERENDERBUFFERSEXTPROC, glDeleteRenderbuffersEXT);
	CHECK_ADDRESS(PFNGLDELETEFRAMEBUFFERSEXTPROC, glDeleteFramebuffersEXT);
	CHECK_ADDRESS(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC, glGetRenderbufferParameterivEXT);
	CHECK_ADDRESS(PFNGLGENERATEMIPMAPEXTPROC, glGenerateMipmapEXT);
#endif

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferBlit(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferBlit);
	CHECK_EXT("GL_EXT_framebuffer_blit");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLBLITFRAMEBUFFEREXTPROC, glBlitFramebufferEXT);
#endif

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferMultisample(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferMultisample);
	CHECK_EXT("GL_EXT_framebuffer_multisample");

#ifndef USE_OPENGLES
	CHECK_ADDRESS(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC, glRenderbufferStorageMultisampleEXT);
#endif

	return true;
}

// ***************************************************************************
static bool	setupPackedDepthStencil(const char	*glext)
{
	H_AUTO_OGL(setupPackedDepthStencil);

#ifdef USE_OPENGLES
	CHECK_EXT("GL_OES_packed_depth_stencil");
#else
	CHECK_EXT("GL_EXT_packed_depth_stencil");
#endif

	return true;
}

// ***************************************************************************
static bool	setupNVXGPUMemoryInfo(const char *glext)
{
	H_AUTO_OGL(setupNVXGPUMemoryInfo);
	CHECK_EXT("GL_NVX_gpu_memory_info");
	return true;
}

// ***************************************************************************
static bool	setupATIMeminfo(const char *glext)
{
	H_AUTO_OGL(setupATIMeminfo);
	CHECK_EXT("GL_ATI_meminfo");
	return true;
}

// ***************************************************************************
// Extension Check.
void	registerGlExtensions(CGlExtensions &ext)
{
	H_AUTO_OGL(registerGlExtensions);

#ifdef NL_OS_MAC
	CGLContextObj ctx = CGLGetCurrentContext();
	if (ctx == NULL)
	{
		nlerror("No OpenGL context set");
	}
#endif

	// OpenGL 1.2 ??
	const char	*nglVersion = (const char *)glGetString (GL_VERSION);

	if (nglVersion)
	{
		sint a = 0, b = 0;

		// 1.2***  ???
		sscanf(nglVersion, "%d.%d", &a, &b);
		ext.Version1_2 = (a==1 && b>=2) || (a>=2);
	}
	else
	{
		nlwarning("3D: Unable to get GL_VERSION, OpenGL 1.2 should be supported on all recent GPU...");
		ext.Version1_2 = true;
	}

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);

	// Log GPU details
	nlinfo("3D: OpenGL %s / %s / %s", nglVersion, vendor, renderer);

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

	// Check NVVertexArrayRange
	// Disable feature ???
	if(!ext.DisableHardwareVertexArrayAGP)
		ext.NVVertexArrayRange= setupNVVertexArrayRange(glext);

	if(ext.NVVertexArrayRange)
	{
		GLint nverts = 10;
#ifndef USE_OPENGLES
		glGetIntegerv((GLenum)GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV, &nverts);
#endif
		ext.NVVertexArrayRangeMaxVertex= nverts;
	}

	// Compression S3TC OK iff ARBTextureCompression.
	ext.EXTTextureCompressionS3TC= (ext.ARBTextureCompression && setupEXTTextureCompressionS3TC(glext));

	// Check if NVidia GL_EXT_vertex_weighting is available.
	ext.EXTVertexWeighting= setupEXTVertexWeighting(glext);

	// Check EXTSeparateSpecularColor.
	ext.EXTSeparateSpecularColor= setupEXTSeparateSpecularColor(glext);

	// Check NVTextureEnvCombine4.
	ext.NVTextureEnvCombine4= setupNVTextureEnvCombine4(glext);

	// Check for cube mapping
	ext.ARBTextureCubeMap = setupARBTextureCubeMap(glext);

	// Check vertex program
	// Disable feature ???
	if(!ext.DisableHardwareVertexProgram)
	{
		ext.NVVertexProgram = setupNVVertexProgram(glext);
		ext.EXTVertexShader = setupEXTVertexShader(glext);
		ext.ARBVertexProgram = setupARBVertexProgram(glext);
	}
	else
	{
		ext.NVVertexProgram = false;
		ext.EXTVertexShader = false;
		ext.ARBVertexProgram = false;
	}

	// Check pixel program
	// Disable feature ???
	if (!ext.DisableHardwarePixelProgram)
	{
		ext.ARBFragmentProgram = setupARBFragmentProgram(glext);
		ext.NVFragmentProgram2 = setupNVFragmentProgram2(glext);
		ext.ARBFragmentShader = setupARBFragmentShader(glext);
	}
	else
	{
		ext.ARBFragmentProgram = false;
		ext.NVFragmentProgram2 = false;
		ext.ARBFragmentShader = false;
	}

	ext.OESDrawTexture = setupOESDrawTexture(glext);
	ext.OESMapBuffer = setupOESMapBuffer(glext);

	// Check texture shaders
	// Disable feature ???
	if(!ext.DisableHardwareTextureShader)
	{
		ext.NVTextureShader = setupNVTextureShader(glext);
		ext.ATIEnvMapBumpMap = setupATIEnvMapBumpMap(glext);
		ext.ATIFragmentShader = setupATIFragmentShader(glext);
	}
	else
	{
		ext.ATIEnvMapBumpMap = false;
		ext.NVTextureShader = false;
		ext.ATIFragmentShader = false;
	}

	// For now, the only way to know if emulation, is to test some extension which exist only on GeForce3.
	// if GL_NV_texture_shader is not here, then we are not on GeForce3.
	ext.NVVertexProgramEmulated= ext.NVVertexProgram && (strstr(glext, "GL_NV_texture_shader")==NULL);

	// Check EXTSecondaryColor
	ext.EXTSecondaryColor= setupEXTSecondaryColor(glext);

	// Check EXTBlendColor
	ext.EXTBlendColor= setupEXTBlendColor(glext);

	// Check NVVertexArrayRange2
	ext.NVVertexArrayRange2= setupNVVertexArrayRange2(glext);

#ifdef GL_NV_vertex_array_range2
	// if supported
	if(ext.NVVertexArrayRange2)
		// VBHard swap without flush of the VAR.
		ext.NVStateVARWithoutFlush= GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV;
	else
		// VBHard with useless flush of the VAR.
		ext.NVStateVARWithoutFlush= GL_VERTEX_ARRAY_RANGE_NV;
#endif

	// Check NV_occlusion_query
	ext.NVOcclusionQuery = setupNVOcclusionQuery(glext);

	// Check ARB_occlusion_query
	ext.ARBOcclusionQuery = setupARBOcclusionQuery(glext);

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

	// Check ATIVertexArrayObject
	// Disable feature ???
	if(!ext.DisableHardwareVertexArrayAGP)
	{
		ext.ATIVertexArrayObject= setupATIVertexArrayObject(glext);
		ext.ATIMapObjectBuffer= setupATIMapObjectBuffer(glext);
		ext.ATIVertexAttribArrayObject = setupATIVertexAttribArrayObject(glext);
	}
	// Check ATIXTextureEnvCombine3.
	ext.ATITextureEnvCombine3= setupATITextureEnvCombine3(glext);
	// Check ATIXTextureEnvRoute
	ext.ATIXTextureEnvRoute= setupATIXTextureEnvRoute(glext);

	// ARB extensions
	// -------------
	if (!ext.DisableHardwareVertexArrayAGP)
	{
		ext.ARBVertexBufferObject = setupARBVertexBufferObject(glext);
		if (ext.ARBVertexBufferObject)
		{
			ext.ARBMapBufferRange = setupARBMapBufferRange(glext);
		}
	}

	// fix for radeon 7200 -> disable agp
	if (ext.NbTextureStages == 3 && (ext.ATIVertexArrayObject || ext.ARBVertexBufferObject))
	{
		ext.ATIVertexArrayObject = false;
		ext.ARBVertexBufferObject = false;
		ext.ATIMapObjectBuffer = false;
		ext.ATIVertexAttribArrayObject = false;
	}

#ifndef USE_OPENGLES
	ext.NVXGPUMemoryInfo = setupNVXGPUMemoryInfo(glext);
	ext.ATIMeminfo = setupATIMeminfo(glext);
#endif
}


// *********************************
static bool	setupWGLEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupWGLEXTSwapControl);
	CHECK_EXT("WGL_EXT_swap_control");

#ifndef USE_OPENGLES
#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
	CHECK_ADDRESS(PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);
#endif
#endif

	return true;
}

// ***************************************************************************
static bool	setupWGLAMDGPUAssociation(const char *glext)
{
	H_AUTO_OGL(setupWGLAMDGPUAssociation);
	CHECK_EXT("WGL_AMD_gpu_association");

#if !defined(USE_OPENGLES) && defined(NL_OS_WINDOWS)
	CHECK_ADDRESS(PFNWGLGETGPUIDSAMDPROC, wglGetGPUIDsAMD);
	CHECK_ADDRESS(PFNWGLGETGPUINFOAMDPROC, wglGetGPUInfoAMD);
	CHECK_ADDRESS(PFNWGLGETCONTEXTGPUIDAMDPROC, wglGetContextGPUIDAMD);
	CHECK_ADDRESS(PFNWGLCREATEASSOCIATEDCONTEXTAMDPROC, wglCreateAssociatedContextAMD);
	CHECK_ADDRESS(PFNWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC, wglCreateAssociatedContextAttribsAMD);
	CHECK_ADDRESS(PFNWGLDELETEASSOCIATEDCONTEXTAMDPROC, wglDeleteAssociatedContextAMD);
	CHECK_ADDRESS(PFNWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC, wglMakeAssociatedContextCurrentAMD);
	CHECK_ADDRESS(PFNWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC, wglGetCurrentAssociatedContextAMD);
	CHECK_ADDRESS(PFNWGLBLITCONTEXTFRAMEBUFFERAMDPROC, wglBlitContextFramebufferAMD);
#endif

	return true;
}

// ***************************************************************************
static bool	setupWGLNVGPUAssociation(const char *glext)
{
	H_AUTO_OGL(setupWGLNVGPUAssociation);
	CHECK_EXT("WGL_NV_gpu_affinity");

#if !defined(USE_OPENGLES) && defined(NL_OS_WINDOWS)
	CHECK_ADDRESS(PFNWGLENUMGPUSNVPROC, wglEnumGpusNV);
	CHECK_ADDRESS(PFNWGLENUMGPUDEVICESNVPROC, wglEnumGpuDevicesNV);
	CHECK_ADDRESS(PFNWGLCREATEAFFINITYDCNVPROC, wglCreateAffinityDCNV);
	CHECK_ADDRESS(PFNWGLENUMGPUSFROMAFFINITYDCNVPROC, wglEnumGpusFromAffinityDCNV);
	CHECK_ADDRESS(PFNWGLDELETEDCNVPROC, wglDeleteDCNV);
#endif

	return true;
}

static bool	setupGLXEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXEXTSwapControl);
	CHECK_EXT("GLX_EXT_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(PFNGLXSWAPINTERVALEXTPROC, glXSwapIntervalEXT);
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
	CHECK_ADDRESS(PFNGLXSWAPINTERVALMESAPROC, glXSwapIntervalMESA);
	CHECK_ADDRESS(PFNGLXGETSWAPINTERVALMESAPROC, glXGetSwapIntervalMESA);
#endif

	return true;
}

// *********************************
static bool	setupGLXMESAQueryRenderer(const char	*glext)
{
	H_AUTO_OGL(setupGLXMESAQueryRenderer);
	CHECK_EXT("GLX_MESA_query_renderer");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC, glXQueryCurrentRendererIntegerMESA);
#endif

	return true;
}

#ifdef USE_OPENGLES
// ***************************************************************************
bool registerEGlExtensions(CGlExtensions &ext, EGLDisplay dpy)
{
	H_AUTO_OGL(registerEGlExtensions);

	// Get extension string
	const char *glext = eglQueryString(dpy, EGL_EXTENSIONS);
	if (glext == NULL)
	{
		nlwarning ("neglGetExtensionsStringARB failed");
		return false;
	}

	nldebug("3D: Available EGL Extensions:");

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

	return true;
}
#elif defined(NL_OS_WINDOWS)
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

	ext.WGLAMDGPUAssociation = setupWGLAMDGPUAssociation(glext);

	ext.WGLNVGPUAffinity = setupWGLNVGPUAssociation(glext);

	if (ext.WGLNVGPUAffinity)
	{
		uint gpuIndex = 0;

		HGPUNV hGPU;

		// list all GPUs
		while (nwglEnumGpusNV(gpuIndex, &hGPU))
		{
			uint j = 0;

			_GPU_DEVICE gpuDevice;
			gpuDevice.cb = sizeof(gpuDevice);

			// list all devices connected to GPU
			while(nwglEnumGpuDevicesNV(hGPU, j, &gpuDevice))
			{
				nlinfo("Device: %s / %s / flags: %u", gpuDevice.DeviceName, gpuDevice.DeviceString, (uint)gpuDevice.Flags);

				if (gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
				{
					nlinfo("Virtual screen: (%d,%d)-(%d,%d)", (sint)gpuDevice.rcVirtualScreen.left, (sint)gpuDevice.rcVirtualScreen.top, (sint)gpuDevice.rcVirtualScreen.right, (sint)gpuDevice.rcVirtualScreen.bottom);
				}

				++j;
			}

			++gpuIndex;
		}
	}

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

	// check for renderer information
	ext.GLXMESAQueryRenderer= setupGLXMESAQueryRenderer(glext);

	return true;
}
#endif // USE_OPENGLES

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
