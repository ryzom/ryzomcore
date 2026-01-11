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
#include "game_share/entity_structure/statistic.h"
#include "game_share/magic_fx.h"
#include "s_link_effect_dot.h"
#include "phrase_utilities_functions.h"

using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;



class CMagicActionDot : public IMagicAction
{
public:
	CMagicActionDot()
		:_DmgHp(0),_DmgSap(0),_DmgSta(0),_DmgType(DMGTYPE::UNDEFINED),_CostPerUpdate(0),_Power(0) {}
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
		return PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0],ACTNATURE::OFFENSIVE);
	}
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav , bool isMad )
	{
		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;
		behav.Spell.SpellIntensity = 5;
		
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
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		
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

			const std::vector< CSEffect* > & effects = target->getSEffects();
			for (uint j = 0; j < effects.size(); j++ )
			{
				if ( effects[j]->getFamily() == EFFECT_FAMILIES::Dot && effects[j]->getCreatorRowId() == phrase->getActor( ) )
				{
					return;
				}
			}	

			if ( isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::OFFENSIVE) )
			{

				CSLinkEffectDot* dot = new CSLinkEffectDot( phrase->getActor(),
																  targets[i],
																  _CostPerUpdate,
																  linkEnergy,
																   _Skill,
																   _DmgType,
																   _Power,
																   sint32(_DmgHp * successFactor* CSLinkEffect::getUpdatePeriod()),
																  sint32(_DmgSap * successFactor* CSLinkEffect::getUpdatePeriod()),
																  sint32(_DmgSta * successFactor* CSLinkEffect::getUpdatePeriod()) );
				actor->addLink( dot );
				target->addSabrinaEffect( dot );
				behav.Spell.SpellId =  MAGICFX::toMagicFx( _DmgType ,false);
			}
		}
	}
	DMGTYPE::EDamageType	_DmgType;
	sint32					_DmgHp;
	sint32					_DmgSap;
	sint32					_DmgSta;
	sint32					_CostPerUpdate;
	uint8					_Power;
};
BEGIN_MAGIC_ACTION_FACTORY(CMagicActionDot)
	ADD_MAGIC_ACTION_TYPE( "mlod" )	
	ADD_MAGIC_ACTION_TYPE( "mloe" )	
END_MAGIC_ACTION_FACTORY(CMagicActionDot)

