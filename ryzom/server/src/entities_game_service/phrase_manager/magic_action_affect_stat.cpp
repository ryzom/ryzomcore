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
#include "s_link_effect_debuff_stat.h"
#include "entity_structure/statistic.h"
#include "entity_manager/entity_manager.h"
#include "game_share/magic_fx.h"
#include "ai_share/ai_event_report.h"
#include "progression/progression_pvp.h"


using namespace NLNET;
using namespace NLMISC;
using namespace std;

NL_INSTANCE_COUNTER_IMPL(IMagicAiActionFactory);

/// affect a stat.
/// Put an update period of 0 ( updtae each tick )
class CMagicActionAffectStat : public IMagicAction
{
public:
	CMagicActionAffectStat()
		:_Modifier(0),_Multiplier(0),_Stat(0),_Type(STAT_TYPES::Unknown),_CostPerUpdate(0),_EffectFamily(EFFECT_FAMILIES::Unknown),_Power(0),_IsOffensive(true)
	{}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
		bool		Immune;
		float		ResistFactor;
	};

protected:
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams )
	{
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
				// set the main phrase effect ID
				phrase->setMagicFxType(MAGICFX::toMagicFx( _EffectFamily ), brick.SabrinaValue);
				// set action sheetid
				_ActionBrickSheetId = brick.SheetId;
				break;
			case TBrickParam::MA_EFFECT_MOD:
				INFOLOG("MA_EFFECT_MOD: %u",((CSBrickParamMagicEffectMod *)param)->EffectMod);
				_Modifier = ((CSBrickParamMagicEffectMod *)param)->EffectMod;
				break;

			case TBrickParam::MA_EFFECT_MULT:
				INFOLOG("MA_EFFECT_MULT: %f",((CSBrickParamMagicEffectMult *)param)->EffectMult);
				_Multiplier = ((CSBrickParamMagicEffectMult *)param)->EffectMult;
				break;
				
			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)param)->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)param)->Cost;
				break;
				
			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)param)->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)param)->Power );
				break;
			
			case TBrickParam::MA_STAT:
			{
				INFOLOG("MA_STAT: type %s",((CSBrickParamMagicStat *)param)->Type.c_str());
				INFOLOG("MA_STAT: stat %s",((CSBrickParamMagicStat *)param)->Stat.c_str());
				_Type = STAT_TYPES::toStatType( ((CSBrickParamMagicStat *)param)->Type );
				const std::string & value = ((CSBrickParamMagicStat *)param)->Stat;
				switch( _Type )
				{
				case STAT_TYPES::Skill:
					_Stat = (uint) SKILLS::toSkill(value);
					if ( _Stat == (uint) SKILLS::unknown )
					{
						nlwarning("<CMagicActionAffectStat addBrick> invalid skill %s", value.c_str() );
						return false;
					}
					break;
				case STAT_TYPES::Score:
					_Stat = (uint) SCORES::toScore(value);
					if ( _Stat == (uint) SCORES::unknown )
					{
						nlwarning("<CMagicActionAffectStat addBrick> invalid score %s", value.c_str() );
						return false;
					}
					break;
				case STAT_TYPES::Speed:
					_Stat = 0;
					break;
				default:
					nlwarning("<CMagicActionAffectStat addBrick> invalid stat type %s", ((CSBrickParamMagicStat *)param)->Type.c_str() );
					return false;
				}
				
				break;
			}
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
		if ( _Modifier < 0 || _Multiplier < 1.0f )
		{
			_IsOffensive = true;
		}
		
		if (_IsOffensive)
		{
			if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, true) )
			{
				return false;
			}
			return true;
		}
		else if( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::CURATIVE_MAGIC, errorCode, true) )
		{
			return false;
		}

		return true;
	}

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
	{
		if ( successFactor <= 0.0f )
			return;

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		
		/// apply success factor
		_Modifier = uint32( _Modifier *  successFactor);
		_Multiplier *= successFactor;
	
		// apply used item power factor on cost per update
		//_CostPerUpdate = uint( _CostPerUpdate / (1.0 + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost())) );

		// compute caster level
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

		if ( actor->getId().getType() == RYZOMID::player )
		{
			// boost magic skill for low level chars
			sint sb = (sint)MagicSkillStartValue.get();
			skillValue = max( sb, skillValue );

			// add magic boost from consumable
			CCharacter * pC = dynamic_cast<CCharacter *>(actor);
			if(pC)
				skillValue += pC->magicSuccessModifier();
		}
				
		const std::vector< CSpellTarget > & targets = phrase->getTargets();

		resists.clearAll();
		
		const ACTNATURE::TActionNature nature = (_IsOffensive?ACTNATURE::OFFENSIVE_MAGIC:ACTNATURE::CURATIVE_MAGIC);

		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;

			string errorCode;
			if ( !isMad && ! PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),nature, errorCode, i==0) )
			{
				// dont warn because of multi target
				//PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				affectedTargets.clear(i);
				continue;
			}

			affectedTargets.set(i);

			CTargetInfos targetInfos;
			targetInfos.RowId		= target->getEntityRowId();
			targetInfos.MainTarget	= (i == 0);
			targetInfos.Immune		= false;
			targetInfos.ResistFactor= 1.0f;
			
			float & resistFactor = targetInfos.ResistFactor;
			
			// test if target really affected
			if ( invulnerabilityAll[i] || (invulnerabilityOffensive[i] && _IsOffensive ) )
			{
				resistFactor = 0.0f;
			}
			// test resistance
			else if (!EntitiesNoResist)
			{
				uint32 resistValue = target->getMagicResistance(_EffectFamily);
				
				if (resistValue == CCreatureResists::ImmuneScore)
				{
					resistFactor = 0.0f;
					targetInfos.Immune = true;
				}
				else
				{
					// get the chances
					const uint8 roll = (uint8)RandomGenerator.rand( 99 );
					resistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistLink, skillValue - resistValue, roll);
					
					// increase resists modifier for next resists test
					if (resistFactor > 0.0f)
						target->incResistModifier(_EffectFamily, resistFactor);
				}
			}
			else
			{
				resistFactor = 1.0f;
			}
			
			if (resistFactor <= 0.0f)
			{
				resists.set(i);
			}
			
			if ( resistFactor > 1.0f )
				resistFactor = 1.0f;
					
			_ApplyTargets.push_back(targetInfos);
		}
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{
		if (!phrase)
			return;

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		TReportAction reportAction = actionReport;
		reportAction.ActionNature = (_IsOffensive?ACTNATURE::OFFENSIVE_MAGIC:ACTNATURE::CURATIVE_MAGIC);

		const uint nbTargets = (uint)_ApplyTargets.size();
		for ( uint i = 0; i < nbTargets; i++ )
		{
			reportAction.TargetRowId = _ApplyTargets[i].RowId;
			reportAction.DeltaLvl = deltaLevel;
			reportAction.Skill = _Skill;
			if (!reportXp)
			{
				reportAction.Skill = SKILLS::unknown;						// no xp gain but damage must be registered
				reportAction.SkillLevel = phrase->getBrickMaxSabrinaCost();	// use the real level of the enchantment
			}

			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( _ApplyTargets[i].RowId );
			if ( !target)
				continue;

			if( target->getId().getType() != RYZOMID::player )
			{
				const CStaticCreatures * creatureSheet = target->getForm();
				if( creatureSheet != NULL )
				{
					reportAction.DeltaLvl = skillLevel - creatureSheet->getXPLevel();
				}
			}
			
			// check target
			string errorCode;
			if ( ! PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget) )
			{
				// dont warn because of multi target
				//PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

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

			bool sendAggro = false;
			
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
					if ( _Modifier < 0 || _Multiplier < 1.0f )
					{
						CSLinkEffectDebuffStat* effect = new CSLinkEffectDebuffStat(
							phrase->getActor(),
							_ApplyTargets[i].RowId,
							_EffectFamily,
							_CostPerUpdate,
							SCORES::sap,
							_Skill,
							phrase->getSpellRange(),
							_Modifier,
							_Multiplier,
							_Type,
							_Stat,
							_Power,
							reportAction);

						if (!effect)
						{
							nlwarning("Failed to allocate new CSLinkEffectDebuffStat");
							return;
						}
						
						effect->setPhraseBookIndex(phrase->phraseBookIndex());
						effect->setSpellPower(phrase->getBrickMaxSabrinaCost());

						if (phrase->breakNewLink())
						{
							// use CEntityBase methods not to cancel the current action
							actor->CEntityBase::addLink(effect);
							target->addSabrinaEffect(effect);
							actor->CEntityBase::removeLink(effect, _ApplyTargets[i].ResistFactor);
							sendAggro = true;

							PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
							PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
						}
						else
						{
							actor->addLink( effect );
							target->addSabrinaEffect( effect );

							// if partial resist, break link but gain XP
							if (_ApplyTargets[i].ResistFactor < 1.0f)
							{
								PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
								PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
								actor->stopAllLinks(_ApplyTargets[i].ResistFactor);
							}
						}
					}
					else
					{
						///\todo nico : buff...
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

	sint32							_Modifier;
	float							_Multiplier;
	uint8							_Stat;
	STAT_TYPES::TStatType			_Type;
	uint8							_Power;
	bool							_IsOffensive;
	
	sint32							_CostPerUpdate;
	EFFECT_FAMILIES::TEffectFamily	_EffectFamily;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>		_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionAffectStat)
	ADD_MAGIC_ACTION_TYPE( "mlosmp" )
END_MAGIC_ACTION_FACTORY(CMagicActionAffectStat)

