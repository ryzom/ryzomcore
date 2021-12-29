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

#ifndef INTERFACE_H
#define INTERFACE_H

//
// Includes
//

#include <string>

#include <nel/misc/rgba.h>

namespace SBCLIENT {

//
// External functions
//

void	initInterface ();
void	updateInterface ();
void	releaseInterface ();

// Display the interface with a string and waiting a user event
// queryString -> the string that will be display to the user
// defaultString -> default prompt string
// prompt -> set the user prompt format (0=normal, 1=with_star 2=no_prompt)
// color -> the background color
void	askString (const std::string &queryString, const std::string &defaultString="", sint prompt=0, const NLMISC::CRGBA &color=NLMISC::CRGBA(0,0,64,128));

// Return true if the answer have answered with the user answer in answer string
bool	haveAnswer (std::string &answer);

// Return true if the interface is open
bool	interfaceOpen ();

} /* namespace SBCLIENT */

#endif // INTERFACE_H

/* End of interface.h */
