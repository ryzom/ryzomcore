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



#ifndef RY_ACTION_NATURE_H
#define RY_ACTION_NATURE_H

#include "nel/misc/types_nl.h"

namespace ACTNATURE
{
	// action nature
	enum TActionNature
	{
		FIGHT = 0,
		OFFENSIVE_MAGIC,
		CURATIVE_MAGIC,
		CRAFT,
		HARVEST,
		SEARCH_MP,
		DODGE,
		PARRY,
		SHIELD_USE,
		RECHARGE,

		NEUTRAL,	//only for multi effect on spell,progression consider it as OFFENSIVE_MAGIC

		UNKNOWN,
		NB_ACTION_NATURE = UNKNOWN
	};


	/**
	 * get action nature corresponding to input string
	 * \param str the input string
	 * \return the TActionNature associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	TActionNature toActionNature(const std::string &str);

	/**
	 * get the action nature string corresponding to enum
	 * \param nature the TActionNature value
	 * \return nature as a string (or UNKNOWN)
	 */
	const std::string& toString(TActionNature nature);
}; // ACTNATURE

#endif // RY_ACTION_NATURE_H
/* End of action_nature.h */
