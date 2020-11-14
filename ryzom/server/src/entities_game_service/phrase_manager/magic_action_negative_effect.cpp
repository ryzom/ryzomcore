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
#include "magic_action.h"
#include "phrase_manager/magic_phrase.h"
#include "creature_manager/creature.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "game_share/magic_fx.h"
#include "ai_share/ai_event_report.h"
#include "phrase_manager/s_link_effect.h"
#include "progression/progression_pvp.h"

#include "entity_manager/entity_manager.h"
#include "entity_structure/statistic.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;


class CMagicActionNegativeEffect : public IMagicAction
{
public:
	CMagicActionNegativeEffect()
		:_EffectValue(0),_EffectFamily(EFFECT_FAMILIES::Unknown),_CostPerUpdate(0),_Power(0){}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
		float		ResistFactor;
		bool		Immune;
		uint		FlagIndex;
	};

protected:
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams )
	{
#ifdef NL_DEBUG
		nlassert(phrase);
#endif
		for ( uint i=0 ; i<brick.Params.size() ; ++i)
		{
			const TBrickParam::IId* param = brick.Params[i];

			switch(param->id())
			{
			case TBrickParam::MA_END:
				INFOLOG("MA_END Found: end of effect");
				effectEnd = true;
				return true;
			case TBrickParam::MA_EFFECT:
				INFOLOG("MA_EFFECT: %s",((CSBrickParamMagicEffect *)param)->Effect.c_str());
				_EffectFamily = EFFECT_FAMILIES::toEffectFamily( ((CSBrickParamMagicEffect *)param)->Effect );
				if ( _EffectFamily == EFFECT_FAMILIES::Unknown )
				{
					nlwarning("<CMagicActionNegativeEffect addBrick> invalid effect type %s", ((CSBrickParamMagicEffect *)param)->Effect.c_str());
					return false;
				}
				// set main phrase effect id
				phrase->setMagicFxType(MAGICFX::toMagicFx(_EffectFamily ), brick.SabrinaValue);
				// set action sheetid
				_ActionBrickSheetId = brick.SheetId;
				break;
			case TBrickParam::MA_EFFECT_MOD:
				INFOLOG("MA_EFFECT_MOD: %u",((CSBrickParamMagicEffectMod *)param)->EffectMod);
				_EffectValue = ((CSBrickParamMagicEffectMod *)param)->EffectMod;
				break;
				
			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)param)->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)param)->Cost;
				break;

			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)param)->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)param)->Power );
				break;
				
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( param, brick, buildParams );
				break;
			}
		}
		///\todo nico: check if everything is set
		return true;
	}
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode)
	{
#ifdef NL_DEBUG
		nlassert(phrase);
#endif
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, true ) )
		{
//			PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
			return false;
		}
		return true;
	}

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
	{
		H_AUTO(CMagicActionNegativeEffect_launch);

		if ( successFactor <= 0.0f )
			return;

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		// if it is a "degradable" spell, i.e. a spell with a value, it is successful if successFactor > 0, otherwise, successFactor must be >=1)
		
		// apply success factor
		_EffectValue = uint32( _EffectValue *  successFactor);

		// apply used item power factor on cost per update
		_CostPerUpdate = uint( _CostPerUpdate / (1.0 + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost())) );
		
		//const std::vector< TDataSetRow > & targets = phrase->getTargets();
		const std::vector< CSpellTarget > & targets = phrase->getTargets();

		sint skillValue = 0;
		if ( actor->getId().getType() == RYZOMID::player )
		{
			CCharacter * pC = (CCharacter *) actor;
			skillValue = pC->getSkillValue( _Skill );
		}
		else
		{
			const CStaticCreatures * form = actor->getForm();
			if ( !form )
			{
				nlwarning( "<MAGIC>invalid creature form %s in entity %s", actor->getType().toString().c_str(), actor->getId().toString().c_str() );
				return;
			}	
			skillValue = form->getAttackLevel();
		}
		
		const CSEffect * debuff = actor->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );		
		if ( debuff )
			skillValue -= debuff->getParamValue();

		const CSEffect * outPostBuff = actor->lookForActiveEffect( EFFECT_FAMILIES::OutpostMagic );
		if ( outPostBuff )
			skillValue += outPostBuff->getParamValue();

		// cap skill values with brick power and phrase multi target power factor
		if ( skillValue > (sint32)_Power )
			skillValue = (sint32)_Power;

		resists.clearAll();
		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;

			string errorCode;
			if ( !isMad && !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, i==0)  )
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}
			affectedTargets.set(i);

			// apply skill modifier accroding to target race
			sint32 localSkillValue = skillValue + actor->getSkillModifierForRace(target->getRace());

			if ( actor->getId().getType() == RYZOMID::player )
			{
				// boost magic skill for low level chars
				sint32 sb = (sint32)MagicSkillStartValue.get();
				localSkillValue = max( sb, localSkillValue );

				// add magic boost from consumable
				CCharacter * pC = dynamic_cast<CCharacter *>(actor);
				if(pC)
					localSkillValue += pC->magicSuccessModifier();
			}

			CTargetInfos targetInfos;
			targetInfos.RowId			= target->getEntityRowId();
			targetInfos.MainTarget		= (i == 0);
			targetInfos.Immune			= false;
			targetInfos.ResistFactor	= 1.0f;
			targetInfos.FlagIndex		= i;

			float & resistFactor = targetInfos.ResistFactor;

			// test if target really affected
			if (invulnerabilityAll[i] || invulnerabilityOffensive[i])
			{
				resistFactor = 0.0f;
				resists.set(i);

				_ApplyTargets.push_back(targetInfos);
				continue;
			}

			// test resistance
			if (!EntitiesNoResist)
			{
				uint32 resistValue = target->getMagicResistance(_EffectFamily);

				if (resistValue == CCreatureResists::ImmuneScore )
				{
					targetInfos.Immune = true;
					resistFactor = 0.0f;
				}
				else
				{
					// get the chances
					const uint8 roll = (uint8)RandomGenerator.rand( 99 );
					resistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistLink, localSkillValue - resistValue, roll);

					// increase resists modifier for next resists test
					if (targetInfos.ResistFactor > 0.0f)
						target->incResistModifier(_EffectFamily, targetInfos.ResistFactor);
				}
			}

			if (resistFactor <= 0.0f)
			{
				resists.set(i);
			}

			_ApplyTargets.push_back(targetInfos);
		}
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{
		H_AUTO(CMagicActionNegativeEffect_apply);

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		const uint nbTargets = (uint)_ApplyTargets.size();
		for ( uint i = 0; i < nbTargets; i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( _ApplyTargets[i].RowId );
			if ( !target)
				continue;

			TReportAction reportAction = actionReport;

			reportAction.TargetRowId = _ApplyTargets[i].RowId;
			reportAction.DeltaLvl = deltaLevel;
			reportAction.Skill = _Skill;
			if (!reportXp)
			{
				reportAction.Skill = SKILLS::unknown;						// no xp gain but damage must be registered
				reportAction.SkillLevel = phrase->getBrickMaxSabrinaCost();	// use the real level of the enchantment
			}

			if( target->getId().getType() != RYZOMID::player )
			{
				const CStaticCreatures * creatureSheet = target->getForm();
				if( creatureSheet != 0 )
				{
					reportAction.DeltaLvl = skillLevel - creatureSheet->getXPLevel();
				}
			}

			string errorCode;
			if ( !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget) )
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

			bool sendAggro = false;

			// test if target really affected
			const uint & fi = _ApplyTargets[i].FlagIndex;
			if (invulnerabilityAll[fi] || invulnerabilityOffensive[fi])
			{
				PHRASE_UTILITIES::sendSpellResistMessages( actor->getEntityRowId(), target->getEntityRowId());
				sendAggro = true;
			}
			else
			{
				// check resistance
				const float & resistFactor = _ApplyTargets[i].ResistFactor;

				if (_ApplyTargets[i].Immune)
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
					params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_TARGET_IMMUNE", params);
				}
				else if ( resistFactor <= 0.0f )
				{
					PHRASE_UTILITIES::sendSpellResistMessages( actor->getEntityRowId(), target->getEntityRowId());
				}

				if (resistFactor < 1.0f)
				{
					sendAggro = true;
				}

				if (resistFactor > 0.0f)
				{
					CCreature * npc = dynamic_cast<CCreature*>(target);
					CCharacter * c = dynamic_cast<CCharacter*>(actor);
					if(npc && c && !PHRASE_UTILITIES::testRange(*actor, *target, (uint32)npc->getMaxHitRangeForPC()*1000) )
					{
						c->sendDynamicSystemMessage(c->getId(), "UNEFFICENT_RANGE");
						sendAggro = false;
						behav.DeltaHP = 0;
					}
					else
					{
						CSLinkEffectOffensive* effect = new CSLinkEffectOffensive(
							phrase->getActor(),
							_ApplyTargets[i].RowId,
							_EffectFamily,
							_CostPerUpdate,
							SCORES::sap,//linkEnergy,
							_Skill,
							phrase->getSpellRange(),
							_EffectValue,
							_Power,
							reportAction);

						if (!effect)
						{
							nlwarning("Failed to allocate new CSLinkEffectOffensive");
							return;
						}
						
						effect->setSpellPower(phrase->getBrickMaxSabrinaCost());
						effect->setPhraseBookIndex(phrase->phraseBookIndex());

						if (phrase->breakNewLink())
						{
							// use CEntityBase methods not to cancel the current action
							actor->CEntityBase::addLink(effect);
							target->addSabrinaEffect(effect);
							actor->CEntityBase::removeLink(effect, resistFactor);
							PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
							PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
							sendAggro = true;
						}
						else
						{
							actor->addLink( effect );
							target->addSabrinaEffect( effect );

							// if partial resist, break link
							if (resistFactor < 1.0f)
							{
								PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
								PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
								actor->stopAllLinks(resistFactor);
							}
						}
					}
				}
			}

			if (sendAggro)
			{
				CAiEventReport report;
				report.Originator = phrase->getActor();
				report.Target = _ApplyTargets[i].RowId;
				report.Type = ACTNATURE::OFFENSIVE_MAGIC;
				// TEMPORARY : set a small value for resist aggro
				report.AggroAdd = -0.02f;
				CPhraseManager::getInstance().addAiEventReport(report);
			}
		}
	}

	EFFECT_FAMILIES::TEffectFamily	_EffectFamily;
	sint32							_EffectValue;	
	uint							_CostPerUpdate;
	uint8							_Power;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>		_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionNegativeEffect)
	ADD_MAGIC_ACTION_TYPE( "mdal" )
	ADD_MAGIC_ACTION_TYPE( "moal" )
END_MAGIC_ACTION_FACTORY(CMagicActionNegativeEffect)


