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
#include "action_nature.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace ACTNATURE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TActionNature)
		NL_STRING_CONVERSION_TABLE_ENTRY(FIGHT)
		NL_STRING_CONVERSION_TABLE_ENTRY(OFFENSIVE_MAGIC)
		NL_STRING_CONVERSION_TABLE_ENTRY(CURATIVE_MAGIC)
		NL_STRING_CONVERSION_TABLE_ENTRY(CRAFT)
		NL_STRING_CONVERSION_TABLE_ENTRY(HARVEST)
		NL_STRING_CONVERSION_TABLE_ENTRY(SEARCH_MP)
		NL_STRING_CONVERSION_TABLE_ENTRY(DODGE)
		NL_STRING_CONVERSION_TABLE_ENTRY(PARRY)
		NL_STRING_CONVERSION_TABLE_ENTRY(SHIELD_USE)
		NL_STRING_CONVERSION_TABLE_ENTRY(RECHARGE)
	NL_END_STRING_CONVERSION_TABLE(TActionNature, ActionNatureConversion, UNKNOWN)



	//-----------------------------------------------
	// toActionNature :
	//-----------------------------------------------
	TActionNature toActionNature(const std::string &str)
	{
		return ActionNatureConversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TActionNature nature)
	{
		return ActionNatureConversion.toString(nature);
	}
}; // ACTNATURE
