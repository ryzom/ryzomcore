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
#include "combat.h"
//
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

uint8 CCombat::_MaxHistoryDepth	= 2;

namespace COMBAT_HISTORY
{

	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TCombatHistory)
		NL_STRING_CONVERSION_TABLE_ENTRY (Miss)
		NL_STRING_CONVERSION_TABLE_ENTRY (Fumble)
		NL_STRING_CONVERSION_TABLE_ENTRY (Parry)
		NL_STRING_CONVERSION_TABLE_ENTRY (Dodge)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitHead)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitChest)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitArms)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitHands)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitLegs)
		NL_STRING_CONVERSION_TABLE_ENTRY (HitFeet)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitHead)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitChest)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitArms)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitHands)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitLegs)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHitFeet)
		NL_STRING_CONVERSION_TABLE_ENTRY (Unknown)
	NL_END_STRING_CONVERSION_TABLE(TCombatHistory, CombatHistoryConversion, Unknown)


	//--------------------------------------------------------------
	//					CCombat::toString()
	//--------------------------------------------------------------
	const string &toString(TCombatHistory event)
	{
		return CombatHistoryConversion.toString(event);
	}
}; // COMBAT_HISTORY //


