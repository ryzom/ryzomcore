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
#include "nel/misc/string_conversion.h"
#include "lift_icons.h"

namespace LIFT_ICONS
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TLiftIcon)
		NL_STRING_CONVERSION_TABLE_ENTRY(RoleMasterFight)
		NL_STRING_CONVERSION_TABLE_ENTRY(RoleMasterMagic)
		NL_STRING_CONVERSION_TABLE_ENTRY(RoleMasterCraft)
		NL_STRING_CONVERSION_TABLE_ENTRY(RoleMasterHarvest)
		NL_STRING_CONVERSION_TABLE_ENTRY(MainGuildRoom)
		NL_STRING_CONVERSION_TABLE_ENTRY(Exit)
		NL_STRING_CONVERSION_TABLE_ENTRY(PlayerRoom)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
	NL_END_STRING_CONVERSION_TABLE(TLiftIcon, LiftIconConversion, None)

	const std::string & toString( TLiftIcon icon )
	{
		return LiftIconConversion.toString( icon );
	}
	TLiftIcon toLiftIcon( const std::string & str )
	{
		return LiftIconConversion.fromString( str );
	}
}
