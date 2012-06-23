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



#ifndef RY_SLOT_TYPES_H
#define RY_SLOT_TYPES_H

#include "nel/misc/types_nl.h"

namespace SLOTTYPE
{
	// Mode
	enum TSlotType
	{
		UNDEFINED = 0,

		HEADDRESS,
		HEAD,
		FACE,
		EARS,
		NECKLACE,
		SHOULDER,
		BACK,
		CHEST,
		ARMS,
		WRIST,
		HANDS,
		FINGERS,
		LEGS,
		ANKLE,
		FEET,
		RIGHT_HAND,
		LEFT_HAND,
		TWO_HANDS,
		RIGHT_HAND_EXCLUSIVE, // Left hand is empty but used by object in right hand (fire weapons)
		AMMO,
		//SHEATH,		// Fourreau

		NB_SLOT_TYPE
	};

	// Enum all visual slots.
	enum EVisualSlot
	{
		HIDDEN_SLOT = 0,
		CHEST_SLOT,
		LEGS_SLOT,
		HEAD_SLOT,
		ARMS_SLOT,
		FACE_SLOT,
		HANDS_SLOT,
		FEET_SLOT,
		RIGHT_HAND_SLOT,
		LEFT_HAND_SLOT,

		NB_SLOT
	};

	/**
	 * get the right effectType from the input string
	 * \param str the input string
	 * \return the TSlotType associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	TSlotType stringToSlotType(const std::string &str, bool warning=true);

	/// Convert a slot type into a visual slot.
	EVisualSlot convertTypeToVisualSlot(TSlotType type);
}; // SLOTTYPE

#endif // RY_SLOT_TYPES_H
/* End of slot_types.h */
