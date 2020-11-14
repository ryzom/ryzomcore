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
#include "player_manager/character.h"
#include "game_share/effect_families.h"
#include "game_share/magic_fx.h"
#include "s_link_effect_dot.h"
#include "phrase_manager/phrase_utilities_functions.h"


using namespace NLNET;
using namespace NLMISC;
using namespace std;



class CMagicActionCure : public IMagicAction
{
public:
	CMagicActionCure()
		:_Power(0){}

protected:
	struct CTargetInfos
	{
		TDataSetRow	RowId;
		bool		MainTarget;
	};

protected:
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
			case TBrickParam::MA_EFFECT:
				{
					INFOLOG("MA_EFFECT: %s",((CSBrickParamMagicEffect *)param)->Effect.c_str());
					EFFECT_FAMILIES::TEffectFamily effectFamily = EFFECT_FAMILIES::toEffectFamily( ((CSBrickParamMagicEffect *)param)->Effect );
					if ( effectFamily == EFFECT_FAMILIES::Unknown )
					{
						nlwarning("<CMagicActionCure addBrick> invalid effect type %s", ((CSBrickParamMagicEffect *)param)->Effect.c_str());
						return false;
					}
					_AffectedEffects.push_back(effectFamily);
					break;
				}
			case TBrickParam::MA_DMG_TYPE:
				{
					INFOLOG("MA_DMG_TYPE: %s",((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
					DMGTYPE::EDamageType dmgType = DMGTYPE::stringToDamageType( ((CSBrickParamMagicDmgType *)param)->DmgType );
					if ( dmgType == DMGTYPE::UNDEFINED )
					{
						nlwarning("<CMagicActionCure addBrick> invalid dmg type %s", ((CSBrickParamMagicDmgType *)param)->DmgType.c_str());
						return false;
					}
					_AffectedDots.push_back(dmgType);
					break;
				}
			case TBrickParam::MA_LINK_POWER:	
				INFOLOG("MA_LINK_POWER: %u",((CSBrickParamMagicLinkPower *)param)->Power);
				_Power = uint16 ( ((CSBrickParamMagicLinkPower *)param)->Power );
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
		if ( !PHRASE_UTILITIES::validateSpellTarget(phrase->getActor(),phrase->getTargets()[0].getId(), ACTNATURE::CURATIVE_MAGIC, errorCode, true) )
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
		TReportAction reportAction = actionReport;

		reportAction.DeltaLvl = deltaLevel;
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if ( !actor)
		{
			nlwarning("<CMagicActionCure launch> invalid actor: %u", phrase->getActor().getIndex() );
			return;
		}
		if ( successFactor <= 0.0f )
		{
//			if ( actor->getId().getType() == RYZOMID::player )
//				CCharacter::sendMessageToClient( actor->getId(),"MAGIC_TOTAL_MISS" );
			return;
		}
		const std::vector< CSpellTarget > & targets = phrase->getTargets();
		for ( uint i = 0; i < targets.size(); i++ )
		{
			TReportAction reportAction = actionReport;

			reportAction.TargetRowId = targets[i].getId();
			// check target
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( targets[i].getId() );
			if ( !target)
				continue;

			string errorCode;
			if( !isMad && !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, i==0))
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}
			affectedTargets.set(i);

			CTargetInfos targetInfos;
			targetInfos.RowId		= target->getEntityRowId();
			targetInfos.MainTarget	= (i == 0);

			_ApplyTargets.push_back(targetInfos);
		}

		//behav.Spell.Resist = 0;
		//behav.Spell.KillingBlow = 0;
		resists.clearAll();
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{
		CEntityBase* actor = CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if ( !actor)
		{
			return;
		}

		const uint nbTargets = _ApplyTargets.size();
		for ( uint i = 0; i < nbTargets; i++ )
		{
			CEntityBase* target = CEntityBaseManager::getEntityBasePtr( _ApplyTargets[i].RowId );
			if ( !target)
				continue;

			string errorCode;
			if( !PHRASE_UTILITIES::validateSpellTarget(actor->getEntityRowId(),target->getEntityRowId(),ACTNATURE::CURATIVE_MAGIC, errorCode, _ApplyTargets[i].MainTarget))
			{
				// dont warn because of multi target
				// PHRASE_UTILITIES::sendSimpleMessage(phrase->getActor(), errorCode);
				continue;
			}

			for (uint i = 0; i < target->getSEffects().size(); )
			{
				CSEffect* effect = target->getSEffects()[i];
				if ( effect )
				{
					bool removed = false;
					if ( effect->getPower() <= _Power )
					{
						if ( effect->getFamily() == EFFECT_FAMILIES::Dot )
						{
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
			}
		}
	}

	std::vector<EFFECT_FAMILIES::TEffectFamily> _AffectedEffects;
	std::vector<DMGTYPE::EDamageType>			_AffectedDots;
	uint16										_Power;

	/// targets that need to be treated by apply()
	std::vector<CTargetInfos>		_ApplyTargets;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionCure)
	ADD_MAGIC_ACTION_TYPE( "mtcb" )
END_MAGIC_ACTION_FACTORY(CMagicActionCure)
