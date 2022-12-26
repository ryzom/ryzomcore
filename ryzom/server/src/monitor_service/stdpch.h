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




//----------------------------------------------------------------
// external files
//----------------------------------------------------------------

// this is up top because it contains a certain number of #pragmas to
// control compiler warnings with stlport

#include "nel/misc/types_nl.h"


//----------------------------------------------------------------
// std libs

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>


//----------------------------------------------------------------
// stl

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


//----------------------------------------------------------------
// nel

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/random.h"
#include "nel/misc/smart_ptr.h"

#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"

//----------------------------------------------------------------
// nel net
#include "nel/net/message.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"
#include "nel/net/callback_server.h"

//----------------------------------------------------------------
// game share

#include "game_share/ryzom_entity_id.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/player_visual_properties.h"
#include "ai_share/ai_event.h"

