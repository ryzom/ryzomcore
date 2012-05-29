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
// misc
#include "nel/misc/bit_mem_stream.h"
// game_share
#include "game_share/generic_xml_msg_mngr.h"
//
#include "combat_action_bleed.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "combat_phrase.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionBleed::apply(CCombatPhrase *phrase)
{
	H_AUTO(CCombatActionBleed_apply);

	if (!phrase || !_BleedDuration)
		return;

	const std::vector<CCombatPhrase::TTargetInfos> & targets = phrase->getTargets();
	for (uint i = 0; i < targets.size() ; ++i)
	{
//		if ( !phrase->hasTargetDodged(i) )
		if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			applyOnTarget(i,phrase);
	}
} // apply //

//--------------------------------------------------------------
//					applyOnTarget()  
//--------------------------------------------------------------
void CCombatActionBleed::applyOnTarget(uint8 targetIndex, CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif

	const CCombatDefenderPtr &targetDefender = phrase->getTarget(targetIndex);
	if(!targetDefender) return;

	// get damage already inflicted and compute bleed damage
	_TotalBleedDamage = sint32(phrase->getInflictedNaturalDamage(targetIndex) * getApplyValue(phrase->weaponSabrinaValue()));
	// damage per cycle
	_CycleDamage = float(_TotalBleedDamage * ( _BleedCycleDuration / float(_BleedDuration)));

	if(_CycleDamage == 0.0f) return;

	CEntityBase *entity = targetDefender->getEntity();
	if (!entity)
	{
		nlwarning("COMBAT : <CCombatActionBleed::applyOnTarget> Cannot find the target entity, cancel");
		return;
	}

//	applyOnEntity(entity, phrase->getPhraseSuccessDamageFactor());
	applyOnEntity(entity, 1.0f-phrase->getTargetDodgeFactor(targetIndex));

} // applyOnTarget //


//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionBleed::applyOnEntity( CEntityBase *entity, float )
{
	if (!entity) return;

	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	TGameCycle endDate = _BleedDuration + CTickEventHandler::getGameCycle();

	bool createNewEffect = true;
	// check if entity is already bleeding, is this is the case check the bleeding effect to get same actor and same cycle length
	const vector<CSEffectPtr> &effects = entity->getSEffects();
	for ( uint i = 0 ; i < effects.size() ; ++i )
	{
		if ( effects[i] != NULL && effects[i]->getFamily() == EFFECT_FAMILIES::CombatBleed && effects[i]->getCreatorRowId() == _ActorRowId )
		{
			CBleedEffect *bleedEffect = dynamic_cast<CBleedEffect *> (& (*effects[i]));
			if ( bleedEffect && _BleedCycleDuration == bleedEffect->getCycleLength() )
			{
				bleedEffect->stackWithEffect(_CycleDamage, endDate);
				createNewEffect = false;
				break;
			}
		}
	}

	if (createNewEffect)
	{
		CBleedEffect *bleedEffect = new CBleedEffect( _ActorRowId, entity->getEntityRowId(), EFFECT_FAMILIES::CombatBleed, uint8(_CycleDamage)/*_BleedPower*/, endDate, _BleedCycleDuration, _CycleDamage);
		if (!bleedEffect)
		{
			nlwarning("COMBAT : <CCombatActionBleed::applyOnTarget> Failed to allocate new CBleedEffect object !");
			return;
		}

		bleedEffect->bleedingEntity(entity);
		entity->addSabrinaEffect(bleedEffect);

		// send messages
		// to target
		if (entity->getId().getType() == RYZOMID::player)
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), "EFFECT_BLEED_BEGIN");
		}
		// to actor
		if ( _ActorRowId != entity->getEntityRowId() && TheDataset.isAccessible(_ActorRowId))
		{
			CCharacter *actor = PlayerManager.getChar(_ActorRowId);
			if (actor != NULL)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
				params[0].setEIdAIAlias( entity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(entity->getId()) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "EFFECT_BLEED_BEGIN_ACTOR", params);
			}
		}
		///  todo : send to spectators
	}

} // applyOnEntity //
