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
const char * const OBJECT_VIEWER_PLUGIN	= "ObjectViewer";

//mainwindow
const char * const MAIN_WINDOW = "ObjectViewer.MainWindow";

//settings
const char * const OBJECT_VIEWER_SECTION = "ObjectViewer";
const char * const GRAPHICS_DRIVER = "GraphicsDriver";
const char * const ENABLE_BLOOM = "EnableBloom";
const char * const ENABLE_SQUARE_BLOOM = "EnableSquareBloom";
const char * const BLOOM_DENSITY  = "BloomDensity";
const char * const QT_STYLE  = "QtStyle";
const char * const QT_PALETTE  = "QtPalette";
const char * const FONT  = "Font";

const char * const SOUND_ENABLE = "SoundEnable";
const char * const SOUND_DRIVER = "SoundDriver";
const char * const SOUND_DEVICE = "SoundDevice";
const char * const SOUND_AUTO_LOAD_SAMPLE = "SoundAutoLoadSample";
const char * const SOUND_ENABLE_OCCLUDE_OBSTRUCT = "SoundEnableOccludeObstruct";
const char * const SOUND_ENABLE_REVERB = "SoundEnableReverb";
const char * const SOUND_MANUAL_ROLL_OFF = "SoundManualRolloff";
const char * const SOUND_FORCE_SOFTWARE = "SoundForceSoftware";
const char * const SOUND_USE_ADCPM = "SoundUseADPCM";
const char * const SOUND_MAX_TRACK = "SoundMaxTrack";
const char * const SOUND_PACKED_SHEET_PATH = "SoundPackedSheetPath";
const char * const SOUND_SAMPLE_PATH = "SoundSamplePath";

const char * const VEGET_TILE_BANK = "VegetTileBank";
const char * const VEGET_TILE_FAR_BANK = "VegetTileFarBank";
const char * const VEGET_TEXTURE = "VegetTexture";
const char * const VEGET_LANDSCAPE_ZONES = "VegetLandscapeZones";
const char * const COARSE_MESH_TEXTURE = "CoarseMeshTexture";

} // namespace Constants
} // namespace NLQT

#endif // OBJECT_VIEWER_CONSTANTS_H
