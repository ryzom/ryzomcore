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

#ifndef NL_STD3D_H
#define NL_STD3D_H

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

#include <deque>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <exception>
#include <memory>
#include <functional>
#include <iostream>
#include <limits>
#include <iterator>

#include "nel/misc/types_nl.h"

#include "nel/misc/rgba.h"
#include "nel/misc/debug.h"

#include "nel/misc/common.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/time_nl.h"

#endif
