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

// nel
#include "nel/misc/i18n.h"
#include "nel/misc/string_conversion.h"

#include "item_special_effect.h"

using namespace std;
using namespace NLMISC;

namespace ITEM_SPECIAL_EFFECT
{
	// The conversion table
	const CStringConversion<TItemSpecialEffect>::CPair stringTable [] =
	{
		 { "ISE_FIGHT_ADD_CRITICAL", ISE_FIGHT_ADD_CRITICAL },
		 { "ISE_FIGHT_VAMPIRISM", ISE_FIGHT_VAMPIRISM },
		 { "ISE_MAGIC_DIVINE_INTERVENTION", ISE_MAGIC_DIVINE_INTERVENTION },
		 { "ISE_MAGIC_SHOOT_AGAIN", ISE_MAGIC_SHOOT_AGAIN },
		 { "ISE_CRAFT_ADD_STAT_BONUS", ISE_CRAFT_ADD_STAT_BONUS },
		 { "ISE_CRAFT_ADD_LIMIT", ISE_CRAFT_ADD_LIMIT },
		 { "ISE_FORAGE_ADD_RM", ISE_FORAGE_ADD_RM },
		 { "ISE_FORAGE_NO_RISK", ISE_FORAGE_NO_RISK },

	};

	CStringConversion<TItemSpecialEffect> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  UNDEFINED);


	// convert item special effect id to item special effect name string
	const std::string& toString( TItemSpecialEffect itemSpecialEffect )
	{
		return conversion.toString(itemSpecialEffect);
	}


	// convert item special effect name to item special effect enum value
	TItemSpecialEffect fromString( const std::string& str )
	{
		return conversion.fromString(str);
	}
	// convert item special effect name to item special effect enum value
	TItemSpecialEffect stringToItemSpecialEffect( const std::string& str )
	{
		return conversion.fromString(str);
	}
}; // ITEM_SPECIAL_EFFECT
