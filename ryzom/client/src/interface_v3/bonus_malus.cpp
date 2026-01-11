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

#include "bonus_malus.h"

using namespace std;
using namespace NLMISC;

namespace BONUS_MALUS
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TBonusMalusSpecialTT)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
		NL_STRING_CONVERSION_TABLE_ENTRY(XpCatalyser)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostPVPOn)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostPVPOutOfZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostPVPInRound)
		NL_STRING_CONVERSION_TABLE_ENTRY(DeathPenalty)
	NL_END_STRING_CONVERSION_TABLE(TBonusMalusSpecialTT, BonusMalusTTConversion, Unknown)

	TBonusMalusSpecialTT fromString(const std::string & str)
	{
		return BonusMalusTTConversion.fromString(str);
	}

	const std::string & toString(TBonusMalusSpecialTT clan)
	{
		return BonusMalusTTConversion.toString(clan);
	}


} // namespace BONUS_MALUS

