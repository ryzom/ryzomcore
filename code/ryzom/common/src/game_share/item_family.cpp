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

#include "item_family.h"

using namespace std;
using namespace NLMISC;

namespace ITEMFAMILY
{
	// The conversion table
	const CStringConversion<EItemFamily>::CPair stringTable [] =
	{
		{ "UNDEFINED", UNDEFINED },
		{ "SERVICE", SERVICE },
		{ "ARMOR", ARMOR },
		{ "MELEE_WEAPON", MELEE_WEAPON },
		{ "RANGE_WEAPON", RANGE_WEAPON },
		{ "AMMO", AMMO },
		{ "RAW_MATERIAL", RAW_MATERIAL },
		{ "SHIELD", SHIELD },
		{ "CRAFTING_TOOL", CRAFTING_TOOL },
		{ "HARVEST_TOOL", HARVEST_TOOL },
		{ "TAMING_TOOL", TAMING_TOOL },
		{ "TRAINING_TOOL", TRAINING_TOOL },
		{ "AI", AI },
		{ "BRICK", BRICK },
		{ "FOOD", FOOD },
		{ "JEWELRY", JEWELRY },
		{ "CORPSE", CORPSE },
		{ "CARRION", CARRION },
		{ "BAG", BAG },
		{ "STACK", STACK },
		{ "DEAD_SEED", DEAD_SEED },
		{ "TELEPORT", TELEPORT },
		{ "GUILD_FLAG", GUILD_FLAG },
		{ "LIVING_SEED", LIVING_SEED },
		{ "LITTLE_SEED", LITTLE_SEED },
		{ "MEDIUM_SEED", MEDIUM_SEED },
		{ "BIG_SEED", BIG_SEED },
		{ "VERY_BIG_SEED", VERY_BIG_SEED },
		{ "MISSION_ITEM", MISSION_ITEM },
		{ "CRYSTALLIZED_SPELL", CRYSTALLIZED_SPELL },
		{ "ITEM_SAP_RECHARGE", ITEM_SAP_RECHARGE },
		{ "PET_ANIMAL_TICKET", PET_ANIMAL_TICKET },
		{ "GUILD_OPTION", GUILD_OPTION },
		{ "COSMETIC", COSMETIC },
		{ "HANDLED_ITEM", HANDLED_ITEM },
		{ "CONSUMABLE", CONSUMABLE },
		{ "XP_CATALYSER", XP_CATALYSER },
		{ "SCROLL", SCROLL },
		{ "SCROLL_R2", SCROLL_R2 },
		{ "COMMAND_TICKET", COMMAND_TICKET },
		{ "GENERIC_ITEM", GENERIC_ITEM },
	};

	CStringConversion<EItemFamily> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  UNDEFINED);


	// convert item family id to item family name string
	const std::string& toString( EItemFamily itemFamily )
	{
		return conversion.toString(itemFamily);
	}


	// convert item family name to item family enum value
	EItemFamily stringToItemFamily( const std::string& str )
	{
		return conversion.fromString(str);
	}

	bool	isSellableByPlayer( EItemFamily fam )
	{
		return fam!=FOOD;
	}

	bool	isResellable( EItemFamily fam )
	{
		return
			fam == ARMOR ||
			fam == MELEE_WEAPON ||
			fam == RANGE_WEAPON ||
			fam == AMMO ||
			fam == RAW_MATERIAL ||
			fam == SHIELD ||
			fam == JEWELRY ||
			fam == CRYSTALLIZED_SPELL ||
			fam == CONSUMABLE ||
			fam == ITEM_SAP_RECHARGE;
	}


	bool	isTextCustomizable( EItemFamily fam )
	{
		return
			fam == ARMOR ||
			fam == MELEE_WEAPON ||
			fam == RANGE_WEAPON ||
			fam == SHIELD ||
			fam == JEWELRY ||
			fam == SCROLL;	
	
	}

	/**
	 * returns true if items of this family are destroyed when they are completely worned out
	 */
	bool destroyedWhenWorned(EItemFamily family)
	{
		switch(family)
		{
		case ITEMFAMILY::MELEE_WEAPON:
		case ITEMFAMILY::RANGE_WEAPON:
		case ITEMFAMILY::ARMOR:
		case ITEMFAMILY::SHIELD:
			return true;

		default:
			return false;
		}
	}


}; // ITEMFAMILY
