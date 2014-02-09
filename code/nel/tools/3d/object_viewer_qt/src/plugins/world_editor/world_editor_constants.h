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

#ifndef WORLD_EDITOR_CONSTANTS_H
#define WORLD_EDITOR_CONSTANTS_H

namespace WorldEditor
{
namespace Constants
{
const char *const WORLD_EDITOR_PLUGIN	= "WorldEditor";

const int USER_TYPE = 65536;
const int NODE_PERISTENT_INDEX = USER_TYPE + 1;
const int WORLD_EDITOR_NODE = USER_TYPE + 2;
const int GRAPHICS_DATA_QT4_2D = USER_TYPE + 3;
const int GRAPHICS_DATA_NEL3D = USER_TYPE + 4;
const int PRIMITIVE_IS_MODIFIED = USER_TYPE + 5;
const int PRIMITIVE_FILE_IS_CREATED = USER_TYPE + 6;
const int PRIMITIVE_IS_VISIBLE = USER_TYPE + 7;
const int PRIMITIVE_IS_ENABLD = USER_TYPE + 8;
const int PRIMITIVE_FILE_NAME = USER_TYPE + 9;
const int PRIMITIVE_NON_REMOVABLE = USER_TYPE + 10;
const int ROOT_PRIMITIVE_CONTEXT = USER_TYPE + 20;
const int ROOT_PRIMITIVE_DATA_DIRECTORY = USER_TYPE + 21;

//properties editor
const char *const DIFFERENT_VALUE_STRING = "<different values>";
const char *const DIFFERENT_VALUE_MULTI_STRING = "<diff>";

//settings
const char *const WORLD_EDITOR_SECTION = "WorldEditor";
const char *const WORLD_WINDOW_STATE = "WorldWindowState";
const char *const WORLD_WINDOW_GEOMETRY = "WorldWindowGeometry";
const char *const WORLD_EDITOR_CELL_SIZE = "WorldEditorCellSize";
const char *const WORLD_EDITOR_SNAP = "WorldEditorSnap";
const char *const WORLD_EDITOR_USE_OPENGL = "WorldEditorUseOpenGL";
const char *const ZONE_SNAPSHOT_RES = "WorldEditorZoneSnapshotRes";
const char *const PRIMITIVE_CLASS_FILENAME = "WorldEditorPrimitiveClassFilename";

//resources
const char *const ICON_WORLD_EDITOR = ":/icons/ic_nel_world_editor.png";
const char *const ICON_ROOT_PRIMITIVE = "./old_ico/root.ico";
const char *const ICON_PROPERTY = "./old_ico/property.ico";
const char *const ICON_FOLDER = "./old_ico/folder_h.ico";
const char *const PATH_TO_OLD_ICONS = "./old_ico";

} // namespace Constants
} // namespace WorldEditor

#endif // WORLD_EDITOR_CONSTANTS_H
