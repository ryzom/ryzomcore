// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef LANDSCAPE_EDITOR_CONSTANTS_H
#define LANDSCAPE_EDITOR_CONSTANTS_H

namespace LandscapeEditor
{
namespace Constants
{
const char *const LANDSCAPE_EDITOR_PLUGIN	= "LandscapeEditor";

//settings
const char *const LANDSCAPE_EDITOR_SECTION = "LandscapeEditor";
const char *const LANDSCAPE_WINDOW_STATE = "LandscapeWindowState";
const char *const LANDSCAPE_WINDOW_GEOMETRY = "LandscapeWindowGeometry";
const char *const LANDSCAPE_DATA_DIRECTORY = "LandscapeDataDirectory";
const char *const LANDSCAPE_USE_OPENGL = "LandscapeUseOpenGL";

//resources
const char *const ICON_LANDSCAPE_ITEM = ":/icons/ic_nel_landscape_item.png";
const char *const ICON_ZONE_ITEM = ":/icons/ic_nel_zone.png";
const char *const ICON_LANDSCAPE_ZONES = ":/icons/ic_nel_zones.png";


} // namespace Constants
} // namespace LandscapeEditor

#endif // LANDSCAPE_EDITOR_CONSTANTS_H
