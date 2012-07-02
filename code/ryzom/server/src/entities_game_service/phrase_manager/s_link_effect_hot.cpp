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
#include "s_link_effect_hot.h"
#include "entity_manager/entity_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_structure/statistic.h"
#include "progression/progression_pvp.h"

#include "ai_share/ai_event_report.h"

#include "nel/misc/random.h"

using namespace std;
using namespace NLMISC;

extern CRandom RandomGenerator;


bool CSLinkEffectHot::update(CTimerEvent * event, bool applyEffect)
{
	if ( CSLinkEffect::update(event,applyEffect) )
		return true;

	if (_FirstUpdate)
	{
		_FirstUpdate = false;
		return false;
	}

	if (!applyEffect)
		return false;

	CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
	if ( !caster )
	{
		nlwarning("<CSLinkEffectHot update> Invalid caster %u",_CreatorRowId.getIndex() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectHot update> Invalid target %u",_TargetRowId.getIndex() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	
	applyOnScore( caster, target,SCORES::hit_points, _HealHp );
	_Report.Hp = _HealHp;
	applyOnScore( caster, target,SCORES::sap, _HealSap );
	_Report.Sap = _HealSap;
	applyOnScore( caster, target,SCORES::stamina, _HealSta );
	_Report.Sta = _HealSta;
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( _Report );
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(_Report);

	return false;
}


void CSLinkEffectHot::applyOnScore( CEntityBase * caster, CEntityBase * target,SCORES::TScores scoreType, sint32& value )
{
	SCharacteristicsAndScores & score = target->getScores()._PhysicalScores[ scoreType ];
	score.Current = score.Current + sint( value );
	sint32 realHeal=value;
	
	if ( score.Current >= score.Max)
	{
		sint32 realHeal = value + score.Max - score.Current;
		value = realHeal;
		if ( realHeal )
			PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), value, value ,scoreType , ACTNATURE::CURATIVE_MAGIC);
		score.Current = score.Max;

		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::score);
		params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
		params[1].Enum = scoreType;
		PHRASE_UTILITIES::sendDynamicSystemMessage(caster->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_ACTOR", params);

		SM_STATIC_PARAMS_1(params1, STRING_MANAGER::score);
		params1[0].Enum = scoreType;
		PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "MAGIC_HEAL_FULL_SCORE_TARGET", params1);
	}
	else
		PHRASE_UTILITIES::sendScoreModifierSpellMessage( caster->getId(), target->getId(), value, value,scoreType ,ACTNATURE::CURATIVE_MAGIC);

	// send  Ai event report
	CAiEventReport aiReport;
	aiReport.Originator = caster->getEntityRowId();
	aiReport.Target = target->getEntityRowId();
	aiReport.Type = ACTNATURE::CURATIVE_MAGIC;

	float aggro = float(realHeal)/float(score.Max);
	aiReport.AggroAdd = aggro;
	aiReport.addDelta(AI_EVENT_REPORT::scoreToStat(scoreType), realHeal);
	CPhraseManager::getInstance().addAiEventReport(aiReport);
}
