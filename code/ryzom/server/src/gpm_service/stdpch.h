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

#include <errno.h>
#include <fcntl.h>
#include <libxml/parser.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STL
#include <algorithm>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// NeL georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

// NeL ligo
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"

// NeL misc
#include "nel/misc/aabbox.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/command.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/variable.h"
#include "nel/misc/vector.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"

// NeL net
#include "nel/net/message.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"

// NeL pacs
#include "nel/pacs/u_collision_desc.h"
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_primitive_block.h"

// GameShare
#include "game_share/mirror_prop_value.h"
#include "game_share/player_vision_delta.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/synchronised_message.h"
#include "game_share/tick_event_handler.h"
#include "game_share/utils.h"

// ServerShare
#include "server_share/effect_manager.h"
#include "server_share/msg_gpm_service.h"
#include "server_share/pet_interface_msg.h"
#include "server_share/r2_variables.h"
#include "server_share/r2_vision.h"
#include "server_share/used_continent.h"
