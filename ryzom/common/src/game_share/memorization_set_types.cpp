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
//
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
//
#include "memorization_set_types.h"

using namespace std;
using namespace NLMISC;

// leave not static else this workaround don't work
void	dummyToAvoidStupidCompilerWarning_game_sshare_memorisation_set_types_cpp()
{
}


namespace MEM_SET_TYPES
{
/*	NL_BEGIN_STRING_CONVERSION_TABLE (TMemorizationSetType)
	  NL_STRING_CONVERSION_TABLE_ENTRY(EmptyHand)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Autolaunch)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Axe)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Bowpistol)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Bowrifle)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Dagger)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Grenade)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Harpoon)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Launcher)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Mace)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Pike)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Pistol)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Rifle)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Spear)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Staff)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Sword)
	  NL_STRING_CONVERSION_TABLE_ENTRY(TwoHandedAxe)
	  NL_STRING_CONVERSION_TABLE_ENTRY(TwoHandedMace)
	  NL_STRING_CONVERSION_TABLE_ENTRY(TwoHandedSword)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Craft)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Harvest)
	NL_END_STRING_CONVERSION_TABLE(TMemorizationSetType, Conversion, Unknown)


	TMemorizationSetType toMemSetType( const std::string &str )
	{
		return Conversion.fromString(str);
	}

	const std::string& toString( TMemorizationSetType type )
	{
		return Conversion.toString(type);
	}

	SKILLS::ESkills memSetTypeToSkill(TMemorizationSetType memType)
	{
//		nlctassert( (sizeof(MemToSkill)/sizeof(MemToSkill[0])) == NbMemSetTypes );

		if(memType>=NbMemSetTypes)
			return SKILLS::unknown;

		return MemToSkill[memType];
	}

	TMemorizationSetType skillToMemSetType(SKILLS::ESkills skill)
	{
		switch(skill)
		{
		case SKILLS::SFMCH :
			return EmptyHand;
		case SKILLS::SFR2A:
			return Autolaunch;
		case SKILLS::SFM1SA:
			return Axe;
		case SKILLS::SFR1B:
			return Bowpistol;
		case SKILLS::SFR2B:
			return Bowrifle;
		case SKILLS::SFMCD:
			return Dagger;
//		case SKILLS::SFR1P:
//			return Grenade;
//		case SKILLS::SFR1P:
//			return Harpoon;
		case SKILLS::SFR2L:
			return Launcher;
		case SKILLS::SFM1BM:
			return Mace;
		case SKILLS::SFM2PP:
			return Pike;
		case SKILLS::SFR1P:
			return Pistol;
		case SKILLS::SFR2R:
			return Rifle;
		case SKILLS::SFM1PS:
			return Spear;
		case SKILLS::SFM1BS:
			return Staff;
		case SKILLS::SFM1SS:
			return Sword;
		case SKILLS::SFM2SA:
			return TwoHandedAxe;
		case SKILLS::SFM2BM:
			return TwoHandedMace;
		case SKILLS::SFM2SS:
			return TwoHandedSword;

		case SKILLS::SC:
			return Craft;
		case SKILLS::SH:
			return Harvest;
		default:
			return EmptyHand;
		};
	}
*/
}; // MEM_SET_TYPES
