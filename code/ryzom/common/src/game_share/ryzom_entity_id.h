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



#ifndef RYZOM_ENTITY_ID
#define RYZOM_ENTITY_ID

#include "nel/misc/entity_id.h"

#include <string>

/// typedef of EntityId
namespace RYZOMID
{
	enum TTypeId
	{
		// warning: respect separation with creature /object, it's used in front-end vision prioritizer
		// creature part (include player)
		creature_begin = 0,
			player = creature_begin,
			bot_ai_begin,
				npc = bot_ai_begin,	//1
				creature,			//2
				mount,				//3
				pack_animal,		//4
				flora,				//5
			bot_ai_end = flora,
		creature_end = bot_ai_end,

		// objects part
		object,				// 6
		building,			// 7
		position,			// 8

		// other
		chatGroup,			// 9
		dynChatGroup,		// 10
		deposit,			// 11 deposit = 'gisement', used for Harvest

		// id for fame management
		guild,				// 12
		civilisation,		// 13
		fame_memory,		// 14

		// id for pacs system
		trigger,			// 15
		// id for forage system
		forageSource,		// 16
		fx_entity,			// 17

		// id for persistant strings
		guildName,			// 18
		guildDescription,	// 19
		characterName,		// 20

		// id for ais named entities
		ais_named_entity,	// 21

/*
		// Inventory type of entity
		INVENTORY = 96,
		handling = INVENTORY,							//Hand for inventory manipulation
		sheath,											//Sheath inventory (6 max)
		pickup = sheath + NB_MAX_SHEATH,				//Pickup inventory
		equipment,										//equipment inventory
		bag,											//bag inventory
		creature_loot,									//dead creature loot content
	//	knownBrick,										//know bricks inventory
	//	KnownSentence,									//know sentence inventory
	//	memorizedSentence,								//memorized sentence inventory
		pack_animal,									//pack animal inventories (5 max)
		seed_stand = pack_animal + MAX_INVENTORY_ANIMAL,//flat inventory
		harvest,										//harvest inventory
		exchange,										//exchange inventory
		END_INVENTORY,
*/
		// Special type
/*		world = 32,
		weather,
		day_cycle,
*/
		// for unknown type
		unknown = 255,

	};

	TTypeId fromString( const std::string& str );

	const std::string& toString( TTypeId TypeId );
} // RYZOM_ID

#endif // RYZOM_ENTITY_ID
