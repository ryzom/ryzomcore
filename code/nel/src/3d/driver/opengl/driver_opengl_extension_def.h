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

#ifdef NL_MAC_NATIVE
# define GL_GLEXT_LEGACY
# include <OpenGL/gl.h>
# include "mac/glext.h"
#else
# include <GL/gl.h>
# include <GL/glext.h>	// Please download it from http://www.opengl.org/registry/
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif


#ifdef NL_OS_WINDOWS

#ifndef HPBUFFERARB
DECLARE_HANDLE (HPBUFFERARB);
#endif // HPBUFFERARB

/* NV_vertex_array_range WGL extension not defined in glext.h */
typedef void *(APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority);
typedef void *(APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);
typedef HPBUFFERARB (APIENTRY * PFNWGLCREATEPBUFFERARBPROC) (HDC hdc, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC			(APIENTRY * PFNWGLGETPUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int			(APIENTRY * PFNWGLRELEASEPUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC);
typedef BOOL		(APIENTRY * PFNWGLDESTROYPUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL		(APIENTRY * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);

typedef BOOL		(APIENTRY * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC, int, int, UINT, const int *, int *);
typedef BOOL		(APIENTRY * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC, int, int, UINT, const int *, FLOAT *);
typedef BOOL		(APIENTRY * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC, const int *, const FLOAT *, UINT, int *, UINT *);

typedef BOOL		(APIENTRY * PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int			(APIENTRY * PFNWGLGETSWAPINTERVALEXTPROC) (void);

typedef	const char*	(APIENTRY * PFNWGFGETEXTENSIONSSTRINGARB) (HDC);
#endif


/* NV_vertex_program */
#ifndef GL_NV_vertex_program
#define GL_NV_vertex_program              1

#define GL_VERTEX_PROGRAM_NV              0x8620
#define GL_VERTEX_STATE_PROGRAM_NV        0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV           0x8625
#define GL_CURRENT_ATTRIB_NV              0x8626
#define GL_PROGRAM_LENGTH_NV              0x8627
#define GL_PROGRAM_STRING_NV              0x8628
#define GL_MODELVIEW_PROJECTION_NV        0x8629
#define GL_IDENTITY_NV                    0x862A
#define GL_INVERSE_NV                     0x862B
#define GL_TRANSPOSE_NV                   0x862C
#define GL_INVERSE_TRANSPOSE_NV           0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV          0x862F
#define GL_MATRIX0_NV                     0x8630
#define GL_MATRIX1_NV                     0x8631
#define GL_MATRIX2_NV                     0x8632
#define GL_MATRIX3_NV                     0x8633
#define GL_MATRIX4_NV                     0x8634
#define GL_MATRIX5_NV                     0x8635
#define GL_MATRIX6_NV                     0x8636
#define GL_MATRIX7_NV                     0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV  0x8640
#define GL_CURRENT_MATRIX_NV              0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV     0x8643
#define GL_PROGRAM_PARAMETER_NV           0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV        0x8645
#define GL_PROGRAM_TARGET_NV              0x8646
#define GL_PROGRAM_RESIDENT_NV            0x8647
#define GL_TRACK_MATRIX_NV                0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV      0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV      0x864A
#define GL_PROGRAM_ERROR_POSITION_NV      0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV        0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV        0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV        0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV        0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV        0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV        0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV        0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV        0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV        0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV        0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV       0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV       0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV       0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV       0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV       0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV       0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV       0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV       0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV       0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV       0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV       0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV       0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV       0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV       0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV       0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV       0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV      0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV      0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV      0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV      0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV      0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV      0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV       0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV       0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV       0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV       0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV       0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV       0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV       0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV       0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV       0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV       0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV      0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV      0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV      0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV      0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV      0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV      0x867F


#ifdef GL_GLEXT_PROTOTYPES
extern GLboolean APIENTRY glAreProgramsResidentNV(GLsizei n, const GLuint *programs, GLboolean *residences);
extern void APIENTRY glBindProgramNV(GLenum target, GLuint id);
extern void APIENTRY glDeleteProgramsNV(GLsizei n, const GLuint *programs);
extern void APIENTRY glExecuteProgramNV(GLenum target, GLuint id, const GLfloat *params);
extern void APIENTRY glGenProgramsNV(GLsizei n, GLuint *programs);
extern void APIENTRY glGetProgramParameterdvNV(GLenum target, GLuint index, GLenum pname, GLdouble *params);
extern void APIENTRY glGetProgramParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat *params);
extern void APIENTRY glGetProgramivNV(GLuint id, GLenum pname, GLint *params);
extern void APIENTRY glGetProgramStringNV(GLuint id, GLenum pname, GLubyte *program);
extern void APIENTRY glGetTrackMatrixivNV(GLenum target, GLuint address, GLenum pname, GLint *params);
extern void APIENTRY glGetVertexAttribdvNV(GLuint index, GLenum pname, GLdouble *params);
extern void APIENTRY glGetVertexAttribfvNV(GLuint index, GLenum pname, GLfloat *params);
extern void APIENTRY glGetVertexAttribivNV(GLuint index, GLenum pname, GLint *params);
extern void APIENTRY glGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid* *pointer);
extern GLboolean APIENTRY glIsProgramNV(GLuint id);
extern void APIENTRY glLoadProgramNV(GLenum target, GLuint id, GLsizei len, const GLubyte *program);
extern void APIENTRY glProgramParameter4dNV(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void APIENTRY glProgramParameter4dvNV(GLenum target, GLuint index, const GLdouble *v);
extern void APIENTRY glProgramParameter4fNV(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void APIENTRY glProgramParameter4fvNV(GLenum target, GLuint index, const GLfloat *v);
extern void APIENTRY glProgramParameters4dvNV(GLenum target, GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glProgramParameters4fvNV(GLenum target, GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glRequestResidentProgramsNV(GLsizei n, const GLuint *programs);
extern void APIENTRY glTrackMatrixNV(GLenum target, GLuint address, GLenum matrix, GLenum transform);
extern void APIENTRY glVertexAttribPointerNV(GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void APIENTRY glVertexAttrib1dNV(GLuint index, GLdouble x);
extern void APIENTRY glVertexAttrib1dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib1fNV(GLuint index, GLfloat x);
extern void APIENTRY glVertexAttrib1fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib1sNV(GLuint index, GLshort x);
extern void APIENTRY glVertexAttrib1svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib2dNV(GLuint index, GLdouble x, GLdouble y);
extern void APIENTRY glVertexAttrib2dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib2fNV(GLuint index, GLfloat x, GLfloat y);
extern void APIENTRY glVertexAttrib2fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib2sNV(GLuint index, GLshort x, GLshort y);
extern void APIENTRY glVertexAttrib2svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib3dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void APIENTRY glVertexAttrib3dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib3fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void APIENTRY glVertexAttrib3fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib3sNV(GLuint index, GLshort x, GLshort y, GLshort z);
extern void APIENTRY glVertexAttrib3svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib4dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void APIENTRY glVertexAttrib4dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib4fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void APIENTRY glVertexAttrib4fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib4sNV(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void APIENTRY glVertexAttrib4svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib4ubvNV(GLuint index, const GLubyte *v);
extern void APIENTRY glVertexAttribs1dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs1fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs1svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs2dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs2fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs2svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs3dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs3fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs3svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs4dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs4fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs4svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs4ubvNV(GLuint index, GLsizei count, const GLubyte *v);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLboolean (APIENTRY * PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
typedef void (APIENTRY * PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
typedef void (APIENTRY * PFNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
typedef void (APIENTRY * PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
typedef void (APIENTRY * PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef GLboolean (APIENTRY * PFNGLISPROGRAMNVPROC) (GLuint id);
typedef void (APIENTRY * PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);
#endif	/* GL_NV_vertex_program */

#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_pbuffer
#define WGL_ARB_pbuffer 1
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036
#endif

/*
 * GL_ARB_texture_env_combine
 */
#ifndef GL_ARB_texture_env_combine
#define GL_ARB_texture_env_combine  1

#define GL_COMBINE_ARB        0x8570
#define GL_COMBINE_RGB_ARB    0x8571
#define GL_COMBINE_ALPHA_ARB  0x8572
#define GL_SOURCE0_RGB_ARB    0x8580
#define GL_SOURCE1_RGB_ARB    0x8581
#define GL_SOURCE2_RGB_ARB    0x8582
#define GL_SOURCE0_ALPHA_ARB  0x8588
#define GL_SOURCE1_ALPHA_ARB  0x8589
#define GL_SOURCE2_ALPHA_ARB  0x858A
#define GL_OPERAND0_RGB_ARB   0x8590
#define GL_OPERAND1_RGB_ARB   0x8591
#define GL_OPERAND2_RGB_ARB   0x8592
#define GL_OPERAND0_ALPHA_ARB 0x8598
#define GL_OPERAND1_ALPHA_ARB 0x8599
#define GL_OPERAND2_ALPHA_ARB 0x859A
#define GL_RGB_SCALE_ARB      0x8573
#define GL_ADD_SIGNED_ARB     0x8574
#define GL_INTERPOLATE_ARB    0x8575
#define GL_CONSTANT_ARB       0x8576
#define GL_PRIMARY_COLOR_ARB  0x8577
#define GL_PREVIOUS_ARB       0x8578
#define GL_SUBTRACT_ARB       0x84E7

#endif /* GL_ARB_texture_env_combine */

/* NV_fence extension */
#ifndef GL_NV_fence
#define GL_NV_fence			1

#define GL_ALL_COMPLETED_NV               0x84F2
#define GL_FENCE_STATUS_NV                0x84F3
#define GL_FENCE_CONDITION_NV             0x84F4

#ifdef GL_GLEXT_PROTOTYPES
extern void APIENTRY glDeleteFencesNV(GLsizei n, const GLuint *fences);
extern void APIENTRY glGenFencesNV(GLsizei n, GLuint *fences);
extern GLboolean APIENTRY glIsFenceNV(GLuint fence);
extern GLboolean APIENTRY glTestFenceNV(GLuint fence);
extern void APIENTRY glGetFenceivNV(GLuint fence, GLenum pname, GLint *params);
extern void APIENTRY glFinishFenceNV(GLuint fence);
extern void APIENTRY glSetFenceNV(GLuint fence, GLenum condition);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef void (APIENTRY * PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef GLboolean (APIENTRY * PFNGLISFENCENVPROC) (GLuint fence);
typedef GLboolean (APIENTRY * PFNGLTESTFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLFINISHFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);

#endif	/* GL_NV_fence */


/* NV_texture_shader */
#define GL_OFFSET_TEXTURE_RECTANGLE_NV    0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV 0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV 0x864E
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV 0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV      0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV  0x86DB
#define GL_DSDT_MAG_INTENSITY_NV          0x86DC
#define GL_SHADER_CONSISTENT_NV           0x86DD
#define GL_TEXTURE_SHADER_NV              0x86DE
#define GL_SHADER_OPERATION_NV            0x86DF
#define GL_CULL_MODES_NV                  0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV       0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV        0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV         0x86E3
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV    GL_OFFSET_TEXTURE_MATRIX_NV
#define GL_OFFSET_TEXTURE_2D_SCALE_NV     GL_OFFSET_TEXTURE_SCALE_NV
#define GL_OFFSET_TEXTURE_2D_BIAS_NV      GL_OFFSET_TEXTURE_BIAS_NV
#define GL_PREVIOUS_TEXTURE_INPUT_NV      0x86E4
#define GL_CONST_EYE_NV                   0x86E5
#define GL_PASS_THROUGH_NV                0x86E6
#define GL_CULL_FRAGMENT_NV               0x86E7
#define GL_OFFSET_TEXTURE_2D_NV           0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV     0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV     0x86EA
#define GL_DOT_PRODUCT_NV                 0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV   0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV      0x86EE
#define GL_DOT_PRODUCT_TEXTURE_3D_NV      0x86EF
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV 0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV 0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV 0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3
#define GL_HILO_NV                        0x86F4
#define GL_DSDT_NV                        0x86F5
#define GL_DSDT_MAG_NV                    0x86F6
#define GL_DSDT_MAG_VIB_NV                0x86F7
#define GL_HILO16_NV                      0x86F8
#define GL_SIGNED_HILO_NV                 0x86F9
#define GL_SIGNED_HILO16_NV               0x86FA
#define GL_SIGNED_RGBA_NV                 0x86FB
#define GL_SIGNED_RGBA8_NV                0x86FC
#define GL_SIGNED_RGB_NV                  0x86FE
#define GL_SIGNED_RGB8_NV                 0x86FF
#define GL_SIGNED_LUMINANCE_NV            0x8701
#define GL_SIGNED_LUMINANCE8_NV           0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV      0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV    0x8704
#define GL_SIGNED_ALPHA_NV                0x8705
#define GL_SIGNED_ALPHA8_NV               0x8706
#define GL_SIGNED_INTENSITY_NV            0x8707
#define GL_SIGNED_INTENSITY8_NV           0x8708
#define GL_DSDT8_NV                       0x8709
#define GL_DSDT8_MAG8_NV                  0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV       0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV   0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#define GL_HI_SCALE_NV                    0x870E
#define GL_LO_SCALE_NV                    0x870F
#define GL_DS_SCALE_NV                    0x8710
#define GL_DT_SCALE_NV                    0x8711
#define GL_MAGNITUDE_SCALE_NV             0x8712
#define GL_VIBRANCE_SCALE_NV              0x8713
#define GL_HI_BIAS_NV                     0x8714
#define GL_LO_BIAS_NV                     0x8715
#define GL_DS_BIAS_NV                     0x8716
#define GL_DT_BIAS_NV                     0x8717
#define GL_MAGNITUDE_BIAS_NV              0x8718
#define GL_VIBRANCE_BIAS_NV               0x8719
#define GL_TEXTURE_BORDER_VALUES_NV       0x871A
#define GL_TEXTURE_HI_SIZE_NV             0x871B
#define GL_TEXTURE_LO_SIZE_NV             0x871C
#define GL_TEXTURE_DS_SIZE_NV             0x871D
#define GL_TEXTURE_DT_SIZE_NV             0x871E
#define GL_TEXTURE_MAG_SIZE_NV            0x871F


/* GL_EXT_vertex_shader */


#ifndef GL_EXT_vertex_shader
#define GL_EXT_vertex_shader
#define      GL_VERTEX_SHADER_EXT					 0x8780
#define      GL_VARIANT_VALUE_EXT                   0x87e4
#define      GL_VARIANT_DATATYPE_EXT                0x87e5
#define      GL_VARIANT_ARRAY_STRIDE_EXT            0x87e6
#define      GL_VARIANT_ARRAY_TYPE_EXT              0x87e7
#define      GL_VARIANT_ARRAY_EXT                   0x87e8
#define      GL_VARIANT_ARRAY_POINTER_EXT           0x87e9
#define      GL_INVARIANT_VALUE_EXT                 0x87ea
#define      GL_INVARIANT_DATATYPE_EXT              0x87eb
#define      GL_LOCAL_CONSTANT_VALUE_EXT            0x87ec
#define      GL_LOCAL_CONSTANT_DATATYPE_EXT         0x87ed
#define      GL_OP_INDEX_EXT                        0x8782
#define      GL_OP_NEGATE_EXT                       0x8783
#define      GL_OP_DOT3_EXT                         0x8784
#define      GL_OP_DOT4_EXT                         0x8785
#define      GL_OP_MUL_EXT                          0x8786
#define      GL_OP_ADD_EXT                          0x8787
#define      GL_OP_MADD_EXT                         0x8788
#define      GL_OP_FRAC_EXT                         0x8789
#define      GL_OP_MAX_EXT                          0x878a
#define      GL_OP_MIN_EXT                          0x878b
#define      GL_OP_SET_GE_EXT                       0x878c
#define      GL_OP_SET_LT_EXT                       0x878d
#define      GL_OP_CLAMP_EXT                        0x878e
#define      GL_OP_FLOOR_EXT                        0x878f
#define      GL_OP_ROUND_EXT                        0x8790
#define      GL_OP_EXP_BASE_2_EXT                   0x8791
#define      GL_OP_LOG_BASE_2_EXT                   0x8792
#define      GL_OP_POWER_EXT                        0x8793
#define      GL_OP_RECIP_EXT                        0x8794
#define      GL_OP_RECIP_SQRT_EXT                   0x8795
#define      GL_OP_SUB_EXT                          0x8796
#define      GL_OP_CROSS_PRODUCT_EXT                0x8797
#define      GL_OP_MULTIPLY_MATRIX_EXT              0x8798
#define      GL_OP_MOV_EXT                          0x8799
#define      GL_OUTPUT_VERTEX_EXT                   0x879a
#define      GL_OUTPUT_COLOR0_EXT                   0x879b
#define      GL_OUTPUT_COLOR1_EXT                   0x879c
#define      GL_OUTPUT_TEXTURE_COORD0_EXT           0x879d
#define      GL_OUTPUT_TEXTURE_COORD1_EXT           0x879e
#define      GL_OUTPUT_TEXTURE_COORD2_EXT           0x879f
#define      GL_OUTPUT_TEXTURE_COORD3_EXT           0x87a0
#define      GL_OUTPUT_TEXTURE_COORD4_EXT           0x87a1
#define      GL_OUTPUT_TEXTURE_COORD5_EXT           0x87a2
#define      GL_OUTPUT_TEXTURE_COORD6_EXT           0x87a3
#define      GL_OUTPUT_TEXTURE_COORD7_EXT           0x87a4
#define      GL_OUTPUT_TEXTURE_COORD8_EXT           0x87a5
#define      GL_OUTPUT_TEXTURE_COORD9_EXT           0x87a6
#define      GL_OUTPUT_TEXTURE_COORD10_EXT          0x87a7
#define      GL_OUTPUT_TEXTURE_COORD11_EXT          0x87a8
#define      GL_OUTPUT_TEXTURE_COORD12_EXT          0x87a9
#define      GL_OUTPUT_TEXTURE_COORD13_EXT          0x87aa
#define      GL_OUTPUT_TEXTURE_COORD14_EXT          0x87ab
#define      GL_OUTPUT_TEXTURE_COORD15_EXT          0x87ac
#define      GL_OUTPUT_TEXTURE_COORD16_EXT          0x87ad
#define      GL_OUTPUT_TEXTURE_COORD17_EXT          0x87ae
#define      GL_OUTPUT_TEXTURE_COORD18_EXT          0x87af
#define      GL_OUTPUT_TEXTURE_COORD19_EXT          0x87b0
#define      GL_OUTPUT_TEXTURE_COORD20_EXT          0x87b1
#define      GL_OUTPUT_TEXTURE_COORD21_EXT          0x87b2
#define      GL_OUTPUT_TEXTURE_COORD22_EXT          0x87b3
#define      GL_OUTPUT_TEXTURE_COORD23_EXT          0x87b4
#define      GL_OUTPUT_TEXTURE_COORD24_EXT          0x87b5
#define      GL_OUTPUT_TEXTURE_COORD25_EXT          0x87b6
#define      GL_OUTPUT_TEXTURE_COORD26_EXT          0x87b7
#define      GL_OUTPUT_TEXTURE_COORD27_EXT          0x87b8
#define      GL_OUTPUT_TEXTURE_COORD28_EXT          0x87b9
#define      GL_OUTPUT_TEXTURE_COORD29_EXT          0x87ba
#define      GL_OUTPUT_TEXTURE_COORD30_EXT          0x87bb
#define      GL_OUTPUT_TEXTURE_COORD31_EXT          0x87bc
#define      GL_OUTPUT_FOG_EXT                      0x87bd
#define      GL_SCALAR_EXT                          0x87be
#define      GL_VECTOR_EXT                          0x87bf
#define      GL_MATRIX_EXT                          0x87c0
#define      GL_VARIANT_EXT									  0x87c1
#define      GL_INVARIANT_EXT									  0x87c2
#define      GL_LOCAL_CONSTANT_EXT							      0x87c3
#define      GL_LOCAL_EXT										  0x87c4
#define      GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT               0x87c5
#define      GL_MAX_VERTEX_SHADER_VARIANTS_EXT                   0x87c6
#define      GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                 0x87c7
#define      GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT            0x87c8
#define      GL_MAX_VERTEX_SHADER_LOCALS_EXT                     0x87c9
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT     0x87ca
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT         0x87cb
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT  0x87cc
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT       0x87cd
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT           0x87ce
#define      GL_VERTEX_SHADER_INSTRUCTIONS_EXT                   0x87cf
#define      GL_VERTEX_SHADER_VARIANTS_EXT                       0x87d0
#define      GL_VERTEX_SHADER_INVARIANTS_EXT                     0x87d1
#define      GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                0x87d2
#define      GL_VERTEX_SHADER_LOCALS_EXT                         0x87d3
#define      GL_VERTEX_SHADER_BINDING_EXT                        0x8781
#define      GL_VERTEX_SHADER_OPTIMIZED_EXT                     0x87d4
#define      GL_X_EXT                               0x87d5
#define      GL_Y_EXT                               0x87d6
#define      GL_Z_EXT                               0x87d7
#define      GL_W_EXT                               0x87d8
#define      GL_NEGATIVE_X_EXT                      0x87d9
#define      GL_NEGATIVE_Y_EXT                      0x87da
#define      GL_NEGATIVE_Z_EXT                      0x87db
#define      GL_NEGATIVE_W_EXT                      0x87dc
#define      GL_ZERO_EXT                            0x87dd
#define      GL_ONE_EXT                             0x87de
#define      GL_NEGEXTVE_ONE_EXT                    0x87df
#define      GL_NORMALIZED_RANGE_EXT                0x87e0
#define      GL_FULL_RANGE_EXT                      0x87e1
#define      GL_CURRENT_VERTEX_EXT                  0x87e2
#define      GL_MVP_MATRIX_EXT                      0x87e3
#endif // GL_EXT_vertex_shader
// Bad glext.h definition.....
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT       0x87cd
#endif

/* NV_vertex_array_range2 */
#ifndef GL_NV_vertex_array_range2
#define GL_NV_vertex_array_range2			1
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV 0x8533
#endif


/* GL_ATI_vertex_array_object */
#ifndef GL_ATI_vertex_array_object
#define GL_STATIC_ATI                     0x8760
#define GL_DYNAMIC_ATI                    0x8761
#define GL_PRESERVE_ATI                   0x8762
#define GL_DISCARD_ATI                    0x8763
#define GL_OBJECT_BUFFER_SIZE_ATI         0x8764
#define GL_OBJECT_BUFFER_USAGE_ATI        0x8765
#define GL_ARRAY_OBJECT_BUFFER_ATI        0x8766
#define GL_ARRAY_OBJECT_OFFSET_ATI        0x8767
#endif

#ifndef GL_ATI_vertex_array_object
#define GL_ATI_vertex_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLuint APIENTRY glNewObjectBufferATI (GLsizei, const GLvoid *, GLenum);
GLAPI GLboolean APIENTRY glIsObjectBufferATI (GLuint);
GLAPI void APIENTRY glUpdateObjectBufferATI (GLuint, GLuint, GLsizei, const GLvoid *, GLenum);
GLAPI void APIENTRY glGetObjectBufferfvATI (GLuint, GLenum, GLfloat *);
GLAPI void APIENTRY glGetObjectBufferivATI (GLuint, GLenum, GLint *);
GLAPI void APIENTRY glDeleteObjectBufferATI (GLuint);
GLAPI void APIENTRY glArrayObjectATI (GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void APIENTRY glGetArrayObjectfvATI (GLenum, GLenum, GLfloat *);
GLAPI void APIENTRY glGetArrayObjectivATI (GLenum, GLenum, GLint *);
GLAPI void APIENTRY glVariantArrayObjectATI (GLuint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void APIENTRY glGetVariantArrayObjectfvATI (GLuint, GLenum, GLfloat *);
GLAPI void APIENTRY glGetVariantArrayObjectivATI (GLuint, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLuint (APIENTRY * PFNGLNEWOBJECTBUFFERATIPROC) (GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (APIENTRY * PFNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef void (APIENTRY * PFNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLDELETEOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLARRAYOBJECTATIPROC) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * PFNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * PFNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id, GLenum pname, GLint *params);
#endif

/* GL_ATI_texture_env_combine3 */
#ifndef GL_ATI_texture_env_combine3
#define GL_ATI_texture_env_combine3

#define GL_MODULATE_ADD_ATI				0x8744
#define GL_MODULATE_SIGNED_ADD_ATI			0x8745
#define GL_MODULATE_SUBTRACT_ATI			0x8746

#endif /* GL_ATI_texture_env_combine3 */



/* GL_ATI_envmap_bumpmap */

#ifndef GL_ATI_envmap_bumpmap
#define GL_ATI_envmap_bumpmap 1

#define GL_BUMP_ROT_MATRIX_ATI					   0x8775
#define GL_BUMP_ROT_MATRIX_SIZE_ATI				   0x8776
#define GL_BUMP_NUM_TEX_UNITS_ATI				   0x8777
#define GL_BUMP_TEX_UNITS_ATI					   0x8778
#define GL_DUDV_ATI								   0x8779
#define GL_DU8DV8_ATI							   0x877A
#define GL_BUMP_ENVMAP_ATI						   0x877B
#define GL_BUMP_TARGET_ATI						   0x877C

typedef void (APIENTRY * PFNGLTEXBUMPPARAMETERIVATIPROC) (GLenum pname, GLint *param);
typedef void (APIENTRY * PFNGLTEXBUMPPARAMETERFVATIPROC) (GLenum pname, GLfloat *param);
typedef void (APIENTRY * PFNGLGETTEXBUMPPARAMETERIVATIPROC) (GLenum pname, GLint *param);
typedef void (APIENTRY * PFNGLGETTEXBUMPPARAMETERFVATIPROC) (GLenum pname, GLfloat *param);


#endif /* GL_ATI_envmap_bumpmap */


/* GL_ATI_fragment_shader extension */

#ifndef GL_ATI_fragment_shader
#define GL_ATI_fragment_shader                   1

#define GL_FRAGMENT_SHADER_ATI                   0x8920
#define GL_REG_0_ATI                             0x8921
#define GL_REG_1_ATI                             0x8922
#define GL_REG_2_ATI                             0x8923
#define GL_REG_3_ATI                             0x8924
#define GL_REG_4_ATI                             0x8925
#define GL_REG_5_ATI                             0x8926
#define GL_REG_6_ATI                             0x8927
#define GL_REG_7_ATI                             0x8928
#define GL_REG_8_ATI                             0x8929
#define GL_REG_9_ATI                             0x892A
#define GL_REG_10_ATI                            0x892B
#define GL_REG_11_ATI                            0x892C
#define GL_REG_12_ATI                            0x892D
#define GL_REG_13_ATI                            0x892E
#define GL_REG_14_ATI                            0x892F
#define GL_REG_15_ATI                            0x8930
#define GL_REG_16_ATI                            0x8931
#define GL_REG_17_ATI                            0x8932
#define GL_REG_18_ATI                            0x8933
#define GL_REG_19_ATI                            0x8934
#define GL_REG_20_ATI                            0x8935
#define GL_REG_21_ATI                            0x8936
#define GL_REG_22_ATI                            0x8937
#define GL_REG_23_ATI                            0x8938
#define GL_REG_24_ATI                            0x8939
#define GL_REG_25_ATI                            0x893A
#define GL_REG_26_ATI                            0x893B
#define GL_REG_27_ATI                            0x893C
#define GL_REG_28_ATI                            0x893D
#define GL_REG_29_ATI                            0x893E
#define GL_REG_30_ATI                            0x893F
#define GL_REG_31_ATI                            0x8940
#define GL_CON_0_ATI                             0x8941
#define GL_CON_1_ATI                             0x8942
#define GL_CON_2_ATI                             0x8943
#define GL_CON_3_ATI                             0x8944
#define GL_CON_4_ATI                             0x8945
#define GL_CON_5_ATI                             0x8946
#define GL_CON_6_ATI                             0x8947
#define GL_CON_7_ATI                             0x8948
#define GL_CON_8_ATI                             0x8949
#define GL_CON_9_ATI                             0x894A
#define GL_CON_10_ATI                            0x894B
#define GL_CON_11_ATI                            0x894C
#define GL_CON_12_ATI                            0x894D
#define GL_CON_13_ATI                            0x894E
#define GL_CON_14_ATI                            0x894F
#define GL_CON_15_ATI                            0x8950
#define GL_CON_16_ATI                            0x8951
#define GL_CON_17_ATI                            0x8952
#define GL_CON_18_ATI                            0x8953
#define GL_CON_19_ATI                            0x8954
#define GL_CON_20_ATI                            0x8955
#define GL_CON_21_ATI                            0x8956
#define GL_CON_22_ATI                            0x8957
#define GL_CON_23_ATI                            0x8958
#define GL_CON_24_ATI                            0x8959
#define GL_CON_25_ATI                            0x895A
#define GL_CON_26_ATI                            0x895B
#define GL_CON_27_ATI                            0x895C
#define GL_CON_28_ATI                            0x895D
#define GL_CON_29_ATI                            0x895E
#define GL_CON_30_ATI                            0x895F
#define GL_CON_31_ATI                            0x8960
#define GL_MOV_ATI                               0x8961
#define GL_ADD_ATI                               0x8963
#define GL_MUL_ATI                               0x8964
#define GL_SUB_ATI                               0x8965
#define GL_DOT3_ATI                              0x8966
#define GL_DOT4_ATI                              0x8967
#define GL_MAD_ATI                               0x8968
#define GL_LERP_ATI                              0x8969
#define GL_CND_ATI                               0x896A
#define GL_CND0_ATI                              0x896B
#define GL_DOT2_ADD_ATI                          0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI            0x896D
#define GL_NUM_FRAGMENT_REGISTERS_ATI            0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI            0x896F
#define GL_NUM_PASSES_ATI                        0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI         0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI            0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI 0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI           0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI               0x8975
#define GL_SWIZZLE_STR_ATI                       0x8976
#define GL_SWIZZLE_STQ_ATI                       0x8977
#define GL_SWIZZLE_STR_DR_ATI                    0x8978
#define GL_SWIZZLE_STQ_DQ_ATI                    0x8979
#define GL_SWIZZLE_STRQ_ATI                      0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI                   0x897B
#define GL_RED_BIT_ATI                           0x00000001
#define GL_GREEN_BIT_ATI                         0x00000002
#define GL_BLUE_BIT_ATI                          0x00000004
#define GL_2X_BIT_ATI                            0x00000001
#define GL_4X_BIT_ATI                            0x00000002
#define GL_8X_BIT_ATI                            0x00000004
#define GL_HALF_BIT_ATI                          0x00000008
#define GL_QUARTER_BIT_ATI                       0x00000010
#define GL_EIGHTH_BIT_ATI                        0x00000020
#define GL_SATURATE_BIT_ATI                      0x00000040
#define GL_COMP_BIT_ATI                          0x00000002
#define GL_NEGATE_BIT_ATI                        0x00000004
#define GL_BIAS_BIT_ATI                          0x00000008


typedef GLuint (APIENTRY *PFNGLGENFRAGMENTSHADERSATIPROC)(GLuint range);
typedef GLvoid (APIENTRY *PFNGLBINDFRAGMENTSHADERATIPROC)(GLuint id);
typedef GLvoid (APIENTRY *PFNGLDELETEFRAGMENTSHADERATIPROC)(GLuint id);
typedef GLvoid (APIENTRY *PFNGLBEGINFRAGMENTSHADERATIPROC)();
typedef GLvoid (APIENTRY *PFNGLENDFRAGMENTSHADERATIPROC)();
typedef GLvoid (APIENTRY *PFNGLPASSTEXCOORDATIPROC)(GLuint dst, GLuint coord, GLenum swizzle);
typedef GLvoid (APIENTRY *PFNGLSAMPLEMAPATIPROC)(GLuint dst, GLuint interp, GLenum swizzle);
typedef GLvoid (APIENTRY *PFNGLCOLORFRAGMENTOP1ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
														GLuint dstMod, GLuint arg1, GLuint arg1Rep,
														GLuint arg1Mod);
typedef GLvoid (APIENTRY *PFNGLCOLORFRAGMENTOP2ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
														GLuint dstMod, GLuint arg1, GLuint arg1Rep,
														GLuint arg1Mod, GLuint arg2, GLuint arg2Rep,
														GLuint arg2Mod);
typedef GLvoid (APIENTRY *PFNGLCOLORFRAGMENTOP3ATIPROC)(GLenum op, GLuint dst, GLuint dstMask,
														GLuint dstMod, GLuint arg1, GLuint arg1Rep,
														GLuint arg1Mod, GLuint arg2, GLuint arg2Rep,
														GLuint arg2Mod, GLuint arg3, GLuint arg3Rep,
														GLuint arg3Mod);
typedef GLvoid (APIENTRY *PFNGLALPHAFRAGMENTOP1ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
								                        GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
typedef GLvoid (APIENTRY *PFNGLALPHAFRAGMENTOP2ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
														GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
														GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
typedef GLvoid (APIENTRY *PFNGLALPHAFRAGMENTOP3ATIPROC)(GLenum op, GLuint dst, GLuint dstMod,
														GLuint arg1, GLuint arg1Rep, GLuint arg1Mod,
														GLuint arg2, GLuint arg2Rep, GLuint arg2Mod,
														GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
typedef GLvoid (APIENTRY *PFNGLSETFRAGMENTSHADERCONSTANTATIPROC)(GLuint dst, const GLfloat *value);

#endif /* GL_ATI_fragment_shader */



/**
  * GL_ATI_map_object_buffer
  */
#ifndef GL_ATI_map_object_buffer
#define GL_ATI_map_object_buffer                1

typedef void *(APIENTRY * PFNGLMAPOBJECTBUFFERATIPROC)(GLuint buffer);
typedef void (APIENTRY * PFNGLUNMAPOBJECTBUFFERATIPROC)(GLuint buffer);


#endif /* GL_ATI_map_object_buffer */




/**
  * GL_ARB_fragment_program
  */

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program                    1

#define GL_FRAGMENT_PROGRAM_ARB 0x8804
#define GL_PROGRAM_FORMAT_ASCII_ARB 0x8875
#define GL_PROGRAM_LENGTH_ARB 0x8627
#define GL_PROGRAM_FORMAT_ARB 0x8876
#define GL_PROGRAM_BINDING_ARB 0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB 0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB 0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB 0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6

#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB 0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB 0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB 0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB 0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB 0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB 0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB 0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810

#define GL_PROGRAM_STRING_ARB 0x8628
#define GL_PROGRAM_ERROR_POSITION_ARB 0x864B
#define GL_CURRENT_MATRIX_ARB 0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB 0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_MAX_PROGRAM_MATRICES_ARB 0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_TEXTURE_COORDS_ARB 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB 0x8872
#define GL_PROGRAM_ERROR_STRING_ARB 0x8874
#define GL_MATRIX0_ARB 0x88C0
#define GL_MATRIX1_ARB 0x88C1
#define GL_MATRIX2_ARB 0x88C2
#define GL_MATRIX3_ARB 0x88C3
#define GL_MATRIX4_ARB 0x88C4
#define GL_MATRIX5_ARB 0x88C5
#define GL_MATRIX6_ARB 0x88C6
#define GL_MATRIX7_ARB 0x88C7
#define GL_MATRIX8_ARB 0x88C8
#define GL_MATRIX9_ARB 0x88C9
#define GL_MATRIX10_ARB 0x88CA
#define GL_MATRIX11_ARB 0x88CB
#define GL_MATRIX12_ARB 0x88CC
#define GL_MATRIX13_ARB 0x88CD
#define GL_MATRIX14_ARB 0x88CE
#define GL_MATRIX15_ARB 0x88CF
#define GL_MATRIX16_ARB 0x88D0
#define GL_MATRIX17_ARB 0x88D1
#define GL_MATRIX18_ARB 0x88D2
#define GL_MATRIX19_ARB 0x88D3
#define GL_MATRIX20_ARB 0x88D4
#define GL_MATRIX21_ARB 0x88D5
#define GL_MATRIX22_ARB 0x88D6
#define GL_MATRIX23_ARB 0x88D7
#define GL_MATRIX24_ARB 0x88D8
#define GL_MATRIX25_ARB 0x88D9
#define GL_MATRIX26_ARB 0x88DA
#define GL_MATRIX27_ARB 0x88DB
#define GL_MATRIX28_ARB 0x88DC
#define GL_MATRIX29_ARB 0x88DD
#define GL_MATRIX30_ARB 0x88DE
#define GL_MATRIX31_ARB 0x88DF

typedef GLvoid (APIENTRY *PFNGLPROGRAMSTRINGARBPROC)(GLenum target, GLenum format, GLsizei len,const GLvoid *string);
typedef GLvoid (APIENTRY *PFNGLBINDPROGRAMARBPROC)(GLenum target, GLuint program);
typedef GLvoid (APIENTRY *PFNGLDELETEPROGRAMSARBPROC)(GLsizei n, const GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLGENPROGRAMSARBPROC)(GLsizei n, GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMIVARBPROC)(GLenum target, GLenum pname, int *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMSTRINGARBPROC)(GLenum target, GLenum pname, GLvoid *string);
typedef GLboolean (APIENTRY *PFNGLISPROGRAMARBPROC)(GLuint program);

#endif /* GL_ARB_fragment_program */

/** GL_ARB_vertex_buffer_object
  */
#ifndef GL_ARB_vertex_buffer_object
	#define GL_BUFFER_SIZE_ARB                0x8764
	#define GL_BUFFER_USAGE_ARB               0x8765
	#define GL_ARRAY_BUFFER_ARB               0x8892
	#define GL_ELEMENT_ARRAY_BUFFER_ARB       0x8893
	#define GL_ARRAY_BUFFER_BINDING_ARB       0x8894
	#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
	#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
	#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
	#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
	#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
	#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
	#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
	#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
	#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
	#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
	#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
	#define GL_READ_ONLY_ARB                  0x88B8
	#define GL_WRITE_ONLY_ARB                 0x88B9
	#define GL_READ_WRITE_ARB                 0x88BA
	#define GL_BUFFER_ACCESS_ARB              0x88BB
	#define GL_BUFFER_MAPPED_ARB              0x88BC
	#define GL_BUFFER_MAP_POINTER_ARB         0x88BD
	#define GL_STREAM_DRAW_ARB                0x88E0
	#define GL_STREAM_READ_ARB                0x88E1
	#define GL_STREAM_COPY_ARB                0x88E2
	#define GL_STATIC_DRAW_ARB                0x88E4
	#define GL_STATIC_READ_ARB                0x88E5
	#define GL_STATIC_COPY_ARB                0x88E6
	#define GL_DYNAMIC_DRAW_ARB               0x88E8
	#define GL_DYNAMIC_READ_ARB               0x88E9
	#define GL_DYNAMIC_COPY_ARB               0x88EA
#endif

/**
  * GL_ARB_vertex_program
  */
#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1

#define GL_VERTEX_PROGRAM_ARB                               0x8620
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB                    0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB                      0x8643
#define GL_COLOR_SUM_ARB                                    0x8458
#define GL_PROGRAM_FORMAT_ASCII_ARB                         0x8875
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB                  0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB                     0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB                   0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB                     0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB               0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB                        0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB                  0x8645
#define GL_PROGRAM_LENGTH_ARB                               0x8627
#define GL_PROGRAM_FORMAT_ARB                               0x8876
#define GL_PROGRAM_BINDING_ARB                              0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB                         0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB                     0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB                  0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB              0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB                          0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB                      0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB                   0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB               0x88A7
#define GL_PROGRAM_PARAMETERS_ARB                           0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB                       0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB                    0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB                0x88AB
#define GL_PROGRAM_ATTRIBS_ARB                              0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB                          0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB                       0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB                   0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB                    0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB                0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB             0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB         0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB                 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB                   0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB                  0x88B6
#define GL_PROGRAM_STRING_ARB                               0x8628
#define GL_PROGRAM_ERROR_POSITION_ARB                       0x864B
#define GL_CURRENT_MATRIX_ARB                               0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB                     0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB                   0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB                           0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB                         0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB               0x862E
#define GL_PROGRAM_ERROR_STRING_ARB                         0x8874
#define GL_MATRIX0_ARB                                      0x88C0
#define GL_MATRIX1_ARB                                      0x88C1
#define GL_MATRIX2_ARB                                      0x88C2
#define GL_MATRIX3_ARB                                      0x88C3
#define GL_MATRIX4_ARB                                      0x88C4
#define GL_MATRIX5_ARB                                      0x88C5
#define GL_MATRIX6_ARB                                      0x88C6
#define GL_MATRIX7_ARB                                      0x88C7
#define GL_MATRIX8_ARB                                      0x88C8
#define GL_MATRIX9_ARB                                      0x88C9
#define GL_MATRIX10_ARB                                     0x88CA
#define GL_MATRIX11_ARB                                     0x88CB
#define GL_MATRIX12_ARB                                     0x88CC
#define GL_MATRIX13_ARB                                     0x88CD
#define GL_MATRIX14_ARB                                     0x88CE
#define GL_MATRIX15_ARB                                     0x88CF
#define GL_MATRIX16_ARB                                     0x88D0
#define GL_MATRIX17_ARB                                     0x88D1
#define GL_MATRIX18_ARB                                     0x88D2
#define GL_MATRIX19_ARB                                     0x88D3
#define GL_MATRIX20_ARB                                     0x88D4
#define GL_MATRIX21_ARB                                     0x88D5
#define GL_MATRIX22_ARB                                     0x88D6
#define GL_MATRIX23_ARB                                     0x88D7
#define GL_MATRIX24_ARB                                     0x88D8
#define GL_MATRIX25_ARB                                     0x88D9
#define GL_MATRIX26_ARB                                     0x88DA
#define GL_MATRIX27_ARB                                     0x88DB
#define GL_MATRIX28_ARB                                     0x88DC
#define GL_MATRIX29_ARB                                     0x88DD
#define GL_MATRIX30_ARB                                     0x88DE
#define GL_MATRIX31_ARB                                     0x88DF

typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1SARBPROC)(GLuint index, GLshort x);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1FARBPROC)(GLuint index, GLfloat x);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1DARBPROC)(GLuint index, GLdouble x);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2SARBPROC)(GLuint index, GLshort x, GLshort y);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2FARBPROC)(GLuint index, GLfloat x, GLfloat y);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2DARBPROC)(GLuint index, GLdouble x, GLdouble y);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3SARBPROC)(GLuint index, GLshort x, GLshort y, GLshort z);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3FARBPROC)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3DARBPROC)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4SARBPROC)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4FARBPROC)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4DARBPROC)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NUBARBPROC)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1SVARBPROC)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1FVARBPROC)(GLuint index, const GLfloat *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB1DVARBPROC)(GLuint index, const GLdouble *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2SVARBPROC)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2FVARBPROC)(GLuint index, const GLfloat *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB2DVARBPROC)(GLuint index, const GLdouble *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3SVARBPROC)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3FVARBPROC)(GLuint index, const GLfloat *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB3DVARBPROC)(GLuint index, const GLdouble *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4BVARBPROC)(GLuint index, const GLbyte *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4SVARBPROC)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4IVARBPROC)(GLuint index, const GLint *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4UBVARBPROC)(GLuint index, const GLubyte *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4USVARBPROC)(GLuint index, const GLushort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4UIVARBPROC)(GLuint index, const GLuint *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4FVARBPROC)(GLuint index, const GLfloat *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4DVARBPROC)(GLuint index, const GLdouble *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NBVARBPROC)(GLuint index, const GLbyte *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NSVARBPROC)(GLuint index, const GLshort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NIVARBPROC)(GLuint index, const GLint *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NUBVARBPROC)(GLuint index, const GLubyte *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NUSVARBPROC)(GLuint index, const GLushort *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIB4NUIVARBPROC)(GLuint index, const GLuint *v);
typedef GLvoid (APIENTRY *PFNGLVERTEXATTRIBPOINTERARBPROC)(GLuint index, GLint size, GLenum type,
                                                           GLboolean normalized, GLsizei stride,
                                                           const GLvoid *pointer);
typedef GLvoid (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYARBPROC)(GLuint index);
typedef GLvoid (APIENTRY *PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)(GLuint index);
typedef GLvoid (APIENTRY *PFNGLPROGRAMSTRINGARBPROC)(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef GLvoid (APIENTRY *PFNGLBINDPROGRAMARBPROC)(GLenum target, GLuint program);
typedef GLvoid (APIENTRY *PFNGLDELETEPROGRAMSARBPROC)(GLsizei n, const GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLGENPROGRAMSARBPROC)(GLsizei n, GLuint *programs);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMENVPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FARBPROC)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DARBPROC)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)(GLenum target, GLuint index, const GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)(GLenum target, GLuint index, const GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMENVPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)(GLenum target, GLuint index, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)(GLenum target, GLuint index, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMIVARBPROC)(GLenum target, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *PFNGLGETPROGRAMSTRINGARBPROC)(GLenum target, GLenum pname, GLvoid *string);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBDVARBPROC)(GLuint index, GLenum pname, GLdouble *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBFVARBPROC)(GLuint index, GLenum pname, GLfloat *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBIVARBPROC)(GLuint index, GLenum pname, GLint *params);
typedef GLvoid (APIENTRY *PFNGLGETVERTEXATTRIBPOINTERVARBPROC)(GLuint index, GLenum pname, GLvoid **pointer);
typedef GLboolean (APIENTRY *PFNGLISPROGRAMARBPROC)(GLuint program);

#endif /* GL_ARB_vertex_program */

#ifndef GL_VERSION_1_5
/* GL types for handling large vertex buffer objects */
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
#endif

#ifndef GL_ARB_vertex_buffer_object
/* GL types for handling large vertex buffer objects */
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;
#endif

#ifndef GL_ARB_vertex_buffer_object
	#define GL_ARB_vertex_buffer_object 1
	typedef void (APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
	typedef void (APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
	typedef void (APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
	typedef GLboolean (APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
	typedef void (APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
	typedef void (APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
	typedef void (APIENTRYP PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
	typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
	typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
	typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
	typedef void (APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif


// ***************************************************************************
// ***************************************************************************
// The NEL Functions Typedefs.
// Must do it for compatibilities with futures version of gl.h
// eg: version 1.2 does not define PFNGLACTIVETEXTUREARBPROC. Hence, do it now, with our special name
// ***************************************************************************
// ***************************************************************************


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


// GL_NV_texture_rectangle
//==================================
#ifndef GL_NV_texture_rectangle
#define GL_NV_texture_rectangle 1

#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8

#endif

// GL_EXT_framebuffer_object
//==================================
#ifndef GL_EXT_framebuffer_object
#define GL_EXT_framebuffer_object 1

#define GL_FRAMEBUFFER_COMPLETE_EXT							0x8CD5
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT						0x8CDD
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT	0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT	0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT			0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT				0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT			0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT			0x8CDC
#define GL_COLOR_ATTACHMENT0_EXT							0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT								0x8D00
#define GL_STENCIL_ATTACHMENT_EXT							0x8D20
#define GL_FRAMEBUFFER_EXT									0x8D40
#define GL_RENDERBUFFER_EXT									0x8D41

#endif

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


#ifndef GL_EXT_packed_depth_stencil
#define GL_EXT_packed_depth_stencil         1

#define GL_DEPTH_STENCIL_EXT                0x84F9
#define GL_UNSIGNED_INT_24_8_EXT            0x84FA
#define GL_DEPTH24_STENCIL8_EXT             0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT         0x88F1

#endif /* GL_EXT_packed_depth_stencil */

#ifndef GL_ARB_depth_texture
#define GL_ARB_depth_texture
#define GL_DEPTH_COMPONENT16_ARB          0x81A5
#define GL_DEPTH_COMPONENT24_ARB          0x81A6
#define GL_DEPTH_COMPONENT32_ARB          0x81A7
#define GL_TEXTURE_DEPTH_SIZE_ARB         0x884A
#define GL_DEPTH_TEXTURE_MODE_ARB         0x884B
#endif

// GL_NV_occlusion_query
//==================================
#ifndef GL_NV_occlusion_query
#define GL_NV_occlusion_query              1

#define GL_PIXEL_COUNTER_BITS_NV           0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV   0x8865
#define GL_PIXEL_COUNT_NV                  0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV        0x8867

#endif

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


#ifdef __cplusplus
}
#endif

#endif // NL_OPENGL_EXTENSION_DEF_H

