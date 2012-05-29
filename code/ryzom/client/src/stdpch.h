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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <errno.h>

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
#include <fstream>
#include <functional>
#include <memory>
#include <limits>
#include <iterator>

#include <nel/misc/common.h>
#include <nel/misc/debug.h>

#include <nel/misc/algo.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/command.h>
#include <nel/misc/events.h>
#include <nel/misc/factory.h>
#include <nel/misc/file.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/i18n.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/log.h>
#include <nel/misc/matrix.h>
#include <nel/misc/md5.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/mouse_device.h>
#include <nel/misc/path.h>
#include <nel/misc/polygon.h>
#include <nel/misc/progress_callback.h>
#include <nel/misc/random.h>
#include <nel/misc/rgba.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/stream.h>
#include <nel/misc/string_conversion.h>
#include <nel/misc/string_mapper.h>
#include <nel/misc/system_info.h>
#include <nel/misc/system_utils.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/types_nl.h>
#include <nel/misc/ucstring.h>
#include <nel/misc/variable.h>
#include <nel/misc/vector.h>
#include <nel/misc/vector_2f.h>
#include <nel/misc/vectord.h>

#include <nel/georges/u_form_loader.h>
#include <nel/georges/u_form.h>

#include <nel/3d/landscapeig_manager.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_bone.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_instance_group.h>
#include <nel/3d/u_instance_material.h>
#include <nel/3d/u_landscape.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_particle_system_instance.h>
#include <nel/3d/u_particle_system_sound.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/u_transform.h>
#include <nel/3d/u_visual_collision_manager.h>

#include <nel/net/callback_client.h>
#include <nel/net/udp_sock.h>
#include <nel/net/email.h>

#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_global_position.h>
#include <nel/pacs/u_move_primitive.h>
#include <nel/pacs/u_retriever_bank.h>
#include <nel/pacs/u_primitive_block.h>

#include <nel/sound/sound_anim_manager.h>

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/action_target_slot.h"

// Foutez pas d'include du client ici svp ! Grrr ! Hulud

#ifdef NL_OS_WINDOWS
#define NOMINMAX
#include	<WinSock2.h>
#include	<windows.h>
#endif // NL_OS_WINDOWS
