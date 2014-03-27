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

#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES

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

