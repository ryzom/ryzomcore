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

#ifndef CL_MAIN_LOOP_UTILITIES_H
#define CL_MAIN_LOOP_UTILITIES_H

#include <nel/misc/types_nl.h>

// Update Utilities (configuration etc)
// Only put utility update functions here that are used in the main loop.
// This is mainly for system configuration related functions 
// such as config file changes.

/// Compare ClientCfg and LastClientCfg to know what we must update
void updateFromClientCfg();

#endif // CL_MAIN_LOOP_UTILITIES_H

/* end of file */
