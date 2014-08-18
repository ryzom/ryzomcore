// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef OBJECT_VIEWER_CONSTANTS_H
#define OBJECT_VIEWER_CONSTANTS_H

namespace NLQT
{
namespace Constants
{
const char *const OBJECT_VIEWER_PLUGIN	= "ObjectViewer";

//mainwindow
const char *const MAIN_WINDOW = "ObjectViewer.MainWindow";

//settings
const char *const OBJECT_VIEWER_SECTION = "ObjectViewer";
const char *const GRAPHICS_DRIVER = "GraphicsDriver";
const char *const ENABLE_BLOOM = "EnableBloom";
const char *const ENABLE_SQUARE_BLOOM = "EnableSquareBloom";
const char *const BLOOM_DENSITY  = "BloomDensity";
const char *const QT_STYLE  = "QtStyle";
const char *const QT_PALETTE  = "QtPalette";
const char *const FONT  = "Font";

const char *const SOUND_ENABLE = "SoundEnable";
const char *const SOUND_DRIVER = "SoundDriver";
const char *const SOUND_DEVICE = "SoundDevice";
const char *const SOUND_AUTO_LOAD_SAMPLE = "SoundAutoLoadSample";
const char *const SOUND_ENABLE_OCCLUDE_OBSTRUCT = "SoundEnableOccludeObstruct";
const char *const SOUND_ENABLE_REVERB = "SoundEnableReverb";
const char *const SOUND_MANUAL_ROLL_OFF = "SoundManualRolloff";
const char *const SOUND_FORCE_SOFTWARE = "SoundForceSoftware";
const char *const SOUND_USE_ADCPM = "SoundUseADPCM";
const char *const SOUND_MAX_TRACK = "SoundMaxTrack";
const char *const SOUND_PACKED_SHEET_PATH = "SoundPackedSheetPath";
const char *const SOUND_SAMPLE_PATH = "SoundSamplePath";

const char *const VEGET_TILE_BANK = "VegetTileBank";
const char *const VEGET_TILE_FAR_BANK = "VegetTileFarBank";
const char *const VEGET_TEXTURE = "VegetTexture";
const char *const VEGET_LANDSCAPE_ZONES = "VegetLandscapeZones";
const char *const COARSE_MESH_TEXTURE = "CoarseMeshTexture";

const char *const ICON_ADD_ITEM = ":/icons/ic_nel_add_item.png";
const char *const ICON_INSERT_ITEM = ":/icons/ic_nel_insert_item.png";
const char *const ICON_DELETE_ITEM = ":/icons/ic_nel_delete_item.png";
const char *const ICON_DOWN_ITEM = ":/icons/ic_nel_down_item.png";
const char *const ICON_UP_ITEM = ":/icons/ic_nel_up_item.png";
const char *const ICON_CAMERA_ADD = ":/icons/ic_nel_camera_add.png";
const char *const ICON_CAMERA_DEL = ":/icons/ic_nel_camera_del.png";
const char *const ICON_CAMERA_3DEDIT = ":/icons/ic_nel_camera_3dedit.png";
const char *const ICON_CAMERA_FPS = ":/icons/ic_nel_camera_fps.png";
const char *const ICON_RESET_CAMERA = ":/icons/ic_nel_reset_camera.png";
const char *const ICON_ANIM = ":/icons/ic_nel_anim.png";
const char *const ICON_ANIMSET = ":/icons/ic_nel_animset.png";
const char *const ICON_BGCOLOR = ":/icons/ic_nel_bgcolor.png";
const char *const ICON_DAYNIGHT = ":/icons/ic_nel_daynight.png";
const char *const ICON_FRAMEDELAY = ":/icons/ic_nel_framedelay.png";
const char *const ICON_MIXER = ":/icons/ic_nel_mixer.png";
const char *const ICON_MRM_MESH = ":/icons/ic_nel_mrm_mesh.png";
const char *const ICON_PARTICLES = ":/icons/ic_nel_particles.png";
const char *const ICON_SKELSCALE = ":/icons/ic_nel_skelscale.png";
const char *const ICON_VEGET = ":/icons/ic_nel_veget.png";
const char *const ICON_VEGETSET = ":/icons/ic_nel_vegetset.png";
const char *const ICON_WATER = ":/icons/ic_nel_water.png";
const char *const ICON_WIND = ":/icons/ic_nel_wind.png";

const char *const ICON_COLLISION_ZONE_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_collision_zone_item_24.png";
const char *const ICON_EMITTER_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_emitter_item_24.png";
const char *const ICON_FORCE_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_force_item_24.png";
const char *const ICON_INSTANCE_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_instance_item_24.png";
const char *const ICON_LIGHT_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_light_item_24.png";
const char *const ICON_LOCATED_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_located_item_24.png";
const char *const ICON_PARTICLE_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_particle_item_24.png";
const char *const ICON_PARTICLE_SYSTEM_SMALL = ":/icons/particles_system_24/ic_nel_particle_system_24.png";
const char *const ICON_PARTICLE_SYSTEM_CLOSE_SMALL = ":/icons/particles_system_24/ic_nel_particle_system_close_24.png";
const char *const ICON_PARTICLES_SMALL = ":/icons/particles_system_24/ic_nel_particles_24.png";
const char *const ICON_SOUND_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_sound_item_24.png";
const char *const ICON_WORKSPACE_ITEM_SMALL = ":/icons/particles_system_24/ic_nel_workspace_item_24.png";

} // namespace Constants
} // namespace NLQT

#endif // OBJECT_VIEWER_CONSTANTS_H
