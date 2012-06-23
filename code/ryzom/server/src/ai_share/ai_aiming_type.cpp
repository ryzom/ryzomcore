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
//
#include "ai_aiming_type.h"

using namespace std;
using namespace NLMISC;

namespace AI_AIMING_TYPE
{
	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TAiAimingType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Random)

		NL_STRING_CONVERSION_TABLE_ENTRY (Head)
		NL_STRING_CONVERSION_TABLE_ENTRY (Chest)
		NL_STRING_CONVERSION_TABLE_ENTRY (Arms)
		NL_STRING_CONVERSION_TABLE_ENTRY (Hands)
		NL_STRING_CONVERSION_TABLE_ENTRY (Legs)
		NL_STRING_CONVERSION_TABLE_ENTRY (Feet)

		NL_STRING_CONVERSION_TABLE_ENTRY (LeastProtected)
		NL_STRING_CONVERSION_TABLE_ENTRY (AveragestProtected)
		NL_STRING_CONVERSION_TABLE_ENTRY (MostProtected)
		
		NL_STRING_CONVERSION_TABLE_ENTRY (Unknown)
	NL_END_STRING_CONVERSION_TABLE(TAiAimingType, Conversion, Unknown)


	const string &toString(TAiAimingType type)
	{
		return Conversion.toString(type);
	}

	TAiAimingType toAimingType( const string &str)
	{
		return Conversion.fromString(str);
	}

	SLOT_EQUIPMENT::TSlotEquipment toSlot(TAiAimingType type)
	{
		switch(type)
		{
		case Head:
			return SLOT_EQUIPMENT::HEAD;
		case Chest:
			return SLOT_EQUIPMENT::CHEST;
		case Arms:
			return SLOT_EQUIPMENT::ARMS;
		case Hands:
			return SLOT_EQUIPMENT::HANDS;
		case Legs:
			return SLOT_EQUIPMENT::LEGS;
		case Feet:
			return SLOT_EQUIPMENT::FEET;
		default:
			return SLOT_EQUIPMENT::UNDEFINED;
		};
	}

}; // AI_AIMING_TYPE
