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
#include "inventories.h"
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace INVENTORIES
{
	uint32 TItemId::_LastCreation = 0;
	uint32 TItemId::_NextSerialNumber = 0;


	NL_BEGIN_STRING_CONVERSION_TABLE(TInventory)
		NL_STRING_CONVERSION_TABLE_ENTRY(handling)
		NL_STRING_CONVERSION_TABLE_ENTRY(temporary)
		NL_STRING_CONVERSION_TABLE_ENTRY(equipment)
		NL_STRING_CONVERSION_TABLE_ENTRY(bag)
		NL_STRING_CONVERSION_TABLE_ENTRY(pet_animal1)
		NL_STRING_CONVERSION_TABLE_ENTRY(pet_animal2)
		NL_STRING_CONVERSION_TABLE_ENTRY(pet_animal3)
		NL_STRING_CONVERSION_TABLE_ENTRY(pet_animal4)
		//		NL_STRING_CONVERSION_TABLE_ENTRY(bot_gift)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNDEFINED)
		NL_STRING_CONVERSION_TABLE_ENTRY(exchange)
		//		NL_STRING_CONVERSION_TABLE_ENTRY(exchange_proposition)
		NL_STRING_CONVERSION_TABLE_ENTRY(trading)
		NL_STRING_CONVERSION_TABLE_ENTRY(reward_sharing)
		NL_STRING_CONVERSION_TABLE_ENTRY(guild)
		NL_STRING_CONVERSION_TABLE_ENTRY(player_room)
		NL_END_STRING_CONVERSION_TABLE(TInventory, InventoryToString, UNDEFINED)


		const std::string& toString( TInventory inv )
	{
		// if this raise, correct the table above
		nlctassert(MAX_INVENTORY_ANIMAL==4);
		return InventoryToString.toString(inv);
	}

	// convert job name to job enum value
	TInventory toInventory( const std::string& str )
	{
		TInventory inv = InventoryToString.fromString(str);

		// support for pack_animalx for backward compatibility (necessary to load old save games)
		if (inv == UNDEFINED)
		{
			if (str == "pack_animal1")
				inv = pet_animal1;
			else if (str == "pack_animal2")
				inv = pet_animal2;
			else if (str == "pack_animal3")
				inv = pet_animal3;
			else if (str == "pack_animal4")
				inv = pet_animal4;
		}

		return inv;
	}


	// String for inventories that are not using CInventoryUpdater
	const char *DatabaseStringFromEInventory [NUM_ALL_INVENTORY] =
	{
		"HAND",			// handling
			"TEMP",			// temporary
			//	"",				// pick-up
			"EQUIP",		// equipment
			"",				// bag
			"",				// pack_animal1
			"",				// pack_animal2
			"",				// pack_animal3
			"",				// pack_animal4
			"",				// harvest
			//	"",				// bot_gift
			"",				// exchange
			//	"",				// exchange_proposition
			"",				// guild
			"",				// player_room
			""				// unknown
	};


	//
	const char *CInventoryCategoryForCharacter::InventoryStr [CInventoryCategoryForCharacter::NbInventoryIds] =
	{ "BAG",		"PACK_ANIMAL0",	"PACK_ANIMAL1",	"PACK_ANIMAL2",	"PACK_ANIMAL3",	"ROOM" };
	const uint CInventoryCategoryForCharacter::InventoryNbSlots [CInventoryCategoryForCharacter::NbInventoryIds] =
	{ NbBagSlots,	NbPackerSlots,	NbPackerSlots,	NbPackerSlots,	NbPackerSlots,	NbRoomSlots };
	// Other values to change according to these InventoryNbSlots:
	// - game_share.h/inventories.h: CInventoryCategoryForCharacter::SlotBitSize
	// - data_common/database.xml: INVENTORY:BAG count
	// - data/gamedev/interfaces_v3/inventory.xml: inventory:content:bag param inv_branch_nb

	const char *CInventoryCategoryForGuild::InventoryStr [CInventoryCategoryForGuild::NbInventoryIds] =
	{ "GUILD" };
	const uint CInventoryCategoryForGuild::InventoryNbSlots [CInventoryCategoryForGuild::NbInventoryIds] =
	{ NbGuildSlots };
	// Other values to change according to this InventoryNbSlots:
	// - game_share.h/inventories.h: CInventoryCategoryForGuild::SlotBitSize
	// - data_common/database.xml: GUILD:INVENTORY count
	// - data/gamedev/interfaces_v3/inventory.xml: inventory:content:guild param inv_branch_nb

	const char *InfoVersionStr = "INFO_VERSION";

	const char *CItemSlot::ItemPropStr [NbItemPropId] =
	{ "SHEET", "QUALITY", "QUANTITY", "USER_COLOR", "LOCKED", "WEIGHT", "NAMEID", "ENCHANT", "RM_CLASS_TYPE", "RM_FABER_STAT_TYPE", "PRICE", "RESALE_FLAG", "PREREQUISIT_VALID", "WORNED" };
	const uint CItemSlot::DataBitSize [NbItemPropId]  =
	{ 32,      10,        10,         3,            10,        16,       32,       10,        3,				  5,				     32,      2,			1,					1 };
}
