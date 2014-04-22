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
#include "entity_structure/statistic.h"
#include "entity_manager/entity_manager.h"
#include "s_link_effect_dot.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/phrase_manager.h"
#include "progression/progression_pvp.h"

#include "game_share/magic_fx.h"
#include "ai_share/ai_event_report.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;


class CMagicActionDot : public IMagicAction
{
public:
	CMagicActionDot()
		:_DmgHp(0),_DmgSap(0),_DmgSta(0),_DmgType(DMGTYPE::UNDEFINED),_CostPerUpdate(0),_Power(0),_Vampirise(0),_VampiriseRatio(1) {}


	/// build from an ai action
	bool initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
	{
#ifdef NL_DEBUG
		nlassert(phrase);
		nlassert(aiAction);
#endif		
/*		if (aiAction->getType() != AI_ACTION::DoTSpell ) return false;
		
		switch(aiAction->getData().LinkSpell.AffectedScore)
		{
		case SCORES::sap:
			_DmgSap = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		case SCORES::stamina:
			_DmgSta = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		case SCORES::hit_points:
			_DmgHp = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		default:
			return false;
		};

		_DmgType = aiAction->getData().LinkSpell.DamageType;
		
		_CostPerUpdate = max(aiAction->getData().LinkSpell.SapCostRate, aiAction->getData().LinkSpell.HpCostRate);
		_Power = (uint8) fabs(aiAction->getData().LinkSpell.SpellParamValue);

		phrase->setMagicFxType( MAGICFX::toMagicFx( _DmgType ,true), 1 );
*/
		return true;
	}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
		float		DmgFactor;
		float		ResistFactor;
		bool		Immune;
	};

protected:
	/// add brick
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
			case TBrickParam::MA_DMG_TYPE:
				INFOLOG("MA_DMG_TYPE: %s",((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
				_DmgType = DMGTYPE::stringToDamageType( ((CSBrickParamMagicDmgType *)param)->DmgType );
				if ( _DmgType == DMGTYPE::UNDEFINED )
				{
					nlwarning("<CMagicActionBasicDamage addBrick> invalid dmg type %s", ((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
					return false;
				}
				// set main phrase spellId
				phrase->setMagicFxType( MAGICFX::toMagicFx( _DmgType ,true), brick.SabrinaValue );
				// set action sheetid
				_ActionBrickSheetId = brick.SheetId;
				break;

			case TBrickParam::MA_DMG:
				INFOLOG("MA_DMG: %u %u %u",((CSBrickParamMagicDmg *)param)->Hp,((CSBrickParamMagicDmg *)param)->Sap,((CSBrickParamMagicDmg *)param)->Sta);
				_DmgHp = ((CSBrickParamMagicDmg *)param)->Hp;
				_DmgSap = ((CSBrickParamMagicDmg *)param)->Sap;
				_DmgSta = ((CSBrickParamMagicDmg *)param)->Sta;
				break;

			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)param)->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)param)->Cost;
				break;

			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)param)->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)param)->Power );
				break;

			case TBrickParam::MA_VAMPIRISE:
				INFOLOG("MA_VAMPIRISE: %u",((CSBrickParamMagicVampirise *)param)->Vampirise );
				_Vampirise = ((CSBrickParamMagicVampirise *)param)->Vampirise;
				break;
				
			case TBrickParam::MA_VAMPIRISE_RATIO:
				INFOLOG("MA_VAMPIRISE_RATIO: %f",((CSBrickParamMagicVampiriseRatio *)param)->VampiriseRatio );
				_VampiriseRatio = ((CSBrickParamMagicVampiriseRatio *)param)->VampiriseRatio;
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
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::OFFENSIVE_MAGIC,errorCode, true) )
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
		H_AUTO(CMagicActionDot_launch);

		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;
		
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		
		///\todo nico:
		//		- location
		//		- armor + shield
		//		- player damages + on armor
		//		- behaviour + chat messages
		//		- aggro

		if ( successFactor <= 0.0f )
		{
//			if ( actor->getId().getType() == RYZOMID::player )
//				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		
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

		// apply used item power factor on cost per update
//		_CostPerUpdate = sint32( _CostPerUpdate / (1.0 + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost())) );
		
		const CSEffectPtr debuff = actor->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );		
		if ( debuff )
			skillValue -= debuff->getParamValue();
		const CSEffect * outPostBuff = actor->lookForActiveEffect( EFFECT_FAMILIES::OutpostMagic );
		if ( outPostBuff )
			skillValue += outPostBuff->getParamValue();

		// cap skillValue with link power
		if (skillValue > (sint32)_Power )
			skillValue = (sint32)_Power;

		resists.clearAll();
		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;
			
			string errorCode;
			if ( !isMad && !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, i==0 ) )
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
			targetInfos.DmgFactor		= 0.0f;

			float & resistFactor = targetInfos.ResistFactor;

			// test if target really affected
			if (invulnerabilityAll[i] || invulnerabilityOffensive[i])
			{
				resistFactor = 0.0f;
			}
			// test resistance
			else if (!EntitiesNoResist)
			{
				uint32 resistValue = target->getMagicResistance(_DmgType);

				if (resistValue == CCreatureResists::ImmuneScore)
				{
					resistFactor = 0.0f;
					targetInfos.Immune = true;
				}
				else
				{
					// get the chances
					const uint8 roll = (uint8)RandomGenerator.rand( 99 );
					resistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistDirect, localSkillValue - resistValue, roll);
					
					// increase resists modifier for next resists test
					if (targetInfos.ResistFactor > 1.0f)
						target->incResistModifier(_DmgType, targetInfos.ResistFactor);
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
			else
			{
				resists.clear(i);
				if ( resistFactor > 1.0f )
					resistFactor = 1.0f;

				targetInfos.DmgFactor = float(resistFactor * successFactor * CSLinkEffect::getUpdatePeriod(EFFECT_FAMILIES::Dot) * CTickEventHandler::getGameTimeStep() );

				// if PVP, apply PVP magic damage factor
				if( actor != target && actor->getId().getType() == RYZOMID::player && target->getId().getType() == RYZOMID::player)
				{
					targetInfos.DmgFactor *= PVPMagicDamageFactor.get();
				}
			}

			_ApplyTargets.push_back(targetInfos);
		}
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{

		H_AUTO(CMagicActionDot_apply);

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		// apply used item power factor on cost per update
//		_CostPerUpdate = sint32( _CostPerUpdate / (1.0 + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost())) );
		
		const uint nbTargets = (uint)_ApplyTargets.size();
		for ( uint i = 0; i < nbTargets; i++ )
		{
			TReportAction reportAction = actionReport;

			reportAction.TargetRowId = _ApplyTargets[i].RowId;
			reportAction.DeltaLvl = deltaLevel;
			reportAction.Skill = _Skill;
			if (!reportXp)
			{
				reportAction.Skill = SKILLS::unknown;						// no xp gain but damage must be registered
				reportAction.SkillLevel = phrase->getBrickMaxSabrinaCost();	// use the real level of the enchantment
			}
				
			// check target
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
			
			string errorCode;
			if ( !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget ) )
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

			bool sendAggro = false;

			// check resist and immunity
			if (_ApplyTargets[i].Immune)
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
				params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
				PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_TARGET_IMMUNE", params);
			}
			else if (_ApplyTargets[i].ResistFactor <= 0.0f)
			{
				PHRASE_UTILITIES::sendSpellResistMessages( actor->getEntityRowId(), target->getEntityRowId());
			}

			if (_ApplyTargets[i].ResistFactor < 1.0f)
			{
				sendAggro = true;
			}

			if (_ApplyTargets[i].ResistFactor > 0.0f)
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
					reportAction.ActionNature = ACTNATURE::OFFENSIVE_MAGIC;

					const float & factor = _ApplyTargets[i].DmgFactor;
					CSLinkEffectDot* dot = new CSLinkEffectDot(
						phrase->getActor(),
						_ApplyTargets[i].RowId,
						_CostPerUpdate,
						SCORES::sap,
						_Skill,
						phrase->getSpellRange(),
						_DmgType,
						_Power,
						reportAction,
						sint32(_DmgHp * factor),
						sint32(_DmgSap * factor),
						sint32(_DmgSta * factor),
						_Vampirise,
						_VampiriseRatio);
					
					if (!dot)
					{
						nlwarning("Failed to allocate new CSLinkEffectDot");
						return;
					}
					
					dot->setPhraseBookIndex(phrase->phraseBookIndex());
					dot->setSpellPower(phrase->getBrickMaxSabrinaCost());

					if (phrase->breakNewLink())
					{
						// use CEntityBase methods not to cancel the current action
						actor->CEntityBase::addLink(dot);
						target->addSabrinaEffect(dot);
						actor->CEntityBase::removeLink(dot,_ApplyTargets[i].ResistFactor);
						PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
						PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
						sendAggro = true;
					}
					else
					{
						actor->addLink( dot );
						target->addSabrinaEffect( dot );

						// if partial resist, break link
						if (_ApplyTargets[i].ResistFactor < 1.0f)
						{
							PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( reportAction );
							PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(reportAction);
							actor->stopAllLinks(_ApplyTargets[i].ResistFactor);
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

	DMGTYPE::EDamageType		_DmgType;
	sint32						_DmgHp;
	sint32						_DmgSap;
	sint32						_DmgSta;
	sint32						_CostPerUpdate;
	uint8						_Power;
	sint32						_Vampirise;
	float						_VampiriseRatio;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>	_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionDot)
	ADD_MAGIC_ACTION_TYPE( "moel" )		
END_MAGIC_ACTION_FACTORY(CMagicActionDot)

