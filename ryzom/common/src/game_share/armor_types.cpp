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
#include "nel/misc/string_conversion.h"
#include "armor_types.h"


namespace ARMORTYPE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (EArmorType)
		NL_STRING_CONVERSION_TABLE_ENTRY(HEAVY)
		NL_STRING_CONVERSION_TABLE_ENTRY(MEDIUM)
		NL_STRING_CONVERSION_TABLE_ENTRY(LIGHT)
		NL_STRING_CONVERSION_TABLE_ENTRY(ALL)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNKNOWN)
	NL_END_STRING_CONVERSION_TABLE(EArmorType, ConversionType, UNKNOWN)

	///
	EArmorType toArmorType( const std::string &str )
	{
		return ConversionType.fromString(str);
	}

	///
	const std::string& toString( EArmorType type )
	{
		return ConversionType.toString(type);
	}

}; // ARMORTYPE
