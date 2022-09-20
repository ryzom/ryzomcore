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
#include "protection_type_back_compat.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;


namespace BACK_COMPAT
{
namespace OLD_PROTECTION_TYPE
{

NL_BEGIN_STRING_CONVERSION_TABLE (TOldProtectionType)
	NL_STRING_CONVERSION_TABLE_ENTRY(Cold)
	NL_STRING_CONVERSION_TABLE_ENTRY(Acid)
	NL_STRING_CONVERSION_TABLE_ENTRY(Rot)
	NL_STRING_CONVERSION_TABLE_ENTRY(Fire)
	NL_STRING_CONVERSION_TABLE_ENTRY(Shockwave)
	NL_STRING_CONVERSION_TABLE_ENTRY(Poison)
	NL_STRING_CONVERSION_TABLE_ENTRY(Electricity)
	NL_STRING_CONVERSION_TABLE_ENTRY(Madness)
	NL_STRING_CONVERSION_TABLE_ENTRY(Slow)
	NL_STRING_CONVERSION_TABLE_ENTRY(Snare)
	NL_STRING_CONVERSION_TABLE_ENTRY(Sleep)
	NL_STRING_CONVERSION_TABLE_ENTRY(Stun)
	NL_STRING_CONVERSION_TABLE_ENTRY(Root)
	NL_STRING_CONVERSION_TABLE_ENTRY(Blind)
	NL_STRING_CONVERSION_TABLE_ENTRY(Fear)
	NL_STRING_CONVERSION_TABLE_ENTRY(None)
NL_END_STRING_CONVERSION_TABLE(TOldProtectionType, OldProtectionTypeConversion, None)

TOldProtectionType fromString(const std::string & str)
{
	return OldProtectionTypeConversion.fromString(str);
}

const std::string & toString(TOldProtectionType oldType)
{
	return OldProtectionTypeConversion.toString(oldType);
}

} // namespace OLD_PROTECTION_TYPE
} // namespace BACK_COMPAT

