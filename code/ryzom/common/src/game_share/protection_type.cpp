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
#include "protection_type.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace PROTECTION_TYPE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TProtectionType)
		NL_STRING_CONVERSION_TABLE_ENTRY(Cold)
		NL_STRING_CONVERSION_TABLE_ENTRY(Acid)
		NL_STRING_CONVERSION_TABLE_ENTRY(Rot)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fire)
		NL_STRING_CONVERSION_TABLE_ENTRY(Shockwave)
		NL_STRING_CONVERSION_TABLE_ENTRY(Poison)
		NL_STRING_CONVERSION_TABLE_ENTRY(Electricity)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
	NL_END_STRING_CONVERSION_TABLE(TProtectionType, ProtectionTypeConversion, None)


	//-----------------------------------------------
	// fromString:
	//-----------------------------------------------
	TProtectionType fromString(const std::string &str)
	{
		return ProtectionTypeConversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TProtectionType protection_type)
	{
		return ProtectionTypeConversion.toString(protection_type);
	}

}; // PROTECTION_TYPE

