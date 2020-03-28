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
#include "rolemaster_flags.h"
#include "nel/misc/string_conversion.h"

namespace ROLEMASTER_FLAGS
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TRolemasterFlag)
		NL_STRING_CONVERSION_TABLE_ENTRY(FightActions)
		NL_STRING_CONVERSION_TABLE_ENTRY(MagicActions)
		NL_STRING_CONVERSION_TABLE_ENTRY(ForageActions)
		NL_STRING_CONVERSION_TABLE_ENTRY(CraftActions)
		NL_STRING_CONVERSION_TABLE_ENTRY(CaracteristicUpgrades)
		NL_STRING_CONVERSION_TABLE_ENTRY(SpecialPowers)
		NL_STRING_CONVERSION_TABLE_ENTRY(NbFlags)
	NL_END_STRING_CONVERSION_TABLE(TRolemasterFlag, ConversionTable, NbFlags)


	TRolemasterFlag fromString(const std::string &str)
	{
		return ConversionTable.fromString(str);
	}

	const std::string & toString(TRolemasterFlag type)
	{
		return ConversionTable.toString(type);
	}

	static bool	isPhraseAvailable(const std::string &brickFilter, const std::string &sheetName)
	{
		return sheetName.substr( 1, brickFilter.size() ) == brickFilter;
	}

	bool	canSellPhrase(uint32 rmfBitField, const std::string &sheetName)
	{
		// Brick filters :
		// b = all phrases
		// bf = combat phrase
		// bm = magic phrase
		// bc = craft phrase
		// bhf = forage phrase
		// bp = charac phrase
		// bsf = fight related special powers
		// bsm = magic related special powers
		// bsd = defense related special powers (~bsx)
		// bsx = unspecialized special powers
		// bsg = craft/forage special powers

		// *** Test Carac upgrade
		if(rmfBitField & (1<<CaracteristicUpgrades) )
		{
			//{ "c", constitution },
			//{ "m", metabolism },
			//{ "i", intelligence },
			//{ "w", wisdom },
			//{ "s", strength },
			//{ "b", well_balanced },
			//{ "d", dexterity },
			//{ "l", will },
			if(rmfBitField & (1<<FightActions) )
			{
				if(isPhraseAvailable("bppc", sheetName))	return true;
				if(isPhraseAvailable("bppm", sheetName))	return true;
				if(isPhraseAvailable("bpps", sheetName))	return true;
				if(isPhraseAvailable("bppb", sheetName))	return true;
			}
			if(rmfBitField & (1<<MagicActions) )
			{
				if(isPhraseAvailable("bppc", sheetName))	return true;
				if(isPhraseAvailable("bppm", sheetName))	return true;
				if(isPhraseAvailable("bppi", sheetName))	return true;
				if(isPhraseAvailable("bppw", sheetName))	return true;
			}
			if(rmfBitField & (1<<ForageActions) )
			{
				if(isPhraseAvailable("bppc", sheetName))	return true;
				if(isPhraseAvailable("bppm", sheetName))	return true;
				if(isPhraseAvailable("bppd", sheetName))	return true;
				if(isPhraseAvailable("bppl", sheetName))	return true;
			}
			if(rmfBitField & (1<<CraftActions) )
			{
				if(isPhraseAvailable("bppc", sheetName))	return true;
				if(isPhraseAvailable("bppm", sheetName))	return true;
				if(isPhraseAvailable("bppd", sheetName))	return true;
				if(isPhraseAvailable("bppl", sheetName))	return true;
			}
		}

		// *** Test Special Power
		if(rmfBitField & (1<<SpecialPowers) )
		{
			bool	testGeneric= false;
			if(rmfBitField & (1<<FightActions) )
			{
				if(isPhraseAvailable("bsf", sheetName))	return true;
				testGeneric= true;
			}
			if(rmfBitField & (1<<MagicActions) )
			{
				if(isPhraseAvailable("bsm", sheetName))	return true;
				testGeneric= true;
			}
			if(rmfBitField & (1<<ForageActions) )
			{
				if(isPhraseAvailable("bsg", sheetName))	return true;
				testGeneric= true;
			}
			if(rmfBitField & (1<<CraftActions) )
			{
				if(isPhraseAvailable("bsg", sheetName))	return true;
				testGeneric= true;
			}
			// test generic
			if(testGeneric)
			{
				if(isPhraseAvailable("bsx", sheetName))	return true;
				if(isPhraseAvailable("bsd", sheetName))	return true;
			}
		}

		// *** Test Normal Action
		if(rmfBitField & (1<<FightActions) )
		{
			if(isPhraseAvailable("bf", sheetName))	return true;
		}
		if(rmfBitField & (1<<MagicActions) )
		{
			if(isPhraseAvailable("bm", sheetName))	return true;
		}
		if(rmfBitField & (1<<ForageActions) )
		{
			if(isPhraseAvailable("bhf", sheetName))	return true;
		}
		if(rmfBitField & (1<<CraftActions) )
		{
			if(isPhraseAvailable("bc", sheetName))	return true;
		}


		// Failed
		return false;
	}

}; // ROLEMASTER_FLAGS
