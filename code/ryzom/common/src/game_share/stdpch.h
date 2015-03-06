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

#include "nel/misc/types_nl.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <exception>
#include <utility>
#include <deque>
#include <limits>
#include <queue>
#include <memory>
#include <functional>

#include <nel/misc/common.h>
#include <nel/misc/debug.h>

#include <nel/misc/stream.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/rgba.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/command.h>
#include <nel/misc/config_file.h>
#include <nel/misc/variable.h>
#include <nel/misc/shared_memory.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/singleton.h>
#include <nel/misc/string_common.h>
#include <nel/misc/sstring.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>

#include <nel/net/udp_sock.h>
#include <nel/net/unified_network.h>
#include <nel/net/service.h>

#include <nel/georges/load_form.h>

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <WinSock2.h>
#	include <Windows.h>
#endif
