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


/////////////
// INCLUDE //
/////////////
#include "entity_manager/bypass_check_flags.h"
#include "nel/misc/string_conversion.h"

CBypassCheckFlags CBypassCheckFlags::NoFlags;


namespace CHECK_FLAG_TYPE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TCheckFlagType)
		NL_STRING_CONVERSION_TABLE_ENTRY (WhileSitting)
		NL_STRING_CONVERSION_TABLE_ENTRY (InWater)
		NL_STRING_CONVERSION_TABLE_ENTRY (OnMount)
		NL_STRING_CONVERSION_TABLE_ENTRY (Fear)
		NL_STRING_CONVERSION_TABLE_ENTRY (Sleep)
		NL_STRING_CONVERSION_TABLE_ENTRY (Invulnerability)
		NL_STRING_CONVERSION_TABLE_ENTRY (Stun)
	NL_END_STRING_CONVERSION_TABLE(TCheckFlagType, Conversion, Unknown)

	const std::string& toString(TCheckFlagType type)
	{
		return Conversion.toString(type);
	}

	TCheckFlagType fromString(const std::string &str)
	{
		return Conversion.fromString(str);
	}
}



void CBypassCheckFlags::setFlag(CHECK_FLAG_TYPE::TCheckFlagType type, bool on)
{
	const uint16 val = (on?1:0);

	switch(type)
	{
	case CHECK_FLAG_TYPE::WhileSitting:
		Flags.WhileSitting = val;
		break;
	case CHECK_FLAG_TYPE::InWater:
		Flags.InWater = val;
		break;
	case CHECK_FLAG_TYPE::OnMount:
		Flags.OnMount = val;
		break;
	case CHECK_FLAG_TYPE::Fear:
		Flags.Fear = val;
		break;
	case CHECK_FLAG_TYPE::Sleep:
		Flags.Sleep = val;
		break;
	case CHECK_FLAG_TYPE::Invulnerability:
		Flags.Invulnerability = val;
		break;
	case CHECK_FLAG_TYPE::Stun:
		Flags.Stun = val;
		break;
	default:;
	};
}
