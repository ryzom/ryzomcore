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


#ifndef R2_EASTER_EGG_H
#define R2_EASTER_EGG_H

#include <string>
#include <vector>
#include <map>

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/entity_id.h"

#include "game_share/far_position.h"

#include "game_share/scenario.h"

#include "game_item_manager/game_item_ptr.h"

class CR2EasterEgg : public NLMISC::CSingleton<CR2EasterEgg>
{
public:
	typedef uint32 TEasterEggId;
	typedef uint64 TEasterEggScenarioId;

	struct TEasterEggLoot
	{
		uint32						EasterEggId;
		std::vector< CGameItemPtr >	Item;
		uint32						ScenarioId;
		uint32						InstanceId;
		NLMISC::CEntityId			LastOwnerCharacterId;
		NLMISC::CEntityId			CreatureIdAsEgg;
		std::string					Name;
		std::string					Look;

		TEasterEggLoot() { ScenarioId = 0; InstanceId = 0; }
	};

	typedef std::map< NLMISC::CEntityId, std::vector<TEasterEggId> > TEntityEasterEggIdContainer;
	typedef std::map< TEasterEggId, TEasterEggLoot > TEasterEggContainer;
	typedef std::map< TEasterEggScenarioId, TEasterEggId > TEasterEggScenarioIdTranslator;

	// constructor
	CR2EasterEgg();

	// build the egg loot content and ask to AIS for spawn an easter egg creature (for player, item are dropped on ground)
	void dropMissionItem(std::vector< CGameItemPtr > items, TSessionId scenarioId, uint32 aiInstanceId, const NLMISC::CEntityId &lastCharacterOwner);

	// associate a creature to an easter egg
	void creatureEggSpawned(const NLMISC::CEntityId &creatureId, uint32 easterEggId );

	// a player character loot an easter egg
	void characterLootEasterEgg(const NLMISC::CEntityId &characterId, NLMISC::CEntityId creatureId);

	// activate an easter egg (spawned by scenario, not by character item drop)
	void activateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector< R2::TItemAndQuantity > &items, const CFarPosition &pos, const std::string& name="", const std::string& look="");

	// deactivate an easter egg (de-spawned by scenario, not by character loot)
	void deactivateEasterEgg(uint32 easterEggId, TSessionId scenarioId);

	// easter egg teleport (dropped by character) with act change
	void easterEggTPActChange(const NLMISC::CEntityId &characterId, const CFarPosition &pos);

	// scenario ended
	void endScenario( TSessionId scenarioId );

	// return all easter egg for a scenario
	void getEasterEggForScenario( TSessionId scenarioId, std::vector<R2::TEasterEggInfo> &easterEgg ) const;

	// return true if creature is an Easter Egg
	bool isEasterEgg( const NLMISC::CEntityId &eid ) const;
	
private:
	struct TEasterEggTPInfo
	{
		NLMISC::CEntityId CreatureId;
		uint32 EasterEggId;
		uint32 InstanceId;
		std::string Name;
		std::string Look;
	};

	uint32 _GetEasterNextId() { return _EasterEggNextId++; } 
	void _SpawnEasterEgg(uint32 easterEggId, NLMISC::CSheetId sheet, uint32 aiInstanceId, sint32 x, sint32 y, sint32 z, float heading, const std::string&name="", const std::string& look="") const;
	void _UnspawnEasterEgg(uint32 easterEggId, uint32 aiInstanceId);
	void _AddEntityEasterAssociation(const NLMISC::CEntityId &entityId, uint32 easterEggId);
	void _RemoveEntityEasterAssociation(const NLMISC::CEntityId &entityId, uint32 easterEggId);
	void _RemoveAllEntitiesAssociation(uint32 easterEggId);
	void _deactivateEasterEgg(uint32 easterEggId);

	TEntityEasterEggIdContainer		_EntityEasterEggId;
	TEasterEggContainer				_EasterEgg;
	TEasterEggScenarioIdTranslator	_EasterEggScenarioIdTranslator;
	uint32							_EasterEggNextId;
};

#endif
