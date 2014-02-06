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

#include <nel/misc/types_nl.h>



// use the default log.log file (not erased on use)
// #define SBCLIENT_USE_LOG_LOG false



// the config file name
// #define SBCLIENT_CONFIG_FILE "snowballs_client.cfg"
// #define SBCLIENT_CONFIG_FILE_DEFAULT "snowballs_client_default.cfg"

// use snowballs log file
// #define SBCLIENT_USE_LOG 1

// snowballs log file name
#define SBCLIENT_LOG_FILE "snowballs_client.log"

// clear snowballs log before use
#define SBCLIENT_ERASE_LOG true

// version number
// 2.1
// - Bloom
// 2.2
// - OculusVR support
#define SBCLIENT_VERSION "2.2"



// temporary dev tags
#define SBCLIENT_DEV_SOUND 0
#define SBCLIENT_DEV_STEREO 0
#define SBCLIENT_DEV_MEMLEAK 0
#define SBCLIENT_DEV_PIXEL_PROGRAM 0



// some default defines
#if FINAL_VERSION
#	if !defined(SBCLIENT_USE_LOG_LOG)
#		define SBCLIENT_USE_LOG_LOG false
#	endif
#	if !defined(SBCLIENT_USE_LOG)
#		define SBCLIENT_USE_LOG 0
#	endif
#endif

#if !defined(SBCLIENT_USE_LOG_LOG)
#	define SBCLIENT_USE_LOG_LOG true
#endif
#if !defined (SBCLIENT_USE_LOG)
#	define SBCLIENT_USE_LOG 1
#endif

// for compatibility with old configuration
#ifndef SBCLIENT_CONFIG_FILE
#	ifndef SNOWBALLS_CONFIG
#		define SBCLIENT_CONFIG_FILE "snowballs_client.cfg"
#	else
#		define SBCLIENT_CONFIG_FILE SNOWBALLS_CONFIG "snowballs_client.cfg"
#	endif
#endif

#ifndef SBCLIENT_CONFIG_FILE_DEFAULT
#	define SBCLIENT_CONFIG_FILE_DEFAULT "snowballs_client_default.cfg"
#endif