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



#ifndef RY_PLACE_TYPE_H
#define RY_PLACE_TYPE_H

#include "nel/misc/types_nl.h"

namespace PLACE_TYPE
{
	// action nature
	enum TPlaceType
	{
		Capital,
		Village,
		Outpost,
		Place,
		Street,
		Undefined,
	};

	/**
	 * convert string to enum
	 * \param str the input string
	 * \return the TPlaceType associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	TPlaceType fromString(const std::string &str);

	/**
	 * convert TPlaceType to string
	 * \param place_type the TPlaceType value
	 * \return nature as a string (or UNKNOWN)
	 */
	const std::string& toString(TPlaceType place_type);
};

#endif // RY_PLACE_TYPE_H
/* End of place_type.h */
