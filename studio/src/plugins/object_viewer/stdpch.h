/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef NL_STDPCH_H
#define NL_STDPCH_H

#include <QtCore/QtCore>
#include <QtGui/QtGui>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <nel/misc/types_nl.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/debug.h>
#include <nel/misc/event_emitter.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/matrix.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/noise_value.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/path.h>
#include <nel/misc/quat.h>
#include <nel/misc/rgba.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/stream.h>
#include <nel/misc/vectord.h>
#include <nel/misc/vector.h>

#include <nel/3d/animation_time.h>
#include <nel/3d/bloom_effect.h>
#include <nel/3d/bone.h>
#include <nel/3d/channel_mixer.h>
#include <nel/3d/driver.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/material.h>
#include <nel/3d/particle_system.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/particle_system_shape.h>
#include <nel/3d/ps_attrib_maker_bin_op.h>
#include <nel/3d/ps_attrib_maker.h>
#include <nel/3d/ps_attrib_maker_template.h>
#include <nel/3d/ps_color.h>
#include <nel/3d/ps_direction.h>
#include <nel/3d/ps_edit.h>
#include <nel/3d/ps_emitter.h>
#include <nel/3d/ps_float.h>
#include <nel/3d/ps_force.h>
#include <nel/3d/ps_int.h>
#include <nel/3d/ps_light.h>
#include <nel/3d/ps_located.h>
#include <nel/3d/ps_mesh.h>
#include <nel/3d/ps_particle2.h>
#include <nel/3d/ps_particle_basic.h>
#include <nel/3d/ps_particle.h>
#include <nel/3d/ps_plane_basis.h>
#include <nel/3d/ps_plane_basis_maker.h>
#include <nel/3d/ps_sound.h>
#include <nel/3d/ps_zone.h>
#include <nel/3d/scene.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/shape_bank.h>
#include <nel/3d/skeleton_model.h>
#include <nel/3d/skeleton_shape.h>
#include <nel/3d/text_context_user.h>
#include <nel/3d/texture_bump.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_grouped.h>
#include <nel/3d/texture.h>
#include <nel/3d/tile_vegetable_desc.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_animation.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_bone.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance_group.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_light.h>
#include <nel/3d/u_particle_system_sound.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_track.h>
#include <nel/3d/u_water.h>
#include <nel/3d/vegetable.h>
#include <nel/3d/visual_collision_manager.h>

#include <nel/sound/sound_animation.h>
#include <nel/sound/sound_anim_manager.h>
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>

#endif
