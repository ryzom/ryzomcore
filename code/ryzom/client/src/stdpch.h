
#include <nel/misc/types_nl.h>

#include <stdlib.h>
#include <stdio.h>
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
#include <fstream>

#define USE_JPEG

#include <nel/misc/common.h>
#include <nel/misc/debug.h>

#include <nel/misc/stream.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/path.h>
#include <nel/misc/rgba.h>
#include <nel/misc/log.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/command.h>
#include <nel/misc/variable.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/algo.h>
#include <nel/misc/types_nl.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/string_mapper.h>
#include <nel/misc/smart_ptr.h>

#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_instance_group.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_texture.h>

#include <nel/net/callback_client.h>
#include <nel/net/udp_sock.h>

#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_global_position.h>
#include <nel/pacs/u_move_primitive.h>

#include <nel/sound/sound_anim_manager.h>

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/action_target_slot.h"

// Foutez pas d'include du client ici svp ! Grrr ! Hulud

#ifdef NL_OS_WINDOWS
#define NOMINMAX
#include	<windows.h>
#endif // NL_OS_WINDOWS
