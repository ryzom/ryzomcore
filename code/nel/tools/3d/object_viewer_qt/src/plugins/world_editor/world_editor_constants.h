// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef WORLD_EDITOR_CONSTANTS_H
#define WORLD_EDITOR_CONSTANTS_H

namespace WorldEditor
{
namespace Constants
{
const char *const WORLD_EDITOR_PLUGIN	= "WorldEditor";

//settings
const char *const WORLD_EDITOR_SECTION = "WorldEditor";
const char *const WORLD_WINDOW_STATE = "WorldWindowState";
const char *const WORLD_WINDOW_GEOMETRY = "WorldWindowGeometry";
const char *const WORLD_EDITOR_CELL_SIZE = "WorldEditorCellSize";
const char *const WORLD_EDITOR_SNAP = "WorldEditorSnap";
const char *const ZONE_SNAPSHOT_RES = "WorldEditorZoneSnapshotRes";
const char *const PRIMITIVE_CLASS_FILENAME = "WorldEditorPrimitiveClassFilename";

//resources
const char *const ICON_WORLD_EDITOR = ":/icons/ic_nel_world_editor.png";


} // namespace Constants
} // namespace WorldEditor

#endif // WORLD_EDITOR_CONSTANTS_H
