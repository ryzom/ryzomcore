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
#include "zc_shard_common.h"
#include "nel/misc/string_conversion.h"

namespace ZCSTATE
{

NL_BEGIN_STRING_CONVERSION_TABLE (TZcState)
	NL_STRING_CONVERSION_TABLE_ENTRY(Tribe)
	NL_STRING_CONVERSION_TABLE_ENTRY(TribeInWar)
//	NL_STRING_CONVERSION_TABLE_ENTRY(TribeInPeace)
	NL_STRING_CONVERSION_TABLE_ENTRY(GuildInWar)
	NL_STRING_CONVERSION_TABLE_ENTRY(GuildInPeace)
	NL_STRING_CONVERSION_TABLE_ENTRY(zs_unknown)
NL_END_STRING_CONVERSION_TABLE(TZcState, ConversionType, zs_unknown)

///
TZcState toZcState( const std::string &str )
{
	return ConversionType.fromString(str);
}


///
const std::string& toString( TZcState type )
{
	return ConversionType.toString(type);
}


}


