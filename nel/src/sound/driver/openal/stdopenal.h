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

#ifndef STDOPENAL_H
#define STDOPENAL_H

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <exception>
#include <utility>
#include <deque>
#include <queue>
#include <cfloat>
#include <algorithm>
#include <limits>

#include "nel/misc/types_nl.h"

#ifdef NL_OS_MAC
#	include <al.h>
#	include <alc.h>
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/singleton.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/path.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/thread.h"

#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"
#include "nel/sound/driver/source.h"
#include "nel/sound/driver/listener.h"
#include "nel/sound/driver/effect.h"

#endif
/* end of file */
