// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/types_nl.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <exception>
#include <utility>
#include <deque>
#include <limits>
#include <sstream>

#ifdef NL_OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <windowsx.h>
#endif

#ifdef USE_OPENGLES3
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#endif

// GL constants not present in GLES3 but referenced by the GL3 driver code
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_POINT
#define GL_POINT 0x1B00
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_SAMPLES_PASSED
#define GL_SAMPLES_PASSED 0x8914
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif
#ifndef GL_TEXTURE_FREE_MEMORY_ATI
#define GL_TEXTURE_FREE_MEMORY_ATI 0x87FC
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Y
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB GL_TEXTURE_CUBE_MAP_POSITIVE_Z
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_CONSTANT_COLOR_EXT
#define GL_CONSTANT_COLOR_EXT GL_CONSTANT_COLOR
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT GL_ONE_MINUS_CONSTANT_COLOR
#define GL_CONSTANT_ALPHA_EXT GL_CONSTANT_ALPHA
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT GL_ONE_MINUS_CONSTANT_ALPHA
#endif

// Desktop GL constants not present in GLES3 used by the driver code
#ifndef GL_RGB5
#define GL_RGB5 0x8050
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif
#ifndef GL_ADD
#define GL_ADD 0x0104
#endif
#ifndef GL_ADD_SIGNED_EXT
#define GL_ADD_SIGNED_EXT 0x8574
#endif
#ifndef GL_INTERPOLATE_EXT
#define GL_INTERPOLATE_EXT 0x8575
#endif
#ifndef GL_CONSTANT_EXT
#define GL_CONSTANT_EXT 0x8576
#endif
#ifndef GL_PREVIOUS_EXT
#define GL_PREVIOUS_EXT 0x8578
#endif
#ifndef GL_PRIMARY_COLOR_EXT
#define GL_PRIMARY_COLOR_EXT 0x8577
#endif
#ifndef GL_BUMP_ENVMAP_ATI
#define GL_BUMP_ENVMAP_ATI 0x877B
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE 0x84F5
#endif
#ifndef GL_TEXTURE_SWIZZLE_RGBA
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#endif
#ifndef GL_CLIP_DISTANCE0
#define GL_CLIP_DISTANCE0 0x3000
#endif
// Desktop GL sampler types not in GLES3
#ifndef GL_SAMPLER_1D
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_1D_ARRAY 0x8DC0
#define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#define GL_SAMPLER_2D_RECT 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#define GL_SAMPLER_BUFFER 0x8DC2
#define GL_INT_SAMPLER_1D 0x8DC9
#define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#define GL_INT_SAMPLER_2D_RECT 0x8DCD
#define GL_INT_SAMPLER_BUFFER 0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#endif
// Separate shader objects bits
#ifndef GL_VERTEX_SHADER_BIT
#define GL_VERTEX_SHADER_BIT 0x00000001
#define GL_FRAGMENT_SHADER_BIT 0x00000002
#endif
#ifndef GL_DOUBLE
#define GL_DOUBLE 0x140A
#endif
#ifndef GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD 0x9160
#endif
// Desktop-only GL functions - provide no-op stubs for GLES3
#ifndef glPolygonMode
inline void glPolygonMode(GLenum, GLenum) { }
#endif
#ifndef glDepthRange
#define glDepthRange glDepthRangef
#endif

#else
#ifdef NL_OS_WINDOWS
#include <GL/gl.h>
#include "GL/wglext.h"
#elif defined(NL_OS_MAC)
#define GL_GLEXT_LEGACY
#include <OpenGL/gl.h>
#elif defined (NL_OS_UNIX)
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include "GL/glxext.h"
#endif
#include "GL/glext.h"
#endif

#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"

#include "nel/3d/driver.h"

#define NL_OPENGL3_MAX_LIGHT 8

#define NL_OPENGL3_MAX_LIGHT_TABLE_CAPACITY 128 // Hard limit for CPU staging buffers
#define NL_OPENGL3_ANGLE_MAX_LIGHT_TABLE 8      // Reduced limit for ANGLE/D3D11
