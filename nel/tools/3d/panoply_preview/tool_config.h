// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLTOOLS_CONFIG_H
#define NLTOOLS_CONFIG_H
#include <nel/misc/types_nl.h>



// use the default log.log file (not erased on use)
// #define NLTOOLS_USE_LOG_LOG false



// the config file name
// #define NLTOOLS_CONFIG_FILE "panoply_preview.cfg"
// #define NLTOOLS_CONFIG_FILE_DEFAULT "panoply_preview_default.cfg"



// use panoply_preview log file
// #define NLTOOLS_USE_LOG 1

// panoply_preview log file name
#define NLTOOLS_LOG_FILE "panoply_preview.log"

// clear panoply_preview log before use
#define NLTOOLS_ERASE_LOG true

// version number
#define NLTOOLS_VERSION "0.10.0"



// use the low fragmentation heap (windows feature)
// #define NLTOOLS_LOW_FRAGMENTATION_HEAP 1



// temporary dev tags
//#define NL_DEV_STEREO 0
//#define NL_DEV_MEMLEAK 1
//#define NL_DEV_NET 0
//#define NL_DEV_NETNEW 1
//#define NL_DEV_CG 0
//#define NL_DEV_BULLET 0



// some default defines
#if FINAL_VERSION
#	if !defined(NLTOOLS_USE_LOG_LOG)
#		define NLTOOLS_USE_LOG_LOG false
#	endif
#	if !defined(NLTOOLS_USE_LOG)
#		define NLTOOLS_USE_LOG 0
#	endif
#endif

#if !defined (NLTOOLS_USE_LOG_LOG)
#	define NLTOOLS_USE_LOG_LOG true
#endif
#if !defined (NLTOOLS_USE_LOG)
#	define NLTOOLS_USE_LOG 1
#endif

#if !defined (NLTOOLS_LOW_FRAGMENTATION_HEAP)
#	ifdef NL_OS_WINDOWS
#		define NLTOOLS_LOW_FRAGMENTATION_HEAP 1
#	else
#		define NLTOOLS_LOW_FRAGMENTATION_HEAP 0
#	endif
#endif

// for compatibility with old configuration
#ifndef NLTOOLS_CONFIG_FILE
#	ifndef NLTOOLS_CONFIG
#		define NLTOOLS_CONFIG_FILE "panoply_preview.cfg"
#	else
#		define NLTOOLS_CONFIG_FILE NLTOOLS_CONFIG "panoply_preview.cfg"
#	endif
#endif

#ifndef NLTOOLS_CONFIG_FILE_DEFAULT
#	define NLTOOLS_CONFIG_FILE_DEFAULT "panoply_preview_default.cfg"
#endif



#endif /* #ifndef NLTOOLS_CONFIG_H */

/* end of file */
