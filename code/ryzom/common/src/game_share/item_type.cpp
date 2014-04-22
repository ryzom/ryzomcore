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

#include "item_type.h"

using namespace std;
using namespace NLMISC;

namespace ITEM_TYPE
{
	// The conversion table
	const CStringConversion<TItemType>::CPair stringTable [] =
	{
		{ "DAGGER", DAGGER },
		{ "SWORD", SWORD },
		{ "MACE", MACE },
		{ "AXE", AXE },
		{ "SPEAR", SPEAR },
		{ "STAFF", STAFF },
		{ "TWO_HAND_SWORD", TWO_HAND_SWORD },
		{ "TWO_HAND_AXE", TWO_HAND_AXE },
		{ "PIKE", PIKE },
		{ "TWO_HAND_MACE", TWO_HAND_MACE },
		{ "AUTOLAUCH", AUTOLAUCH },
		{ "BOWRIFLE", BOWRIFLE },
		{ "LAUNCHER", LAUNCHER },
		{ "PISTOL", PISTOL },
		{ "BOWPISTOL", BOWPISTOL },
		{ "RIFLE", RIFLE },
		{ "AUTOLAUNCH_AMMO", AUTOLAUNCH_AMMO },
		{ "BOWRIFLE_AMMO", BOWRIFLE_AMMO },
		{ "LAUNCHER_AMMO", LAUNCHER_AMMO },
		{ "PISTOL_AMMO", PISTOL_AMMO },
		{ "BOWPISTOL_AMMO", BOWPISTOL_AMMO },
		{ "RIFLE_AMMO", RIFLE_AMMO },
		{ "SHIELD", SHIELD },
		{ "BUCKLER", BUCKLER },
		{ "LIGHT_BOOTS", LIGHT_BOOTS },
		{ "LIGHT_GLOVES", LIGHT_GLOVES },
		{ "LIGHT_PANTS", LIGHT_PANTS },
		{ "LIGHT_SLEEVES", LIGHT_SLEEVES },
		{ "LIGHT_VEST", LIGHT_VEST },
		{ "MEDIUM_BOOTS", MEDIUM_BOOTS },
		{ "MEDIUM_GLOVES", MEDIUM_GLOVES },
		{ "MEDIUM_PANTS", MEDIUM_PANTS },
		{ "MEDIUM_SLEEVES", MEDIUM_SLEEVES },
		{ "MEDIUM_VEST", MEDIUM_VEST },
		{ "HEAVY_BOOTS", HEAVY_BOOTS },
		{ "HEAVY_GLOVES", HEAVY_GLOVES },
		{ "HEAVY_PANTS", HEAVY_PANTS },
		{ "HEAVY_SLEEVES", HEAVY_SLEEVES },
		{ "HEAVY_VEST", HEAVY_VEST },
		{ "HEAVY_HELMET", HEAVY_HELMET },
		{ "ANKLET", ANKLET },
		{ "BRACELET", BRACELET },
		{ "DIADEM", DIADEM },
		{ "EARING", EARING },
		{ "PENDANT", PENDANT },
		{ "RING", RING },
		{ "SHEARS", SHEARS },
		{ "ArmorTool", ArmorTool },
		{ "AmmoTool", AmmoTool },
		{ "MeleeWeaponTool", MeleeWeaponTool },
		{ "RangeWeaponTool", RangeWeaponTool },
		{ "JewelryTool", JewelryTool },
		{ "ToolMaker", ToolMaker },
		{ "CAMPSFIRE", CAMPSFIRE },
		{ "MEKTOUB_PACKER_TICKET", MEKTOUB_PACKER_TICKET },
		{ "MEKTOUB_MOUNT_TICKET", MEKTOUB_MOUNT_TICKET },
		{ "FOOD", FOOD },
		{ "MAGICIAN_STAFF", MAGICIAN_STAFF },
		{ "HAIR_MALE", HAIR_MALE },
		{ "HAIRCOLOR_MALE", HAIRCOLOR_MALE },
		{ "TATOO_MALE", TATOO_MALE },
		{ "HAIR_FEMALE", HAIR_FEMALE },
		{ "HAIRCOLOR_FEMALE", HAIRCOLOR_FEMALE },
		{ "TATOO_FEMALE", TATOO_FEMALE },
		{ "SERVICE_STABLE", SERVICE_STABLE },
		{ "JOB_ELEMENT", JOB_ELEMENT },
		{ "GENERIC", GENERIC },
	};

	CStringConversion<TItemType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  UNDEFINED);


	// convert item family id to item family name string
	const std::string& toString( TItemType item_type )
	{
		return conversion.toString(item_type);
	}


	// convert item type name to item type enum value
	TItemType stringToItemType( const std::string& str )
	{
		return conversion.fromString(str);
	}
}; // ITEM_TYPE
