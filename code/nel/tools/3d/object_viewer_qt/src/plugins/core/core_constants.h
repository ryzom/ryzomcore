// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2010  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef CORE_CONSTANTS_H
#define CORE_CONSTANTS_H

namespace Core
{
namespace Constants
{

const char * const OVQT_VERSION_LONG	= "0.0.1";
const char * const OVQT_VENDOR		= "Dzmitry Kamiahin";
const char * const OVQT_YEAR		= "2010, 2011";
const char * const OVQT_CORE_PLUGIN	= "Core";

//mainwindow
const char * const MAIN_WINDOW		= "ObjectViewerQt.MainWindow";

//menubar
const char * const MENU_BAR		= "ObjectViewerQt.MenuBar";

//menus
const char * const M_FILE		= "ObjectViewerQt.Menu.File";
const char * const M_EDIT		= "ObjectViewerQt.Menu.Edit";
const char * const M_VIEW		= "ObjectViewerQt.Menu.View";
const char * const M_SCENE		= "ObjectViewerQt.Menu.Scene";
const char * const M_TOOLS		= "ObjectViewerQt.Menu.Tools";
const char * const M_WINDOW		= "ObjectViewerQt.Menu.Window";
const char * const M_HELP		= "ObjectViewerQt.Menu.Help";

//actions
const char * const NEW			= "ObjectViewerQt.New";
const char * const OPEN			= "ObjectViewerQt.Open";
const char * const EXIT			= "ObjectViewerQt.Exit";

const char * const SETTINGS		= "ObjectViewerQt.Settings";
const char * const TOGGLE_FULLSCREEN	= "ObjectViewerQt.ToggleFullScreen";

const char * const CLOSE		= "ObjectViewerQt.Close";
const char * const CLOSEALL		= "ObjectViewerQt.CloseAll";
const char * const CLOSEOTHERS		= "ObjectViewerQt.CloseOthers";
const char * const ABOUT		= "ObjectViewerQt.About";
const char * const ABOUT_PLUGINS	= "ObjectViewerQt.AboutPlugins";
const char * const ABOUT_QT		= "ObjectViewerQt.AboutQt";

//settings
const char * const DATA_PATH_SECTION = "DataPath";
const char * const SEARCH_PATHS = "SearchPaths";
const char * const RECURSIVE_SEARCH_PATHS = "RecursiveSearchPathes";
const char * const LEVELDESIGN_PATH = "LevelDesignPath";
const char * const ASSETS_PATH  = "LevelDesignPath";

//resources
const char * const ICON_NEL		= ":/core/images/nel.png";
const char * const ICON_SETTINGS	= ":/core/images/preferences.png";
} // namespace Constants
} // namespace Core

#endif // CORE_CONSTANTS_H
