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
#include <stdio.h>
#include <stddef.h>
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
#include "nel/georges/load_form.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

// NeL ligo
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

// NeL misc
#include "nel/misc/algo.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/command.h"
#include "nel/misc/common.h"
#include "nel/misc/config_file.h"
#include "nel/misc/debug.h"
#include "nel/misc/eid_translator.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/eval_num_expr.h"
#include "nel/misc/file.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/log.h"
#include "nel/misc/matrix.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/progress_callback.h"
#include "nel/misc/random.h"
#include "nel/misc/rgba.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/singleton.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"
#include "nel/misc/stream.h"
#include "nel/misc/string_common.h"
#include "nel/misc/string_conversion.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/variable.h"
#include "nel/misc/vector.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vectord.h"

// NeL net
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"

// GameShare
#include "game_share/action_nature.h"
#include "game_share/backup_service_interface.h"
#include "game_share/base_types.h"
#include "game_share/bot_chat_types.h"
#include "game_share/brick_families.h"
#include "game_share/brick_flags.h"
#include "game_share/brick_types.h"
#include "game_share/character_sync_itf.h"
#include "game_share/characteristics.h"
#include "game_share/chat_group.h"
#include "game_share/client_action_type.h"
#include "game_share/constants.h"
#include "game_share/continent.h"
#include "game_share/damage_types.h"
#include "game_share/dyn_chat.h"
#include "game_share/ecosystem.h"
#include "game_share/effect_families.h"
#include "game_share/entity_types.h"
#include "game_share/fame.h"
#include "game_share/far_position.h"
#include "game_share/gender.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/guild_grade.h"
#include "game_share/inventories.h"
#include "game_share/item_family.h"
#include "game_share/item_type.h"
#include "game_share/lift_icons.h"
#include "game_share/magic_fx.h"
#include "game_share/mainland_summary.h"
#include "game_share/mirror.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/misc_const.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/msg_client_server.h"
#include "game_share/outpost.h"
#include "game_share/people.h"
#include "game_share/persistent_data.h"
#include "game_share/power_types.h"
#include "game_share/protection_type.h"
#include "game_share/pvp_clan.h"
#include "game_share/r2_share_itf.h"
#include "game_share/r2_types.h"
#include "game_share/rm_family.h"
#include "game_share/roles.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/scenario.h"
#include "game_share/scores.h"
#include "game_share/season.h"
#include "game_share/send_chat.h"
#include "game_share/shard_names.h"
#include "game_share/shield_types.h"
#include "game_share/skills.h"
#include "game_share/slot_equipment.h"
#include "game_share/sp_type.h"
#include "game_share/string_manager_sender.h"
#include "game_share/temp_inventory_mode.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "game_share/timer.h"
#include "game_share/utils.h"
#include "game_share/visual_fx.h"
#include "game_share/visual_slot_manager.h"

// ServerShare
#include "server_share/creature_size.h"
#include "server_share/effect_manager.h"
#include "server_share/effect_message.h"
#include "server_share/entity_state.h"
#include "server_share/log_character_gen.h"
#include "server_share/log_item_gen.h"
#include "server_share/mail_forum_validator.h"
#include "server_share/msg_ai_service.h"
#include "server_share/msg_brick_service.h"
#include "server_share/npc_description_messages.h"
#include "server_share/pet_interface_msg.h"
#include "server_share/pvp_relation.h"
#include "server_share/r2_variables.h"
#include "server_share/r2_vision.h"
#include "server_share/respawn_point_type.h"
#include "server_share/stl_allocator_checker.h"
#include "server_share/used_continent.h"

// AIShare
#include "ai_share/ai_event_report.h"

// PDLib
#include "pd_lib/pd_lib.h"

// NO EGS INCLUDE HERE PLEASE !!!

//#include "player_manager/cdb_synchronised.h"
//#include "egs_log_filter.h"
