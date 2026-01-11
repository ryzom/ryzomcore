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
#include "range_weapon_type.h"
//
#include "nel/misc/string_conversion.h"

namespace RANGE_WEAPON_TYPE
{

NL_BEGIN_STRING_CONVERSION_TABLE (TRangeWeaponType)
	NL_STRING_CONVERSION_TABLE_ENTRY (Generic)
	NL_STRING_CONVERSION_TABLE_ENTRY (Gatlin)
	NL_STRING_CONVERSION_TABLE_ENTRY (Missile)
NL_END_STRING_CONVERSION_TABLE(TRangeWeaponType, RangeWeaponTypeConversion, Unknown)



// *********************************************************************************
TRangeWeaponType stringToRangeWeaponType(const std::string &str)
{
	return RangeWeaponTypeConversion.fromString(str);
}

// *********************************************************************************
std::string toString(TRangeWeaponType type)
{
	return RangeWeaponTypeConversion.toString(type);
}

} // namespace RANGE_WEAPON_TYPE
