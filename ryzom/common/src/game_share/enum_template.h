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



#ifndef RY_ENUM_ROLES_H
#define RY_ENUM_ROLES_H

#include "nel/misc/types_nl.h"

namespace ROLES
{
	enum EEnumRole
	{
		melee_warrior = 0,
		range_warrior,
		attack_magician,
		buffer_magician,
		healer_magician,
		harvester,
		shopkeeper,
		faber,
		all_caster,
		all,

		UNKNOWN = 0,

		NUM_ENUM_ROLE
	};


	/**
	 * get the right EEnumTemplate enum from the input string
	 * \param str the input string
	 * \return the EEnumTemplate associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	EEnumTemplate stringToEnum(const std::string &str);

	/**
	 * get the EEnumTemplate type as a string
	 * \param e the EEnumTemplate enum
	 * \return the string associated to this EEnumTemplate
	 */
	const std::string & enumToString (EEnumTemplate e);

}; // namespace ENUM_ROLES

#endif // RY_ENUM_ROLES_H

