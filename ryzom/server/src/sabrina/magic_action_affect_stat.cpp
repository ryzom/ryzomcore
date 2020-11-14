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
#include "s_link_effect_debuff_stat.h"

using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;


/// affect a stat.
/// Put an update period of 0 ( updtae each tick )
class CMagicActionAffectStat : public IMagicAction
{
public:
	CMagicActionAffectStat()
		:_Modifier(0),_Multiplier(0),_Stat(0),_Type(STAT_TYPES::Unknown),_CostPerUpdate(0),_UpdatePeriod(0),_EffectFamily(EFFECT_FAMILIES::Unknown),_Power(0){}
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

			case TBrickParam::MA_EFFECT:
				INFOLOG("MA_EFFECT: %s",((CSBrickParamMagicEffect *)brick.Params[i])->Effect.c_str());
				_EffectFamily = EFFECT_FAMILIES::toEffectFamily( ((CSBrickParamMagicEffect *)brick.Params[i])->Effect );
				if ( _EffectFamily == EFFECT_FAMILIES::Unknown )
				{
					nlwarning("<CMagicActionNegativeEffect addBrick> invalid effect type %s", ((CSBrickParamMagicEffect *)brick.Params[i])->Effect.c_str());
					return false;
				}
				break;
			case TBrickParam::MA_EFFECT_MOD:
				INFOLOG("MA_EFFECT_MOD: %u",((CSBrickParamMagicEffectMod *)brick.Params[i])->EffectMod);
				_Modifier = ((CSBrickParamMagicEffectMod *)brick.Params[i])->EffectMod;
				break;

			case TBrickParam::MA_EFFECT_MULT:
				INFOLOG("MA_EFFECT_MULT: %f",((CSBrickParamMagicEffectMult *)brick.Params[i])->EffectMult);
				_Multiplier = ((CSBrickParamMagicEffectMult *)brick.Params[i])->EffectMult;
				break;
				
			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost;
				break;
				
			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)brick.Params[i])->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)brick.Params[i])->Power );
				break;
			
			case TBrickParam::MA_STAT:
			{
				INFOLOG("MA_STAT: type %s",((CSBrickParamMagicStat *)brick.Params[i])->Type.c_str());
				INFOLOG("MA_STAT: stat %s",((CSBrickParamMagicStat *)brick.Params[i])->Stat.c_str());
				_Type = STAT_TYPES::toStatType( ((CSBrickParamMagicStat *)brick.Params[i])->Type );
				const std::string & value = ((CSBrickParamMagicStat *)brick.Params[i])->Stat;
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
					nlwarning("<CMagicActionAffectStat addBrick> invalid stat type %s", ((CSBrickParamMagicStat *)brick.Params[i])->Type.c_str() );
					return false;
				}
				
				break;
			}
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
		if ( _Modifier < 0 || _Multiplier < 1.0f )
			return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::OFFENSIVE);
		return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::DEFENSIVE);
	}

	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav, bool isMad  )
	{		
		behav.Spell.SpellMode = 0;
		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;
		behav.Spell.SpellIntensity = 5;
		
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		if ( successFactor <= 0.0f )
		{
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		
		/// apply success factor
		_Modifier = uint32( _Modifier *  successFactor);
		_Multiplier *= successFactor;
		
		const std::vector< TDataSetRow > & targets = phrase->getTargets();

		SCORES::TScores linkEnergy;
		if ( phrase->getHPCost() > 0 )
		{
			linkEnergy = SCORES::hit_points;
		}
		else
			linkEnergy = SCORES::sap;

		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i] );
			if ( !target)
				continue;

			/// someone can only have 1 effect of a given type on a creature
			const std::vector< CSEffect* > & effects = target->getSEffects();
			for (uint j = 0; j < effects.size(); j++ )
			{
				if ( effects[j]->getFamily() == _EffectFamily && effects[j]->getCreatorRowId() == phrase->getActor( ) )
				{
					return;
				}
			}		
			
			if ( isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::DEFENSIVE) )
			{
				if ( _Modifier < 0 || _Multiplier < 1.0f )
				{
					CSLinkEffectDebuffStat* effect = new CSLinkEffectDebuffStat(
						phrase->getActor(),
						targets[i],
						_EffectFamily,
						_UpdatePeriod,
						linkEnergy,
						_Skill,
						_Modifier,
						_Multiplier,
						_Type,
						_Stat,
						_Power);

					actor->addLink( effect );
					target->addSabrinaEffect( effect );
					behav.Spell.SpellId =  MAGICFX::toMagicFx( _EffectFamily );
				}
				else
				{
					///\todo nico : buff...
				}
			}
		}
	}
	sint32							_Modifier;
	float							_Multiplier;
	uint8							_Stat;
	STAT_TYPES::TStatType			_Type;
	uint8							_Power;

	
	NLMISC::TGameCycle				_UpdatePeriod;
	sint32							_CostPerUpdate;
	EFFECT_FAMILIES::TEffectFamily	_EffectFamily;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionAffectStat)
	ADD_MAGIC_ACTION_TYPE( "mlosmp" )
END_MAGIC_ACTION_FACTORY(CMagicActionAffectStat)






















