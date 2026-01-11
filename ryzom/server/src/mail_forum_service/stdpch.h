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
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/stream.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/command.h>
#include <nel/misc/variable.h>

// NeL net
#include <nel/net/service.h>

// game share
#include "game_share/utils.h"
#include "server_share/stat_db_msg.h"
#include "server_share/stat_db_tree.h"
#include "server_share/stat_db_tree_pd.h"
#include "server_share/stat_db_tree_visitor.h"
