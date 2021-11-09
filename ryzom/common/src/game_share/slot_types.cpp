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



#include "stdpch.h"

#include "nel/misc/debug.h"
#include "slot_types.h"


namespace SLOTTYPE
{

	TSlotType stringToSlotType (const std::string &str, bool warning)
	{
		if (str == "HEAD" || str == "Head" )
			return HEAD;

		if (str == "ARMS" || str == "Arms" )
			return ARMS;

		if (str == "RIGHT_HAND" )
			return RIGHT_HAND;

		if (str == "LEFT_HAND" )
			return LEFT_HAND;

		if (str == "TWO_HANDS" )
			return TWO_HANDS;

		if (str == "RIGHT_HAND_EXCLUSIVE" )
			return RIGHT_HAND_EXCLUSIVE;

		if (str == "HANDS" || str == "Hands" )
			return HANDS;

		if (str == "CHEST" || str == "Chest" )
			return CHEST;

		if (str == "LEGS" || str == "Legs" )
			return LEGS;

		if (str == "FEET" || str == "Feet" )
			return FEET;

		if (str == "HEADDRESS" || str == "Headdress" )
			return HEADDRESS;

		if (str == "EARS" || str == "Ears" )
			return EARS;

		if (str == "FACE" || str == "Face" )
			return FACE;

		if (str == "NECKLACE" || str == "Necklace" )
			return NECKLACE;

		if (str == "SHOULDER" || str == "Shoulder" )
			return SHOULDER;

		if (str == "BACK" || str == "Back" )
			return BACK;

		if (str == "WRIST" || str == "Wrist" )
			return WRIST;

		if (str == "FINGERS" || str == "Fingers" )
			return FINGERS;

		if (str == "ANKLE" || str == "Ankle" )
			return ANKLE;

		if (str == "AMMO" || str == "Ammo" )
			return AMMO;

//		if (str == "SHEATH" || str == "Sheath" ) // fourreau
//			return SHEATH;

		if (str == "UNDEFINED" )
			return UNDEFINED;

		if (warning)
			nlwarning("<stringToSlotType> Unknown type %s", str.c_str() );
		return UNDEFINED;
	}

	//-----------------------------------------------
	// convertTypeToVisualSlot :
	// Convert a slot type into a visual slot.
	//-----------------------------------------------
	EVisualSlot convertTypeToVisualSlot (TSlotType type)
	{
		switch(type)
		{
		case CHEST:
			return CHEST_SLOT;
			break;

		case LEGS:
			return LEGS_SLOT;
			break;

		case HEAD:
			return HEAD_SLOT;
			break;

		case ARMS:
			return ARMS_SLOT;
			break;

		case FACE:
			return FACE_SLOT;
			break;

		case HANDS:
			return HANDS_SLOT;
			break;

		case FEET:
			return FEET_SLOT;
			break;

		case RIGHT_HAND:
		case TWO_HANDS:
		case RIGHT_HAND_EXCLUSIVE:
			return RIGHT_HAND_SLOT;
			break;

		case LEFT_HAND:
			return LEFT_HAND_SLOT;
			break;

		case UNDEFINED:
		case EARS:
		case NECKLACE:
		case SHOULDER:
		case ANKLE:
		case FINGERS:
		case HEADDRESS:
		case WRIST:
		case BACK:
		default:
			return HIDDEN_SLOT;
			break;
		}
	}// convertTypeToVisualSlot //
}; // SLOTTYPE
