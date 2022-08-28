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
#include "building_enums.h"
#include "nel/misc/string_conversion.h"

namespace ROOM_RESTRICTION
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TRestriction)
		NL_STRING_CONVERSION_TABLE_ENTRY (Rm_Fight)
		NL_STRING_CONVERSION_TABLE_ENTRY (Rm_Magic)
		NL_STRING_CONVERSION_TABLE_ENTRY (Rm_Harvest)
		NL_STRING_CONVERSION_TABLE_ENTRY (Rm_Craft)
	NL_END_STRING_CONVERSION_TABLE(TRestriction, Conversion, Unknown)
			
	//----------------------------------------------------------------------------
	TRestriction fromString( const std::string & str )
	{
		return Conversion.fromString( str );
	}	
}

namespace BUILDING_TYPES
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TBuildingType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Common)
		NL_STRING_CONVERSION_TABLE_ENTRY (Player)
		NL_STRING_CONVERSION_TABLE_ENTRY (Guild)
	NL_END_STRING_CONVERSION_TABLE(TBuildingType, Conversion, Unknown)
		
		//----------------------------------------------------------------------------
	TBuildingType fromString( const std::string & str )
	{
		return Conversion.fromString( str );
	}	
}


