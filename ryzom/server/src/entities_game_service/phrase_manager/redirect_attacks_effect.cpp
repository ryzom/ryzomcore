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
#include "redirect_attacks_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "range_selector.h"
#include "phrase_manager/effect_factory.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager	PlayerManager;
extern CCreatureManager CreatureManager;
extern CRandom			RandomGenerator;

//--------------------------------------------------------------
//		CRedirectAttacksEffect::ctor
//--------------------------------------------------------------
CRedirectAttacksEffect::CRedirectAttacksEffect( const TDataSetRow & creatorRowId, 
					   const TDataSetRow & targetRowId, 
					   EFFECT_FAMILIES::TEffectFamily family, 
					   sint32 effectValue, 
					   NLMISC::TGameCycle endDate,
					   float range
					   )
					   :	CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue,(uint8)0, endDate),
					   _Range(range)
{
	// init affected creature
	_AffectedCreature = CreatureManager.getCreature(_TargetRowId);
	if (!_AffectedCreature)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		nlwarning("EFFECT: failed to init affected creature %s", _TargetRowId.toString().c_str() );
	}
}

//--------------------------------------------------------------
//		CRedirectAttacksEffect::removed()
//--------------------------------------------------------------
void CRedirectAttacksEffect::removed()
{
	if (!_AffectedCreature) return;

	// if entity is dead, do not send messages
	if (_AffectedCreature->isDead())
		return;

	DEBUGLOG("COMBAT EFFECT: Damage aura effect ends on entity %s", _AffectedCreature->getId().toString().c_str());
	
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
//		CRedirectAttacksEffect::getTargetForRedirection()
//--------------------------------------------------------------
CEntityBase *CRedirectAttacksEffect::getTargetForRedirection() const
{
	if (!_AffectedCreature)
		return NULL;

	// init targeted entity
	if (!_TargetEntityForRedirection)
	{
		// get entities in aggro list
		set<TDataSetRow> aggroList = _AffectedCreature->getAggroList();
		
		// select valid target
		const uint size = (uint)aggroList.size();
		
		vector<CEntityBase*> selectedEntities;	
		selectedEntities.reserve(size);
		
		for ( set<TDataSetRow>::const_iterator it = aggroList.begin() ; it != aggroList.end(); ++it)
		{
			CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(*it);
			if ( entity && entity != (CEntityBase*)_AffectedCreature && isEntityValidTarget(entity) )
			{
				selectedEntities.push_back(entity);
			}
		}
		
		if (selectedEntities.empty())
			return NULL;
		
		uint32 num = RandomGenerator.rand((uint16)selectedEntities.size()-1);
		
		_TargetEntityForRedirection = selectedEntities[num];
	}

	// make a random test to determine if an entity is hit
	const uint8 chances = (uint8)_Value;
	if (RandomGenerator.rand(99) >= chances)
		return NULL;
	else
		return _TargetEntityForRedirection;
} // getTargetForRedirection //


//--------------------------------------------------------------
//		CRedirectAttacksEffect::isEntityValidTarget()
//--------------------------------------------------------------
bool CRedirectAttacksEffect::isEntityValidTarget(CEntityBase *entity) const
{
#ifdef NL_DEBUG
	nlassert(entity);
#endif

	// check distance
	if (PHRASE_UTILITIES::getDistance(_TargetRowId, entity->getEntityRowId()) > _Range )
		return false;

	if (entity->getId().getType() == RYZOMID::player)
		return true;

	if ( entity->getContextualProperty().getValue().attackable() )
		return true;
	
	return false;
} // checkTargetValidity //

CEffectTFactory<CRedirectAttacksEffect> *CRedirectAttacksEffectFactory = new CEffectTFactory<CRedirectAttacksEffect>(EFFECT_FAMILIES::RedirectAttacks);

