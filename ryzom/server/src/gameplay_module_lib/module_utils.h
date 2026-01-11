// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef RY_MODULE_UTILS_H
#define RY_MODULE_UTILS_H

#include "nel/misc/types_nl.h"

// comment / uncomment that to have a debug version of the library
#define RY_MODULE_DEBUG


#ifdef RY_MODULE_DEBUG
	inline void MODULE_INFO(const char*,...) {}
	#define MODULE_AST nlassert
	#define MODULE_CAST static_cast
#else
	inline void MODULE_INFO(const char*,...) {}
	inline void MODULE_AST(const char*,...) {}
	#define MODULE_CAST dynamic_cast
#endif


#endif
