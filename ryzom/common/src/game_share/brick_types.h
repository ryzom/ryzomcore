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



#ifndef RY_BRICK_TYPES_H
#define RY_BRICK_TYPES_H

#include "nel/misc/types_nl.h"

namespace BRICK_TYPE
{
	// Mode
	enum EBrickType
	{
		MAGIC = 0,
		COMBAT,
		FABER,
		FORAGE_PROSPECTION,
		FORAGE_EXTRACTION,
		HARVEST,
		QUARTER,
		TRACKING,
		SHOPKEEPER,
		TRAINING,
		MISCELLANEOUS,
		COMMERCE,
		SPECIAL_POWER,
		PROC_ENCHANTEMENT,
		TIMED_ACTION,
		BRICK_TYPE_COUNT,
		BONUS,
		UNKNOWN // Warning: Shouldn't exceed 32
	};


	/**
	 * get the right brick type from the input string
	 * \param str the input string
	 * \return the EBrickType associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	EBrickType toBrickType(const std::string &str);

	/**
	 * get the string associated to a brick type
	 * \param type the EBrickType to convert into a string
	 * \return the type as a string
	 */
	const std::string &toString(EBrickType type);

} // BRICK_TYPE

#endif // RY_BRICK_TYPES_H
/* End of brick_types.h */
