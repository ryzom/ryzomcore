/** \file commands.h
 * Snowballs 2 specific code for managing the command interface
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

//
// Includes
//

#include <string>

#include <nel/misc/log.h>

namespace SBCLIENT {

//
// External variables
//

extern NLMISC::CLog CommandsLog;

//
// External functions
//

void	initCommands ();
void	updateCommands ();
void	releaseCommands ();

void	clearCommands ();
void	addLine (const std::string &line);

} /* namespace SBCLIENT */

#endif // COMMANDS_H

/* End of commands.h */
