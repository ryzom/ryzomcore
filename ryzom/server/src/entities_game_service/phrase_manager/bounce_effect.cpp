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
// net
#include "nel/net/message.h"
//
#include "bounce_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_manager.h"
#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "creature_manager/creature.h"
#include "range_selector.h"
#include "phrase_manager/effect_factory.h"

extern CCreatureManager		CreatureManager;

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;
extern CRandom RandomGenerator;


//--------------------------------------------------------------
//		CBounceEffect::removed()
//--------------------------------------------------------------
void CBounceEffect::removed()
{
	if (!_AffectedEntity) return;

	// if entity is dead, do not send messages
	if (_AffectedEntity->isDead())
		return;

	DEBUGLOG("COMBAT EFFECT: Bounce effect ends on entity %s", _AffectedEntity->getId().toString().c_str());
	
/*	// send messages to target
	if (entity->getId().getType() == RYZOMID::player)
		PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), "EFFECT_BLEED_ENDED");

	// try to inform actor
	if ( _CreatorRowId != _TargetRowId && _CreatorRowId.isValid() && TheDataset.isDataSetRowStillValid(_CreatorRowId))
	{
		CCharacter *actor = PlayerManager.getChar(_CreatorRowId);
		if (actor != NULL)
		{
			TVectorParamCheck params;
			params.resize(1);
			params[0].Type = STRING_MANAGER::entity;
			params[0].EId = entity->getId();
			PHRASE_UTILITIES::sendDynamicSystemMessage( actor->getEntityRowId(), "EFFECT_BLEED_ENDED_ACTOR", params);
		}
	}
*/
} // removed //


//--------------------------------------------------------------
//		CBounceEffect::getTargetForBounce()
//--------------------------------------------------------------
CEntityBase *CBounceEffect::getTargetForBounce(CEntityBase *actor) const
{
	// get entities around
	if (!_AffectedEntity)
		_AffectedEntity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	
	if (!_AffectedEntity)
		return NULL;

	// get entities in surrouding area
	// range of the effect in meters = _Value
	CRangeSelector entitiesSelector;
	entitiesSelector.buildDisc( CEntityBaseManager::getEntityBasePtr(getCreatorRowId()), _AffectedEntity->getX(), _AffectedEntity->getY(), (float) _Value, EntityMatrix, true );

	// select valid entities
	const vector<CEntityBase*> &entities = entitiesSelector.getEntities();
	const uint size = (uint)entities.size();
	
	vector<CEntityBase*> selectedEntities;	
	selectedEntities.reserve(size);

	for (uint i = 0; i < entities.size() ; ++i)
	{
		if ( entities[i] && ( entities[i] != (CEntityBase*)(_AffectedEntity) ) && isEntityValidTarget(entities[i], actor) )
		{
			selectedEntities.push_back(entities[i]);
		}
	}

	if (selectedEntities.empty())
		return NULL;

	uint32 num = RandomGenerator.rand((uint16)selectedEntities.size()-1);

	return selectedEntities[num];
		
} // getTargetForBounce //


//--------------------------------------------------------------
//		CBounceEffect::isEntityValidTarget()
//--------------------------------------------------------------
bool CBounceEffect::isEntityValidTarget(CEntityBase *entity, CEntityBase *actor) const
{
#ifdef NL_DEBUG
	nlassert(entity);
#endif

	if (entity->isDead())
		return false;

	if (entity->getId().getType() == RYZOMID::player)
		return true;

	if ( entity->getContextualProperty().getValue().attackable() )
		return true;

	CCreature * creature = CreatureManager.getCreature( entity->getId() );
	if (creature && actor)
	{
		if (creature->checkFactionAttackable( actor->getId() ))
		{
			return true;
		}
	}
	
	return false;
} // checkTargetValidity //

CEffectTFactory<CBounceEffect> *CBounceEffectFactory = new CEffectTFactory<CBounceEffect>(EFFECT_FAMILIES::Bounce);

