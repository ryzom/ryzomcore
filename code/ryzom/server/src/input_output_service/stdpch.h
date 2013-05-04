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

#ifndef INPUT_OUTPUT_SERVICE_STDPCH_H
#define INPUT_OUTPUT_SERVICE_STDPCH_H

#include "nel/misc/types_nl.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <time.h>
#include <vector>
#include <limits>
#include <stddef.h>

#include "nel/georges/load_form.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

#include "nel/misc/algo.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/command.h"
#include "nel/misc/diff_tool.h"
#include "nel/misc/displayer.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/i18n.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/singleton.h"
#include "nel/misc/sstring.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/variable.h"

#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"

#include "game_share/backup_service_interface.h"
#include "game_share/base_types.h"
#include "game_share/body.h"
#include "game_share/bot_chat_types.h"
#include "game_share/brick_families.h"
#include "game_share/brick_types.h"
#include "game_share/characteristics.h"
#include "game_share/chat_group.h"
#include "game_share/damage_types.h"
#include "game_share/dyn_chat.h"
#include "game_share/ecosystem.h"
#include "game_share/fame.h"
#include "game_share/gender.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/loot_harvest_state.h"
#include "game_share/mirror.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/msg_client_server.h"
#include "game_share/news_types.h"
#include "game_share/people.h"
#include "game_share/player_visual_properties.h"
#include "game_share/power_types.h"
#include "game_share/properties.h"
#include "game_share/r2_basic_types.h"
#include "game_share/r2_share_itf.h"
#include "game_share/roles.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/ryzom_version.h"
#include "game_share/scores.h"
#include "game_share/shard_names.h"
#include "game_share/singleton_registry.h"
#include "game_share/skills.h"
#include "game_share/string_manager_sender.h"
#include "game_share/synchronised_message.h"
#include "game_share/tick_event_handler.h"

#include "server_share/char_name_mapper_itf.h"
#include "server_share/chat_unifier_itf.h"
#include "server_share/log_chat_gen.h"
#include "server_share/logger_service_client.h"
#include "server_share/r2_variables.h"

#endif //INPUT_OUTPUT_SERVICE_STDPCH_H

