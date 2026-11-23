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
#include "game_share/magic_fx.h"
#include "s_link_effect_hot.h"
#include "entity_structure/statistic.h"
#include "entity_manager/entity_manager.h"
#include "egs_sheets/egs_static_ai_action.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;


class CMagicActionHot : public IMagicAction
{
public:
	CMagicActionHot()
		:_HealHp(0),_HealSap(0),_HealSta(0),_CostPerUpdate(0),_Power(0){}

	/// build from an ai action
	virtual bool initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
	{
#ifdef NL_DEBUG
		nlassert(phrase);
		nlassert(aiAction);
#endif		
/*		if (aiAction->getType() != AI_ACTION::LinkHeal ) return false;
		
		switch(aiAction->getData().LinkSpell.AffectedScore)
		{
		case SCORES::sap:
			_HealSap = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		case SCORES::stamina:
			_HealSta = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		case SCORES::hit_points:
			_HealHp = sint32(aiAction->getData().LinkSpell.SpellParamValue);
			break;
		default:
			return false;
		};
		
		_CostPerUpdate = max(aiAction->getData().LinkSpell.SapCostRate, aiAction->getData().LinkSpell.HpCostRate);
		_Power = (uint8)aiAction->getData().LinkSpell.SpellParamValue;

		// set main phrase effect Id 
		phrase->setMagicFxType( MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true ),1 );
*/
		return true;
	}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
		float		HealFactor;
	};

protected:
	/// add brick
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams ) NL_OVERRIDE
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
				
			case TBrickParam::MA_HEAL:
				INFOLOG("MA_HEAL: %d %d %d", ((CSBrickParamMagicHeal *)param)->Hp, ((CSBrickParamMagicHeal *)param)->Sap, ((CSBrickParamMagicHeal *)param)->Sta);
				_HealHp = ((CSBrickParamMagicHeal *)param)->Hp;
				_HealSap = ((CSBrickParamMagicHeal *)param)->Sap;
				_HealSta = ((CSBrickParamMagicHeal *)param)->Sta;
				phrase->setMagicFxType( MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true ), brick.SabrinaValue);
				// set action sheetid
				_ActionBrickSheetId = brick.SheetId;
				break;

			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %d", ((CSBrickParamMagicLinkCost *)param)->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)param)->Cost;
				break;

			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)param)->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)param)->Power );
				break;
				
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( brick.Params[i], brick, buildParams );
				break;
			}
		}
		///\todo nico: check if everything is set
		return true;
	}
	
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode) NL_OVERRIDE
	{
#ifdef NL_DEBUG
		nlassert(phrase);
#endif
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(),ACTNATURE::CURATIVE_MAGIC, errorCode, true) )
		{
//			PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
			return false;
		}

		// set main phrase effect Id 
		phrase->setMagicFxType( MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true ), 1 );

		return true;
	}

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, std::vector<sint16> &targetDeltaHp, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport ) NL_OVERRIDE
	{
		if ( successFactor <= 0.0f )
		{
//			if ( actor->getId().getType() == RYZOMID::player )
//				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;		

		/// apply success factor
		
		const std::vector< CSpellTarget > & targets = phrase->getTargets();

		// targetDeltaHp must be prefilled
		nlassertex( targets.size() == targetDeltaHp.size(), ("%d %d", targets.size(), targetDeltaHp.size() ) );

		resists.clearAll();
		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;

			string errorCode;
			if (!isMad &&!PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, i==0 ))
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

//			behav.Spell.SpellId =  MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true );

			CTargetInfos targetInfos;
			targetInfos.RowId		= target->getEntityRowId();
			targetInfos.MainTarget	= (i == 0);
			// MA_HEAL gives the heal per second, as MA_DMG does for the damage over time links (CMagicActionDot)
			targetInfos.HealFactor	= float(successFactor * CSLinkEffect::getUpdatePeriod(EFFECT_FAMILIES::Hot) * CTickEventHandler::getGameTimeStep());

			_ApplyTargets.push_back(targetInfos);
		}
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp ) NL_OVERRIDE
	{
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		TReportAction reportAction = actionReport;
		reportAction.DeltaLvl = deltaLevel;
		if (!reportXp)
		{
			// no xp gain, as for the damage over time links (CMagicActionDot)
			reportAction.Skill = SKILLS::unknown;
			reportAction.SkillLevel = phrase->getBrickMaxSabrinaCost();
		}

		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;

//		SCORES::TScores linkEnergy;
//		if ( phrase->getHPCost() > 0 )
//			linkEnergy = SCORES::hit_points;
//		else
//			linkEnergy = SCORES::sap;

		const uint nbTargets = _ApplyTargets.size();
		for ( uint i = 0; i < nbTargets; i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( _ApplyTargets[i].RowId );
			if ( !target)
				continue;

			string errorCode;
			if (!PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget ))
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

			// a caster keeps one heal over time on a target: a new one replaces the caster's last one there, whose
			// link the new cast broke and which would otherwise heal on until its survival time ends
			removeHotFrom(target, actor);

			reportAction.TargetRowId = _ApplyTargets[i].RowId;

			const float & factor = _ApplyTargets[i].HealFactor;

			CSLinkEffectHot* hot = new CSLinkEffectHot(
				phrase->getActor(),
				_ApplyTargets[i].RowId,
				_CostPerUpdate,
				SCORES::sap,
				_Skill,
				phrase->getSpellRange(),
				_Power,
				reportAction,
				uint32(_HealHp * factor),
				uint32(_HealSap * factor),
				uint32(_HealSta * factor ));

			// the link slot of the database names the phrase, and focus items scale the cost by the spell's level
			hot->setPhraseBookIndex(phrase->phraseBookIndex());
			hot->setSpellPower(phrase->getBrickMaxSabrinaCost());

			if (phrase->breakNewLink())
			{
				// use CEntityBase methods not to cancel the current action
				actor->CEntityBase::addLink(hot);
				target->addSabrinaEffect(hot);
				actor->CEntityBase::removeLink(hot, 1.0f);
				continue;
			}

			// the links of the other targets join the static action of the first one
			phrase->addLink(actor, hot);
			target->addSabrinaEffect( hot );
		}
	}

	/// Removes the heal over time caster has on target, if any (one at most)
	static void removeHotFrom(CEntityBase *target, CEntityBase *caster)
	{
		const std::vector<CSEffectPtr> &effects = target->getSEffects();
		for (uint j = 0; j < effects.size(); ++j)
		{
			CSEffect *effect = effects[j];
			if (effect && effect->getFamily() == EFFECT_FAMILIES::Hot && effect->getCreatorRowId() == caster->getEntityRowId())
			{
				// the caster's side of the link first: the target ends a link only for its active effect, and the
				// caster's may be a sleeping one behind a stronger heal of another caster (CEntityBase methods, not to
				// cancel the current action)
				CSLinkEffect *link = dynamic_cast<CSLinkEffect *>(effect);
				if (link)
					caster->CEntityBase::removeLink(link, 0.0f);
				// removing changes the list: stop there
				target->removeSabrinaEffect(effect);
				return;
			}
		}
	}

	sint32						_HealHp;
	sint32						_HealSap;
	sint32						_HealSta;
	uint						_CostPerUpdate;
	uint8						_Power;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>	_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionHot)
	ADD_MAGIC_ACTION_TYPE( "mlc" )	
END_MAGIC_ACTION_FACTORY(CMagicActionHot)

//CMagicAiActionTFactory<CMagicActionHot> *CMagicActionHotAiFactoryInstance = new CMagicAiActionTFactory<CMagicActionHot>(AI_ACTION::LinkHeal);
