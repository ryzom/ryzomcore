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
#include "pvp_mode.h"

using namespace std;
using namespace NLMISC;

namespace PVP_MODE
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TPVPMode)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpDuel)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpChallenge)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpZoneFree)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpZoneFaction)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpZoneGuild)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpZoneOutpost)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpFaction)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpFactionFlagged)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpZoneSafe)
		NL_STRING_CONVERSION_TABLE_ENTRY(PvpSafe)
	NL_END_STRING_CONVERSION_TABLE(TPVPMode, PVPModeConversion, Unknown)

	TPVPMode fromString(const std::string & str)
	{
		return PVPModeConversion.fromString(str);
	}

	const std::string & toString(TPVPMode pvpMode)
	{
		return PVPModeConversion.toString(pvpMode);
	}

} // namespace PVP_MODE
