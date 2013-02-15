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

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
//#include <sstream>
#include <exception>
#include <utility>
#include <deque>

#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/buf_fifo.h"
#include "nel/misc/variable.h"

#include "nel/net/unified_network.h"
#include "nel/net/login_server.h"
#include "nel/net/inet_address.h"
#include "nel/misc/sheet_id.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/cst_loader.h"
#include "game_share/action_factory.h"
#include "game_share/action_position.h"
#include "game_share/action_sync.h"
#include "game_share/action_disconnection.h"
#include "game_share/action_association.h"
#include "game_share/action_association.h"
#include "game_share/action_login.h"
#include "game_share/action_generic.h"
#include "game_share/action_generic_multi_part.h"
#include "game_share/action_sint64.h"
#include "game_share/action_dummy.h"
#include "game_share/action_target_slot.h"
#include "game_share/simlag.h"


#include "game_share/entity_types.h"

#ifndef NL_RELEASE
#ifndef TRACE_SHARD_MESSAGES
#define TRACE_SHARD_MESSAGES
#endif
#endif

