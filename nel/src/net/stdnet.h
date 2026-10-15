// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_STDNET_H
#define NL_STDNET_H

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <csignal>
#include <queue>

#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <utility>
#include <cstdlib>
#include <algorithm>
#include <exception>
#include <cctype>
#include <limits>

#include <errno.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/hierarchical_timer.h"

#ifdef NL_OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	ifndef _WIN32_WINDOWS
#		define _WIN32_WINDOWS 0x0410
	#endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0500
#	endif
#	ifndef NL_COMP_MINGW
#		ifndef WINVER
#			define WINVER 0x0500
#		endif
#		ifndef NOMINMAX
#			define NOMINMAX
#		endif
#	endif
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <windows.h>
#endif

#endif
