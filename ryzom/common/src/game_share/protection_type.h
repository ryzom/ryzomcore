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



#ifndef RYZOM_PROTECTION_TYPE_H
#define RYZOM_PROTECTION_TYPE_H

#include "nel/misc/types_nl.h"


namespace PROTECTION_TYPE
{
	enum TProtectionType
	{
		Acid = 0,
		Cold,
		Rot,
		Fire,			// Fyros speciality
		Shockwave,		// Tryker speciality
		Poison,			// Matis speciality
		Electricity,	// Zorai speciality

		None,

		NB_PROTECTION_TYPE = None
	};

	/**
	 * get protection type corresponding to input string
	 * \param str the input string
	 * \return the TProtectionType associated to this string (nothing if the string cannot be interpreted)
	 */
	TProtectionType fromString(const std::string &str);

	/**
	 * get the protection type string corresponding to enum
	 * \param type the TProtectionType value
	 * \return type as a string (or nothing)
	 */
	const std::string& toString(TProtectionType type);

}; // PROTECTION_TYPE

#endif // RYZOM_PROTECTION_TYPE_H
/* End of protection_type.h */
