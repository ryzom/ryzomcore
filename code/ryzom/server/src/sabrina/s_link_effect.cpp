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
#include "s_link_effect.h"
#include "entity_manager.h"
#include "character.h"
#include "phrase_utilities_functions.h"
#include "game_share/entity_structure/statistic.h"
#include "nel/misc/random.h"
#include "s_link_effect.h"

using namespace std;
using namespace NLMISC;
using namespace RY_GAME_SHARE;

extern CRandom RandomGenerator;

double CSLinkEffect::_UpdatePeriod = 4.0f;


//-----------------------------------------------
// CSLinkEffect update
//-----------------------------------------------
bool CSLinkEffect::isTimeToUpdate()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	if ( time >= _NextUpdate )
	{
		_NextUpdate = time +  NLMISC::TGameCycle( _UpdatePeriod / CTickEventHandler::getGameTimeStep() );
		return true;
	}
	return false;
}// CSLinkEffect update



//-----------------------------------------------
// CSLinkEffect update
//-----------------------------------------------
bool CSLinkEffect::update(uint32 & updateFlag)
{
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDot update> Invalid caster %u",_CreatorRowId.getIndex() );
		return true;
	}
	RY_GAME_SHARE::SCharacteristicsAndScores & score = caster->getScores()._PhysicalScores[_EnergyCost];
	if ( score.Current != 0)
	{
		if ( score.Current >  _CostPerUpdate )
		{
			score.Current = score.Current - _CostPerUpdate;	
			return false;
		}
	}
	return true;
}

//-----------------------------------------------
// CSLinkEffect removed
//-----------------------------------------------
void CSLinkEffect::removed()
{
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDot removed> Invalid caster %u",_CreatorRowId.getIndex() );
		return;
	}
	caster->removeLink( this );
}

//-----------------------------------------------
// CSLinkEffectOffensive apply
//-----------------------------------------------
bool CSLinkEffectOffensive::update( uint32 & updateFlag )
{
	if ( CSLinkEffect::update(updateFlag) )
		return true;
	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDot update> Invalid caster %u",_CreatorRowId.getIndex() );
		return true;
	}
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectDot update> Invalid target %u",_TargetRowId.getIndex() );
		return true;
	}

	// test resistance
/*	SSkill * skillResist = target->getSkills().getSkillStruct( SKILLS::MagicDefense ); //TODO skill missing
	if ( ! skillResist )
	{
		nlwarning("<CSLinkEffectDot update> MagicDefense is not a valid skill");
		return true;
	}
*/	SSkill * skillActor = caster->getSkills().getSkillStruct( _Skill );
	if ( ! skillActor )
	{
		nlwarning("<CSLinkEffectDot update> %s is not a valid skill",SKILLS::toString(_Skill).c_str());
		return true;
	}
	const CSEffect * debuff = caster->lookForSEffect( EFFECT_FAMILIES::DebuffSkillMagic );
	sint skillValue = skillActor->Current;
	if ( debuff )
		skillValue -= debuff->getParamValue();

	// cap skill values with brick power
	if ( _Power < skillValue )
		skillValue = _Power;
		
	// get the chances ( delta level is divided by 10 because a level is 10
	const uint8 chances = PHRASE_UTILITIES::getSuccessChance( ( skillValue /*- skillResist->Current*/ )/10 ); //TODO skill missing
	const uint8 roll = (uint8)RandomGenerator.rand( 99 );
	
	_ResistFactor = PHRASE_UTILITIES::getSucessFactor(chances, roll);
	
	bool end = true;
	if ( _ResistFactor > 0.0f  )
	{
		end = false;
		if ( _ResistFactor > 1.0f )
		{
			_ResistFactor = 1.0f;
		}
	}
	else
	{
		if ( caster->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( caster->getId(),"MAGIC_TOTAL_RESIST" );
		if ( target->getId().getType() == RYZOMID::player )
			CCharacter::sendMessageToClient( target->getId(),"MAGIC_U_TOTAL_RESIST");
	}
		
	// compute resist XP gain, if such an effect was not already resisted
	if ( target->getId().getType() == RYZOMID::player &&  _ResistFactor < 1.0f)
	{
		///\todo nico successFactor is the quality factor of the action for xp

// TODO skill missing
//		((CCharacter*) target)->actionReport( caster, ( skillResist->Current - skillValue)/10, ACTNATURE::DEFENSIVE, SKILLS::toString( SKILLS::MagicDefense ) );
	}
	return end;
}
