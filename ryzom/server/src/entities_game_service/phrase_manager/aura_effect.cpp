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



//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
// nel
#include "nel/net/message.h"
// game share
#include "game_share/visual_fx.h"
//
#include "player_manager/character.h"
#include "team_manager/team_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "creature_manager/creature.h"
#include "phrase_manager/area_effect.h"
#include "aura_effect.h"


//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CAuraBaseEffect);
NL_INSTANCE_COUNTER_IMPL(IAuraEffectFactory);

vector< pair< POWERS::TPowerType , IAuraEffectFactory* > >* IAuraEffectFactory::Factories = NULL;


//--------------------------------------------------------------
//		CAuraRootEffect::update()
//--------------------------------------------------------------
bool CAuraRootEffect::update(CTimerEvent * event, bool )
{
	// get creator
	CEntityBase *creator = CEntityBaseManager::getEntityBasePtr(_CreatorRowId);
	if (!creator)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	
	// get entities in surrouding area
	const sint32 posX = creator->getX();
	const sint32 posY = creator->getY();
	if (posX != 0 && posY != 0)
	{
		// entity isn't teleporting
		CAuraEntitySelector entitiesSelector;
		// no offensive aura right now, need to check last bool is this changes
		entitiesSelector.buildTargetList( creator, posX, posY, _AuraRadius, false, true );

		// create or update effect on entities returned
		const vector<CEntityBase*> &entities = entitiesSelector.getEntities();
		const uint size = (uint)entities.size();
		for (uint i = 0; i < size ; ++i)
		{
			if (entities[i] && isEntityValidTarget(entities[i], creator) )
				createEffectOnEntity(entities[i], creator);
		}
	}

#ifdef NL_DEBUG
	nldebug("Root aura effect Delta between last update and update : %d", CTickEventHandler::getGameCycle() - _LastUpdateDate);
	_LastUpdateDate = CTickEventHandler::getGameCycle();
#endif

	// set timer for next update
	_UpdateTimer.setRemaining(AurasUpdateFrequency, event);

	return false;
} // update //

//--------------------------------------------------------------
//		CAuraRootEffect::removed()
//--------------------------------------------------------------
void CAuraRootEffect::removed()
{
	if (TheDataset.isAccessible(_CreatorRowId))
	{
		// remove fx on the actor
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _CreatorRowId, DSPropertyVISUAL_FX );
		CVisualFX fx;
		fx.unpack(visualFx.getValue());
		fx.Aura = MAGICFX::NoAura;
		sint64 prop;
		fx.pack(prop);
		visualFx = (sint16)prop;
	}
} // removed //

//--------------------------------------------------------------
//		CAuraRootEffect::createEffectOnEntity()
//--------------------------------------------------------------
void CAuraRootEffect::createEffectOnEntity(CEntityBase *entity, CEntityBase *creator)
{
	// only apply on players
	if (entity->getId().getType() != RYZOMID::player)
		return;

	CCharacter *character = dynamic_cast<CCharacter*> (entity);
	if (!character)
		return;

	// get effects on entity and search for createdEffectFamily
	const std::vector<CSEffectPtr>& effects = entity->getSEffects();
	for (uint i = 0 ; i < effects.size() ; ++i)
	{
		if (effects[i] && effects[i]->getFamily() == _CreatedEffectFamily)
		{
			// find an effect created by this root effect, refesh it's lifetime
			CAuraBaseEffect *auraEffect = dynamic_cast<CAuraBaseEffect *> (static_cast<CSEffect*>(effects[i]));
			if (!auraEffect)
			{
				nlwarning("<CAuraRootEffect::createEffectOnEntity> Found an effect of family %s, but cannot dynamic cast it in CAuraBaseEffect class", EFFECT_FAMILIES::toString(_CreatedEffectFamily).c_str());
			}
			else
			{
				if (_Value < auraEffect->getParamValue())
				{
					auraEffect->stopEffect();
				}
				else
				{
					auraEffect->addLifeTime();
					return;
				}
			}
		}
	}

	// no effect found, create one if the entity can receive this effect
	static NLMISC::TGameCycle endDate;
	if (character->isAuraEffective(_PowerType, endDate, creator->getId()) || _IsFromConsumable )
	{
		CAuraBaseEffect *effect = IAuraEffectFactory::buildEffectFromPowerType(_PowerType, *this, entity->getEntityRowId());
		if (effect)
		{
			effect->setIsFromConsumable(_IsFromConsumable);
			effect->setEffectDisabledEndDate( CTickEventHandler::getGameCycle() + _TargetDisableTime );
			entity->addSabrinaEffect(effect);
			character->useAura(_PowerType, CTickEventHandler::getGameCycle(), CTickEventHandler::getGameCycle() + _TargetDisableTime, creator->getId());

			//send message to newly affected entity
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::power_type);
			params[0].Enum = _PowerType;
			PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(), "AURA_EFFECT_BEGIN", params);

			// set visual fx on the entity (not for the acting entity)
			if (entity->getEntityRowId() !=  _CreatorRowId)
			{
				character->incNbAura();
			}
		}
	}
} // createEffectOnEntity //


//--------------------------------------------------------------
//		CAuraRootEffect::isEntityValidTarget()
//--------------------------------------------------------------
bool CAuraRootEffect::isEntityValidTarget(CEntityBase *entity, CEntityBase *creator)
{
	if (!entity || entity->isDead()) return false;

	if (entity->getId().getType() != RYZOMID::player)
		return false;

	if (entity == creator)
		return true;

	CCharacter *userPlayer = dynamic_cast<CCharacter*> (creator);
	if (!userPlayer)
		return false;	

	CCharacter *targetPlayer = dynamic_cast<CCharacter*> (entity);
	if (!targetPlayer)
		return false;

	// TODO: kxu: why this???
	//userPlayer->getPVPInterface().canHelp( targetPlayer );

	// test team
	const uint16 teamId = userPlayer->getTeamId();
	if (teamId != CTEAM::InvalidTeamId)
	{
		if ((targetPlayer->getTeamId() == teamId) && CPVPManager2::getInstance()->isCurativeActionValid(dynamic_cast<CCharacter *>(creator), entity, true))
			return true;
	}

	if (_AffectGuild)
	{
		if (targetPlayer->getGuildId() != 0 && targetPlayer->getGuildId() == userPlayer->getGuildId())
			return true;
	}

	CCreature * creature = CreatureManager.getCreature( entity->getId() );
	if (creature && creator)
	{
		if (creature->checkFactionAttackable( creator->getId() ))
		{
			return true;
		}
	}

	return false;
}// isEntityValidTarget //

CAuraBaseEffect::CAuraBaseEffect( const CAuraRootEffect &rootEffect, TDataSetRow targetRowId) :
		CSTimedEffect(	rootEffect.getCreatorRowId(), 
						targetRowId, rootEffect.createdEffectFamily(), 
						false, 
						rootEffect.getParamValue(), 
						0, 
						CTickEventHandler::getGameCycle() + (NLMISC::TGameCycle)AurasUpdateFrequency.get() + 10)
{		
	_PowerType = rootEffect.powerType();
	_IsFromConsumable = false;
}

void CAuraBaseEffect::addLifeTime()
{
	_EndDate += (NLMISC::TGameCycle)AurasUpdateFrequency.get();
	nlassert(_EndTimer.getEvent() != NULL);
	_EndTimer.set(_EndDate, _EndTimer.getEvent());
#ifdef NL_DEBUG
	nldebug("aura effect Delta between current date and end date : %d", _EndDate - CTickEventHandler::getGameCycle());
#endif
}

//--------------------------------------------------------------
//		CAuraBaseEffect::removed()
//--------------------------------------------------------------
void CAuraBaseEffect::removed()
{
	if (!TheDataset.isAccessible(_TargetRowId))
		return;
	
	// send message
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::power_type);
	params[0].Enum = _PowerType;
	PHRASE_UTILITIES::sendDynamicSystemMessage(_TargetRowId, "AURA_EFFECT_END", params);
	
	//remove visual fx
	CCharacter *player = PlayerManager.getChar(_TargetRowId);
	if (!player)
		return;
	
	player->decNbAura();
	
	if (_EffectIndexInDB >= 0)
	{
		if (_IsFromConsumable)
			player->removeEffectInDB((uint8)_EffectIndexInDB, true);
		else
			player->disableEffectInDB( (uint8)_EffectIndexInDB,true, _DisabledEndDate);
	}
} // removed //


CAuraEffectTFactory<CAuraBaseEffect> *CMeleeProtectionEffectFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::MeleeProtection);
CAuraEffectTFactory<CAuraBaseEffect> *CRangeProtectionEffectFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::Umbrella);
CAuraEffectTFactory<CAuraBaseEffect> *CMagicProtectionEffectFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::AntiMagicShield);
CAuraEffectTFactory<CAuraBaseEffect> *CWarCryFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::WarCry);
CAuraEffectTFactory<CAuraBaseEffect> *CFireWallFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::FireWall);
CAuraEffectTFactory<CAuraBaseEffect> *CThornWallFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::ThornWall);
CAuraEffectTFactory<CAuraBaseEffect> *CWaterWallFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::WaterWall);
CAuraEffectTFactory<CAuraBaseEffect> *CLightningWallFactoryInstance = new CAuraEffectTFactory<CAuraBaseEffect>(POWERS::LightningWall);
