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

#ifndef CORE_CONSTANTS_H
#define CORE_CONSTANTS_H

namespace Core {
namespace Constants {

const char * const OVQT_VERSION_LONG      = "0.0.1";
const char * const OVQT_VENDOR            = "Dzmitry Kamiahin";
const char * const OVQT_YEAR              = "2010";

//mainwindow
const char * const MAIN_WINDOW           = "ObjectViewerQt.MainWindow";

//menubar
const char * const MENU_BAR              = "ObjectViewerQt.MenuBar";

//menus
const char * const M_FILE                = "ObjectViewerQt.Menu.File";
const char * const M_EDIT                = "ObjectViewerQt.Menu.Edit";
const char * const M_SCENE               = "ObjectViewerQt.Menu.Scene";
const char * const M_TOOLS               = "ObjectViewerQt.Menu.Tools";
const char * const M_WINDOW              = "ObjectViewerQt.Menu.Window";
const char * const M_HELP                = "ObjectViewerQt.Menu.Help";

//actions
const char * const UNDO                  = "ObjectViewerQt.Undo";
const char * const REDO                  = "ObjectViewerQt.Redo";

const char * const NEW                   = "ObjectViewerQt.New";
const char * const OPEN                  = "ObjectViewerQt.Open";
const char * const SAVE                  = "ObjectViewerQt.Save";
const char * const SAVEAS                = "ObjectViewerQt.SaveAs";
const char * const SAVEALL               = "ObjectViewerQt.SaveAll";
const char * const EXIT                  = "ObjectViewerQt.Exit";

const char * const SETTINGS              = "ObjectViewerQt.Settings";
const char * const TOGGLE_FULLSCREEN     = "ObjectViewerQt.ToggleFullScreen";

const char * const MINIMIZE_WINDOW       = "ObjectViewerQt.MinimizeWindow";
const char * const ZOOM_WINDOW           = "ObjectViewerQt.ZoomWindow";

const char * const CLOSE                 = "ObjectViewerQt.Close";
const char * const CLOSEALL              = "ObjectViewerQt.CloseAll";
const char * const CLOSEOTHERS           = "ObjectViewerQt.CloseOthers";
const char * const ABOUT       		 = "ObjectViewerQt.About";
const char * const ABOUT_PLUGINS         = "ObjectViewerQt.AboutPlugins";
const char * const ABOUT_QT              = "ObjectViewerQt.AboutQt";

const char * const ICON_NEL            	  = ":/images/nel.png";
const char * const ICON_NEL_IDE           = ":/images/nel_ide_load.png</file>
const char * const ICON_OPENFILE          = ":/images/open-file.png";
const char * const ICON_GO_DOWN           = ":/images/go-down.png</file>
const char * const ICON_GO_UP             = ":/images/go-up.png</file>
const char * const ICON_LIST_ADD          = ":/images/list-add.png</file>
const char * const ICON_LIST_REMOVE       = ":/images/list-remove.png</file>
const char * const ICON_PLAY              = ":/images/play.png</file>
const char * const ICON_PAUSE             = ":/images/pause.png</file>
const char * const ICON_STOP              = ":/images/stop.png</file>
const char * const ICON_SEEK_BACKWARD     = ":/images/seek-backward.png</file>
const char * const ICON_SEEK_FORWARD      = ":/images/seek-forward.png</file>
const char * const ICON_SKIP_BACKWARD     = ":/images/skip-backward.png</file>
const char * const ICON_SKIP_FORWARD      = ":/images/skip-forward.png</file>
const char * const ICON_SETTINGS          = ":/images/preferences.png</file>
const char * const ICON_TIME              = ":/images/time.png</file>
const char * const ICON_ANIM              = ":/images/anim.png</file>
const char * const ICON_ANIMSET           = ":/images/animset.png</file>
const char * const ICON_DAY_NIGHT         = ":/images/dqynight.png</file>
const char * const ICON_MIXER             = ":/images/mixer.png</file>
const char * const ICON_PARTICLES         = ":/images/pqrticles.png</file>
const char * const ICON_SOUND             = ":/images/sound.png</file>
const char * const ICON_VEGETABLE         = ":/images/veget.png</file>
const char * const ICON_WATER             = ":/images/water.png</file>
const char * const ICON_WIND              = ":/images/wind.png</file>
const char * const ICON_BACKGROUNDCOLOR   = ":/images/ico_bgcolor.png</file>
const char * const ICON_FRAMEDELAY        = ":/images/ico_framedelay.png</file>
const char * const ICON_SKELSCALE         = ":/images/ico_skelscale.png</file>
const char * const ICON_CLEAR             = ":/images/clear.png</file>
const char * const ICON_INSERT            = ":/images/insert.png</file>
const char * const ICON_NEW               = ":/images/new.png</file>
const char * const ICON_REFRESH           = ":/images/refresh.png</file>
const char * const ICON_SAVE_AS           = ":/images/save-as.png</file>
const char * const ICON_SAVE              = ":/images/save.png</file>
const char * const ICON_INSERT_HORIZONTAL = ":/images/insert-horizontal.png</file>

} // namespace Constants
} // namespace Core

#endif // CORE_CONSTANTS_H
