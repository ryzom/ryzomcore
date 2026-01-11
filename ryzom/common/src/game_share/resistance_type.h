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

#ifndef RYZOM_RESISTANCE_TYPE_H
#define RYZOM_RESISTANCE_TYPE_H

#include "nel/misc/types_nl.h"

namespace RESISTANCE_TYPE
{
	enum TResistanceType
	{
		Desert,
		Forest,
		Lacustre,
		Jungle,
		PrimaryRoot,

		None,

		NB_RESISTANCE_TYPE = None,
	};

	/**
	 * get resistance type corresponding to input string
	 * \param str the input string
	 * \return the TResistanceType associated to this string (None if the string cannot be interpreted)
	 */
	TResistanceType fromString(const std::string &str);

	/**
	 * get the resistance type string corresponding to enum
	 * \param type the TResistanceType value
	 * \return type as a string (or None)
	 */
	const std::string& toString(TResistanceType type);
};

#endif // RYZOM_RESISTANCE_TYPE_H
/* End of resistance_type.h */
