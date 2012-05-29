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
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_manager.h"
#include "player_manager/character.h"
#include "entity_structure/statistic.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "nel/misc/random.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CEndEffectTimerEvent);
NL_INSTANCE_COUNTER_IMPL(CSEffect);

uint32		CSEffect::NbAllocatedEffects = 0;
uint32		CSEffect::NbDesallocatedEffects = 0;

//-----------------------------------------------
// CUpdateEffectTimerEvent timerCallback
//-----------------------------------------------
void CUpdateEffectTimerEvent::timerCallback(CTimer* owner) 
{
	H_AUTO(CUpdateEffectTimerEvent);

	nlassert(_Parent);

	if (_FirstUpdate)
	{
		_FirstUpdate = false;
		_Parent->sendEffectBeginMessages();
	}

	bool applyEffect = true;

	// call effect update method only if stackable or most powerful effect of its kind
	if (!_Parent->isStackable())
	{
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_Parent->getTargetRowId());
		if (entity)
		{
			CSEffect *effect = entity->lookForActiveEffect(_Parent->getFamily());
			if (effect != _Parent)
				applyEffect = false;
		}
	}

	_Parent->update(this, applyEffect);
}


//-----------------------------------------------
// CEndEffectTimerEvent timerCallback
//-----------------------------------------------
void CEndEffectTimerEvent::timerCallback(CTimer* owner) 
{
	H_AUTO(CEndEffectTimerEvent);

	_Parent->endEffect();
}


//-----------------------------------------------
// CSEffect endEffect
//-----------------------------------------------
void CSEffect::endEffect()
{
	if (_IsRemoved)
		return;

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
		return;

	_UpdateTimer.reset();

	entity->removeSabrinaEffect(this);
}

//-----------------------------------------------
// CSEffect sendEffectBeginMessages
//-----------------------------------------------
void CSEffect::sendEffectBeginMessages()
{
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);

	// check an effect name has been set
	if (entity && !_EffectChatName.empty())
	{
		
		CEntityId actorId;
		if (TheDataset.isDataSetRowStillValid(_CreatorRowId))
			actorId = TheDataset.getEntityId(_CreatorRowId);
		else
		{
			nlwarning("EFFECT : creator row id no longer in mirror, cannot access to entityId");
		}
		
//		TVectorParamCheck params;
		
		// send info messages to client and spectators
		if (entity->getEntityRowId() != _CreatorRowId)
		{		
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);

			string msgName = "EFFECT_"+_EffectChatName+"_BEGIN";
			
			// effect creator
			if ( actorId.getType() == RYZOMID::player)
			{
				params[0].setEIdAIAlias( entity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(entity->getId()) );
				
				const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
				PHRASE_UTILITIES::sendDynamicSystemMessage(_CreatorRowId, str, params);
			}
			// effect target
			if (entity->getId().getType() == RYZOMID::player)
			{
				params[0].setEIdAIAlias( actorId, CAIAliasTranslator::getInstance()->getAIAlias(actorId) );
				const string str = NLMISC::toString("%s_TARGET",msgName.c_str());
				PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(), str, params);
			}
			
//			// spectators
//			CEntityId centerId;
//			if ( entity->getId().getType() == RYZOMID::player || entity->getId().getType() == RYZOMID::npc)
//				centerId = entity->getId();
//			else if ( actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc)
//				centerId = actorId;
//			
//			if (centerId != CEntityId::Unknown)
//			{
//				params.resize(2);
//				params[0].Type = STRING_MANAGER::entity;
//				params[0].EId = actorId;
//				params[1].Type = STRING_MANAGER::entity;
//				params[1].EId = entity->getId();
//				
//				vector<CEntityId> excluded;
//				excluded.push_back(actorId);
//				excluded.push_back(entity->getId());
//				const string str = NLMISC::toString("%s_SPECTATORS",msgName.c_str());
//				PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(centerId), excluded, str, params);
//			}
		}
		else
		{
			if (actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc)
			{
				string msgName = "EFFECT_"+_EffectChatName+"_SELF_BEGIN";
				
				if ( actorId.getType() == RYZOMID::player)
				{
					const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
					PHRASE_UTILITIES::sendDynamicSystemMessage(_CreatorRowId, str);
				}
				else if( actorId.getType() != RYZOMID::npc )
				{
					// cannot send spectator messages for creatures
				}
				
//				params.resize(1);
//				// send to spectators
//				switch(actorId.getType())
//				{
//				case RYZOMID::player:
//					params[0].Type = STRING_MANAGER::player;
//					msgName = "EFFECT_"+_EffectChatName+"_SELF_BEGIN_SPECTATORS_PLAYER";
//					break;
//				case RYZOMID::npc:
//					params[0].Type = STRING_MANAGER::bot;
//					msgName = "EFFECT_"+_EffectChatName+"_SELF_BEGIN_SPECTATORS_NPC";
//					break;
//				default:
//					// cannot send spectator messages for creatures
//					//params[0].Type = STRING_MANAGER::creature;
//					//msgName = "EFFECT_"+_EffectChatName+"_SELF_BEGIN_SPECTATORS_CREATURE";
//					break;
//				};
//
//				params[0].EId = actorId;
//				
//				vector<CEntityId> excluded;
//				excluded.push_back(actorId);
//				PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(actorId), excluded, msgName, params);
			}				
		}
	}	
}// sendEffectBeginMessages //



//-----------------------------------------------
// CSEffect sendEffectBeginMessages
//-----------------------------------------------
void CSEffect::sendEffectEndMessages()
{
	// if no effect name, returns
	if (_EffectChatName.empty())
		return;
	
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
		return;

	// returns if entity already dead
	if (entity->isDead())
		return;
	
	// if any other effect of the same family still exists on the target then returns without sending msg
	const std::vector<CSEffectPtr>& effects = entity->getSEffects();
	for (uint i = 0 ; i < effects.size() ; ++i)
	{
		if (effects[i] && effects[i]->_EffectId != _EffectId && effects[i]->getFamily() == _Family)
			return;
	}	
	
	// send messages
	CEntityId actorId;
	if (TheDataset.isDataSetRowStillValid(_CreatorRowId))
		actorId = TheDataset.getEntityId(_CreatorRowId);
	else
		return;
	
	CEntityId targetId;
	if (TheDataset.isDataSetRowStillValid(_TargetRowId))
		targetId = TheDataset.getEntityId(_TargetRowId);
	else
		return;
	
//	TVectorParamCheck params;	
	
	string msgName = "EFFECT_"+_EffectChatName+"_END";
	
	// send chat message to target
	if (targetId.getType() == RYZOMID::player)
	{
		const string str = NLMISC::toString("%s_TARGET",msgName.c_str());
		PHRASE_UTILITIES::sendDynamicSystemMessage(_TargetRowId, str);
	}
	// send chat message to creator if != target
	if (actorId != targetId && actorId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
		const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
		PHRASE_UTILITIES::sendDynamicSystemMessage(_CreatorRowId, str, params);
	}
	
	// spectators
//	CEntityId centerId;
//	if ( targetId.getType() == RYZOMID::player || targetId.getType() == RYZOMID::npc)
//		centerId = targetId;
//	else if ( actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc)
//		centerId = actorId;
//	else
//		return;
//	
//	params.resize(1);
//	params[0].Type = STRING_MANAGER::entity;
//	params[0].EId = targetId;
//	vector<CEntityId> excluded;
//	excluded.push_back(actorId);
//	excluded.push_back(targetId);
//	const string str = NLMISC::toString("%s_SPECTATORS",msgName.c_str());
//	PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(centerId), excluded, str, params);
} // sendEffectEndMessages //


//-----------------------------------------------------------------------------
// Persistent data for CEffect
//-----------------------------------------------------------------------------

// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of perstent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily
#define PERSISTENT_CLASS CSEffect

#define PERSISTENT_DATA\
	LPROP2(_Family,			string,		if (_Family!=EFFECT_FAMILIES::Unknown),		EFFECT_FAMILIES::toString(_Family),		_Family=EFFECT_FAMILIES::toEffectFamily(val))\
	PROP(sint32,_Value)\
	PROP(uint32,_Power)\
	LPROP2(_Skill,			string,		if(_Skill!=SKILLS::unknown),				SKILLS::toString(_Skill),				_Skill=SKILLS::toSkill(val))\
	LPROP2(_EffectSheetId,	string,		if(_EffectSheetId!=CSheetId::Unknown),		_EffectSheetId.toString(),				_EffectSheetId=CSheetId(val))\
	PROP(string,_EffectChatName)\
	PROP(bool,_IsStackable)\
	PROP(bool,_IsRemoved)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CSTimedEffect
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CSTimedEffect

#define PERSISTENT_DATA\
	STRUCT2(SEffect,	CSEffect::store(pdr), CSEffect::apply(pdr))\
	PROP2(_EndDate,		TGameCycle,	_EndDate>CTickEventHandler::getGameCycle()?_EndDate-CTickEventHandler::getGameCycle():(TGameCycle)0,	_EndDate=val)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
