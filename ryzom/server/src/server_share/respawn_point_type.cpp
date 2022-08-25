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

#include "respawn_point_type.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace RESPAWN_POINT
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TRespawnPointType)
		NL_STRING_CONVERSION_TABLE_ENTRY(KAMI)
		NL_STRING_CONVERSION_TABLE_ENTRY(KARAVAN)
		NL_STRING_CONVERSION_TABLE_ENTRY(OUTPOST)
		NL_STRING_CONVERSION_TABLE_ENTRY(NEWBIELAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(RESPAWNABLE)
		NL_STRING_CONVERSION_TABLE_ENTRY(NORMAL)
	NL_END_STRING_CONVERSION_TABLE(TRespawnPointType, RespawnPointTypeConversion, UNKNOWN)
		
	
	
	//-----------------------------------------------
	// toRespawnPointType :
	//-----------------------------------------------
	TRespawnPointType toRespawnPointType(const std::string &str)
	{
		return RespawnPointTypeConversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TRespawnPointType respawn_point_type)
	{
		return RespawnPointTypeConversion.toString(respawn_point_type);
	}
}; // RESPAWN_POINT
