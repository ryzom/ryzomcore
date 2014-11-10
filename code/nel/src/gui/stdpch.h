// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef NELGUI_H
#define NELGUI_H

#include <string>
#include <limits>

#include "nel/misc/types_nl.h"
#include "nel/misc/algo.h"
#include "nel/misc/factory.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/uv.h"
#include "nel/misc/hierarchical_timer.h"

#ifdef NL_OS_WINDOWS
	#ifndef NL_COMP_MINGW
	#	define NOMINMAX
	#endif
	#include <WinSock2.h>
	#include <windows.h>
#endif

#endif
