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
#include "s_link_effect_hot.h"

using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;



class CMagicActionHot : public IMagicAction
{
public:
	CMagicActionHot()
		:_HealHp(0),_HealSap(0),_HealSta(0),_CostPerUpdate(0),_Power(0){}
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
				
			case TBrickParam::MA_LINK_COST:
				INFOLOG("MA_LINK_COST: %u",((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost);
				_CostPerUpdate = ((CSBrickParamMagicLinkCost *)brick.Params[i])->Cost;
				break;

			case TBrickParam::MA_LINK_POWER:
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)brick.Params[i])->Power);
				_Power = (uint8) ( ((CSBrickParamMagicLinkPower *)brick.Params[i])->Power );
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
		return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::DEFENSIVE);
	}
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav , bool isMad )
	{
		// behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;
		behav.Spell.SpellIntensity = 5;
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if (!actor)
			return;
		if ( successFactor <= 0.0f )
		{
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
		}
		
		/// apply success factor
		
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
				if ( effects[j]->getFamily() == EFFECT_FAMILIES::Hot && effects[j]->getCreatorRowId() == phrase->getActor( ) )
				{
					return;
				}
			}		
			if (  isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::DEFENSIVE) )
			{
				CSLinkEffectHot* hot = new CSLinkEffectHot( phrase->getActor(),
					targets[i],
					_CostPerUpdate,
					linkEnergy,
					_Skill,
					_Power,
					uint32(_HealHp * successFactor* CSLinkEffect::getUpdatePeriod()),
					uint32(_HealSap * successFactor* CSLinkEffect::getUpdatePeriod()),
					uint32(_HealSta * successFactor* CSLinkEffect::getUpdatePeriod()) );
				
				actor->addLink( hot );
				target->addSabrinaEffect( hot );


				behav.Spell.SpellId =  MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true );
			}
		}
	}
	sint32					_HealHp;
	sint32					_HealSap;
	sint32					_HealSta;
	uint					_CostPerUpdate;
	uint8					_Power;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionHot)
	ADD_MAGIC_ACTION_TYPE( "mlc" )	
END_MAGIC_ACTION_FACTORY(CMagicActionHot)
