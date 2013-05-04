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
#include "nel/misc/random.h"

#include "phrase_manager/s_link_effect.h"
#include "s_link_effect_dot.h"
#include "entity_manager/entity_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/phrase_manager.h"
#include "entity_structure/statistic.h"
#include "progression/progression_pvp.h"

#include "ai_share/ai_event_report.h"


using namespace std;
using namespace NLMISC;

extern CRandom RandomGenerator;


bool CSLinkEffectDot::update(CTimerEvent * event, bool applyEffect)
{
	if ( CSLinkEffectOffensive::update(event, false) )
		return true;

	if (_FirstUpdate)
	{
		_FirstUpdate = false;
		return false;
	}

	if (!applyEffect)
	{
		return false;
	}

	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectDot update> Invalid caster %s",_CreatorRowId.toString().c_str());
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectDot update> Invalid target %s",_TargetRowId.toString().c_str() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	float mult = _ResistFactor;
		
	sint32 realDmg = sint32( _DmgHp * mult );
	realDmg = target->applyDamageOnArmor( _DmgType, realDmg );

	if ( caster != NULL && _Vampirise && caster->getId().getType() == RYZOMID::player)
	{
		SCharacteristicsAndScores &scoreHp = target->getScores()._PhysicalScores[SCORES::hit_points];
		sint32 vampirise = (sint32)(realDmg * _VampiriseRatio);
		if ( realDmg > scoreHp.Current )
			vampirise = scoreHp.Current;
		if ( vampirise > _Vampirise )
			vampirise = _Vampirise;
		caster->changeCurrentHp( vampirise, caster->getEntityRowId() );
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
		params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
		params[1].Int = vampirise;
		CCharacter::sendDynamicSystemMessage( caster->getId(),"EGS_ACTOR_VAMPIRISE_EI", params );
//		CCharacter::sendMessageToClient( caster->getId(),"EGS_ACTOR_VAMPIRISE_EI",target->getId(),vampirise  );
	}
	// prepare the Ai event report to send
	CAiEventReport report;
	report.Originator = _CreatorRowId;
	report.Target = _TargetRowId;
	report.Type = ACTNATURE::OFFENSIVE_MAGIC;
	
	
	// compute aggro
	sint32 max = target->getPhysScores()._PhysicalScores[SCORES::hit_points].Max;		
	if (max)
	{
		float aggro = (-1) * float(realDmg)/float(max);
		if (aggro < -1 ) aggro = -1.0f;
		report.AggroAdd = (float)aggro;
		report.addDelta(AI_EVENT_REPORT::HitPoints, (-1)*realDmg);
		CPhraseManager::getInstance().addAiEventReport(report);
	}
	
	// sap
	sint32 realDmgSap = 0;
	{
		SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::sap];
		realDmgSap = sint32( _DmgSap * mult );
		realDmgSap = target->applyDamageOnArmor( _DmgType, realDmgSap );
		if ( realDmgSap != 0)
		{
			score.Current = score.Current - realDmgSap;
			if ( score.Current <= 0)
				score.Current = 0;
			PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), -realDmgSap , -sint32( _DmgSap * mult ), SCORES::sap , ACTNATURE::OFFENSIVE_MAGIC);
			_Report.Sap = realDmgSap;
			
			// aggro
			max = target->getPhysScores()._PhysicalScores[SCORES::sap].Max;		
			if (max)
			{
				float aggro = (-1) * float(realDmgSap)/float(max);
				if (aggro < -1 ) aggro = -1.0f;
				report.AggroAdd = (float)aggro;
				report.addDelta(AI_EVENT_REPORT::Sap, -realDmgSap);
				CPhraseManager::getInstance().addAiEventReport(report);
			}
		}
	}
	
	// stamina
	sint32 realDmgSta = 0;
	{
		SCharacteristicsAndScores &score = target->getScores()._PhysicalScores[SCORES::stamina];
		realDmgSta = sint32( _DmgSta * mult );
		realDmgSta = target->applyDamageOnArmor( _DmgType, realDmgSta );
		if (realDmgSta != 0)
		{
			score.Current = score.Current - realDmgSta;
			if ( score.Current <= 0)
				score.Current = 0;
			PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), -realDmgSta, -sint32( _DmgSta * mult ), SCORES::stamina , ACTNATURE::OFFENSIVE_MAGIC);
			_Report.Sta = realDmgSta;
			
			// aggro
			max = target->getPhysScores()._PhysicalScores[SCORES::stamina].Max;		
			if (max)
			{
				float aggro = (-1) * float(realDmgSta)/float(max);
				if (aggro < -1 ) aggro = -1.0f;
				report.AggroAdd = (float)aggro;
				report.addDelta(AI_EVENT_REPORT::Stamina, (-1)*realDmgSta);
				CPhraseManager::getInstance().addAiEventReport(report);
			}
		}
	}
	
	_Report.Hp = realDmg;
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( _Report );
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(_Report);
	
	if ( target->changeCurrentHp( - realDmg, caster->getEntityRowId() ) )
	{
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), -realDmg, -sint32( _DmgHp * mult ), SCORES::hit_points , ACTNATURE::OFFENSIVE_MAGIC);
		PHRASE_UTILITIES::sendDeathMessages( caster->getId(), target->getId() );
		
	}
	else
	{
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), -realDmg, -sint32( _DmgHp * mult ), SCORES::hit_points , ACTNATURE::OFFENSIVE_MAGIC);
	}

	return false;
}
