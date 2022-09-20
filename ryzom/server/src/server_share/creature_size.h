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



#ifndef RY_CREATURE_SIZE_H
#define RY_CREATURE_SIZE_H

#include "nel/misc/types_nl.h"

namespace CREATURE_SIZE
{
	// Size
	enum ECreatureSize
	{
		SMALL = 0,
		HOMIN,
		BIG,

		// the number of size existing
		NB_SIZE,

		UNKNOWN,
	};


	/**
	 * get the ECreatureSize size from the input string
	 * \param str the input string
	 * \return the ECreatureSize associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	ECreatureSize stringToCreatureSize(const std::string &str);


	/**
	 * get the ECreatureSize size as a string
	 * \param size the ECreatureSize size
	 * \return the string associated to this ECreatureSize
	 */
	const std::string &creatureSizeToString(ECreatureSize size);


}; // CREATURE_SIZE

#endif // RY_CREATURE_SIZE_H
/* End of creature_size.h */
