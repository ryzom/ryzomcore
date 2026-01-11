// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef STDLIGO_H
#define STDLIGO_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <algorithm>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <cstdlib>

// Include from libxml2
#include <libxml/parser.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/system_info.h"
#include "nel/misc/mem_displayer.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/path.h"
#include "nel/misc/string_common.h"
#include "nel/misc/config_file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/misc/file.h"

#ifdef NL_OS_WINDOWS
	#ifndef NL_COMP_MINGW
		#define NOMINMAX
	#endif
	#include <windows.h>
#endif

#endif
