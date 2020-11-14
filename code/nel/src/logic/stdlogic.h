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

#ifndef NL_STDLOGIC_H
#define NL_STDLOGIC_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <vector>
#include <map>
#include <string>
#include <limits>

#include <libxml/parser.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/stream.h"

#ifdef NL_OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define _WIN32_WINDOWS 0x0500
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0500
#	endif
#	ifndef NL_COMP_MINGW
#		define WINVER 0x0500
#		define NOMINMAX
#	endif
#	include <WinSock2.h>
#	include <Windows.h>
#endif

#endif // NL_STDMISC_H
