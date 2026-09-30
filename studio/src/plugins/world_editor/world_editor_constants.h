// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011-2012  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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
const char *const PATH_MAP = "WorldEditorPathMap";

/// Directory holding the per-class primitive icons (*.ico) of the original editor.
/// Left empty, the directory is looked up through NeL's search paths, see
/// PrimitiveIcons::resolveIconDirectory().
const char *const ICON_PATH = "WorldEditorIconPath";

/// Directory each file dialog last pointed at. Kept apart per kind of file, because the
/// landscapes, the primitives and the project files live in different trees.
const char *const LAST_LAND_DIR = "WorldEditorLastLandDir";
const char *const LAST_PRIMITIVE_DIR = "WorldEditorLastPrimitiveDir";
const char *const LAST_WORLD_EDIT_DIR = "WorldEditorLastWorldEditDir";

/// Colour a selected primitive is drawn in, as #rrggbb. White - the old fixed value -
/// disappears on the pale zone bitmaps, which is what this is here to let you change.
const char *const SELECTION_COLOR = "WorldEditorSelectionColor";

/// State of the show/hide switches in the tool bar.
const char *const VISIBLE_COLLISIONS = "WorldEditorVisibleCollisions";
const char *const VISIBLE_LAND = "WorldEditorVisibleLand";
const char *const VISIBLE_ZONE_PRIMITIVES = "WorldEditorVisibleZonePrimitives";
const char *const VISIBLE_PATH_PRIMITIVES = "WorldEditorVisiblePathPrimitives";
const char *const VISIBLE_POINT_PRIMITIVES = "WorldEditorVisiblePointPrimitives";
const char *const VISIBLE_DETAILS = "WorldEditorVisibleDetails";
const char *const VISIBLE_GRID = "WorldEditorVisibleGrid";
const char *const VISIBLE_GRID_POINTS = "WorldEditorVisibleGridPoints";
const char *const VISIBLE_PACS = "WorldEditorVisiblePacs";

/// Directory holding one <continent>_pacs directory per continent, as in the unpacked
/// client data. Preferred over the pacs/ directory next to the landscape when it has
/// the continent. Empty: always use the one next to the landscape.
const char *const PACS_ROOT = "WorldEditorPacsRoot";

//resources
const char *const ICON_WORLD_EDITOR = ":/icons/ic_nel_world_editor.png";

/// Prefix of the compiled-in tree icons, converted from the resources of the MFC
/// editor. The names after the prefix are root/folder/property/point/line/zone plus
/// _closed, _opened or _hidden, and erro for a broken structure.
const char *const ICON_TREE_PREFIX = ":/icons/we_";
const char *const ICON_ROOT_PRIMITIVE = ":/icons/we_root_closed.png";

} // namespace Constants
} // namespace WorldEditor

#endif // WORLD_EDITOR_CONSTANTS_H
