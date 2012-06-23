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

#ifndef NLSOUND_STDPCH_XAUDIO2_H
#define NLSOUND_STDPCH_XAUDIO2_H

#include "nel/misc/types_nl.h"

// STL includes
#include <iostream>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <limits>

// 3rd Party Includes
#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>

// NeL includes
#include "nel/misc/singleton.h"
#include "nel/misc/common.h"
#include "nel/misc/big_file.h"
#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/file.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/log.h"
#include "nel/misc/matrix.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/mutex.h"
#include "nel/misc/path.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/variable.h"
#include "nel/misc/vector.h"

#include "nel/sound/driver/buffer.h"
#include "nel/sound/driver/effect.h"
#include "nel/sound/driver/listener.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/source.h"

// Defines
#define NLSOUND_XAUDIO2_NAME "NeLSound XAudio2 Driver"
// prefix for warnings and debug messages
#define NLSOUND_XAUDIO2_PREFIX "XA2: "
// swap y and z
#define NLSOUND_XAUDIO2_X3DAUDIO_VECTOR_FROM_VECTOR(to, from) (to).x = (from).x; (to).y = (from).z; (to).z = (from).y;
#define NLSOUND_XAUDIO2_VECTOR_FROM_X3DAUDIO_VECTOR(to, from) (to).x = (from).x; (to).y = (from).z; (to).z = (from).y;

#endif /* #ifndef NLSOUND_STDPCH_XAUDIO2_H */

/* end of file */
