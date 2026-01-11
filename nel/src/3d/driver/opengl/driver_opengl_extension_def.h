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

// use same defines for OpenGL and OpenGL ES to simplify the code
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
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER_OES
#define GL_DEPTH24_STENCIL8_EXT GL_DEPTH24_STENCIL8_OES
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT_OES
#define GL_STENCIL_ATTACHMENT_EXT GL_STENCIL_ATTACHMENT_OES
#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#define GL_TEXTURE0_ARB GL_TEXTURE0

#define GL_ALPHA8 GL_ALPHA
#define GL_LUMINANCE8_ALPHA8 GL_LUMINANCE_ALPHA
#define GL_LUMINANCE8 GL_LUMINANCE
#define GL_RGBA8 GL_RGBA
#define GL_RGB8 GL_RGB

#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
#define GL_DYNAMIC_DRAW_ARB GL_DYNAMIC_DRAW

#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES

#define nglGenRenderbuffersEXT nglGenRenderbuffersOES
#define nglBindRenderbufferEXT nglBindRenderbufferOES
#define nglDeleteRenderbuffersEXT nglDeleteRenderbuffersOES
#define nglRenderbufferStorageEXT nglRenderbufferStorageOES
#define nglGenFramebuffersEXT nglGenFramebuffersOES
#define nglBindFramebufferEXT nglBindFramebufferOES
#define nglFramebufferTexture2DEXT nglFramebufferTexture2DOES
#define nglFramebufferRenderbufferEXT nglFramebufferRenderbufferOES
#define nglCheckFramebufferStatusEXT nglCheckFramebufferStatusOES
#define nglDeleteBuffersARB glDeleteBuffers
#define nglIsBufferARB glIsBuffer
#define nglDeleteRenderbuffersEXT nglDeleteRenderbuffersOES
#define nglBindFramebufferEXT nglBindFramebufferOES
#define nglDeleteFramebuffersEXT nglDeleteFramebuffersOES
#define nglUnmapBufferARB nglUnmapBufferOES
#define nglActiveTextureARB glActiveTexture
#define nglClientActiveTextureARB glClientActiveTexture
#define nglBindBufferARB glBindBuffer
#define nglGenBuffersARB glGenBuffers
#define nglBufferDataARB glBufferData

#define glFrustum glFrustumf
#define glOrtho glOrthof
#define glDepthRange glDepthRangef

#else

#if defined(NL_OS_MAC)

// Mac GL extensions

#elif defined(NL_OS_UNIX)

// GLX extensions
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

