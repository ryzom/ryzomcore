// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef GEORGES_EDITOR_QT_CONFIG_H
#define GEORGES_EDITOR_QT_CONFIG_H
#include <nel/misc/types_nl.h>

// use the default log.log file (not erased on use)
// #define NLQT_USE_LOG_LOG false

// the config file name
// #define NLQT_CONFIG_FILE "georges_editor_qt.cfg"
// #define NLQT_CONFIG_FILE_DEFAULT "georges_editor_qt_default.cfg"

// use tool log file
// #define NLQT_USE_LOG 1

// tool log file name
#define NLQT_LOG_FILE "georges_editor_qt.log"

// clear log before use
#define NLQT_ERASE_LOG true

// version number
#define NLQT_VERSION "1.0.0"

// use the low fragmentation heap (windows feature)
// #define NLQT_LOW_FRAGMENTATION_HEAP 1

// some default defines
#if FINAL_VERSION
#	if !defined(NLQT_USE_LOG_LOG)
#		define NLQT_USE_LOG_LOG false
#	endif
#	if !defined(NLQT_USE_LOG)
#		define NLQT_USE_LOG 0
#	endif
#endif

#if !defined (NLQT_USE_LOG_LOG)
#	define NLQT_USE_LOG_LOG true
#endif
#if !defined (NLQT_USE_LOG)
#	define NLQT_USE_LOG 1
#endif

#if !defined (NLQT_LOW_FRAGMENTATION_HEAP)
#	ifdef NL_OS_WINDOWS
#		define NLQT_LOW_FRAGMENTATION_HEAP 1
#	else
#		define NLQT_LOW_FRAGMENTATION_HEAP 0
#	endif
#endif

// for compatibility with old configuration
#ifndef NLQT_CONFIG_FILE
#	define NLQT_CONFIG_FILE "georges_editor_qt.cfg"
#endif

#ifndef NLQT_CONFIG_FILE_DEFAULT
#	define NLQT_CONFIG_FILE_DEFAULT "georges_editor_qt_default.cfg"
#endif

#endif /* #ifndef GEORGES_EDITOR_QT_CONFIG_H */

/* end of file */
