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
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_structure/statistic.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "egs_sheets/egs_static_ai_action.h"
#include "progression/progression_pvp.h"
//
#include "game_share/magic_fx.h"
#include "ai_share/ai_event_report.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;

extern CCreatureManager CreatureManager;


class CMagicActionBasicHeal : public IMagicAction
{
public:
	CMagicActionBasicHeal()
		:_HealHp(0),_HealSap(0),_HealSta(0){}

	/// build from AI Action
	bool initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
	{
#ifdef NL_DEBUG
		nlassert(phrase);
		nlassert(aiAction);
#endif
		if (aiAction->getType() != AI_ACTION::HealSpell ) return false;

		sint32 healBonus = 0;
		CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
		if (creature && creature->getForm())
			healBonus = sint32(aiAction->getData().Spell.SpellPowerFactor * creature->getForm()->getAttackLevel());

		switch(aiAction->getData().Spell.AffectedScore)
		{
		case SCORES::sap:
			_HealSap = sint32(aiAction->getData().Spell.SpellParamValue + healBonus);
			break;
		case SCORES::stamina:
			_HealSta = sint32(aiAction->getData().Spell.SpellParamValue + healBonus);
			break;
		case SCORES::hit_points:
			_HealHp = sint32(aiAction->getData().Spell.SpellParamValue + healBonus);
			break;
		default:
			return false;
		};

		phrase->setMagicFxType( MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,false ), 3);

		return true;
	}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
		sint32		HealHp;
		sint32		HealSap;
		sint32		HealSta;
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
			case TBrickParam::MA_HEAL:
				INFOLOG("MA_HEAL: %u %u %u",((CSBrickParamMagicHeal *)param)->Hp,((CSBrickParamMagicHeal *)param)->Sap,((CSBrickParamMagicHeal *)param)->Sta);
				_HealHp = ((CSBrickParamMagicHeal *)param)->Hp;
				_HealSap = ((CSBrickParamMagicHeal *)param)->Sap;
				_HealSta = ((CSBrickParamMagicHeal *)param)->Sta;
				phrase->setMagicFxType( MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,false ), brick.SabrinaValue);
				
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( brick.Params[i], brick, buildParams );
				break;
			}
		}
		///\todo nico: check if everything is set
		return true;
	}
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode)
	{
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::CURATIVE_MAGIC, errorCode, true) )
		{
			return false;
		}

		return true;
	}

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
	{
		///\todo nico:
		//		- behaviour + messages de chat
		//		- aggro

		// Critical Fail
		if(successFactor <= 0.0f)
			return;
		// Get Spell Caster
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		// Get Spell Targets
		const std::vector< CSpellTarget > & targets = phrase->getTargets();
		const uint nbTargets = (uint)targets.size();
		
		// apply power factor os used item
		successFactor *= (1 + phrase->getUsedItemStats().getPowerFactor(_Skill, phrase->getBrickMaxSabrinaCost()) );

		resists.clearAll();
		for ( uint i = 0 ; i < nbTargets ; ++i )
		{
			///\todo nico : healing a bad guy is PVP, but it should be possible to heal escort NPCS
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;

			string errorCode;
			if (!isMad && !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, i==0))
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}
			affectedTargets.set(i);

			// test if target really affected
			if (invulnerabilityAll[i])
			{
				resists.set(i);
				continue;
			}

			CTargetInfos targetInfos;
			targetInfos.RowId		= target->getEntityRowId();
			targetInfos.MainTarget	= (i == 0);
			targetInfos.HealHp		= 0;
			targetInfos.HealSap		= 0;
			targetInfos.HealSta		= 0;

			float factor = successFactor * powerFactors[i] * phrase->getAreaSpellPowerFactor();
			if ( _HealHp != 0)
			{
				targetInfos.HealHp = sint32(_HealHp * factor);

				// update behaviour for healed Hp
				behav.DeltaHP += sint16(targetInfos.HealHp);
			}
			if (_HealSap != 0)
			{
				targetInfos.HealSap = sint32(_HealSap * factor);
			}
			if ( _HealSta != 0 )
			{
				targetInfos.HealSta = sint32(_HealSta * factor);
			}

			_ApplyTargets.push_back(targetInfos);
		}
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{
		CEntityBase * actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		const uint nbTargets = (uint)_ApplyTargets.size();

		vector<TReportAction> actionReports;
		if (reportXp)
			actionReports.reserve(nbTargets);

		for (uint i = 0; i < nbTargets; i++)
		{
			CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _ApplyTargets[i].RowId );

			if ( !target)
				continue;

			// an area heal cannot affect caster !
			if ( target->getId().getType() == RYZOMID::player && target == actor)
				continue;

			string errorCode;
			if (!PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget))
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

			TReportAction reportAction = actionReport;
			reportAction.TargetRowId = _ApplyTargets[i].RowId;
			reportAction.DeltaLvl = deltaLevel;
			reportAction.Skill = _Skill;
			if (!reportXp)
			{
				reportAction.Skill = SKILLS::unknown;						// no xp gain but damage must be registered
				reportAction.SkillLevel = phrase->getBrickMaxSabrinaCost();	// use the real level of the enchantment
			}

			CAiEventReport aiReport;
			aiReport.Originator = phrase->getActor();
			aiReport.Target = _ApplyTargets[i].RowId;
			aiReport.Type = ACTNATURE::CURATIVE_MAGIC;

			if ( _HealHp != 0)
			{
				sint32 & realHealHp = _ApplyTargets[i].HealHp;
				// clip heal hp
				if (realHealHp + target->currentHp() > target->maxHp())
				{
					realHealHp = target->maxHp() - target->currentHp();
				}

				target->changeCurrentHp(realHealHp);
				if ( target->currentHp() >= target->maxHp() )
				{
					if ( realHealHp > 0)
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealHp, realHealHp, SCORES::hit_points , ACTNATURE::CURATIVE_MAGIC);
					reportAction.Hp = realHealHp;

					if (actor != target)
					{
						SM_STATIC_PARAMS_2(params2, STRING_MANAGER::entity, STRING_MANAGER::score);
						params2[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId()) );
						params2[1].Enum = SCORES::hit_points;
						PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_ACTOR", params2);
					}

					SM_STATIC_PARAMS_1(params, STRING_MANAGER::score);
					params[0].Enum = SCORES::hit_points;
					PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_TARGET", params);
				}
				else
				{
					PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealHp, realHealHp, SCORES::hit_points , ACTNATURE::CURATIVE_MAGIC);
					reportAction.Hp = realHealHp;
				}

				// update the report
				if (target->maxHp())
				{
					float aggro = float(realHealHp)/float(target->maxHp());
					aiReport.AggroAdd = aggro;
					aiReport.addDelta(AI_EVENT_REPORT::HitPoints, realHealHp);
					CPhraseManager::getInstance().addAiEventReport(aiReport);
				}
			}

			if (_HealSap != 0)
			{
				sint32 & realHealSap = _ApplyTargets[i].HealSap;
				SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::sap];
				score.Current = score.Current + realHealSap;
				if ( score.Current >= score.Max )
				{
					realHealSap += score.Max - score.Current;
					if ( realHealSap > 0)
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealSap, realHealSap, SCORES::sap , ACTNATURE::CURATIVE_MAGIC);
					score.Current = score.Max;
					reportAction.Sap = realHealSap;

					if ( actor != target)
					{
						SM_STATIC_PARAMS_2(params2, STRING_MANAGER::entity, STRING_MANAGER::score);
						params2[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId()) );
						params2[1].Enum = SCORES::sap;
						PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_ACTOR", params2);
					}

					SM_STATIC_PARAMS_1(params, STRING_MANAGER::score);
					params[0].Enum = SCORES::sap;
					PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_TARGET", params);
				}
				else
				{
					PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealSap, realHealSap, SCORES::sap , ACTNATURE::CURATIVE_MAGIC);
					reportAction.Sap = realHealSap;
				}

				// update the report
				float aggro = float(realHealSap)/float(score.Max);
				aiReport.AggroAdd = aggro;
				aiReport.addDelta(AI_EVENT_REPORT::Sap, realHealSap);
				CPhraseManager::getInstance().addAiEventReport(aiReport);
			}

			if ( _HealSta != 0 )
			{
				sint32 & realHealSta = _ApplyTargets[i].HealSta;
				SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
				score.Current = score.Current + realHealSta;
				if ( score.Current >= score.Max )
				{
					realHealSta += score.Max - score.Current;
					if ( realHealSta > 0)
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealSta, realHealSta, SCORES::stamina , ACTNATURE::CURATIVE_MAGIC);
					score.Current = score.Max;
					reportAction.Sta = realHealSta;

					if (actor != target)
					{
						SM_STATIC_PARAMS_2(params2, STRING_MANAGER::entity, STRING_MANAGER::score);												
						params2[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId() ) );
						params2[1].Enum = SCORES::stamina;
						PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_ACTOR", params2);
					}

					SM_STATIC_PARAMS_1(params, STRING_MANAGER::score);
					params[0].Enum = SCORES::stamina;
					PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_TARGET", params);
				}
				else
				{
					PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), target->getId(), realHealSta, realHealSta, SCORES::stamina , ACTNATURE::CURATIVE_MAGIC);
					reportAction.Sta = realHealSta;
				}

				// update the report
				float aggro = float(realHealSta)/float(score.Max);
				aiReport.AggroAdd = aggro;
				aiReport.addDelta(AI_EVENT_REPORT::Stamina, realHealSta);
				CPhraseManager::getInstance().addAiEventReport(aiReport);
			}

			if ( reportXp )
				actionReports.push_back(reportAction);
		}

		// report action and xp
		if (reportXp)
		{
			// send all reports
			for (uint i = 0 ; i < actionReports.size() ; ++i)
			{
				// only send report if healing succesful
				if (actionReports[i].Hp != 0 || actionReports[i].Sap != 0 || actionReports[i].Sta != 0 || actionReports[i].Focus != 0)
				{
					PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport(actionReports[i], (i==0));
					PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(actionReports[i]);
				}
			}
		}
	}

	sint32						_HealHp;
	sint32						_HealSap;
	sint32						_HealSta;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>	_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionBasicHeal)
	ADD_MAGIC_ACTION_TYPE( "mdht" )	
END_MAGIC_ACTION_FACTORY(CMagicActionBasicHeal)

CMagicAiActionTFactory<CMagicActionBasicHeal> *CMagicActionHealAiFactoryInstance = new CMagicAiActionTFactory<CMagicActionBasicHeal>(AI_ACTION::HealSpell);
