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

#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

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

