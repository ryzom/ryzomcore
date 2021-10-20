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



#ifndef CL_TELEPORT_H
#define CL_TELEPORT_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"


///////////
// CLASS //
///////////
/**
 * Manage teleportations
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CTeleport
{
public:
	// Unknown position
	static const NLMISC::CVectorD Unknown;

public:
	// Load Destinations.
	static void load(const std::string &filename);

	// Get the destination position or CTeleport::Unknown.
	static const NLMISC::CVectorD &getPos(const std::string &dest);


private:
	typedef std::map<std::string, NLMISC::CVectorD> TDestinations;
	static TDestinations _Destinations;
};

#endif // CL_TELEPORT_H

/* End of teleport.h */
