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
#include "magic_phrase.h"
#include "creature.h"
#include "character.h"
#include "phrase_utilities_functions.h"
#include "game_share/entity_structure/statistic.h"
#include "game_share/magic_fx.h"
#include "s_effect.h"

using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;



class CMagicActionBasicHeal : public IMagicAction
{
public:
	CMagicActionBasicHeal()
		:_HealHp(0),_HealSap(0),_HealSta(0){}
protected:
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd )
	{
		for ( uint i=0 ; i<brick.Params.size() ; ++i)
		{
			switch(brick.Params[i]->id())
			{
			case TBrickParam::MA_END:
				INFOLOG("MA_END Found: end of effect");
				effectEnd = true;
				return true;
			case TBrickParam::MA_HEAL:
				INFOLOG("MA_HEAL: %u %u %u",((CSBrickParamMagicHeal *)brick.Params[i])->Hp,((CSBrickParamMagicHeal *)brick.Params[i])->Sap,((CSBrickParamMagicHeal *)brick.Params[i])->Sta);
				_HealHp = ((CSBrickParamMagicHeal *)brick.Params[i])->Hp;
				_HealSap = ((CSBrickParamMagicHeal *)brick.Params[i])->Sap;
				_HealSta = ((CSBrickParamMagicHeal *)brick.Params[i])->Sta;
				
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( brick.Params[i] );
				break;
			}
		}
		///\todo nico: check if everything is set
		return true;
	}
	virtual bool validate(CMagicPhrase * phrase)
	{
		return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::DEFENSIVE);
	}
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav , bool isMad )
	{
		///\todo nico:
		//		- behaviour + messages de chat
		//		- aggro

		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		if ( successFactor <= 0.0f )
		{
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
		}
		const std::vector< TDataSetRow > & targets = phrase->getTargets();

		/// compute XP gain
		SSkill * skillCast = actor->getSkills().getSkillStruct( _Skill );
		if ( ! skillCast )
		{
			nlwarning("<CMagicActionHeal apply> %s is not a valid skill",SKILLS::toString(_Skill).c_str());
			return;
		}
		const CSEffect * debuff = actor->lookForSEffect( EFFECT_FAMILIES::DebuffSkillMagic );
		sint skillValue = skillCast->Current;
		if ( debuff )
			skillValue -= debuff->getParamValue();

		if ( actor->getId().getType() == RYZOMID::player )
		{
			///\todo nico successFactor is the quality factor of the action for xp
			((CCharacter*) actor)->actionReport( NULL, ( skillValue - phrase->getSabrinaCost() )/10, ACTNATURE::DEFENSIVE, SKILLS::toString( _Skill ) );
		}	
		for ( uint i = 0; i < targets.size(); i++ )
		{
			///\todo nico : healing a bad guy is PVP, but it should be possible to heal escort NPCS
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i] );
			if ( !target)
				continue;

			if ( isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::DEFENSIVE) )
			{		
				//behav.Magic.SpellPower = PHRASE_UTILITIES::getAttackIntensity( phrase->getSabrinaCost() );
				//behav.Spell.Resist = 0;
				//behav.Spell.KillingBlow = 0;
			
				///
				sint32 realHealHp = 0;
				{
					RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::hit_points];
					score.Current = score.Current + sint32(_HealHp * successFactor);
					if ( score.Current >= score.Max )
					{
						realHealHp = sint32(_HealHp * successFactor) + score.Max - score.Current;
						if ( realHealHp > 0)
							PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target,  realHealHp,SCORES::hit_points , ACTNATURE::DEFENSIVE);
						score.Current = score.Max;
					}
					else
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realHealHp ,SCORES::hit_points , ACTNATURE::DEFENSIVE);
				}
				sint32 realHealSap = 0;
				{
					RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::sap];
					score.Current = score.Current + sint32(_HealSap * successFactor);
					if ( score.Current >= score.Max )
					{
						realHealSap = sint32(_HealSap * successFactor) + score.Max - score.Current;
						if ( realHealSap > 0)
							PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target,  realHealSap,SCORES::sap , ACTNATURE::DEFENSIVE);
						score.Current = score.Max;
					}
					else
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realHealSap ,SCORES::sap , ACTNATURE::DEFENSIVE);
				}

				sint32 realHealSta = 0;
				{
					RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
					score.Current = score.Current + sint32(_HealSta * successFactor);
					if ( score.Current >= score.Max )
					{
						realHealSta = sint32(_HealSta * successFactor) + score.Max - score.Current;
						if ( realHealSta > 0)
							PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target,  realHealSta,SCORES::stamina , ACTNATURE::DEFENSIVE);
						score.Current = score.Max;
					}
					else
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realHealSta ,SCORES::stamina , ACTNATURE::DEFENSIVE);
				}
				/*behav.Magic.ImpactIntensity = PHRASE_UTILITIES::getImpactIntensity( realHealHp ,targets[i],SCORES::hit_points);
				behav.Magic.ImpactIntensity += PHRASE_UTILITIES::getImpactIntensity( realHealSap ,targets[i],SCORES::sap);
				behav.Magic.ImpactIntensity = PHRASE_UTILITIES::getImpactIntensity( realHealSta ,targets[i],SCORES::stamina);
				*/
				behav.Spell.SpellIntensity = 5;
				behav.Spell.SpellId =  MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,false );
			}
		}
	}
	sint32					_HealHp;
	sint32					_HealSap;
	sint32					_HealSta;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionBasicHeal)
	ADD_MAGIC_ACTION_TYPE( "mtch" )	
END_MAGIC_ACTION_FACTORY(CMagicActionBasicHeal)
