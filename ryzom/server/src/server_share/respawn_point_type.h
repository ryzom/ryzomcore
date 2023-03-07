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



#ifndef RYZOM_RESPAWN_POINT_TYPE_H
#define RYZOM_RESPAWN_POINT_TYPE_H

#include "nel/misc/types_nl.h"

#include <string>

namespace RESPAWN_POINT
{
	enum TRespawnPointType
	{
		KAMI = 0,
		KARAVAN,
		NEWBIELAND,
		OUTPOST,
		RESPAWNABLE,
		NORMAL,// invalid flag ( but != from unknown as we use this enum for spawn zone that are not player respawn points after death ). I know it is not the best name but we had to choose it because of AI compatiblity purposes
		UNKNOWN,
		NB_RESPAWN_POINT_TYPE = UNKNOWN
	};


	/**
	 * get respawn point type corresponding to input string
	 * \param str the input string
	 * \return the TRespawnPointType associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	TRespawnPointType toRespawnPointType(const std::string &str);

	/**
	 * get the respawn point type string corresponding to enum
	 * \param type the TRespawnPointType value
	 * \return type as a string (or UNKNOWN)
	 */
	const std::string& toString(TRespawnPointType type);
}; // RESPAWN_POINT

#endif // RYZOM_RESPAWN_POINT_TYPE_H
/* End of respawn_point_type.h */
