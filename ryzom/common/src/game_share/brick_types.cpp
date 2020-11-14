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
#include "brick_types.h"
//
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"


/*
 * Binding brick family -> EBrickType is done in brick_families.cpp
 */
namespace BRICK_TYPE
{

	NL_BEGIN_STRING_CONVERSION_TABLE (EBrickType)
		NL_STRING_CONVERSION_TABLE_ENTRY(MAGIC)
		NL_STRING_CONVERSION_TABLE_ENTRY(COMBAT)
		NL_STRING_CONVERSION_TABLE_ENTRY(FABER)
		NL_STRING_CONVERSION_TABLE_ENTRY(FORAGE_PROSPECTION)
		NL_STRING_CONVERSION_TABLE_ENTRY(FORAGE_EXTRACTION)
		NL_STRING_CONVERSION_TABLE_ENTRY(HARVEST)
		NL_STRING_CONVERSION_TABLE_ENTRY(TRACKING)
		NL_STRING_CONVERSION_TABLE_ENTRY(SHOPKEEPER)
		NL_STRING_CONVERSION_TABLE_ENTRY(TRAINING)
		NL_STRING_CONVERSION_TABLE_ENTRY(MISCELLANEOUS)
		NL_STRING_CONVERSION_TABLE_ENTRY(COMMERCE)
		NL_STRING_CONVERSION_TABLE_ENTRY(SPECIAL_POWER)
		NL_STRING_CONVERSION_TABLE_ENTRY(PROC_ENCHANTEMENT)
		NL_STRING_CONVERSION_TABLE_ENTRY(TIMED_ACTION)
		NL_STRING_CONVERSION_TABLE_ENTRY(BONUS)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNKNOWN)
	NL_END_STRING_CONVERSION_TABLE(EBrickType, Conversion, UNKNOWN)


	EBrickType toBrickType(const std::string &str)
	{
		return Conversion.fromString(str);
	}

	const std::string &toString(EBrickType type)
	{
		return Conversion.toString(type);
	}
}; // BRICK_TYPE
