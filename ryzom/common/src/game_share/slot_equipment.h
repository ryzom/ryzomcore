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



#ifndef RY_SLOT_EQUIPMENT_H
#define RY_SLOT_EQUIPMENT_H

#include "nel/misc/types_nl.h"

namespace SLOT_EQUIPMENT
{
	// Mode
	enum TSlotEquipment
	{
		HEADDRESS,
		HEAD,
		FACE,
		EARL,
		EARR,
		NECKLACE,
		CHEST,
		ARMS,
		WRISTL,
		WRISTR,
		HANDS,
		HANDL,
		HANDR,
		FINGERL,
		FINGERR,
		LEGS,
		ANKLEL,
		ANKLER,
		FEET,

		NB_SLOT_EQUIPMENT,
		UNDEFINED = NB_SLOT_EQUIPMENT
	};

	/**
	 * Convert a slot name to slot equipment enum
	 * \param str the input string
	 * \return the TSlotEquipment associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	TSlotEquipment stringToSlotEquipment(const std::string &str);

	/**
	 * Convert a slot enum to george string (only for george read, do not use this for game)
	 */
	const std::string& toString( TSlotEquipment );

}; // SLOTTYPE

#endif // RY_SLOT_EQUIPMENT_H
/* End of slot_equipment.h */

