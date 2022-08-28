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
#include "game_share/entity_structure/statistic.h"
#include "s_link_effect_debuff_skill.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;


class CMagicActionDebuffSkill : public IMagicAction
{
public:
	CMagicActionDebuffSkill()
		:_TargetSkill(SKILLS::unknown),_DebuffValue(0){}
protected:
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, CBuildParameters &buildParams )
	{
		for ( uint i=0 ; i<brick.Params.size() ; ++i)
		{
			switch(brick.Params[i]->id())
			{	
				
			case TBrickParam::MA_DEBUFF:
				INFOLOG("MA_DEBUFF: %u",((CSBrickParamMagicDebuff *)brick.Params[i])->Debuff);
				_DebuffValue = ((CSBrickParamMagicDebuff *)brick.Params[i])->Debuff;
				break;
				
			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost;
				break;
				
			case TBrickParam::SKILL:
				{
					INFOLOG("SKILL: %s",((CSBrickParamSkill *)brick.Params[i])->Skill.c_str());
					_Skill = SKILLS::toSkill( ((CSBrickParamSkill *)brick.Params[i])->Skill );
					if ( _Skill == SKILLS::unknown )
					{
						nlwarning( "<CMagicActionDebuffSkill addBrick> invalid skill type %s", ((CSBrickParamSkill *)brick.Params[i])->Skill.c_str() );
						return false;
					}
					break;
				}
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( brick.Params[i], brick, buildParams );
				break;
			}
		}
		///\todo nico: check if everything is set
		return true;
	}
	virtual bool validate(CMagicPhrase * phrase)
	{
		string errorCode;
		if (!PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::OFFENSIVE, errorCode))
		{
			PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
			return false;
		}
		return true;
	}
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav )
	{
		NL_ALLOC_CONTEXT(MADSAPY);
		///\todo nico:
		//		- location
		//		- armor + shield
		//		- player damages + on armor
		//		- behaviour + chat messages
		//		- aggro
		
		/// test resistance
		if ( successFactor < 0.0f )
		{
			///\todo nico
			return;
		}
		
		/// apply success factor
		_DebuffValue = uint32( _DebuffValue *  successFactor);
		
		const std::vector< TDataSetRow > & targets = phrase->getTargets();
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;

//		SCORES::EScores linkEnergy;
//		if ( phrase->getSapCost() > 0 )
//		{
//			linkEnergy = SCORES::sap;
//		}
//		else
//			linkEnergy = SCORES::hit_points;;
		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;
			
			string errorCode;
			if ( PHRASE_UTILITIES::validateSpellTarget(actor,target,ACTNATURE::OFFENSIVE, errorCode, i==0) )
			{
				CSLinkEffectDebuffSkill* debuff = new CSLinkEffectDebuffSkill( phrase->getActor(),
					targets[i].getId(),
					_UpdatePeriod,
					_CostPerUpdate,
					SCORES::sap,
					_Skill,
					_TargetSkill,
					_DebuffValue  );

				if (phrase->breakNewLink())
				{
					// use CEntityBase methods not to cancel the current action
					actor->CEntityBase::addLink(effect);
					target->addSabrinaEffect(effect);
					actor->CEntityBase::removeLink(effect);
					return;
				}

				actor->addLink( debuff );
				target->addSabrinaEffect( debuff );
			}
			else
			{
				PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				return;
			}
		}
	}
	
	SKILLS::ESkills			_TargetSkill;
	sint32					_DebuffValue;
	uint					_CostPerUpdate;
};
MAGIC_ACTION_FACTORY( CMagicActionDebuffSkill,BRICK_FAMILIES::MADebuffSkill )


