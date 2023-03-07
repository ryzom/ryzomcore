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

#ifdef NL_OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <windowsx.h>
#endif

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

#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"

#include "nel/3d/driver.h"

#define NL_OPENGL3_MAX_LIGHT 8
