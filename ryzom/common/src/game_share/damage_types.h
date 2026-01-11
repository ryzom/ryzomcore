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



#ifndef RY_DAMAGE_TYPES_H
#define RY_DAMAGE_TYPES_H

#include "nel/misc/types_nl.h"
#include "resistance_type.h"

namespace DMGTYPE
{
	//
	enum EDamageType
	{
		SLASHING = 0,		// tranchant (T)
		PIERCING,		// perforant (P)
		BLUNT,			// contondant (C)
		ROT,
		ACID,
		COLD,
		FIRE,
		POISON,
		ELECTRICITY,
		SHOCK,
		UNDEFINED,
		NBTYPES = UNDEFINED,
	};


	/**
	 * get the right damage type from the input string
	 * \param str the input string
	 * \return the EDamageType associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	EDamageType stringToDamageType(const std::string &str);

	const std::string & toString(EDamageType type);

	/// Return the ResistanceType associated to this damage type
	RESISTANCE_TYPE::TResistanceType	getAssociatedResistanceType(EDamageType dmgType);

}; // DMGTYPE

#endif // RY_DAMAGE_TYPES_H
/* End of damage_types.h */
