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



#ifndef RYZOM_TEMP_INV_MODE_H
#define RYZOM_TEMP_INV_MODE_H

#include "nel/misc/types_nl.h"

namespace TEMP_INV_MODE
{
	// All the mode the temporary inventory can be opened (needed for Database SERVER:INVENTORY:TEMP:TYPE)
	enum TInventoryMode
	{
		Harvest = 0,
		Loot,
		Quarter,
		Forage,
		BagFull,
		Craft,
		MissionReward,
		Crystallize,

		NbModes,
		Unknown,
	};

	/**
	 * get the right mode from the input string
	 * \param str the input string
	 * \return the TInventoryMode associated to this string (Unknown if the string cannot be interpreted)
	 */
	TInventoryMode toInvMode(const std::string &str);

	/**
	 * get the string associated to a inv mode
	 * \param family the TInventoryMode to convert into a string
	 * \return the family as a string
	 */
	const std::string &toString(TInventoryMode mode);

}; // TEMP_INV_MODE

#endif // RYZOM_TEMP_INV_MODE_H
/* End of temp_inventory_mode.h */
