// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2017  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef NELGUI_H
#define NELGUI_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <string>
#include <limits>
#include <ctime>

#include "nel/misc/types_nl.h"
#include "nel/misc/algo.h"
#include "nel/misc/factory.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/uv.h"
#include "nel/misc/hierarchical_timer.h"

#include <libxml/parser.h>

#ifdef NL_OS_WINDOWS
	#ifndef NL_COMP_MINGW
	#	define NOMINMAX
	#endif
	#include <winsock2.h>
	#include <windows.h>
#endif

#endif
