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

#include "nel/misc/types_nl.h"

#define EAX_AVAILABLE 0

#if EAX_AVAILABLE
#	include <eax.h>
#endif

#include <dsound.h>

#include <iostream>
#include <algorithm>
#include <cmath>

#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/log.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"
#include "nel/sound/driver/source.h"
#include "nel/sound/driver/listener.h"

/* end of file */
