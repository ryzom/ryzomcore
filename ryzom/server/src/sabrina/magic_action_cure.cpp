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
#include "character.h"
#include "game_share/effect_families.h"
#include "game_share/magic_fx.h"
#include "s_link_effect_dot.h"
#include "phrase_utilities_functions.h"


using namespace NLNET;
using namespace NLMISC;
using namespace RY_GAME_SHARE;
using namespace std;



class CMagicActionCure : public IMagicAction
{
public:
	CMagicActionCure()
		:_Power(0){}
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
				{
					INFOLOG("MA_EFFECT: %s",((CSBrickParamMagicEffect *)brick.Params[i])->Effect.c_str());
					EFFECT_FAMILIES::TEffectFamily effectFamily = EFFECT_FAMILIES::toEffectFamily( ((CSBrickParamMagicEffect *)brick.Params[i])->Effect );
					if ( effectFamily == EFFECT_FAMILIES::Unknown )
					{
						nlwarning("<CMagicActionCure addBrick> invalid effect type %s", ((CSBrickParamMagicEffect *)brick.Params[i])->Effect.c_str());
						return false;
					}
					_AffectedEffects.push_back(effectFamily);
					break;
				}
			case TBrickParam::MA_DMG_TYPE:
				{
					INFOLOG("MA_DMG_TYPE: %s",((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType.c_str());
					DMGTYPE::EDamageType dmgType = DMGTYPE::stringToDamageType( ((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType );
					if ( dmgType == DMGTYPE::UNDEFINED )
					{
						nlwarning("<CMagicActionCure addBrick> invalid dmg type %s", ((CSBrickParamMagicDmgType *)brick.Params[i])->DmgType.c_str());
						return false;
					}
					_AffectedDots.push_back(dmgType);
					break;
				}
			case TBrickParam::MA_LINK_POWER:	
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)brick.Params[i])->Power);
				_Power = uint16 ( ((CSBrickParamMagicLinkPower *)brick.Params[i])->Power );
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
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if ( !actor)
		{
			nlwarning("<CMagicActionCure apply> invalid actor: %u", phrase->getActor().getIndex() );
			return;
		}
		if ( successFactor < 1.0f )
		{
			if ( actor->getId().getType() == RYZOMID::player )
				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		const std::vector< TDataSetRow > & targets = phrase->getTargets();
		for ( uint i = 0; i < targets.size(); i++ )
		{
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i] );
			if ( !target)
				continue;
			if (  isMad || PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::DEFENSIVE) )
			{
				//behav.Spell.Resist = 0;
				//behav.Spell.KillingBlow = 0;

				for (uint i = 0; i < target->getSEffects().size(); )
				{
					CSEffect* effect = target->getSEffects()[i];
					if ( effect )
					{
						bool removed = false;
						if ( effect && effect->getPower() <= _Power )
						{
							if ( effect->getFamily() == EFFECT_FAMILIES::Dot )
							{
								behav.Spell.SpellId =  MAGICFX::DoTBreak;
								CSLinkEffectDot * dot = (CSLinkEffectDot *)effect;
								for ( uint j = 0; j < _AffectedDots.size(); j++ )
								{
									if ( _AffectedDots[j] == dot->getDamageType() )
									{
										target->removeSabrinaEffect( dot );
										removed = true;
										break;
									}
								}
							}
							else
							{
								for ( uint j = 0; j < _AffectedEffects.size(); j++ )
								{
									if ( _AffectedEffects[j] >= EFFECT_FAMILIES::BeginCurse && _AffectedEffects[j] <= EFFECT_FAMILIES::EndCurse)
										behav.Spell.SpellId =  MAGICFX::CurseBreak;
									else
										behav.Spell.SpellId =  MAGICFX::SicknessBreak;
									if ( _AffectedEffects[j] == effect->getFamily() )
									{
										target->removeSabrinaEffect( effect );
										removed = true;
										break;
									}
								}
							}
						}
						if ( !removed )
							i++;
					}
					else
						nlwarning("<CEntityBase dispellEffects> NULL effect #%u found in an entity. Debug needed",i);
					behav.Spell.SpellIntensity = 5;
				}
			}
		}
	}
	std::vector<EFFECT_FAMILIES::TEffectFamily> _AffectedEffects;
	std::vector<DMGTYPE::EDamageType>			_AffectedDots;
	uint16										_Power;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionCure)
	ADD_MAGIC_ACTION_TYPE( "mtcb" )
END_MAGIC_ACTION_FACTORY(CMagicActionCure)
