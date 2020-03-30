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



#ifndef RY_ITEM_FAMILY_H
#define RY_ITEM_FAMILY_H

#include "nel/misc/types_nl.h"

namespace ITEMFAMILY
{
	// Mode
	enum EItemFamily
	{
		UNDEFINED = 0,
		SERVICE,
		ARMOR,
		MELEE_WEAPON,
		RANGE_WEAPON,
		AMMO,
		RAW_MATERIAL,
		SHIELD,
		CRAFTING_TOOL,
		HARVEST_TOOL,
		TAMING_TOOL,
		TRAINING_TOOL,
		AI,
		BRICK,
		FOOD,
		JEWELRY,
		CORPSE,
		CARRION,
		BAG,
		STACK,
		DEAD_SEED,
		TELEPORT,
		GUILD_FLAG,
		LIVING_SEED,
		LITTLE_SEED,
		MEDIUM_SEED,
		BIG_SEED,
		VERY_BIG_SEED,
		MISSION_ITEM,
		CRYSTALLIZED_SPELL,
		ITEM_SAP_RECHARGE,
		PET_ANIMAL_TICKET,
		GUILD_OPTION,
		HANDLED_ITEM,
		COSMETIC,
		CONSUMABLE,
		XP_CATALYSER,
		SCROLL,
		SCROLL_R2,
		COMMAND_TICKET,
		GENERIC_ITEM
	};


	/**
	 * get the right item family from the input string
	 * \param str the input string
	 * \return the EItemFamily associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	EItemFamily stringToItemFamily( const std::string& str );

	/**
	 * return the item family as a string
	 * \param itemFamily family to transform into a string
	 * \return the item family as a string
	 */
	const std::string& toString( EItemFamily itemFamily );

	/**
	 * returns true if items of this family are destroyed when they are completely worned out
	 */
	bool destroyedWhenWorned(EItemFamily family);

	/// return true if this family of item can be sold to a bot
	bool	isSellableByPlayer( EItemFamily fam );
	/// return true if this family of item can be resold to players
	bool	isResellable( EItemFamily fam );
	/// return true if this family of item can display a custom text
	bool	isTextCustomizable( EItemFamily fam );
	/// is craftable

}; // ITEMFAMILY

#endif // RY_ITEM_FAMILY_H
/* End of item_family.h */
