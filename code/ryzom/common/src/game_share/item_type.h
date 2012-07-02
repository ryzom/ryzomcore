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



#ifndef RY_ITEM_TYPE_H
#define RY_ITEM_TYPE_H

#include "nel/misc/types_nl.h"

namespace ITEM_TYPE
{
	// Mode
	// nb : sell filter uses two 64b database values to build a bitfield, so the item type limit is 128 for now
	enum TItemType
	{
		DAGGER,
		SWORD,
		MACE,
		AXE,
		SPEAR,
		STAFF,
		TWO_HAND_SWORD,
		TWO_HAND_AXE,
		PIKE,
		TWO_HAND_MACE,
		AUTOLAUCH,
		BOWRIFLE,
		LAUNCHER,
		PISTOL,
		BOWPISTOL,
		RIFLE,
		AUTOLAUNCH_AMMO,
		BOWRIFLE_AMMO,
		LAUNCHER_AMMO,
		PISTOL_AMMO,
		BOWPISTOL_AMMO,
		RIFLE_AMMO,
		SHIELD,
		BUCKLER,
		LIGHT_BOOTS,
		LIGHT_GLOVES,
		LIGHT_PANTS,
		LIGHT_SLEEVES,
		LIGHT_VEST,
		MEDIUM_BOOTS,
		MEDIUM_GLOVES,
		MEDIUM_PANTS,
		MEDIUM_SLEEVES,
		MEDIUM_VEST,
		HEAVY_BOOTS,
		HEAVY_GLOVES,
		HEAVY_PANTS,
		HEAVY_SLEEVES,
		HEAVY_VEST,
		HEAVY_HELMET,
		ANKLET,
		BRACELET,
		DIADEM,
		EARING,
		PENDANT,
		RING,
		SHEARS,
		ArmorTool,
		AmmoTool,
		MeleeWeaponTool,
		RangeWeaponTool,
		JewelryTool,
		ToolMaker,
		CAMPSFIRE,
		MEKTOUB_PACKER_TICKET,
		MEKTOUB_MOUNT_TICKET,
		FOOD,
		MAGICIAN_STAFF,
		HAIR_MALE,
		HAIRCOLOR_MALE,
		TATOO_MALE,
		HAIR_FEMALE,
		HAIRCOLOR_FEMALE,
		TATOO_FEMALE,
		SERVICE_STABLE,
		JOB_ELEMENT,
		GENERIC,

		UNDEFINED,
		NB_ITEM_TYPE = UNDEFINED,
		LIMIT_64 = 64
	};


	/**
	 * get the right item type from the input string
	 * \param str the input string
	 * \return the TItemType associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	TItemType stringToItemType( const std::string& str );

	/**
	 * return the item family as a string
	 * \param itemFamily family to transform into a string
	 * \return the item family as a string
	 */
	const std::string& toString( TItemType item_type );

}; // ITEM_TYPE

#endif // RY_ITEM_TYPE_H
/* End of item_type.h */
