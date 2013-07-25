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

#ifndef NL_OPENGL_EXTENSION_DEF_H
#define NL_OPENGL_EXTENSION_DEF_H


#include "nel/misc/types_nl.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_OPENGLES
// OES_mapbuffer
//==============
typedef void* (APIENTRY * NEL_PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * NEL_PFNGLUNMAPBUFFEROESPROC) (GLenum target);
typedef void (APIENTRY * NEL_PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, void** params);

typedef void (APIENTRY * NEL_PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);

// GL_OES_framebuffer_object
//==================================
typedef GLboolean (APIENTRY * NEL_PFNGLISRENDERBUFFEROESPROC) (GLuint renderbuffer);
typedef void (APIENTRY * NEL_PFNGLBINDRENDERBUFFEROESPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRY * NEL_PFNGLDELETERENDERBUFFERSOESPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void (APIENTRY * NEL_PFNGLGENRENDERBUFFERSOESPROC) (GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRY * NEL_PFNGLRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY * NEL_PFNGLGETRENDERBUFFERPARAMETERIVOESPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLboolean (APIENTRY * NEL_PFNGLISFRAMEBUFFEROESPROC) (GLuint framebuffer);
typedef void (APIENTRY * NEL_PFNGLBINDFRAMEBUFFEROESPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY * NEL_PFNGLDELETEFRAMEBUFFERSOESPROC) (GLsizei n, const GLuint* framebuffers);
typedef void (APIENTRY * NEL_PFNGLGENFRAMEBUFFERSOESPROC) (GLsizei n, GLuint* framebuffers);
typedef GLenum (APIENTRY * NEL_PFNGLCHECKFRAMEBUFFERSTATUSOESPROC) (GLenum target);
typedef void (APIENTRY * NEL_PFNGLFRAMEBUFFERRENDERBUFFEROESPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY * NEL_PFNGLFRAMEBUFFERTEXTURE2DOESPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY * NEL_PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void (APIENTRY * NEL_PFNGLGENERATEMIPMAPOESPROC) (GLenum target);

// GL_OES_texture_cube_map
//==================================
typedef void (APIENTRY * NEL_PFNGLTEXGENFOESPROC) (GLenum coord, GLenum pname, GLfloat param);
typedef void (APIENTRY * NEL_PFNGLTEXGENFVOESPROC) (GLenum coord, GLenum pname, const GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLTEXGENIOESPROC) (GLenum coord, GLenum pname, GLint param);
typedef void (APIENTRY * NEL_PFNGLTEXGENIVOESPROC) (GLenum coord, GLenum pname, const GLint *params);
typedef void (APIENTRY * NEL_PFNGLTEXGENXOESPROC) (GLenum coord, GLenum pname, GLfixed param);
typedef void (APIENTRY * NEL_PFNGLTEXGENXVOESPROC) (GLenum coord, GLenum pname, const GLfixed *params);
typedef void (APIENTRY * NEL_PFNGLGETTEXGENFVOESPROC) (GLenum coord, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETTEXGENIVOESPROC) (GLenum coord, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETTEXGENXVOESPROC) (GLenum coord, GLenum pname, GLfixed *params);

#define GL_MULTISAMPLE_ARB GL_MULTISAMPLE
#define GL_TEXTURE_CUBE_MAP_ARB GL_TEXTURE_CUBE_MAP_OES
#define GL_NONE 0
#define GL_MAX_TEXTURE_UNITS_ARB GL_MAX_TEXTURE_UNITS
#define GL_REFLECTION_MAP_ARB GL_REFLECTION_MAP_OES
#define GL_RGB_SCALE_EXT GL_RGB_SCALE
#define GL_REFLECTION_MAP_ARB GL_REFLECTION_MAP_OES
#define GL_PREVIOUS_EXT GL_PREVIOUS
#define GL_PRIMARY_COLOR_EXT GL_PRIMARY_COLOR
#define GL_CONSTANT_EXT GL_CONSTANT
#define GL_ADD_SIGNED_EXT GL_ADD_SIGNED
#define GL_INTERPOLATE_EXT GL_INTERPOLATE
#define GL_BUMP_ENVMAP_ATI GL_INTERPOLATE

#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES

#else

// ***************************************************************************
// ***************************************************************************
// The NEL Functions Typedefs.
// Must do it for compatibilities with futures version of gl.h
// eg: version 1.2 does not define PFNGLACTIVETEXTUREARBPROC. Hence, do it now, with our special name
// ***************************************************************************
// ***************************************************************************

#define WGL_COVERAGE_SAMPLES_NV            0x2042
#define WGL_COLOR_SAMPLES_NV               0x20B9

// ARB_multitexture
//=================
typedef void (APIENTRY * NEL_PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * NEL_PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);


// ARB_TextureCompression.
//========================
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);


// VertexArrayRangeNV.
//====================
typedef void (APIENTRY * NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (APIENTRY * NEL_PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);


// FenceNV.
//====================
typedef void (APIENTRY * NEL_PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef void (APIENTRY * NEL_PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef GLboolean (APIENTRY * NEL_PFNGLISFENCENVPROC) (GLuint fence);
typedef GLboolean (APIENTRY * NEL_PFNGLTESTFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * NEL_PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLFINISHFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * NEL_PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);


// VertexWeighting.
//==================
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);


// VertexProgramExtension.
//========================
typedef GLboolean (APIENTRY * NEL_PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
typedef void (APIENTRY * NEL_PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
typedef void (APIENTRY * NEL_PFNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
typedef void (APIENTRY * NEL_PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef GLboolean (APIENTRY * NEL_PFNGLISPROGRAMNVPROC) (GLuint id);
typedef void (APIENTRY * NEL_PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);

// VertexShaderExtension (EXT)
//============================
typedef void	(APIENTRY * NEL_PFNGLBEGINVERTEXSHADEREXTPROC) ( void );
typedef void	(APIENTRY * NEL_PFNGLENDVERTEXSHADEREXTPROC) ( void );
typedef void	(APIENTRY * NEL_PFNGLBINDVERTEXSHADEREXTPROC) ( GLuint id );
typedef GLuint	(APIENTRY * NEL_PFNGLGENVERTEXSHADERSEXTPROC) ( GLuint range );
typedef void	(APIENTRY * NEL_PFNGLDELETEVERTEXSHADEREXTPROC) ( GLuint id );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP1EXTPROC) ( GLenum op, GLuint res, GLuint arg1 );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP2EXTPROC) ( GLenum op, GLuint res, GLuint arg1, GLuint arg2 );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP3EXTPROC) ( GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3 );
typedef void	(APIENTRY * NEL_PFNGLSWIZZLEEXTPROC) ( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW );
typedef void	(APIENTRY * NEL_PFNGLWRITEMASKEXTPROC) ( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW );
typedef void	(APIENTRY * NEL_PFNGLINSERTCOMPONENTEXTPROC) ( GLuint res, GLuint src, GLuint num );
typedef void	(APIENTRY * NEL_PFNGLEXTRACTCOMPONENTEXTPROC) ( GLuint res, GLuint src, GLuint num );
typedef GLuint	(APIENTRY * NEL_PFNGLGENSYMBOLSEXTPROC) ( GLenum datatype, GLenum storagetype, GLenum range, GLuint components ) ;
typedef void	(APIENTRY * NEL_PFNGLSETINVARIANTEXTPROC) ( GLuint id, GLenum type, void *addr );
typedef void	(APIENTRY * NEL_PFNGLSETLOCALCONSTANTEXTPROC) ( GLuint id, GLenum type, void *addr );
typedef void	(APIENTRY * NEL_PFNGLVARIANTPOINTEREXTPROC) ( GLuint id, GLenum type, GLuint stride, void *addr );
typedef void	(APIENTRY * NEL_PFNGLENABLEVARIANTCLIENTSTATEEXTPROC) ( GLuint id);
typedef void	(APIENTRY * NEL_PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC) ( GLuint id);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDLIGHTPARAMETEREXTPROC) ( GLenum light, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDMATERIALPARAMETEREXTPROC) ( GLenum face, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDTEXGENPARAMETEREXTPROC) ( GLenum unit, GLenum coord, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDTEXTUREUNITPARAMETEREXTPROC) ( GLenum unit, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDPARAMETEREXTPROC) ( GLenum value);
typedef GLboolean (APIENTRY * NEL_PFNGLISVARIANTENABLEDEXTPROC) ( GLuint id, GLenum cap);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTPOINTERVEXTPROC) ( GLuint id, GLenum value, void **data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);


// SecondaryColor extension
//========================
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green, GLint blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3IVEXTPROC) (const GLint *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3USEXTPROC) (GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer);


// BlendColor extension
//========================
typedef void (APIENTRY * NEL_PFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);


// GL_ATI_vertex_array_object extension
//========================
typedef GLuint (APIENTRY * NEL_PFNGLNEWOBJECTBUFFERATIPROC) (GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (APIENTRY * NEL_PFNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * NEL_PFNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef void (APIENTRY * NEL_PFNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLDELETEOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * NEL_PFNGLARRAYOBJECTATIPROC) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * NEL_PFNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * NEL_PFNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id, GLenum pname, GLint *params);


// GL_ATI_fragment_shader extension
//==================================
typedef GLuint (APIENTRY *NEL_PFNGLGENFRAGMENTSHADERSATIPROC)(GLuint range);
typedef GLvoid (APIENTRY *NEL_PFNGLBINDFRAGMENTSHADERATIPROC)(GLuint id);
typedef GLvoid (APIENTRY *NEL_PFNGLDELETEFRAGMENTSHADERATIPROC)(GLuint id);
typedef GLvoid (APIENTRY *NEL_PFNGLBEGINFRAGMENTSHADERATIPROC)();
typedef GLvoid (APIENTRY *NEL_PFNGLENDFRAGMENTSHADERATIPROC)();
typedef GLvoid (APIENTRY *NEL_PFNGLPASSTEXCOORDATIPROC)(GLuint dst, GLuint coord, GLenum swizzle);
typedef GLvoid (APIENTRY *NEL_PFNGLSAMPLEMAPATIPROC)(GLuint dst, GLuint interp, GLenum swizzle);
typedef GLvoid (APIENTRY *NEL_PFNGLCOLORFRAGMENTOP1ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
															GLuint dstMod, GLuint arg1, GLuint arg1Rep,
															GLuint arg1Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLCOLORFRAGMENTOP2ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
															GLuint dstMod, GLuint arg1, GLuint arg1Rep,
															GLuint arg1Mod, GLuint arg2, GLuint arg2Rep,
															GLuint arg2Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLCOLORFRAGMENTOP3ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
															GLuint dstMod, GLuint arg1, GLuint arg1Rep,
															GLuint arg1Mod, GLuint arg2, GLuint arg2Rep,
															GLuint arg2Mod, GLuint arg3, GLuint arg3Rep,
															GLuint arg3Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLALPHAFRAGMENTOP1ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
									                        GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLALPHAFRAGMENTOP2ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
															GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
															GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLALPHAFRAGMENTOP3ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
															GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
															GLuint arg2, GLuint arg2Rep, GLuint arg2Mod,
															GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef GLvoid (APIENTRY *NEL_PFNGLSETFRAGMENTSHADERCONSTANTATIPROC)(GLuint dst, const GLfloat *value);



// GL_ATI_map_object_buffer
//==================================
typedef void *(APIENTRY * NEL_PFNGLMAPOBJECTBUFFERATIPROC)(GLuint buffer);
typedef void (APIENTRY * NEL_PFNGLUNMAPOBJECTBUFFERATIPROC)(GLuint buffer);


// GL_ATI_vertex_attrib_array_object
//==================================

typedef GLvoid (APIENTRY * NEL_PFNGLVERTEXATTRIBARRAYOBJECTATIPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset);
typedef GLvoid (APIENTRY * NEL_PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC)(GLuint index, GLenum pname, GLfloat *params);
typedef GLvoid (APIENTRY * NEL_PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC)(GLuint index, GLenum pname, GLint *params);




// GL_ARB_fragment_program
//==================================
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMSTRINGARBPROC)(GLenum target, GLenum format, GLsizei len,const GLvoid *string);
typedef GLvoid (APIENTRY *NEL_PFNGLBINDPROGRAMARBPROC)(GLenum target, GLuint program);
typedef GLvoid (APIENTRY *NEL_PFNGLDELETEPROGRAMSARBPROC)(GLsizei n, const GLuint *programs);
typedef GLvoid (APIENTRY *NEL_PFNGLGENPROGRAMSARBPROC)(GLsizei n, GLuint *programs);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMENVPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMENVPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMENVPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMENVPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMLOCALPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMLOCALPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *NEL_PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMENVPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMENVPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMIVARBPROC)(GLenum target, GLenum pname, int *params);
typedef GLvoid (APIENTRY *NEL_PFNGLGETPROGRAMSTRINGARBPROC)(GLenum target, GLenum pname, GLvoid *string);
typedef GLboolean (APIENTRY *NEL_PFNGLISPROGRAMARBPROC)(GLuint program);


typedef GLboolean (APIENTRY * NEL_PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef GLboolean (APIENTRY * NEL_PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef GLenum (APIENTRY * NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum pname);
typedef GLvoid (APIENTRY * NEL_PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLvoid (APIENTRY * NEL_PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef GLvoid (APIENTRY * NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLvoid (APIENTRY * NEL_PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef GLvoid (APIENTRY * NEL_PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef GLvoid (APIENTRY * NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef GLvoid (APIENTRY * NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLvoid (APIENTRY * NEL_PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef GLvoid (APIENTRY * NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef GLvoid (APIENTRY * NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY * NEL_PFNGENERATEMIPMAPEXTPROC) (GLenum target);

typedef GLvoid (APIENTRY * NEL_PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

typedef GLvoid (APIENTRY * NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

#ifndef NL_GL_NV_occlusion_query
#define NL_GL_NV_occlusion_query              1

typedef GLvoid    (APIENTRY * NEL_PFNGLGENOCCLUSIONQUERIESNVPROC) (GLsizei n, GLuint *ids);
typedef GLvoid    (APIENTRY * NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (APIENTRY * NEL_PFNGLISOCCLUSIONQUERYNVPROC) (GLuint id);
typedef GLvoid    (APIENTRY * NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC) (GLuint id);
typedef GLvoid    (APIENTRY * NEL_PFNGLENDOCCLUSIONQUERYNVPROC) ();
typedef GLvoid    (APIENTRY * NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef GLvoid    (APIENTRY * NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC) (GLuint id, GLenum pname, GLuint *params);

#endif /* GL_NV_occlusion_query */

#ifndef NL_GL_ARB_multisample
#define NL_GL_ARB_multisample 1
typedef GLvoid    (APIENTRY * NEL_PFNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);
#endif

#if defined(NL_OS_MAC)

// Mac GL extensions

#elif defined(NL_OS_UNIX)

// GLX extensions
#ifndef NL_GLX_EXT_swap_control
#define NL_GLX_EXT_swap_control 1

#ifndef GLX_EXT_swap_control
#define GLX_SWAP_INTERVAL_EXT              0x20F1
#define GLX_MAX_SWAP_INTERVAL_EXT          0x20F2
#endif

typedef GLint (APIENTRY * NEL_PFNGLXSWAPINTERVALEXTPROC) (Display *dpy, GLXDrawable drawable, GLint interval);

#endif // NL_GLX_EXT_swap_control

#ifndef NL_GLX_MESA_swap_control
#define NL_GLX_MESA_swap_control 1

typedef GLint (APIENTRY * NEL_PFNGLXSWAPINTERVALMESAPROC) (GLuint interval);
typedef GLint (APIENTRY * NEL_PFNGLXGETSWAPINTERVALMESAPROC) ();

#endif // NL_GLX_MESA_swap_control

#ifndef NL_GLX_NV_vertex_array_range
#define NL_GLX_NV_vertex_array_range 1
typedef void* (APIENTRY * NEL_PFNGLXALLOCATEMEMORYNVPROC) (GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority);
typedef void (APIENTRY * NEL_PFNGLXFREEMEMORYNVPROC) (void *pointer);
#endif // NL_GLX_NV_vertex_array_range

#endif // NL_OS_MAC

#endif // USE_OPENGLES

#ifdef __cplusplus
}
#endif

#endif // NL_OPENGL_EXTENSION_DEF_H

