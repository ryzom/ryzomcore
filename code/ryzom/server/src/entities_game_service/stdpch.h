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

#include <nel/misc/types_nl.h>


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


// STL
#include <algorithm>
#include <deque>
#include <exception>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


// NeL misc
#include <nel/misc/algo.h>
#include <nel/misc/command.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/eval_num_expr.h>
#include <nel/misc/file.h>
#include <nel/misc/matrix.h>
#include <nel/misc/random.h>
#include <nel/misc/rgba.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/stream.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/variable.h>
#include <nel/misc/vector.h>


// GameShare
#include "game_share/generic_xml_msg_mngr.h"
#include "server_share/msg_ai_service.h"
#include "game_share/people.h"
#include "game_share/persistent_data.h"
#include "game_share/skills.h"
#include "game_share/string_manager_sender.h"
#include "game_share/tick_event_handler.h"
#include "game_share/outpost.h"


// NO EGS INCLUDE HERE PLEASE !!!

//#include "player_manager/cdb_synchronised.h"
//#include "egs_log_filter.h"





