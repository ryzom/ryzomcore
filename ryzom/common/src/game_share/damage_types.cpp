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
#include "damage_types.h"
#include "nel/misc/string_conversion.h"

namespace DMGTYPE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (EDamageType)
		NL_STRING_CONVERSION_TABLE_ENTRY(SLASHING)
		NL_STRING_CONVERSION_TABLE_ENTRY(PIERCING)
		NL_STRING_CONVERSION_TABLE_ENTRY(BLUNT)
		NL_STRING_CONVERSION_TABLE_ENTRY(ROT)
		NL_STRING_CONVERSION_TABLE_ENTRY(ACID)
		NL_STRING_CONVERSION_TABLE_ENTRY(COLD)
		NL_STRING_CONVERSION_TABLE_ENTRY(FIRE)
		NL_STRING_CONVERSION_TABLE_ENTRY(POISON)
		NL_STRING_CONVERSION_TABLE_ENTRY(ELECTRICITY)
		NL_STRING_CONVERSION_TABLE_ENTRY(SHOCK)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNDEFINED)
	NL_END_STRING_CONVERSION_TABLE(EDamageType, DMGTYPEConversion, UNDEFINED)


	EDamageType stringToDamageType(const std::string &str)
	{
		return DMGTYPEConversion.fromString(str);
	}

	const std::string & toString(EDamageType type)
	{
		return DMGTYPEConversion.toString(type);
	}

	RESISTANCE_TYPE::TResistanceType	getAssociatedResistanceType(EDamageType dmgType)
	{
		switch(dmgType)
		{
		case ACID:
		case ROT:
			return RESISTANCE_TYPE::PrimaryRoot;
		case COLD:
		case SHOCK:
			return RESISTANCE_TYPE::Lacustre;
		case ELECTRICITY:
			return RESISTANCE_TYPE::Jungle;
		case FIRE:
			return RESISTANCE_TYPE::Desert;
		case POISON:
			return RESISTANCE_TYPE::Forest;
		default:
			return RESISTANCE_TYPE::None;
		};
	}

}; // DMGTYPE
