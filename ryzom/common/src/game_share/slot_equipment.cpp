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

#include "slot_equipment.h"
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace SLOT_EQUIPMENT
{
	// The conversion table
	const CStringConversion<TSlotEquipment>::CPair stringTable [] =
	{
		{ "Headdress", HEADDRESS },
		{ "Head", HEAD },
		{ "Face", FACE },
		{ "EarL", EARL },
		{ "EarR", EARR },
		{ "Neck", NECKLACE },
		{ "Body", CHEST },
		{ "Arms", ARMS },
		{ "WristL", WRISTL },
		{ "WristR", WRISTR },
		{ "Hands", HANDS },
		{ "HandL", HANDL },
		{ "HandR", HANDR },
		{ "FingerL", FINGERL },
		{ "FingerR", FINGERR },
		{ "Legs", LEGS },
		{ "AnkleL", ANKLEL },
		{ "AnkleR", ANKLER },
		{ "Feet", FEET },
	};

	CStringConversion<TSlotEquipment> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  UNDEFINED);

	// convert type id to type name string
	const std::string& toString( TSlotEquipment slot_equipment )
	{
		return conversion.toString(slot_equipment);
	}

	// convert type name to type enum value
	TSlotEquipment stringToSlotEquipment( const std::string& str )
	{
		return conversion.fromString(str);
	}
}; // SLOT_EQUIPMENT
