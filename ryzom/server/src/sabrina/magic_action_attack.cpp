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
#include "phrase_manager.h"

using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;

std::vector< std::pair< std::string , IMagicActionFactory* > >* IMagicActionFactory::Factories = NULL;


class CMagicActionBasicDamage : public IMagicAction
{
public:
	CMagicActionBasicDamage()
		:_DmgHp(0),_DmgSap(0),_DmgSta(0),_DmgType(DMGTYPE::UNDEFINED){}
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
				
			case TBrickParam::MA_DMG_TYPE:
				INFOLOG("MA_DMG_TYPE: %s",((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType.c_str());
				_DmgType = DMGTYPE::stringToDamageType( ((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType );
				if ( _DmgType == DMGTYPE::UNDEFINED )
				{
					nlwarning("<CMagicActionBasicDamage addBrick> invalid dmg type %s", ((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType.c_str());
					return false;
				}
				break;

			case TBrickParam::MA_DMG:
				INFOLOG("MA_DMG: %u %u %u",((CSBrickParamMagicDmg *)brick.Params[i])->Hp,((CSBrickParamMagicDmg *)brick.Params[i])->Sap,((CSBrickParamMagicDmg *)brick.Params[i])->Sta);
				_DmgHp = ((CSBrickParamMagicDmg *)brick.Params[i])->Hp;
				_DmgSap = ((CSBrickParamMagicDmg *)brick.Params[i])->Sap;
				_DmgSta = ((CSBrickParamMagicDmg *)brick.Params[i])->Sta;
				break;
				
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
		return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::OFFENSIVE);
	}
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav , bool isMad )
	{
		///\todo nico:
		//		- location
		//		- armor + shield
		//		- player damages + on armor
		//		- behaviour + chat messages
		//		- aggro
		
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

		/// test resistance
		if ( successFactor <= 0.0f )
		{
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		
		const std::vector< TDataSetRow > & targets = phrase->getTargets();
		
		SSkill * skillAtt = actor->getSkills().getSkillStruct( _Skill );
		if ( ! skillAtt )
		{
			nlwarning("<CMagicActionBasicDamage apply> %s is not a valid skill",SKILLS::toString(_Skill).c_str());
			return;
		}
		
		const CSEffect * debuff = actor->lookForSEffect( EFFECT_FAMILIES::DebuffSkillMagic );
		sint skillValue = skillAtt->Current;
		if ( debuff )
			skillValue -= debuff->getParamValue();

		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i] );
			if ( !target)
				continue;
			if ( isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE) )
			{	
				///\NOT USED NOW
				//behav.Magic.SpellPower = PHRASE_UTILITIES::getAttackIntensity( phrase->getSabrinaCost() );

				
				// test resistance
/*				SSkill * skillResist = target->getSkills().getSkillStruct( SKILLS::MagicDefense ); //TODO skill missing
				if ( ! skillResist )
				{
					nlwarning("<CMagicActionBasicDamage apply> MagicDefense is not a valid skill");
					return;
				}
*/				// get the chances ( delta level is divided by 10 because a level is 10
				const uint8 chances = PHRASE_UTILITIES::getSuccessChance( ( skillValue /*- skillResist->Current*/ )/10 ); //TODO skillResist
				const uint8 roll = (uint8)RandomGenerator.rand( 99 );
				float resistFactor = PHRASE_UTILITIES::getSucessFactor(chances, roll);

				if ( resistFactor > 0.0f  )
				{
					if ( resistFactor > 1.0f )
						resistFactor = 1.0f;
					//behav.Spell.Resist = 0;
					behav.Spell.SpellId =  MAGICFX::toMagicFx( _DmgType ,false);

					float mult = resistFactor;
					const CSEffect * effect = target->lookForSEffect( EFFECT_FAMILIES::MagicDmgAmpli );
					if ( effect )
						mult *=  ( effect->getParamValue() / 100.0f );

					sint32 realDmgHp =  sint32 ( _DmgHp * mult );
					realDmgHp = sint32( target->applyDamageOnArmor( _DmgType, realDmgHp ) );
					if ( target->changeCurrentHp( - realDmgHp ) )
					{
						// send mission event
						if ( actor->getId().getType()== RYZOMID::player )
						{
							CMissionEventKill event ( target->getEntityRowId() );
							((CCharacter*) actor)->processMissionEvent( event );
						}
					}
					if ( target->getScores()._PhysicalScores[SCORES::hit_points].Current <= 0)
					{
						target->getScores()._PhysicalScores[SCORES::hit_points].Current = 0;
						//behav.Spell.KillingBlow = 1;
					}
					PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realDmgHp ,SCORES::hit_points , ACTNATURE::OFFENSIVE);

					sint32 realDmgSap;
					{
						RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::sap];
						realDmgSap = sint32( _DmgSap * mult );
						realDmgSap = target->applyDamageOnArmor( _DmgType, realDmgSap );
						score.Current = score.Current - realDmgSap;
						if ( score.Current < 0)
							score.Current = 0;
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realDmgSap ,SCORES::sap , ACTNATURE::OFFENSIVE);
					}

					sint32 realDmgSta;
					{
						RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
						realDmgSta =  sint32( _DmgSta * mult );
						realDmgSta = target->applyDamageOnArmor( _DmgType, realDmgSta );
						score.Current = score.Current - realDmgSta;
						if ( score.Current < 0)
							score.Current = 0;
						PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor, target, realDmgSta ,SCORES::stamina , ACTNATURE::OFFENSIVE);
					}

					///\todo nico: real value
					behav.Spell.SpellIntensity = 5;
					
					// compute aggro
					sint32 max = target->getPhysScores()._PhysicalScores[SCORES::hit_points].Max;
					if (max)
					{
						const sint32 aggro = (-1) * sint32((100.0 * float(realDmgHp))/float(max) );
						// update the report
						CAiEventReport report;
						report.AggroMul = 1.0f;
						report.AggroAdd = aggro;
						report.addDelta(AI_EVENT_REPORT::HitPoints, (-1)*realDmgHp);
						CPhraseManager::getInstance()->addAiEventReport(report);
					}

					max = target->getPhysScores()._PhysicalScores[SCORES::sap].Max;
					if (max)
					{
						const sint32 aggro = (-1) * sint32((100.0 * float(realDmgSap))/float(max) );
						// update the report
						CAiEventReport report;
						report.AggroMul = 1.0f;
						report.AggroAdd = aggro;
						report.addDelta(AI_EVENT_REPORT::Sap, (-1)*realDmgSap);
						CPhraseManager::getInstance()->addAiEventReport(report);
					}

					max = target->getPhysScores()._PhysicalScores[SCORES::stamina].Max;
					if (max)
					{
						const sint32 aggro = (-1) * sint32((100.0 * float(realDmgSta))/float(max) );
						// update the report
						CAiEventReport report;
						report.AggroMul = 1.0f;
						report.AggroAdd = aggro;
						report.addDelta(AI_EVENT_REPORT::Stamina, (-1)*realDmgSta);
						CPhraseManager::getInstance()->addAiEventReport(report);
					}	
				}
				else
				{
					if ( actor->getId().getType() == RYZOMID::player )
						CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_RESIST" );
					if ( target->getId().getType() == RYZOMID::player )
						CCharacter::sendMessageToClient( target->getId(),"MAGIC_U_TOTAL_RESIST" );
					///\todo nico msgs
					//behav.Spell.Resist = 1;
				}
				/// compute resist XP gain
/*				if ( target->getId().getType() == RYZOMID::player && resistFactor < 1.0f)
				{
					///\todo nico resistFactor  is the quality factor of the action for xp
					((CCharacter*) target)->actionReport( actor, (skillResist->Current - skillValue)/10, ACTNATURE::DEFENSIVE, SKILLS::toString( SKILLS::MagicDefense ) );
				}
*/			}
		}
	}
	DMGTYPE::EDamageType	_DmgType;
	sint32					_DmgHp;
	sint32					_DmgSap;
	sint32					_DmgSta;
};
BEGIN_MAGIC_ACTION_FACTORY(CMagicActionBasicDamage)
	ADD_MAGIC_ACTION_TYPE( "mto" )	
END_MAGIC_ACTION_FACTORY(CMagicActionBasicDamage)

