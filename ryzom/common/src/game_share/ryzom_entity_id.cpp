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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.

#include "ryzom_entity_id.h"

// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace RYZOMID
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TTypeId)
		NL_STRING_CONVERSION_TABLE_ENTRY(player)
		NL_STRING_CONVERSION_TABLE_ENTRY(npc)
		NL_STRING_CONVERSION_TABLE_ENTRY(creature)
		NL_STRING_CONVERSION_TABLE_ENTRY(mount)
		NL_STRING_CONVERSION_TABLE_ENTRY(pack_animal)
		NL_STRING_CONVERSION_TABLE_ENTRY(flora)
		NL_STRING_CONVERSION_TABLE_ENTRY(object)
		NL_STRING_CONVERSION_TABLE_ENTRY(building)
		NL_STRING_CONVERSION_TABLE_ENTRY(position)
		NL_STRING_CONVERSION_TABLE_ENTRY(chatGroup)
		NL_STRING_CONVERSION_TABLE_ENTRY(dynChatGroup)
		NL_STRING_CONVERSION_TABLE_ENTRY(deposit)
		NL_STRING_CONVERSION_TABLE_ENTRY(guild)
		NL_STRING_CONVERSION_TABLE_ENTRY(civilisation)
		NL_STRING_CONVERSION_TABLE_ENTRY(fame_memory)
		NL_STRING_CONVERSION_TABLE_ENTRY(trigger)
		NL_STRING_CONVERSION_TABLE_ENTRY(forageSource)
		NL_STRING_CONVERSION_TABLE_ENTRY(fx_entity)
NL_END_STRING_CONVERSION_TABLE(TTypeId, TypeConversion, unknown)

	//-----------------------------------------------
	// toRespawnPointType :
	//-----------------------------------------------
	TTypeId fromString(const std::string &str)
	{
		return TypeConversion.fromString(str);
	}

	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TTypeId respawn_point_type)
	{
		return TypeConversion.toString(respawn_point_type);
	}
};// namespace RYZOMID
