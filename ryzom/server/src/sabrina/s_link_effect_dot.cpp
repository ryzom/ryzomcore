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
#include "s_link_effect_dot.h"
#include "entity_manager.h"
#include "character.h"
#include "game_share/entity_structure/statistic.h"
#include "nel/misc/random.h"
#include "s_link_effect.h"
#include "phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;
using namespace RY_GAME_SHARE;

extern CRandom RandomGenerator;


bool CSLinkEffectDot::update(uint32 & updateFlag)
{
	if ( CSLinkEffectOffensive::update(updateFlag) )
		return true;

	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDot update> Invalid target %u",_CreatorRowId.getIndex() );
		return true;
	}

	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectDot update> Invalid target %u",_TargetRowId.getIndex() );
		return true;
	}
	float mult = _ResistFactor;
	// get damage amplifier on target
	const CSEffect * effect = target->lookForSEffect( EFFECT_FAMILIES::MagicDmgAmpli );
	if ( effect )
		mult *=  (effect->getParamValue() / 100.0f);
	
	

	
	sint32 realDmg = sint32( _DmgHp * mult );
	realDmg = target->applyDamageOnArmor( _DmgType, realDmg );
	if ( target->changeCurrentHp( - realDmg ) )
	{
		// send mission event
		if ( caster->getId().getType() == RYZOMID::player )
		{
			CMissionEventKill event ( target->getEntityRowId() );
			((CCharacter*) caster)->processMissionEvent( event );
		}
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster, target, realDmg ,SCORES::hit_points , ACTNATURE::OFFENSIVE);
	}

	{
		RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::sap];
		realDmg = sint32( _DmgSap * mult );
		realDmg = target->applyDamageOnArmor( _DmgType, realDmg );
		score.Current = score.Current - realDmg;
		if ( score.Current <= 0)
			score.Current = 0;
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster, target, realDmg ,SCORES::sap , ACTNATURE::OFFENSIVE);
	}

	{
		RY_GAME_SHARE::SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
		realDmg = sint32( _DmgSta * mult );
		realDmg = target->applyDamageOnArmor( _DmgType, realDmg );
		score.Current = score.Current - realDmg;
		if ( score.Current <= 0)
			score.Current = 0;
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster, target, realDmg ,SCORES::stamina , ACTNATURE::OFFENSIVE);
	}
	
	updateFlag |= _Family;
	return false;
}
