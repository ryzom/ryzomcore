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
#include "nel/misc/sstring.h"

#include "pvp_clan.h"
#include "fame.h"

using namespace std;
using namespace NLMISC;

namespace PVP_CLAN
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TPVPClan)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
		NL_STRING_CONVERSION_TABLE_ENTRY(Neutral)
		NL_STRING_CONVERSION_TABLE_ENTRY(Kami)
		NL_STRING_CONVERSION_TABLE_ENTRY(Karavan)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fyros)
		NL_STRING_CONVERSION_TABLE_ENTRY(Matis)
		NL_STRING_CONVERSION_TABLE_ENTRY(Tryker)
		NL_STRING_CONVERSION_TABLE_ENTRY(Zorai)
	NL_END_STRING_CONVERSION_TABLE(TPVPClan, PVPClanConversion, Unknown)

	TPVPClan fromString(const std::string & str)
	{
		return PVPClanConversion.fromString(str);
	}

	const std::string & toString(TPVPClan clan)
	{
		return PVPClanConversion.toString(clan);
	}

	std::string toLowerString(TPVPClan clan)
	{
		CSString s = PVPClanConversion.toString(clan);
		CSString sl = s.toLower();
		return sl;
	}

	uint32 getFactionIndex(TPVPClan clan)
	{
		static vector<uint32> factionIndexes;
		if ( factionIndexes.empty() )
		{
			factionIndexes.resize(NbClans, CStaticFames::INVALID_FACTION_INDEX);

			factionIndexes[Kami]	= CStaticFames::getInstance().getFactionIndex("kami");
			factionIndexes[Karavan]	= CStaticFames::getInstance().getFactionIndex("karavan");
			factionIndexes[Fyros]	= CStaticFames::getInstance().getFactionIndex("fyros");
			factionIndexes[Matis]	= CStaticFames::getInstance().getFactionIndex("matis");
			factionIndexes[Tryker]	= CStaticFames::getInstance().getFactionIndex("tryker");
			factionIndexes[Zorai]	= CStaticFames::getInstance().getFactionIndex("zorai");

			for (uint i = BeginClans; i <= EndClans; i++)
				nlassert( factionIndexes[i] != CStaticFames::INVALID_FACTION_INDEX );
		}

		if (clan >= NbClans)
			return CStaticFames::INVALID_FACTION_INDEX;

		return factionIndexes[clan];
	}

	TPVPClan getClanFromIndex(uint32 theIndex)
	{
		// These names are in order of the enum TPVPClan
		// The first two clans, "None" and "Neutral", don't count.  Subtract 2 from the lookup.
		std::string FactionNames[] = { "kami","karavan","fyros","matis","tryker","zorai" };

		for (int looper = BeginClans; looper <= EndClans; looper += 1)
		{
			if (CStaticFames::getInstance().getFactionIndex(FactionNames[looper-BeginClans]) == theIndex)
			{
				return (TPVPClan)looper;
			}
		}

		// It wasn't found, return unknown to indicate error.
		return Unknown;
	}

	NLMISC::CSheetId getFactionSheetId(TPVPClan clan)
	{
		static vector<NLMISC::CSheetId> factionSheetIds;
		if ( factionSheetIds.empty() )
		{
			factionSheetIds.resize(NbClans, NLMISC::CSheetId::Unknown);

			factionSheetIds[Kami]		= "kami.faction";
			factionSheetIds[Karavan]	= "karavan.faction";
			factionSheetIds[Fyros]		= "fyros.faction";
			factionSheetIds[Matis]		= "matis.faction";
			factionSheetIds[Tryker]		= "tryker.faction";
			factionSheetIds[Zorai]		= "zorai.faction";

			for (uint i = BeginClans; i <= EndClans; i++)
				nlassert( factionSheetIds[i] != NLMISC::CSheetId::Unknown );
		}

		if (clan >= NbClans)
			return NLMISC::CSheetId::Unknown;

		return factionSheetIds[clan];
	}

	std::string toIconDefineString( TPVPClan clan )
	{
		return string("pvp_faction_icon_") + PVP_CLAN::toString(clan);
	}

	TPVPClan getClanFromPeople( EGSPD::CPeople::TPeople people )
	{
		static vector<TPVPClan> peopleToClan;
		if( peopleToClan.empty() )
		{
			peopleToClan.resize( EGSPD::CPeople::EndPlayable );
			peopleToClan[ EGSPD::CPeople::Fyros ] = Fyros;
			peopleToClan[ EGSPD::CPeople::Matis ] = Matis;
			peopleToClan[ EGSPD::CPeople::Tryker ] = Tryker;
			peopleToClan[ EGSPD::CPeople::Zorai ] = Zorai;
		}

		if( people < EGSPD::CPeople::EndPlayable && people >= EGSPD::CPeople::Playable )
			return peopleToClan[ people ];

		return Unknown;
	}

} // namespace PVP_CLAN
