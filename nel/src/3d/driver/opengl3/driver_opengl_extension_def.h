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

// ***************************************************************************
// ***************************************************************************
// The NEL Functions Typedefs.
// Must do it for compatibilities with futures version of gl.h
// eg: version 1.2 does not define PFNGLACTIVETEXTUREARBPROC. Hence, do it now, with our special name
// ***************************************************************************
// ***************************************************************************

#define WGL_COVERAGE_SAMPLES_NV            0x2042
#define WGL_COLOR_SAMPLES_NV               0x20B9

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

#ifdef __cplusplus
}
#endif

#endif // NL_OPENGL_EXTENSION_DEF_H

